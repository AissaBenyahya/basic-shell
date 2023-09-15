#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>

#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

// Function prototypes
void lsh_loop(void);
int lsh_launch(char **args);
char *lsh_read_line(void);
char **lsh_split_line(char *line);
int lsh_execute(char **args);
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

char *builtin_str[] = {
        "cd",
        "help",
        "exit"
};

int (*builtin_func[])(char **) = {
        &lsh_cd,
        &lsh_help,
        &lsh_exit
};


char *lsh_read_line(void) {
        char *line = NULL;
        size_t bufsize = 0;

        if (getline(&line, &bufsize, stdin) == -1) {
                if (feof(stdin)) {
                // End of file (Ctrl+D) or other read error
                exit(EXIT_SUCCESS);
        } else {
                // Handle error
                fprintf(stderr, "error: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
        }
    }

        return line;
}

int main(int argc, char **argv) {
        // Load config files, if any

        // Run command loop
        lsh_loop();

        // Perform any shutdown/cleanup.

        return EXIT_SUCCESS;
}

void lsh_loop(void) {
        char *line;
        char **args;
        int status;

        do {
                printf("$ "); // Prompt user for input
                line = lsh_read_line();
                args = lsh_split_line(line);
                status = lsh_execute(args);

                free(line);
                free(args);
        } while (status);
}

char **lsh_split_line(char *line) {
        int bufsize = LSH_TOK_BUFSIZE, position = 0;
        char **tokens = malloc(bufsize * sizeof(char *));
        char *token, **tokens_backup;

        if (!tokens) {
                fprintf(stderr, "error: %s\n", "lsh: allocation error\n");
                exit(EXIT_FAILURE);
        }

        token = strtok(line, LSH_TOK_DELIM);
        while (token != NULL) {
                tokens[position] = token;
                position++;

                if (position >= bufsize) {
                        bufsize += LSH_TOK_BUFSIZE;
                        tokens_backup = tokens;
                        tokens = realloc(tokens, bufsize * sizeof(char *));
                        if (!tokens) {
                                free(tokens_backup);
                                fprintf(stderr, "error: %s\n", "lsh: allocation error\n");
                                exit(EXIT_FAILURE);
                        }
                }

                token = strtok(NULL, LSH_TOK_DELIM);
        }
        tokens[position] = NULL;
        return tokens;
}

int lsh_execute(char **args) {
        if (args[0] == NULL) {
                // The command is empty
                return 1;
        }

        for (int i = 0; i < sizeof(builtin_str) / sizeof(char *); i++) {
                if (strcmp(args[0], builtin_str[i]) == 0) {
                        return (*builtin_func[i])(args);
                }
        }

        return lsh_launch(args);
}

int lsh_launch(char **args) {
        pid_t pid;
        int status;

        pid = fork();
        if (pid == 0) {
                // Child process
                if (execvp(args[0], args) == -1) {
                        fprintf(stderr,"error: %s\n", "lsh: allocation error");
                }
                exit(EXIT_FAILURE);
        } else if (pid < 0) {
                // Error forking
                fprintf(stderr,"error: %s\n", "lsh: allocation error");
        } else {
                // Parent process
                do {
                        waitpid(pid, &status, WUNTRACED);
                } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }

        return 1;
}

int lsh_cd(char **args) {
        if (args[1] == NULL) {
                fprintf(stderr, "error: %s\n", "lsh: missing argument to 'cd'");
        } else {
                if (chdir(args[1]) != 0) {
                        fprintf(stderr, "error: %s\n", "lsh: missing argument to 'cd'");
                }
        }
        return 1;
}

int lsh_help(__attribute__((unused)) char **args) {
        printf("w4118_sh - A Simple Shell\n");
        printf("Commands:\n");
        printf("  cd <directory>: Change the current directory\n");
        printf("  help: Display this help message\n");
        printf("  exit: Exit the shell\n");
        return 1;
}

int lsh_exit(__attribute__((unused)) char **args) {
        printf("Goodbye! Exiting w4118_sh.\n");
        return 0;
}

