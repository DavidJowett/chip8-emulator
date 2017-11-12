#include <stdlib.h>
#include <string.h>
#include "runtime_error.h"

struct runtime_error *runtime_error_init(char *msg){
        struct runtime_error *re = malloc(sizeof(struct runtime_error));
        if(re == NULL) goto mFail;
        re->msg = calloc(strlen(msg), sizeof(char));
        if(re->msg == NULL) goto msgFail;
        strcpy(re->msg, msg);

        return re;
msgFail:
        free(re);
mFail:
        return NULL;
}

void runtime_error_destroy(struct runtime_error **re){
        free((*re)->msg);
        free(*re);
        *re = NULL;
}
