#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include "shader.h"

static char *readFile(const char *fPath){
        FILE *fp;
        char *buff = NULL;
        size_t len = 0;
        fp = fopen(fPath, "r");
        if(fp == NULL)
                return NULL;
        fseek(fp, 0L, SEEK_END);
        len =  ftell(fp);
        rewind(fp);
        buff = calloc(len + 1, sizeof(char));
        fread(buff, sizeof(char), len, fp);
        buff[len] = 0;
        fclose(fp);

        return buff;
}

struct shader *shader_load(const char *vfp, const char *ffp){
        unsigned int vsid;
        unsigned int fsid;
        int result;
        size_t logLen;
        struct shader *s = malloc(sizeof(struct shader));
        if(s == NULL) goto sfail;

        vsid = glCreateShader(GL_VERTEX_SHADER);
        fsid = glCreateShader(GL_FRAGMENT_SHADER);

        char *vSource = readFile(vfp);
        if(vSource == NULL){
                fprintf(stderr, "Could not open shader file \"%s\"\n", vfp);
                goto vsfail;
        }
        char *fSource = readFile(ffp);
        if(fSource == NULL){
                fprintf(stderr, "Could not open shader file \"%s\"\n", ffp);
                goto fsfail;
        }
        glShaderSource(vsid, 1, &vSource, NULL);
        glCompileShader(vsid);
        glGetShaderiv(vsid, GL_COMPILE_STATUS, &result);
        glGetShaderiv(vsid, GL_INFO_LOG_LENGTH, &logLen);
        if(result == GL_FALSE){
                printf("%d %s\n", result, vSource);
                fprintf(stderr, "Could not compile shader file \"%s\"\n", vfp);
                if(logLen > 0){
                        char *msg = calloc(logLen + 1, sizeof(char));
                        glGetShaderInfoLog(vsid, logLen, NULL, &msg);
                        fprintf(stderr, "%s\n", msg);
                        free(msg);
                }
                goto vcfail;
        }
        glShaderSource(fsid, 1, &fSource, NULL);
        glCompileShader(fsid);
        glGetShaderiv(fsid, GL_COMPILE_STATUS, &result);
        glGetShaderiv(fsid, GL_INFO_LOG_LENGTH, &logLen);
        if(!result){
                fprintf(stderr, "Could not compile shader file \"%s\"", ffp);
                if(logLen > 0){
                        char *msg = calloc(logLen + 1, sizeof(char));
                        glGetShaderInfoLog(fsid, logLen, NULL, &msg);
                        fprintf(stderr, "%s\n", msg);
                        free(msg);
                }
                goto fcfail;
        }

        s->id = glCreateProgram();
        glAttachShader(s->id, vsid);
        glAttachShader(s->id, fsid);
        glLinkProgram(s->id);
        glGetProgramiv(s->id, GL_LINK_STATUS, &result);
        glGetProgramiv(s->id, GL_INFO_LOG_LENGTH, &logLen);
        if(!result){
                fprintf(stderr, "Could not link shader files!");
                if(logLen > 0){
                        char *msg = calloc(logLen + 1, sizeof(char));
                        glGetProgramInfoLog(s->id, logLen, NULL, &msg);
                        fprintf(stderr, "%s\n", msg);
                        free(msg);
                }
                goto lfail;
        }

        free(vSource);
        free(fSource);
        glDetachShader(s->id, vsid);
        glDetachShader(s->id, fsid);
        glDeleteShader(vsid);
        glDeleteShader(fsid);


        return s;
lfail:
        glDeleteProgram(s->id);
fcfail:
        glDeleteShader(vsid);
vcfail:
        free(fSource);
fsfail:
        free(vSource);

vsfail:
        free(s);
sfail:
        return NULL;
}

void shader_delete(struct shader **s){
        glDeleteProgram((*s)->id);
        free(*s);
        *s = NULL;
}

void shader_use(struct shader *s){
        glUseProgram(s->id);
}

void shader_set_bool(struct shader *s, const char *name, int value){
        int vLoc = glGetUniformLocation(s->id,  name);
        glUniform1i(vLoc, value);
}

void shader_set_int(struct shader *s, const char *name, int value){
        int vLoc = glGetUniformLocation(s->id, name);
        glUniform1i(vLoc, value);
}

void shader_set_float(struct shader *s, const char *name, float value){
        int vLoc = glGetUniformLocation(s->id, name);
        glUniform1f(vLoc, value);
}

void shader_set_4float(struct shader *s, const char *name, float v1, float v2, float v3, float v4){
        int vLoc = glGetUniformLocation(s->id, name);
        glUniform4f(vLoc, v1, v2, v3, v4);
}
