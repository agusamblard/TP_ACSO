
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char **argv)
{	
	int start, status, pid, n;
	int buffer[1];

	if (argc != 4){ 
        printf("Uso: anillo <n> <c> <s> \n"); 
        exit(0);
    }
    
    /* Parsing of arguments */
	n = atoi(argv[1]);
    buffer[0] = atoi(argv[2]);
    start = atoi(argv[3]);

    /* Validaciones */
    if (n < 2) {
        printf("Error: la cantidad de procesos debe ser al menos 2.\n");
        exit(1);
    }
    if (start < 0 || start >= n) {
        printf("Error: el proceso inicial debe estar entre 0 y n-1.\n");
        exit(1);
    }
    if (buffer[0] < 0) {
        printf("Error: el valor del mensaje debe ser un entero positivo.\n");
        exit(1);
    }

    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n", n, buffer[0], start);
    
    /* Algoritmo de anillo */
    int pipes[n][2];
    for (int i = 0; i < n; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    for (int i = 0; i < n; i++) {
        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(1);
        }
        if (pid == 0) { // Proceso hijo
            int prev = (i == 0) ? n - 1 : i - 1;
            close(pipes[prev][1]); // Cierra escritura del pipe anterior
            close(pipes[i][0]);    // Cierra lectura del pipe propio

            if (i == start) {
                // Proceso inicial: imprime y envía el mensaje
                printf("Proceso %d (PID %d) recibió el mensaje: %d\n", i, getpid(), buffer[0]);
                buffer[0] += 1;
                write(pipes[i][1], buffer, sizeof(int));
                // Espera el mensaje de vuelta
                read(pipes[prev][0], buffer, sizeof(int));
                printf("Proceso %d (PID %d) recibió el mensaje final: %d\n", i, getpid(), buffer[0]);
                // No reenvía el mensaje, termina el anillo
            } else {
                // Espera el mensaje del anterior
                read(pipes[prev][0], buffer, sizeof(int));
                printf("Proceso %d (PID %d) recibió el mensaje: %d\n", i, getpid(), buffer[0]);
                // Suma 1 al mensaje antes de pasarlo al siguiente
                buffer[0] += 1;
                // Pasa el mensaje al siguiente
                write(pipes[i][1], buffer, sizeof(int));
            }

            // Cierra los pipes usados
            close(pipes[prev][0]);
            close(pipes[i][1]);
            exit(0);
        }
        // Proceso padre: sigue creando hijos
    }

    // Proceso padre: cierra todos los pipes y espera a los hijos
    for (int i = 0; i < n; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    for (int i = 0; i < n; i++) {
        wait(NULL);
    }
}
