
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h> // Necesario para atoi

int main(int argc, char **argv)
{

    int start_node, initial_value, num_processes;
    int status;

    if (argc != 4)
    {
        printf("Uso: ./anillo <n> <c> <s> \n");
        printf("  <n>: cantidad de procesos hijos del anillo.\n");
        printf("  <c>: valor del mensaje inicial.\n");
        printf("  <s>: número de proceso que inicia la comunicación (0 a n-1).\n");
        exit(0);
    }

    /* Parsing of arguments */
    num_processes = atoi(argv[1]); // <n>
    initial_value = atoi(argv[2]); // <c>
    start_node = atoi(argv[3]);    // <s>

    if (num_processes <= 0) {
        fprintf(stderr, "Error: El número de procesos (n) debe ser mayor que 0.\n");
        exit(EXIT_FAILURE);
    }
    if (start_node < 0 || start_node >= num_processes) {
        fprintf(stderr, "Error: El nodo de inicio (s) debe estar entre 0 y n-1.\n");
        exit(EXIT_FAILURE);
    }

    printf("Se crearán %i procesos, se enviará el valor %i desde proceso %i \n", num_processes, initial_value, start_node);

    int pipes[num_processes][2]; // Array de pipes para la comunicación entre hijos
    int parent_to_child_pipe[2]; // Pipe para que el padre envíe el valor inicial al nodo de inicio
    int child_to_parent_pipe[2]; // Pipe para que el último nodo envíe el valor final al padre

    // Crear todos los pipes necesarios
    for (int i = 0; i < num_processes; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }
    if (pipe(parent_to_child_pipe) == -1) {
        perror("parent_to_child_pipe");
        exit(EXIT_FAILURE);
    }
    if (pipe(child_to_parent_pipe) == -1) {
        perror("child_to_parent_pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pids[num_processes];

    for (int i = 0; i < num_processes; i++)
    {
        pids[i] = fork();
        if (pids[i] == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pids[i] == 0)
        { // Código del proceso hijo i
            int current_val;
            int read_fd, write_fd;

            // Cerrar pipes no utilizados por este hijo
            close(parent_to_child_pipe[1]); // Extremo de escritura del pipe padre-hijo
            close(child_to_parent_pipe[0]); // Extremo de lectura del pipe hijo-padre

            for (int j = 0; j < num_processes; j++) {
                if (i == j) { // Pipe de escritura de este proceso
                    close(pipes[j][0]); // Cerrar lectura de su propio pipe de salida
                } else if (i == (j + 1) % num_processes) { // Pipe de lectura de este proceso
                    close(pipes[j][1]); // Cerrar escritura de su propio pipe de entrada
                } else { // Otros pipes no usados
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
            }


            if (i == start_node) { // Nodo de inicio
                close(pipes[(i - 1 + num_processes) % num_processes][1]); // Cerrar escritura del pipe de entrada si es el de otro hijo
                close(parent_to_child_pipe[1]); // Cierra el extremo de escritura del pipe del padre al hijo
                if (read(parent_to_child_pipe[0], &current_val, sizeof(current_val)) == -1) {
                    perror("read from parent");
                    exit(EXIT_FAILURE);
                }
                close(parent_to_child_pipe[0]); // Cierra el extremo de lectura
                printf("Proceso %d (PID %d) recibió %d del padre.\n", i, getpid(), current_val);
            } else { // No es el nodo de inicio, recibe del anterior en el anillo
                read_fd = pipes[(i - 1 + num_processes) % num_processes][0];
                close(pipes[(i - 1 + num_processes) % num_processes][1]); // Cierra escritura del pipe de entrada
                if (read(read_fd, &current_val, sizeof(current_val)) == -1) {
                     perror("read from predecessor");
                     exit(EXIT_FAILURE);
                }
                close(read_fd);
                printf("Proceso %d (PID %d) recibió %d del proceso %d.\n", i, getpid(), current_val, (i - 1 + num_processes) % num_processes);
            }

            current_val++;
            printf("Proceso %d (PID %d) incrementó a %d.\n", i, getpid(), current_val);

            // Determinar a quién enviar el valor
            // Si este es el proceso (start_node - 1 + num_processes) % num_processes (el anterior al que inició, completando el círculo)
            // entonces envía al padre. Sino, envía al siguiente en el anillo.
            if (i == (start_node - 1 + num_processes) % num_processes) {
                write_fd = child_to_parent_pipe[1];
                close(child_to_parent_pipe[0]); // Cierra lectura del pipe hijo-padre
                close(pipes[i][0]); // Cierra lectura de su pipe de salida (si no es el mismo que el child_to_parent_pipe)
                if (write(write_fd, &current_val, sizeof(current_val)) == -1) {
                    perror("write to parent");
                    exit(EXIT_FAILURE);
                }
                close(write_fd);
                 printf("Proceso %d (PID %d) envió %d al padre.\n", i, getpid(), current_val);
            } else {
                write_fd = pipes[i][1];
                close(pipes[i][0]); // Cierra lectura de su pipe de salida
                if (write(write_fd, &current_val, sizeof(current_val)) == -1) {
                    perror("write to successor");
                    exit(EXIT_FAILURE);
                }
                close(write_fd);
                printf("Proceso %d (PID %d) envió %d al proceso %d.\n", i, getpid(), current_val, (i + 1) % num_processes);
            }
            exit(EXIT_SUCCESS);
        }
    }

    // Código del proceso padre
    // Cerrar extremos no utilizados de los pipes de los hijos
    for (int i = 0; i < num_processes; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Enviar valor inicial al nodo de inicio
    close(parent_to_child_pipe[0]); // Cerrar lectura del pipe padre-hijo
    printf("Padre (PID %d) enviando valor inicial %d al proceso %d.\n", getpid(), initial_value, start_node);
    if (write(parent_to_child_pipe[1], &initial_value, sizeof(initial_value)) == -1) {
        perror("Padre: write to start_node");
        exit(EXIT_FAILURE);
    }
    close(parent_to_child_pipe[1]);

    // Recibir valor final del último nodo del anillo (que es el anterior al start_node)
    int final_value;
    close(child_to_parent_pipe[1]); // Cerrar escritura del pipe hijo-padre
    if (read(child_to_parent_pipe[0], &final_value, sizeof(final_value)) == -1) {
        perror("Padre: read from last_node");
        exit(EXIT_FAILURE);
    }
    close(child_to_parent_pipe[0]);

    printf("Padre (PID %d) recibió el valor final: %d\n", getpid(), final_value);

    // Esperar a que todos los hijos terminen
    for (int i = 0; i < num_processes; i++)
    {
        waitpid(pids[i], &status, 0);
    }

    printf("Todos los procesos hijos han terminado.\n");
    return 0;
}
