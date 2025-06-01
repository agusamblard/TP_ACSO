
#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_COMMANDS 100
#define MAX_ARGS 64
#define CMD_BUF_SIZE 4096



// ========================= setup senales =========================

void setup_signals() {
    struct sigaction sa;

    // ignoro ctrl c (SIGINT) en el shell principal
    memset( &sa, 0, sizeof(sa) );
    sa.sa_handler = SIG_IGN;
    sigaction( SIGINT, &sa, NULL );

    // ignoro ctrl z (SIGTSTP)
    sigaction( SIGTSTP, &sa, NULL );
}



// ========================= parser con comillas y validaciones =========================

bool parse_args_with_comillas(char *input, char *args[]) {
    int i = 0;
    while( *input ){
        while( isspace(*input) ) input++;
        if( *input == '\0' ) break;

        if(i >= MAX_ARGS ){
            fprintf(stderr, "Error: exceso de argumentos\n");
            return false;
        }

        if( *input == '"' ){
            input++;
            args[i++] = input;
            while( *input && *input != '"' ) input++;
            if( *input != '"' ){
                fprintf(stderr, "Error: comillas abiertas sin cerrar\n");
                return false;
            }
        } else {
            args[i++] = input;
            while( *input && !isspace(*input) ) input++;
        }

        if( *input ){
            *input = '\0';
            input++;
        }
    }

    args[i] = NULL;
    return true;
}


int count_quotes(const char *line) {
    int count = 0;
    while( *line ){
        if( *line == '"' ) count++;
        line++;
    }
    return count;
}

int is_syntax_error(const char *line) {
    int len = strlen(line);
    if(len == 0 || line[0] == '|' || line[len - 1] == '|' ) return 1;

    for( int i = 0; i < len - 1; ++i ){
        if( line[i] == '|' ){
            int j = i + 1;
            while( j < len && isspace((unsigned char)line[j]) ) j++;
            if( j < len && line[j] == '|' ) return 1;
        }
    }
    return 0;
}

// ========================= ejecuto con pipes =========================

void execute_commands_wit_pipes(char *commands[], int count) {
    int pipes[MAX_COMMANDS - 1][2];
    pid_t pids[MAX_COMMANDS];

    for( int i = 0; i < count - 1; i++ ){
        if( pipe(pipes[i]) == -1 ){
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for( int i = 0; i < count; i++){
        pids[i] = fork();
        if( pids[i] == 0 ){
            if( i > 0 ) dup2(pipes[i - 1][0], STDIN_FILENO);
            if( i < count - 1 ) dup2(pipes[i][1], STDOUT_FILENO);

            for( int j = 0; j < count - 1; j++ ){
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            char *args[MAX_ARGS + 1];
            if( !parse_args_with_comillas(commands[i], args) ){
                exit(EXIT_FAILURE);
            }

            if( strcmp(args[0], "exit") == 0 ){
                exit(0);
            }

            if( execvp(args[0], args) == -1 ){
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        }
    }

    for( int i = 0; i < count - 1; i++ ){
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for( int i = 0; i < count; i++ ){
        waitpid(pids[i], NULL, 0);
    }
}



// ========================= main =========================

int main() {
    char command[CMD_BUF_SIZE];
    char *commands[MAX_COMMANDS];
    int command_count;

    setup_signals();

    if( isatty(STDIN_FILENO) ){
        printf("Shell started ok. Type 'exit' or 'q' to quit.\n");
    }

    while( 1 ){
        if( isatty(STDIN_FILENO)){
            printf("Shell> ");
            fflush(stdout);
        }

        if( fgets(command, sizeof(command), stdin) == NULL ){
            break;
        }

        command[strcspn(command, "\n")] = '\0';
        if( strcmp(command, "exit") == 0 || strcmp(command, "q") == 0 ){
            break;
        }

        if(count_quotes(command) % 2 != 0 ){
            fprintf(stderr, "Error: comillas no balanceadas\n");
            continue;
        }

        if( is_syntax_error(command) ){
            fprintf(stderr, "Error de sintaxis\n");
            continue;
        }

        command_count = 0;
        char *start = command;
        bool in_quotes = false;

        for( char *p = command; ; ++p ){
            if( *p == '"' ){
                in_quotes = !in_quotes;
            } else if( *p == '|' && !in_quotes ){
                *p = '\0';
                commands[command_count++] = start;
                start = p + 1;
            } else if( *p == '\0' ){
                commands[command_count++] = start;
                break;
            }
        }


        for( int i = 0; i < command_count; i++ ){
            while( isspace(*commands[i]) ) commands[i]++;
            if( *commands[i] == '\0' ){
                fprintf(stderr, "Error de sintaxis: comando vacio entre pipes\n");
                command_count = 0;
                break;
            }
        }

        if( command_count == 0 ) continue;

        // cd
        if( strncmp(commands[0], "cd", 2) == 0 ){
            char *path = commands[0] + 2;
            while( isspace(*path) ) path++;
            if( *path == '\0' ){
                path = getenv("HOME");
            }
            if( chdir(path) != 0 ){
                perror("cd");
            }
            continue;
        }

        execute_commands_wit_pipes(commands, command_count);
    }

    if( isatty(STDIN_FILENO) ){
        printf("Shell terminated ok\n");
    }

    return 0;
}
