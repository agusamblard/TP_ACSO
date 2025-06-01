
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#define MAX_COMMANDS 200
#define MAX_ARGS 64

void parse_args(char *line, char **args) {
    int i = 0;
    while (*line && i < MAX_ARGS - 1) {
        while (isspace(*line)) line++;
        if (*line == '\0') break;

        if (*line == '\"' || *line == '\'') {
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
    char command[1024];
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

        if (command[0] == '|' || command[strlen(command) - 1] == '|') {
            fprintf(stderr, "Error de sintaxis: pipe al inicio o al final\n");
            continue;
        }
        command_count = 0;
        char *token = strtok(command, "|");
        int pipe_expected = 0;

        while (token != NULL && command_count < MAX_COMMANDS) {
            while (*token && isspace(*token)) token++;
            if (*token == '\0') {
                fprintf(stderr, "Error de sintaxis: pipe vacío\n");
                command_count = -1;
                break;
            }
            commands[command_count++] = token;
            token = strtok(NULL, "|");

            // Si hay otro token, esperamos contenido. Si es vacío, será detectado arriba.
            pipe_expected = 1;
        }

        if (pipe_expected && command_count == 0) {
            fprintf(stderr, "Error de sintaxis: solo pipes\n");
            continue;
        }


        if (command_count == -1) continue;

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
    }

    return 0;
}
