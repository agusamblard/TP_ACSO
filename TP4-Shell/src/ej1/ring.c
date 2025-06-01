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

    int pipes[n][2];
    int pipe_padre[2]; // comunicación entre hijo s y padre

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
            // Proceso hijo
            for (int j = 0; j < n; j++) {
                if (j != (i - 1 + n) % n) close(pipes[j][READ]);   // leo del anterior
                if (j != i) close(pipes[j][WRITE]);                // escribo al siguiente
            }

            // Cierro lado de lectura del padre
            close(pipe_padre[READ]);

            int valor;
            read(pipes[(i - 1 + n) % n][READ], &valor, sizeof(int));
            valor++;

            if (i == s) {
                // El mensaje volvió a s, se lo manda al padre
                write(pipe_padre[WRITE], &valor, sizeof(int));
            } else {
                // Lo reenvía al siguiente en el anillo
                write(pipes[i][WRITE], &valor, sizeof(int));
            }

            exit(0);
        }
    }

    // PADRE
    for (int i = 0; i < n; i++) {
        close(pipes[i][READ]);
        close(pipes[i][WRITE]);
    }
    close(pipe_padre[WRITE]); // solo leerá

    // Enviar valor inicial al proceso s
    write(pipes[s][WRITE], &c, sizeof(int));

    int resultado;
    read(pipe_padre[READ], &resultado, sizeof(int));

    printf("El resultado final es: %d\n", resultado);

    for (int i = 0; i < n; i++) {
        wait(NULL);
    }

    return 0;
}
