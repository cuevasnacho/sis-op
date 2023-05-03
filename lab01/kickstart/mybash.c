#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "command.h"
#include "execute.h"
#include "parser.h"
#include "parsing.h"
#include "builtin.h"

#define MAX_SIZE 256

static void get_relative_path(char cwd[]) {
    unsigned int slash_cnt = 0, i=0, j=0;
    while (cwd[i]!='\0' && slash_cnt<3) {
    	slash_cnt = (cwd[i] == '/') ? slash_cnt + 1 : slash_cnt;
	    ++i;
    }
    if (slash_cnt == 3) {
        while (cwd[i]!='\0') {
            cwd[j] = cwd[i];
            ++i;
            ++j;
        }
        cwd[j] = '\0';
    }
    return;
}

static void show_prompt(char * usr, char host[]) {
    char pwd[MAX_SIZE];
    getcwd(pwd,sizeof(pwd));
    get_relative_path(pwd);

    printf ("[%s@%s:~/%s]> ",usr,host,pwd);
    fflush (stdout);
}

int main(int argc, char *argv[]) {
    pipeline pipe;
    Parser input;
    bool quit = false;

    char * usr = getlogin();
    char host[MAX_SIZE];
    gethostname(host,sizeof(host));

    input = parser_new(stdin);
    while (!quit) {
        show_prompt(usr,host);
        pipe = parse_pipeline(input);           //el parse pipeline
                                                //si el pipe no es null, se ejecuta. c.c. pasa de largo y 
                                                //me vuelve a pedir una entrada
        /* Hay que salir luego de ejecutar? */
        if (pipe!=NULL){
	        execute_pipeline(pipe,&quit);
            pipeline_destroy(pipe);             //destruye el pipe para crear uno nuevo en la siguiente vuelta
        }        
        if(!quit){
            quit = parser_at_eof(input);
        }
    }
    parser_destroy(input); input = NULL;

    return EXIT_SUCCESS;
}