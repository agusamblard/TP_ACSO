
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#define MAX_COMMANDS 200
#define MAX_ARGS 65  // 64 args + NULL

int comillas_balanceadas(const char *line) {
    char quote = 0;
    while (*line) {
        if (*line == '\'' || *line == '"') {
            if (quote == 0) {
                quote = *line;
            } else if (quote == *line) {
                quote = 0;
            }
        }
        line++;
    }
    return quote == 0;
}

void parse_args(char *line, char **args) {
    int i = 0;
    while (*line) {
        while (isspace(*line)) line++;
        if (*line == '\0') break;

        if (i >= MAX_ARGS - 1) {
            fprintf(stderr, "Error: se excedió el máximo de argumentos (%d)\n", MAX_ARGS - 1);
            exit(1);
        }

        if (*line == '"' || *line == '\'') {
            char quote = *line++;
            args[i++] = line;
            while (*line && *line != quote) line++;
            if (*line) {
                *line = '\0';
                line++;
            }
        } else {
            args[i++] = line;
            while (*line && !isspace(*line)) line++;
            if (*line) {
                *line = '\0';
                line++;
            }
        }
    }
    args[i] = NULL;
}

int main() {
    char command[4096];
    char *commands[MAX_COMMANDS];
    int command_count = 0;

    while (1) {
        if (isatty(STDIN_FILENO)) {
            printf("Shell> ");
            fflush(stdout);
        }

        if (fgets(command, sizeof(command), stdin) == NULL) break;
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "exit") == 0) break;

        if (!comillas_balanceadas(command)) {
            fprintf(stderr, "Error de sintaxis: comillas no cerradas\n");
            continue;
        }

        if (command[0] == '|' || command[strlen(command) - 1] == '|') {
            fprintf(stderr, "Error de sintaxis: pipe al inicio o al final\n");
            continue;
        }

        for (int i = 0; command[i] != '\0' && command[i + 1] != '\0'; i++) {
            if (command[i] == '|' && command[i + 1] == '|') {
                fprintf(stderr, "Error de sintaxis: pipe vacío\n");
                goto continuar_loop;
            }
        }

        command_count = 0;
        char *token = strtok(command, "|");

        while (token != NULL && command_count < MAX_COMMANDS) {
            while (*token && isspace(*token)) token++;
            if (*token == '\0' || strspn(token, " \t") == strlen(token)) {
                fprintf(stderr, "Error de sintaxis: pipe vacío\n");
                command_count = -1;
                break;
            }
            commands[command_count++] = token;
            token = strtok(NULL, "|");
        }

        if (command_count == -1) continue;
        if (command_count >= MAX_COMMANDS) {
            fprintf(stderr, "Error: se excedió el número máximo de comandos encadenados (%d)\n", MAX_COMMANDS);
            continue;
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
                if (i > 0) {
                    dup2(pipes[i - 1][0], STDIN_FILENO);
                }
                if (i < command_count - 1) {
                    dup2(pipes[i][1], STDOUT_FILENO);
                }

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

        for (int i = 0; i < command_count - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        for (int i = 0; i < command_count; i++) {
            wait(NULL);
        }

        continuar_loop:
        continue;
    }

    return 0;
}
