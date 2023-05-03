#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "command.h"
#include "strextra.h"

/* SCOMMAND */
typedef struct scommand_s {
    GList* scom_args;   // argumentos del comando
    char * output_file; // archivos redireccion de la salida
    char * input_file;  // archivos redireccion de la entrada
} scommand_s;


scommand scommand_new(void) {
    // Nuevo `scommand', sin comandos o argumentos y los redirectores vacíos
    scommand_s * self = malloc(sizeof(scommand_s));
    self->scom_args = NULL;
    self->input_file = NULL;
    self->output_file = NULL;

    assert(self != NULL && scommand_is_empty(self)
                        && scommand_get_redir_in(self) == NULL 
                        && scommand_get_redir_out(self) == NULL);

    return self;
}

scommand scommand_destroy(scommand self) {
    // Destruye `self'
    assert(self!=NULL);

    if (self->output_file != NULL) {
        free(self->output_file);
    }
    self->output_file = NULL;
    if (self->input_file != NULL) {
        free(self->input_file);
    }
    self->input_file = NULL;
    
    unsigned int length = scommand_length(self);

    for (unsigned int i = 0; i < length; i++) {
        free(g_list_nth_data(self->scom_args, i));
    }
    
    g_list_free(self->scom_args);
    self->scom_args = NULL;
    free(self);
    self = NULL;

    assert(self==NULL);

    return self;
}

void scommand_push_back(scommand self, char * argument) {
    // Agrega por detrás una cadena a la secuencia de cadenas
    assert(self!=NULL && argument!=NULL);

    self->scom_args = g_list_append(self->scom_args,argument);
    
    assert(!scommand_is_empty(self));
}

void scommand_pop_front(scommand self) {
    // Quita la cadena de adelante de la secuencia de cadenas
    assert(self!=NULL && !scommand_is_empty(self));
    
    gpointer elem =g_list_nth_data(self->scom_args,0);
    self->scom_args = g_list_remove(self->scom_args,elem);
    free(elem);
}

void scommand_set_redir_in(scommand self, char * filename) {
    // Define la redirección de entrada
    assert(self!=NULL);

    if (self->input_file != NULL) {
        free(self->input_file);
    }

    self->input_file = filename;
}

void scommand_set_redir_out(scommand self, char * filename) {
    // Define la redirección de salida
    assert(self!=NULL);

    if (self->output_file != NULL) {
        free(self->output_file);
    }

    self->output_file = filename;
}

bool scommand_is_empty(const scommand self) {
    // Indica si la secuencia de cadenas tiene longitud 0
    assert(self!=NULL);

    return(g_list_length(self->scom_args) == 0);
}

unsigned int scommand_length(const scommand self) {
    // Da la longitud de la secuencia cadenas que contiene el comando simple
    assert(self!=NULL);

    unsigned int length = g_list_length(self->scom_args);

    assert((length==0) == scommand_is_empty(self));

    return length;
}

char * scommand_front(const scommand self) {
    // Toma la cadena de adelante de la secuencia de cadenas
    assert(self!=NULL && !scommand_is_empty(self));

    char * result = self->scom_args->data;

    assert(result!=NULL);

    return result;
}
char * scommand_get_redir_in(const scommand self) {
    // apunta con str_in lo apuntado por self->input_file
    assert(self!=NULL);

    char * str_in = self->input_file;

    return str_in;
}

char * scommand_get_redir_out(const scommand self) {
    assert(self!=NULL);

    char * str_out = self->output_file;

    return str_out;
}    

char * scommand_to_string(const scommand self) {
    assert(self != NULL);
    char * merge = strdup("");
    char * aux_merge = NULL;
    char * aux_str = NULL;
    unsigned int length = g_list_length(self->scom_args);

    if(length != 0) {   // revisamos si la lista esta vacia
        for (unsigned int i=0; i < length; i++) {
            if (i != length-1){
                aux_str=strmerge(g_list_nth_data(self->scom_args,i)," ");
                aux_merge = strmerge(merge,aux_str);  // hacemos la mezcla del merge hasta el momento 
            } else {
                aux_merge = strmerge(merge,g_list_nth_data(self->scom_args,i));
            }
            free(merge);                                                     // y el siguiente y la guardamos en aux_merge
            merge = NULL;                                                    // liberamos la memoria en merge 
            free(aux_str);
            aux_str =NULL;
            merge = aux_merge;                                               // le asignamos aux_merrge a merge
        }
    }

    if (self->output_file != NULL) {
        aux_str= strmerge(" > ", self->output_file);
        aux_merge =strmerge(merge,aux_str);
        free(aux_str);
        aux_str = NULL;
        free(merge);
        merge = NULL;

        merge = aux_merge;
    }

    if (self->input_file != NULL) {
        aux_str= strmerge(" < ", self->input_file);
        aux_merge =strmerge(merge,aux_str);
        free(aux_str);
        aux_str = NULL;
        free(merge);
        merge = NULL;

        merge = aux_merge;
    }

    return merge;
}

