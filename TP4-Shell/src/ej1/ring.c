
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

    int n = atoi(argv[1]);
    int c = atoi(argv[2]);
    int s = atoi(argv[3]);

    if (n < 3 || s < 0 || s >= n) {
        fprintf(stderr, "Error: n >= 3 y 0 <= s < n\n");
        exit(EXIT_FAILURE);
    }

    int pipes[n][2];                // pipes entre hijos (anillo)
    int pipe_padre_hijo[2];         // padre -> hijo s
    int pipe_hijo_padre[2];         // hijo (nodo final) -> padre

    for (int i = 0; i < n; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    if (pipe(pipe_padre_hijo) == -1 || pipe(pipe_hijo_padre) == -1) {
        perror("pipe padre-hijo");
        exit(EXIT_FAILURE);
    }

    pid_t pids[n];
    for (int i = 0; i < n; i++) {
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pids[i] == 0) {
            // Proceso hijo i
            int valor;
            for (int j = 0; j < n; j++) {
                if (j != i) close(pipes[j][WRITE]);
                if ((j + 1) % n != i) close(pipes[j][READ]);
            }

            close(pipe_padre_hijo[WRITE]); // los hijos no escriben al padre
            close(pipe_hijo_padre[READ]);  // los hijos no leen del padre

            if (i == s) {
                // nodo de inicio: lee del padre
                if (read(pipe_padre_hijo[READ], &valor, sizeof(valor)) <= 0) {
                    perror("read inicial del padre");
                    exit(EXIT_FAILURE);
                }
                close(pipe_padre_hijo[READ]);
            } else {
                // resto: lee del anterior en el anillo
                int prev = (i - 1 + n) % n;
                if (read(pipes[prev][READ], &valor, sizeof(valor)) <= 0) {
                    perror("read del anterior");
                    exit(EXIT_FAILURE);
                }
                close(pipes[prev][READ]);
            }

            valor++;

            if ((i + 1) % n == s) {
                // soy el último nodo antes del que inició → le devuelvo al padre
                if (write(pipe_hijo_padre[WRITE], &valor, sizeof(valor)) == -1) {
                    perror("write al padre");
                    exit(EXIT_FAILURE);
                }
                close(pipe_hijo_padre[WRITE]);
            } else {
                // paso al siguiente en el anillo
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
    close(pipe_padre_hijo[READ]);
    close(pipe_hijo_padre[WRITE]);

    // Enviar valor inicial
    if (write(pipe_padre_hijo[WRITE], &c, sizeof(c)) == -1) {
        perror("padre write a hijo s");
        exit(EXIT_FAILURE);
    }
    close(pipe_padre_hijo[WRITE]);

    // Esperar valor final
    int resultado;
    if (read(pipe_hijo_padre[READ], &resultado, sizeof(resultado)) <= 0) {
        perror("padre read final");
        exit(EXIT_FAILURE);
    }
    close(pipe_hijo_padre[READ]);

    printf("El resultado final es: %d\n", resultado);

    for (int i = 0; i < n; i++) waitpid(pids[i], NULL, 0);
    return 0;
}
