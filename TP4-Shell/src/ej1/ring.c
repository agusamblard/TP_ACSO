
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

    int pipe_padre_hijo[2];
    int pipe_hijo_padre[2];
    if (pipe(pipe_padre_hijo) == -1 || pipe(pipe_hijo_padre) == -1) {
        perror("pipe padre-hijo");
        exit(EXIT_FAILURE);
    }

    int prev_read = -1;  // extremo de lectura del anterior proceso
    pid_t pid;

    for (int i = 0; i < n; i++) {
        int actual_pipe[2];
        if (i != (s - 1 + n) % n) {
            if (pipe(actual_pipe) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // hijo i
            int val;

            if (i == s) {
                close(pipe_padre_hijo[WRITE]);
                if (read(pipe_padre_hijo[READ], &val, sizeof(val)) <= 0) {
                    perror("read padre-hijo");
                    exit(EXIT_FAILURE);
                }
                close(pipe_padre_hijo[READ]);
            } else {
                if (read(prev_read, &val, sizeof(val)) <= 0) {
                    perror("read anillo");
                    exit(EXIT_FAILURE);
                }
                close(prev_read);
            }

            val++;

            if ((i + 1) % n == s) {
                close(pipe_hijo_padre[READ]);
                if (write(pipe_hijo_padre[WRITE], &val, sizeof(val)) == -1) {
                    perror("write hijo-padre");
                    exit(EXIT_FAILURE);
                }
                close(pipe_hijo_padre[WRITE]);
            } else {
                close(actual_pipe[READ]);
                if (write(actual_pipe[WRITE], &val, sizeof(val)) == -1) {
                    perror("write anillo");
                    exit(EXIT_FAILURE);
                }
                close(actual_pipe[WRITE]);
            }

            exit(0);
        } else {
            // padre
            if (prev_read != -1) close(prev_read);
            if (i != (s - 1 + n) % n) {
                close(actual_pipe[WRITE]);
                prev_read = actual_pipe[READ];
            }
        }
    }

    // proceso padre
    close(pipe_padre_hijo[READ]);
    if (write(pipe_padre_hijo[WRITE], &c, sizeof(c)) == -1) {
        perror("padre write inicio");
        exit(EXIT_FAILURE);
    }
    close(pipe_padre_hijo[WRITE]);

    close(pipe_hijo_padre[WRITE]);
    int result;
    if (read(pipe_hijo_padre[READ], &result, sizeof(result)) <= 0) {
        perror("padre read final");
        exit(EXIT_FAILURE);
    }
    close(pipe_hijo_padre[READ]);

    printf("El resultado final es: %d\n", result);

    // Esperar todos los hijos
    while (wait(NULL) > 0);
    return 0;
}
