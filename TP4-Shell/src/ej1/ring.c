
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    int start, pid, n;
    int buffer[1];

    if (argc != 4) {
        printf("Uso: anillo <n> <c> <s> \n");
        exit(0);
    }

    n = atoi(argv[1]);
    buffer[0] = atoi(argv[2]);
    start = atoi(argv[3]);

    if (n < 2) {
        printf("Error: la cantidad de procesos debe ser al menos 2.\n");
        exit(1);
    }
    if (start < 0 || start >= n) {
        printf("Error: el proceso inicial debe estar entre 0 y n-1.\n");
        exit(1);
    }

    printf("Se crearán %i procesos, se enviará el mensaje %i desde proceso %i\n", n, buffer[0], start);

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
        if (pid == 0) {
            int prev = (i == 0) ? n - 1 : i - 1;
            close(pipes[prev][1]); // Cierro escritura del anterior
            close(pipes[i][0]);    // Cierro lectura del mío

            if (i == start) {
                // inicio: imprime, envía y espera el valor de vuelta
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
