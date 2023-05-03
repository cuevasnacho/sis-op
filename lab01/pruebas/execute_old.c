#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "tests/syscall_mock.h"//permite hacer los test
#include "execute.h"
#include "builtin.h"
static void redirection_in(scommand sc) {

    /* Redirecciona es stdin al input_file */
    char * redir_in = scommand_get_redir_in(sc);
    if (redir_in!=NULL) {
        int fp = open(redir_in, O_RDONLY);
        dup2(fp,STDIN_FILENO);
	    close(fp);
    }
}

static void redirection_out(scommand sc) {
    /* Redirecciona es stdin al output_file */
    char * redir_out = scommand_get_redir_out(sc);
    if (redir_out!=NULL) {
        int fp = open(redir_out, O_WRONLY | O_CREAT, 0777);
        dup2(fp,STDOUT_FILENO);
        close(fp);
    }
}

static scommand copy_scommand(scommand sc){

    scommand aux_scommand = scommand_new();
    unsigned int length = scommand_length(sc);
    char * aux_string ;

    for (unsigned int i = 0; i < length; i++){    
        aux_string = strdup(scommand_front(sc));
        scommand_push_back(aux_scommand,aux_string);
        scommand_pop_front(sc);
    }
    
    if (scommand_get_redir_in(sc) != NULL){
        aux_string = strdup(scommand_get_redir_in(sc));
        scommand_set_redir_in(aux_scommand, aux_string);
    }
    if (scommand_get_redir_out(sc) != NULL){
        aux_string= strdup(scommand_get_redir_out(sc));
        scommand_set_redir_out(aux_scommand, aux_string);
    }

    return aux_scommand;

}


static char ** generate_argv(scommand sc) {
    unsigned int cmd_length = scommand_length(sc);
    /* Aca voy a guardar los argumentos que le paso al execvp */
    char ** argv = calloc(cmd_length+1,sizeof(char*));
    
    /* Ahora cargo todos los argumentos en el arreglo */
    for (unsigned int i=0; i<cmd_length; ++i) {
        argv[i] = strdup(scommand_front(sc));

        scommand_pop_front(sc);
    }
    /* El ultimo argumento del execvp TIENE que ser NULL */
    argv[cmd_length] = NULL;

    return argv;
}

void execute_pipeline(pipeline apipe,bool * quit) {
    /* Lo primero seria dividir en comandos internos y
     * comandos externos
     */
    if (builtin_alone(apipe)) {
        builtin_run(pipeline_front(apipe),quit);
    } else {
    /* Este seria el caso en que hay mas de un comando en
     * el pipeline, no asegura que sean internos o externos
     */
        if (pipeline_length(apipe) == 1) {
            /* Este es el caso de un comando simple */
	        scommand cmd = pipeline_front(apipe);
            char ** argv = generate_argv(cmd);

            /* Empieza la ejecucion */
            int fp = fork();
            if (fp == 0) {
                /* Primero debo chequear si existe alguna redireccion */
		        redirection_out(cmd);
		        redirection_in(cmd);
		        int result = execvp(argv[0],argv);
			    if (result < 0) {
				    printf("An error ocurred executing the command");
			    }
            } else {
                if (pipeline_get_wait(apipe)) {
			        wait(NULL);
                }
            }
        } else if (pipeline_length(apipe) == 2) {
            scommand cmd1 = copy_scommand(pipeline_front(apipe));
            pipeline_pop_front(apipe);
            scommand cmd2 = copy_scommand(pipeline_front(apipe));
            pipeline_pop_front(apipe);

            int fd_pipe[2];
            if (pipe(fd_pipe) == -1) {
                printf("An error ocurred creating the pipe");
            }

            if (builtin_is_internal(cmd1)){
                dup2(fd_pipe[1],STDOUT_FILENO);
                close(fd_pipe[0]);
                close(fd_pipe[1]);

                builtin_run(cmd1,quit);
            } else {
                
                char ** argv1 = generate_argv(cmd1);
                int pid_cmd1 = fork();
                if (pid_cmd1 == 0) {
                    redirection_in(cmd1);
                    dup2(fd_pipe[1],STDOUT_FILENO);
                    
                    close(fd_pipe[0]);
                    close(fd_pipe[1]);
                    int result = execvp(argv1[0],argv1);
                    if (result < 0) {
    				    printf("An error ocurred executing the command");
                        free(argv1);
    			    }
                }
                free(argv1);
            }
            
            if (builtin_is_internal(cmd2)) {
                dup2(fd_pipe[0],STDOUT_FILENO);
                close(fd_pipe[0]);
                close(fd_pipe[1]);

                builtin_run(cmd2,quit);
            } else {
                char ** argv2 = generate_argv(cmd2);
                int pid_cmd2 = fork();
                if (pid_cmd2 == 0) {
                    redirection_out(cmd2);
                    
                    dup2(fd_pipe[0],STDIN_FILENO);

                    redirection_in(cmd2);
                    close(fd_pipe[0]);
                    close(fd_pipe[1]);
                    int result = execvp(argv2[0],argv2);
                    if (result < 0) {
    				    printf("An error ocurred executing the command");
                        free(argv2);
    			    }
                }
                free(argv2);
            }

            close(fd_pipe[0]);
            close(fd_pipe[1]);

            if (pipeline_get_wait(apipe)) {
                wait(NULL);
                wait(NULL);
            }
        }
    }
}