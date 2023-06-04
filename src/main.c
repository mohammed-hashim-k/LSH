#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


char *lsh_read_line(void){
    #ifdef LSH_USE_STD_GETLINE
        char *line = NULL;
        ssize_t bufsize = 0; // have getline allocate a buffer for us
        if (getline(&line, &bufsize, stdin) ==-1){ // -1 means error or end-of-file
            if (feof(stdin)){ // we recieved an EOF
                exit(EXIT_SUCCESS); // we recieved an EOF
            } else {
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

        if (!buffer){
            fprintf(stderr, "lsh: allocation error\n");
            exit(EXIT_FAILURE);
        }

        while(1){
            c = getchar();
            if (c == EOF){
                exit(EXIT_SUCCESS);
            }
            else if (c == '\n'){
                buffer[position] = '\0';
                return buffer;
            }
            else {
                buffer[position] = c;
            }

            position++;


            // if we have exceeded the buffer, reallocate

            if (position >= bufsize){
                bufsize += LSH_RL_BUFSIZE;
                buffer = realloc(buffer, bufsize);
                if (!buffer){
                    fprintf(stderr, "lsh: allocation error\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    #endif
}

void lsh_loop(void)
{
    char *line;
    char **args;
    int status;

    do
    {
        printf("> "); // to look like a shell
        line = lsh_read_line(); // read command from standard input
        args = lsh_split_line(line); // separate command string into program and arguments
        status = lsh_execute(args); //execute the command

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