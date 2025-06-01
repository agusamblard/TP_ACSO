
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

    int pipes[n][2];        // pipes entre hijos en anillo
    int pipe_padre[2];      // comunicación hijo 's' → padre

    for (int i = 0; i < n; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    if (pipe(pipe_padre) == -1) {
        perror("pipe padre");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // HIJO

            // Cerrar extremos innecesarios
            for (int j = 0; j < n; j++) {
                if (j != (i - 1 + n) % n) close(pipes[j][READ]);
                if (j != i) close(pipes[j][WRITE]);
            }
            // Cierro extremos del pipe padre
            if (i != s) {
                close(pipe_padre[WRITE]);
            }
            close(pipe_padre[READ]); // ningún hijo lee del padre

            int valor;
            read(pipes[(i - 1 + n) % n][READ], &valor, sizeof(int));
            valor++;

            if (i == s) {
                write(pipe_padre[WRITE], &valor, sizeof(int));
                close(pipe_padre[WRITE]);
            } else {
                write(pipes[i][WRITE], &valor, sizeof(int));
            }

            close(pipes[(i - 1 + n) % n][READ]);
            close(pipes[i][WRITE]);
            exit(0);
        }
    }

    // PADRE
    for (int i = 0; i < n; i++) {
        close(pipes[i][READ]);
        close(pipes[i][WRITE]);
    }
    close(pipe_padre[WRITE]); // sólo va a leer

    // Enviar valor inicial al hijo 's'
    write(pipes[s][WRITE], &c, sizeof(int));

    int resultado;
    read(pipe_padre[READ], &resultado, sizeof(int));

    printf("El resultado final es: %d\n", resultado);

    close(pipe_padre[READ]);

    for (int i = 0; i < n; i++) wait(NULL);
    return 0;
}
