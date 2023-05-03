#include <stdio.h>      // ferror()
#include <stdlib.h>     // exit(EXIT_SUCCESS)
#include <stdbool.h>    // bool
#include <assert.h>     // assert()
#include <string.h>     // strcmp()
#include <unistd.h>     // chdir()

#include "tests/syscall_mock.h" // hacer los test

#include "command.h"    // scommand y pipeline
#include "builtin.h"

/* Indica si el comando alojado en `cmd` es un comando 
 * interno
 * Lista de comandos internos: cd, help, exit
 */
bool builtin_is_internal(scommand cmd) {
    assert(cmd!=NULL);
    bool is_int = false;
    char * cmd_alloc = scommand_front(cmd);
    
    if (!strcmp(cmd_alloc,"cd") || !strcmp(cmd_alloc,"help") || !strcmp(cmd_alloc, "exit")) {
        is_int = true;
    }

    return is_int;
}

/* Indica si el pipeline tiene solo un elemento y si este 
 * se corresponde a un comando interno
 */
bool builtin_alone(pipeline p) {
    assert(p!=NULL);
    return (pipeline_length(p)==1 &&
            builtin_is_internal(pipeline_front(p)));
}

/* Ejecuta un comando interno
 */
void builtin_run(scommand cmd, bool* quit) {
    assert(builtin_is_internal(cmd));
    unsigned int length = scommand_length(cmd);
    char * str_cmd = scommand_front(cmd);
    *quit = *quit;

    if (!strcmp(str_cmd,"cd")) {
        if (length == 2) {
            scommand_pop_front(cmd);
            char * direction =scommand_front(cmd);
            
            int result = chdir(direction);
            if (result < 0) {
                printf("mybash: cd: %s: No such file or directory \n", direction);
            }
            
        } else if (length == 1) {  
	        chdir(getenv("HOME"));
	    } else {
            perror("mybash: cd: too many arguments");
        }
    } else if (!strcmp(str_cmd,"help")) {
        if (length == 1) {
            printf( "GNU mybash, version 1.0\n"
                    "These shell commands are defined internally. Type `help' to see this list\n\n"
                    "cd [dir]: Change the current directory to DIR.\n\n"
                    "The default DIR is the value of the HOME shell variable.\n\n"
                    "`..' is processed by removing the immediately previous pathname component"
                    "back to a slash or the beginning of DIR.\n\n\n"
                    "help: Displays brief summaries of builtin commands.\n\n\n"
                    "exit: Exit the shell.\n");
        } else {
            perror("mybash: help: too many arguments");
        }
    } else {
        
        *quit = true;
    }
}