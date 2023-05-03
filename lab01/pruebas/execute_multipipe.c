#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "tests/syscall_mock.h"//permite hacer los test
#include "execute.h"
#include "builtin.h"

/*
static void redirection_in(scommand sc) {
    char * redir_in = scommand_get_redir_in(sc);
    if (redir_in!=NULL) {
        int fp = open(redir_in, O_RDONLY);
        dup2(fp,STDIN_FILENO);
	    close(fp);
    }
}

static void redirection_out(scommand sc) {
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
*/
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

static void exec_argv(pipeline apipe) {
    /* Tomo el primer command y genero los argumentos */
    scommand cmd = pipeline_front(apipe);
    pipeline_pop_front(apipe);
    char ** argv = generate_argv(cmd);

    execvp(argv[0],argv);
}

static void execute_pipeline_rec(pipeline apipe, int read) {
    /* Caso para comandos internos */
    if (builtin_alone(apipe)) {
	    builtin_run(pipeline_front(apipe));
    }
    else {
    /* Caso para comandos externos */
	    unsigned int length = pipeline_length(apipe);
	    /* Caso base */
	    if (length == 1) {
		    int pid = fork();
		    if (pid < 0) {
			    fprintf(stderr,"An error ocurred forking");
		    } else if (pid == 0) {
			    if (read != STDIN_FILENO) { 	// Siempre que no sea el stdin
				    dup2(read,STDIN_FILENO);	// Redirige el stdin a la punta de lectura anterior
				    close(read);		// Cierra la punta de lectura en el hijo
			    }
			    exec_argv(apipe);			// Ejecuta el comando simple
		    }
		    if (read != STDIN_FILENO) {
			    close(read);			// Cierra la punta de lectura en el padre
		    }
		    if (pipeline_get_wait(apipe)) {
			    waitpid(pid,NULL,0);		// Espera si es necesario
		    }
	    }
	    /* Caso recursivo */
	    else if (length > 1) {
		    int fd[2];
		    if (pipe(fd) < 0) {				// Primero crea el pipe que se va a usar
			    fprintf(stderr,"An error ocurred creating the pipe");
		    }
		    int pid = fork();
		    if (pid < 0) {
			    fprintf(stderr,"An error ocurred forking");
		    } else if (pid == 0) {
			    dup2(fd[1],STDOUT_FILENO);		// Redirige el stdout a la escritura del pipe
			    dup2(read,STDIN_FILENO);		// Redirige el stdin a la punta de lectura anterior

			    close(fd[0]);			// Cierra las dos puntas antes de ejecutar
			    close(fd[1]);

			    exec_argv(apipe);			// Ejecuta el comando simple
		    }
		    pipeline_pop_front(apipe);			// Quita el primer comando del pipe en el padre

		    read = fd[0];				// Guardo la punta de lectura para el proximo comando
		    close(fd[1]);				// Cierro la punta de escritura
		    if (pipeline_get_wait(apipe)) {
			    waitpid(pid,NULL,0);		// ***** Hay que chequear espera al comando anterior
			    					// o si espera en el prompt
		    }

		    execute_pipeline_rec(apipe,read);		// Se llama recursivamente
	    }
    }
}

/* La idea es ejecutar el primer comando simple, si no hay mas comandos simples lo
 * manda a stdout
 * En cambio, si hay mas comandos simples deberia redirigir el pipe y ejecutar el
 * siguiente comando simple. Asi sucesivamente hasta llegar al ultimo comando simple.
 */

void execute_pipeline(pipeline apipe) {
	execute_pipeline_rec(apipe, STDIN_FILENO);		// La punta de lectura debe comenzar en STDIN
}
