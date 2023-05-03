#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

#include "command.h"
#include "execute.h"
#include "parser.h"
#include "parsing.h"
#include "builtin.h"

#define MAX_SIZE 256

#define ANSI_COLOR_GRENN "\x1b[01;32m"
#define ANSI_COLOR_RESET "\x1b[01;0m"
#define ANSI_COLOR_BLUE "\x1b[01;34m"

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

    printf (ANSI_COLOR_GRENN"%s@%s:",usr,host);
    printf(ANSI_COLOR_BLUE "~/%s",pwd);

    printf(ANSI_COLOR_RESET" $ ");
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
        while(waitpid(-1, NULL, WNOHANG)>0);
        show_prompt(usr,host);
        pipe = parse_pipeline(input);           //el parse pipeline
                                                //si el pipe no es null, se ejecuta. c.c. pasa de largo y 
                                                //me vuelve a pedir una entrada
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
