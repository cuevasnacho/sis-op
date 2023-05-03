#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "parsing.h"
#include "parser.h"
#include "command.h"

static scommand parse_scommand(Parser p) {
    
    scommand cmd = NULL;
    char * args;
    arg_kind_t type;

    if(!parser_at_eof(p)) {
        bool remaing_arg = true;
        cmd = scommand_new();
        parser_skip_blanks(p);

        while(remaing_arg) {
            args = parser_next_argument(p, &type);
            
            if (args != NULL) {
                if (type == ARG_NORMAL) {
                    if (strcmp(args,"\n") == 0) {
                        remaing_arg = false;  
                    } else {
                        scommand_push_back(cmd, args);
                    }
                } else if (type == ARG_INPUT) {
                    scommand_set_redir_in(cmd, args);

                } else if (type == ARG_OUTPUT) {
                    scommand_set_redir_out(cmd, args);
                }

            } else {
                remaing_arg = false;
            }
            parser_skip_blanks(p);
        }
    }
    
    return cmd;
}

pipeline parse_pipeline(Parser p) {

    assert(p != NULL);

    pipeline result = pipeline_new();
    scommand cmd = NULL;
    bool error = false, another_pipe=true;
    bool auxbool = pipeline_get_wait(result);
    char * args;

    cmd = parse_scommand(p);
    
    if (cmd != NULL){
        if (scommand_length(cmd)==0) {
            cmd = scommand_destroy(cmd);
            
            if(!parser_at_eof(p)){
                printf("invalid command there is a empty command inside the pipe \n");
            }
        }
    }
    
        
    error = (cmd==NULL); /* Comando inválido al empezar */
    while (another_pipe && !error) {
        pipeline_push_back(result,cmd);
        
        parser_op_pipe(p,&another_pipe);
        
        if (another_pipe) {
            cmd = parse_scommand(p);
            if (cmd != NULL){
                if (scommand_length(cmd)==0) {
                    cmd = scommand_destroy(cmd);
                    printf("invalid command there is a empty command inside the pipe \n");
                }
            }
            error = (cmd == NULL);
        } else {
            parser_op_background(p,&auxbool);
            pipeline_set_wait(result,!auxbool);
        }
    }
    
    parser_skip_blanks(p);
    parser_garbage(p,&auxbool);

    if (auxbool && !error){
        args = parser_last_garbage(p);
        printf("Problem at read, this part of the command culdn`t read '%s' try again \n", args);
        result = pipeline_destroy(result);
    }
    if (error) {
        result = pipeline_destroy(result);
    }
    
    /* Tolerancia a espacios posteriores */
    /* Consumir todo lo que hay inclusive el \n */
    /* Si hubo error, hacemos cleanup */

    return result;
}
