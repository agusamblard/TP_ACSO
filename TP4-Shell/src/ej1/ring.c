
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    int start, pid, n;
    int buffer[1];

    if (argc != 4) {
        fprintf(stderr, "Uso: %s <n> <c> <s>\n", argv[0]);
        fprintf(stderr, "  <n>: cantidad de procesos del anillo (mínimo 3).\n");
        fprintf(stderr, "  <c>: valor entero inicial a enviar.\n");
        fprintf(stderr, "  <s>: índice del proceso que inicia (entre 0 y n-1).\n");
        exit(EXIT_FAILURE);
    }

    n = atoi(argv[1]);
    buffer[0] = atoi(argv[2]);
    start = atoi(argv[3]);

    if (n < 3) {
        fprintf(stderr, "Error: La cantidad de procesos (n = %d) debe ser mayor o igual a 3.\n", n);
        exit(EXIT_FAILURE);
    }

    if (start < 0) {
        fprintf(stderr, "Error: El índice del nodo de inicio (s = %d) no puede ser negativo.\n", start);
        exit(EXIT_FAILURE);
    }

    if (start >= n) {
        fprintf(stderr, "Error: El índice del nodo de inicio (s = %d) debe ser menor que n (n = %d).\n", start, n);
        exit(EXIT_FAILURE);
    }

    printf("Se crearán %i procesos, se enviará el mensaje %i desde proceso %i\n", n, buffer[0], start);

    int pipes[n][2];
    for (int i = 0; i < n; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < n; i++) {
        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            int prev = (i == 0) ? n - 1 : i - 1;
            close(pipes[prev][1]); // Cierro escritura del anterior
            close(pipes[i][0]);    // Cierro lectura del mío

            if (i == start) {
                printf("Proceso %d (PID %d) recibió el mensaje: %d\n", i, getpid(), buffer[0]);
                buffer[0]++;
                write(pipes[i][1], buffer, sizeof(int));
                read(pipes[prev][0], buffer, sizeof(int));
                printf("Proceso %d (PID %d) recibió el mensaje final: %d\n", i, getpid(), buffer[0]);
            } else {
                read(pipes[prev][0], buffer, sizeof(int));
                printf("Proceso %d (PID %d) recibió el mensaje: %d\n", i, getpid(), buffer[0]);
                buffer[0]++;
                write(pipes[i][1], buffer, sizeof(int));
            }

            close(pipes[prev][0]);
            close(pipes[i][1]);
            exit(0);
        }
    }

    for (int i = 0; i < n; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    for (int i = 0; i < n; i++) {
        wait(NULL);
    }

    return 0;
}
