#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#define MAX_COMMANDS 200
#define MAX_ARGS 64

// Función para separar los argumentos de un comando
void parse_args(char *command, char **args) {
    int i = 0;
    char *token = strtok(command, " \t");
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t");
    }
    args[i] = NULL;
}

int main() {
    char command[1024];
    char *commands[MAX_COMMANDS];
    int command_count = 0;

    while (1) {
        if (isatty(STDIN_FILENO)) {
            printf("Shell> ");
            fflush(stdout);
        }

        if (fgets(command, sizeof(command), stdin) == NULL) {
            break; // EOF (Ctrl+D)
        }

        command[strcspn(command, "\n")] = '\0'; // Eliminar salto de línea

        if (strcmp(command, "exit") == 0) {
            break;
        }

        // Separar comandos por '|'
        command_count = 0;
        char *token = strtok(command, "|");
        while (token != NULL && command_count < MAX_COMMANDS) {
            while (*token && isspace(*token)) token++; // Trim left
            commands[command_count++] = token;
            token = strtok(NULL, "|");
        }

        int pipes[MAX_COMMANDS - 1][2];

        for (int i = 0; i < command_count - 1; i++) {
            if (pipe(pipes[i]) == -1) {
                perror("pipe");
                exit(1);
            }
        }

        for (int i = 0; i < command_count; i++) {
            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                exit(1);
            }

            if (pid == 0) {
                // Redirección de entrada si no es el primero
                if (i > 0) {
                    dup2(pipes[i - 1][0], STDIN_FILENO);
                }

                // Redirección de salida si no es el último
                if (i < command_count - 1) {
                    dup2(pipes[i][1], STDOUT_FILENO);
                }

                // Cerrar todos los extremos de pipes en el hijo
                for (int j = 0; j < command_count - 1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }

                char *args[MAX_ARGS];
                parse_args(commands[i], args);
                execvp(args[0], args);
                perror("execvp");
                exit(1);
            }
        }

        // Cerrar todos los pipes en el padre
        for (int i = 0; i < command_count - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        // Esperar todos los hijos
        for (int i = 0; i < command_count; i++) {
            wait(NULL);
        }
    }

    return 0;
}
