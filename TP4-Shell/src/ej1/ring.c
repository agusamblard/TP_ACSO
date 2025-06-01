
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define READ 0
#define WRITE 1

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <n> <c> <s>\n", argv[0]);
        fprintf(stderr, "  <n>: cantidad de procesos hijos del anillo (mínimo 3).\n");
        fprintf(stderr, "  <c>: valor entero inicial a enviar.\n");
        fprintf(stderr, "  <s>: índice del proceso que inicia (entre 0 y n-1).\n");
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[1]);
    int c = atoi(argv[2]);
    int s = atoi(argv[3]);

    if (n < 3) {
        fprintf(stderr, "Error: La cantidad de procesos (n = %d) debe ser mayor o igual a 3.\n", n);
        exit(EXIT_FAILURE);
    }

    if (s < 0) {
        fprintf(stderr, "Error: El índice del nodo de inicio (s = %d) no puede ser negativo.\n", s);
        exit(EXIT_FAILURE);
    }

    if (s >= n) {
        fprintf(stderr, "Error: El índice del nodo de inicio (s = %d) debe ser menor que n (n = %d).\n", s, n);
        exit(EXIT_FAILURE);
    }

    int pipes[n][2];                // pipes entre procesos en anillo
    int pipe_padre[2];              // padre <-> proceso s

    for (int i = 0; i < n; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    if (pipe(pipe_padre) == -1) {
        perror("pipe padre-hijo s");
        exit(EXIT_FAILURE);
    }

    pid_t pids[n];
    for (int i = 0; i < n; i++) {
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pids[i] == 0) {
            // HIJO i
            for (int j = 0; j < n; j++) {
                if (j != i) close(pipes[j][WRITE]);
                if ((j + 1) % n != i) close(pipes[j][READ]);
            }

            if (i != s) {
                close(pipe_padre[READ]);
                close(pipe_padre[WRITE]);
            }

            int valor;

            if (i == s) {
                // lee del padre primero
                close(pipe_padre[WRITE]);
                if (read(pipe_padre[READ], &valor, sizeof(valor)) <= 0) {
                    perror("read inicial del padre");
                    exit(EXIT_FAILURE);
                }
                close(pipe_padre[READ]);
            } else {
                int prev = (i - 1 + n) % n;
                if (read(pipes[prev][READ], &valor, sizeof(valor)) <= 0) {
                    perror("read del anterior");
                    exit(EXIT_FAILURE);
                }
                close(pipes[prev][READ]);
            }

            valor++;

            if ((i + 1) % n == s) {
                // si el siguiente es el iniciador, se lo devuelve al padre
                if (i == s) {
                    fprintf(stderr, "Error: ciclo detectado en el mismo proceso\n");
                    exit(EXIT_FAILURE);
                }
                if (write(pipe_padre[WRITE], &valor, sizeof(valor)) == -1) {
                    perror("write final al padre");
                    exit(EXIT_FAILURE);
                }
                close(pipe_padre[WRITE]);
            } else {
                if (write(pipes[i][WRITE], &valor, sizeof(valor)) == -1) {
                    perror("write al siguiente");
                    exit(EXIT_FAILURE);
                }
                close(pipes[i][WRITE]);
            }

            exit(EXIT_SUCCESS);
        }
    }

    // PADRE
    for (int i = 0; i < n; i++) {
        close(pipes[i][READ]);
        close(pipes[i][WRITE]);
    }

    close(pipe_padre[READ]);
    if (write(pipe_padre[WRITE], &c, sizeof(c)) == -1) {
        perror("padre write a hijo s");
        exit(EXIT_FAILURE);
    }
    close(pipe_padre[WRITE]);

    close(pipe_padre[WRITE]); // asegurarse
    int resultado;
    if (read(pipe_padre[READ], &resultado, sizeof(resultado)) <= 0) {
        perror("padre read final");
        exit(EXIT_FAILURE);
    }
    close(pipe_padre[READ]);

    printf("El resultado final es: %d\n", resultado);

    for (int i = 0; i < n; i++) waitpid(pids[i], NULL, 0);
    return 0;
}
