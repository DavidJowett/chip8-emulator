#ifndef _RUNTIME_ERROR_H
#define _RUNTIME_ERROR_H
/* A simply struct to provide more detailed errors
 */

struct runtime_error {
        char *msg;
};


struct runtime_error *runtime_error_init(char *msg);
void runtime_error_destroy(struct runtime_error **re);

#endif
