#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// function declarations for builtin shell commands
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

// list of builtin commands, followed by their corresponding functions
char *builtin_str[] = {
    "cd",
    "help",
    "exit"};

int (*builtin_func[])(char **) = { // array of pointers to functions
    &lsh_cd,
    &lsh_help,
    &lsh_exit};

int lsh_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

// builtin function implementations

int lsh_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("lsh");
        }
    }
    return 1; // return 1 to continue executing
}

int lsh_help(char **args)
{
    int i;
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < lsh_num_builtins(); i++)
    {
        printf(" %s\n", builtin_str[i]);
    }

    printf("Use the man commands for information on ther programs.\n");
    return 1;
}

int lsh_exit(char **args)
{
    return 0;
}

int lsh_launch(char **args)
{
    pid_t pid, wpid; // process id
    int status;

    pid = fork(); // create a child process

    if (pid == 0)
    {
        if (execvp(args[0], args) == -1)
        { // execute the command
            perror("lsh");
        }
        exit(EXIT_FAILURE); // exit the child process
    }
    else if (pid < 0)
    {
        // error forking
        perror("lsh");
    }
    else
    {
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED); // wait for the child process to finish

        } while (!WIFEXITED(status) && !WIFSIGNALED(status)); // wait until the child process exits or is killed by a signal
    }
    return 1;
}

int lsh_execute(char **args)
{
    int i;
    if (args[0] == NULL)
    {
        // an empty command was entered
        return 1;
    }

    // handle builtin commands
    for (i = 0; i < lsh_num_builtins(); i++)
    {
        if (strcmp(args[0], builtin_str[i]) == 0)
        {
            return (*builtin_func[i])(args); // call the function
        }
    }

    return lsh_launch(args);
}

char *lsh_read_line(void)
{
#ifdef LSH_USE_STD_GETLINE
    char *line = NULL;
    ssize_t bufsize = 0; // have getline allocate a buffer for us
    if (getline(&line, &bufsize, stdin) == -1)
    { // -1 means error or end-of-file
        if (feof(stdin))
        {                       // we recieved an EOF
            exit(EXIT_SUCCESS); // we recieved an EOF
        }
        else
        {
            perror("lsh: getline\n");
            exit(EXIT_FAILURE);
        }
    }
    return line;
#else
#define LSH_RL_BUFSIZE 1024
    int bufsize = LSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer)
    {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        c = getchar();
        if (c == EOF)
        {
            exit(EXIT_SUCCESS);
        }
        else if (c == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else
        {
            buffer[position] = c;
        }

        position++;

        // if we have exceeded the buffer, reallocate

        if (position >= bufsize)
        {
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer)
            {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
#endif
}

#define LSH_TOK_BUFSIZE 64        // max number of tokens in a line
#define LSH_TOK_DELIM " \t\r\n\a" // delimiters for strtok

char **lsh_split_line(char *line)
{
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *)); // array of strings
    char *token;
    char **token_backup;

    if (!tokens)
    {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM); // split line into tokens
    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        // if we have exceeded the buffer, reallocate
        if (position >= bufsize)
        {
            bufsize += LSH_TOK_BUFSIZE;
            token_backup = tokens; // save the old tokens
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens)
            {
                free(token_backup); // free the old tokens
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, LSH_TOK_DELIM); // continue splitting line into tokens NULL means continue from last position
    }
    tokens[position] = NULL; // set the last token to NULL
    return tokens;
}

void lsh_loop(void)
{
    char *line;
    char **args;
    int status;

    do
    {
        printf("> ");                // to look like a shell
        line = lsh_read_line();      // read command from standard input
        args = lsh_split_line(line); // separate command string into program and arguments
        status = lsh_execute(args);  // execute the command

        free(line);
        free(args);

    } while (status);
}

int main(int argc, char **argv)
{
    // load config files

    // run command loop
    lsh_loop();

    // perform shutdown

    return EXIT_SUCCESS;
}