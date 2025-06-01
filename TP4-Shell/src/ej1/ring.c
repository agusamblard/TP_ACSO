
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

    int valor = c;

    for (int i = 0; i < n; i++) {
        int fds[2];
        if (pipe(fds) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // hijo: lee valor, lo incrementa, lo pasa al stdout si último
            close(fds[WRITE]);
            if (read(fds[READ], &valor, sizeof(valor)) <= 0) {
                perror("read");
                exit(EXIT_FAILURE);
            }
            close(fds[READ]);
            valor++;

            // si soy el último proceso (antes de volver a s)
            if ((i + 1) % n == s) {
                write(STDOUT_FILENO, &valor, sizeof(valor));
            } else {
                // paso al siguiente proceso vía stdout → heredado
                write(STDOUT_FILENO, &valor, sizeof(valor));
            }
            exit(0);
        } else {
            // padre: escribe en pipe y espera al hijo
            close(fds[READ]);
            write(fds[WRITE], &valor, sizeof(valor));
            close(fds[WRITE]);
            waitpid(pid, NULL, 0);

            // leer del stdout del hijo para siguiente iteración
            if (i < n - 1) {
                read(STDIN_FILENO, &valor, sizeof(valor));
            }
        }
    }

    printf("El resultado final es: %d\n", valor);
    return 0;
}
