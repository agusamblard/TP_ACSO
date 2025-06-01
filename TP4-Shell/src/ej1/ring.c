#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define READ 0
#define WRITE 1

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <n> <c> <s>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[1]);  // cantidad de procesos
    int c = atoi(argv[2]);  // valor inicial
    int s = atoi(argv[3]);  // proceso que inicia la comunicación

    if (n < 3 || s < 0 || s >= n) {
        fprintf(stderr, "Error: n debe ser >= 3 y 0 <= s < n\n");
        exit(EXIT_FAILURE);
    }

    int pipes[n][2];         // Pipes entre procesos en anillo
    int pipe_padre[2];       // Pipe para que el hijo 's' le devuelva el mensaje al padre

    // Crear pipe del anillo
    for (int i = 0; i < n; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Pipe entre hijo s y el padre
    if (pipe(pipe_padre) == -1) {
        perror("pipe padre");
        exit(EXIT_FAILURE);
    }

    // Crear procesos hijos
    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // CIERRO PIPES INNECESARIOS
            for (int j = 0; j < n; j++) {
                if (j != (i - 1 + n) % n) close(pipes[j][READ]);     // leo del anterior
                if (j != i) close(pipes[j][WRITE]);                  // escribo al siguiente
            }

            close(pipe_padre[READ]); // hijo nunca lee del padre

            int value;
            read(pipes[(i - 1 + n) % n][READ], &value, sizeof(int));
            value++;

            if (i == s) {
                // Proceso s envía el valor final al padre
                write(pipe_padre[WRITE], &value, sizeof(int));
            } else {
                // Enviar al siguiente en el anillo
                write(pipes[i][WRITE], &value, sizeof(int));
            }

            exit(EXIT_SUCCESS);
        }
    }

    // PROCESO PADRE
    // Cierra extremos que no necesita
    for (int i = 0; i < n; i++) {
        close(pipes[i][READ]);
        close(pipes[i][WRITE]);
    }
    close(pipe_padre[WRITE]); // Padre solo lee de este pipe

    // Enviar valor inicial al proceso s
    write(pipes[s][WRITE], &c, sizeof(int));

    // Esperar resultado final del proceso s
    int resultado;
    read(pipe_padre[READ], &resultado, sizeof(int));

    printf("El resultado final es: %d\n", resultado);

    // Esperar hijos
    for (int i = 0; i < n; i++) {
        wait(NULL);
    }

    return 0;
}
