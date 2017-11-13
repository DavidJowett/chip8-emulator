#ifndef _SRC_SHADER_LOADER_H
#define _SRC_SHADER_LOADER_H
#include <GLFW/glfw3.h>

struct shader {
        unsigned int id;
};


struct shader *shader_load(const char *vfp, const char *ffp);
void shader_delete(struct shader **s);
void shader_use(struct shader *s);
void shader_set_bool(struct shader *s, const char *name, int value);
void shader_set_int(struct shader *s, const char *name, int value);
void shader_set_float(struct shader *s, const char *name, float value);
void shader_set_4float(struct shader *s, const char *name, float v1, float v2, float v3, float v4);


#endif
