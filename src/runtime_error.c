/* Copyright (C) 2017 David Jowett
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */
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