/* PIPELINE */
typedef struct pipeline_s {
    GList* proc_pipe;
    bool is_expecting; // dice si debe correrse en background o foregroud
                       // true: foreground (espera). false: background (no espera)
} pipeline_s;

pipeline pipeline_new(void) {
    // solicitar memoria para un puntero a la estructura y rellenarlo con lista vacia, NUll y NUll
    pipeline_s* self = malloc(sizeof(pipeline_s));
    self->proc_pipe = NULL;
    self->is_expecting = true;
    
    assert(self!=NULL && pipeline_is_empty(self)
                      && pipeline_get_wait(self));
    
    return self;
}

pipeline pipeline_destroy(pipeline self) {
    /*liberar la memoria en los elementos de la estructura y luego liberar la memoria del puntero*/
    assert(self!=NULL);

    unsigned int length = g_list_length(self->proc_pipe);

    for (unsigned int i = 0; i < length; i++) {
	    scommand_destroy(g_list_nth_data(self->proc_pipe, i));
    }
    g_list_free(self->proc_pipe);
    
    free(self);
    self = NULL;

    return self;
}

void pipeline_push_back(pipeline self, scommand sc){
    /*agrega al final de la lista en self el scommand sc*/
    assert(self!=NULL && sc!=NULL);
    
    self->proc_pipe = g_list_append(self->proc_pipe, sc);

    assert(!pipeline_is_empty(self));
}

void pipeline_pop_front(pipeline self) {
    /*guardar el primer elemento de la lista en un puntero, eliminarlo de
    la lista, liberar la memoria de donde esta el scommand*/

    assert(self!=NULL && !pipeline_is_empty(self));

    //con g_slist_nth_data obtengo la info de (lista, posicion del
    //elemento en la lista)

    gpointer aux =g_list_nth_data(self->proc_pipe,0);

    self->proc_pipe = g_list_remove(self->proc_pipe, aux);

    scommand_destroy(aux);
}

void pipeline_set_wait(pipeline self, const bool w) {
    /* revisa el valor de w para saber si debe esperar o no, si debe esperar */
    assert(self!=NULL);

    self->is_expecting = w;
}

bool pipeline_is_empty(const pipeline self) {
    assert(self!=NULL);
    
    return(g_list_length(self->proc_pipe) == 0);
}

unsigned int pipeline_length(const pipeline self) {
    assert(self!=NULL);

    unsigned int length = g_list_length(self->proc_pipe);

    assert((length == 0) == pipeline_is_empty(self));

    return length;
}

scommand pipeline_front(const pipeline self) {
    assert(self!=NULL && !pipeline_is_empty(self));

    return g_list_nth_data(self->proc_pipe,0);
}

bool pipeline_get_wait(const pipeline self) {
    assert(self!=NULL);

    return self->is_expecting;
}


char * pipeline_to_string(const pipeline self) {
    assert(self!= NULL);
    char * merge = strdup("");
    char * sc_merge = NULL;
    char * aux_merge = NULL;
    unsigned int length = g_list_length(self->proc_pipe);

    if(length != 0) {   // revisamos si la lista esta vacia
        for (unsigned int i = 0; i < length; i++) {
            sc_merge = scommand_to_string(g_list_nth_data(self->proc_pipe,i));
            
            if (i!= length-1) {
                aux_merge = strmerge(sc_merge," | ");
                free(sc_merge);
                sc_merge = NULL;
                
            } else {
                aux_merge = sc_merge;
            }
            
            sc_merge = strmerge(merge,aux_merge);
            free(merge);
            merge = NULL;
            free(aux_merge);
            aux_merge = NULL;

            merge = sc_merge;
        }
    
        if (!self->is_expecting) {
            sc_merge=strmerge(merge," &");
            free(merge);
            merge = NULL;
            merge = sc_merge;
        }
    }

    return merge;
}
