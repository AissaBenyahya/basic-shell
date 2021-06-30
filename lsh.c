#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <editline/readline.h>
#include <editline/history.h>


#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM  " \t\r\n\a"


void lsh_loop(void);
char *lsh_read_line(void);
char **lsh_split_line(char *line);
int lsh_execute(char **args);

//Built in functions
int lsh_num_builtin(void);
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

char *builtin_str[] = {
	"cd",
	"help",
	"exit"
};

int (*builtin_func[]) (char **) = {
	&lsh_cd,
	&lsh_help,
	&lsh_exit
};

int main(int argc, char **argv)
{
    // Load config files, if any

    // Run command loop
    lsh_loop();

    // Perform any shutdown/cleanup.

    return EXIT_SUCCESS;    
}

void lsh_loop(void)
{
    char *line;
    char **args;
    int status;

    do{
        //printf("> ");
        line = readline(">>>");//lsh_read_line();
	add_history(line);
        args = lsh_split_line(line);
        status = lsh_execute(args);

        free(line);
        free(args);
       }while(status);
}

/*char *lsh_read_line(void)
{
    
  
    char *line = NULL;
    ssize_t bufsize = 0; // have getline allocate a buffer for us

    if (getline(&line, &bufsize, stdin) == -1){
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);  // We recieved an EOF
        } else{
            perror("readline");
        exit(EXIT_FAILURE);
        }
     return line;
    

    int bufsize = LSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize); 
    int c;

    if(!buffer)
    {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        // Read a character
        c = getchar();

        // If we hit EOF, replace it with a null character and return.
        if(c == EOF || c == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }else{
            buffer[position] = c;
        }
        position++;

        // If we have exceeded the buffer, reallocate.
        if(position >= bufsize){
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if(!buffer)
            {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}*/

char **lsh_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(sizeof(char*) * bufsize);
  char *token;

  if(!tokens)
  {
      fprintf(stderr, "lsh allocation error\n");
      exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while(token != NULL)
  {
      tokens[position] = token;
      position++;

      if(position >= bufsize){
          bufsize += LSH_TOK_BUFSIZE;
          token = realloc(tokens, bufsize * sizeof(char *));
          if(!token){
              fprintf(stderr, "lsh allocation error");
              exit(EXIT_FAILURE);
          }
      }

      token = strtok(NULL, LSH_TOK_DELIM);
  }
  
  tokens[position] = NULL;
  return tokens;
}

int lsh_launch(char **args) // Command execution
{
   pid_t pid, wpid;
   int status;

   pid = fork();
   if(pid == 0){
      // pid is a child
      
       if(execvp(args[0], args) == -1){
           perror("lsh");
       }
       exit(EXIT_FAILURE);
   }else if (pid < 0){
       // Error forking
       perror("lsh");
   }else{
       // the parent process
       do{           
           wpid = waitpid(pid, &status, WUNTRACED);
       }while(!WIFEXITED(status) && !WIFSIGNALED(status)); 
   }

   return 1;
}

int lsh_num_builtin(void){
    return sizeof(builtin_str) / sizeof(char *);
}

int lsh_cd(char **args)
{
    if(args[1] == NULL){
        fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    }else{
        if(chdir(args[1]) != 0){
            perror("lsh");
        }
    }

    return 1;
}

int lsh_help(char **args)
{
    int i;
    printf("Aissa ben yahya's shell\n");
    printf("Use: <program name> <argument>. \n");
    printf("-------------------------------------\n");
    printf("The following are a built in: \n");

    for(i = 0; i < lsh_num_builtin(); i++){
        printf(" %s\n", builtin_str[i]);
    }
    
    return 1;
}

int lsh_exit(char **args){
    return 0;
}


int lsh_execute(char **args)
{
    if(args[0] == NULL){
        // The command is empty
        return 1;
    }

    for (int i = 0; i < lsh_num_builtin(); i++){
        if(strcmp(args[0], builtin_str[i]) == 0){
           return (*builtin_func[i])(args);
        }
    }

   return lsh_launch(args); 
}
