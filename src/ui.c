#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "chip8.h"
#include "shader.h"
#include "ui.h"

/* Keymap translates key presses to Chip-8 key codes.The index of the character
 * is the chip-8 key code. For example if "5" is pressed, 0x5 is sent to the
 * chip-8 emulator core */
static const char keycodes[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

static void resize_callback(GLFWwindow* window, int width, int height);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static size_t chip8_disp_to_indices(uint8_t disp[32][8], unsigned int **indices);

static void resize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
        const char* key_name = glfwGetKeyName(key, scancode);
        if((action == GLFW_PRESS || action == GLFW_RELEASE) && key_name != NULL){
                int i;
                for(i = 0; i < 16; i++){
                        if(keycodes[i] == key_name[0]) break;
                }
                if(i != 16){
                        struct ui *u = glfwGetWindowUserPointer(window);
                        if(u == NULL){
                                return;
                        }
                        struct keyEvent ke;
                        ke.key = i;
                        ke.type = (action == GLFW_PRESS) ? Pressed : Released;
                        chip8_key_event_notify(u->chip, ke);

                }
        }
}

static size_t chip8_disp_to_indices(uint8_t disp[32][8], unsigned int **indices){
        if(*indices != NULL)
                free(*indices);
        *indices = calloc(32 * 64 * 6, sizeof(unsigned int));
        size_t currentIndex = 0;
        for(int y = 0; y < 32; y++){
                for(int x = 0; x < 64; x++){
                        uint8_t byte = disp[y][x/8];
                        uint8_t state = byte >> (7 - (x % 8)) & 1;
                        if(state){
                                /* Tringale 1 */
                                (*indices)[currentIndex++] = 65 * y + x;             // top left
                                (*indices)[currentIndex++] = 65 * y + x + 1;         // top right
                                (*indices)[currentIndex++] = 65 * (y + 1) + x;       // bottom left
                                /* Triangle 2 */
                                (*indices)[currentIndex++] = 65 * (y + 1) + x;       // bottom left
                                (*indices)[currentIndex++] = 65 * (y + 1) + x + 1;   // bottom right
                                (*indices)[currentIndex++] = 65 * y + x + 1;         // top right
                        }

                }
        }
        return currentIndex;
}
static void *ui_render(void *arg){
        struct ui *u = (struct ui *) arg;
        int *retVal = malloc(sizeof(int));
        if(!glfwInit()){
                fprintf(stderr, "Failed to start GLFW\n");
                *retVal = -1;
                pthread_exit(retVal);
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        GLFWwindow *win;
        win = glfwCreateWindow(250, 250, "TEST", NULL, NULL);
        if(win == NULL){
                fprintf(stderr, "Could not create window\n");
                glfwTerminate();
                *retVal = -1;
                pthread_exit(retVal);
        }
        glfwMakeContextCurrent(win);
        glfwSetFramebufferSizeCallback(win, resize_callback);
        glfwSetKeyCallback(win, key_callback);
        glfwSetWindowUserPointer(win, u);
        glewExperimental = GL_TRUE;
        if(glewInit() != GLEW_OK){
                fprintf(stderr, "Failed to start GLEW\n");
                *retVal = -1;
                pthread_exit(retVal);
        }
        /* Load the shaders */
        struct shader *s = shader_load("shaders/vs.glsl", "shaders/fs.glsl");
        if(s == NULL){
                fprintf(stderr, "Failed to load shaders\n");
                *retVal = -1;
                pthread_exit(retVal);
        }

        float points[4290];
        /* calculate the ventices for a 64x32 pixel screen */
        for(int y = 0; y < 33; y++){
                for(int x = 0; x < 65; x ++){
                        points[y * 130 + (x * 2)] = (-1.0f + x * 2.0f / 64.0f);
                        points[y * 130 + (x * 2) + 1] = (0.5f - y * 2.0f / 64.0f);
                }
                
        }
        size_t indicesLen;
        unsigned int *indices = NULL;
        unsigned int EBO;
        unsigned int vao;
        unsigned int vbo;
        indicesLen = chip8_disp_to_indices(u->chip8Disp, &indices);

        /* generate buffers and vertex arrays */
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &EBO);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indicesLen, indices, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glfwSetInputMode(win, GLFW_STICKY_KEYS, GL_TRUE);

        do {
                pthread_mutex_lock(&u->dispMutex);
                if(u->newData){
                        indicesLen = chip8_disp_to_indices(u->chip8Disp, &indices);
                        u->newData = 0;
                }
                pthread_mutex_unlock(&u->dispMutex);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indicesLen, indices, GL_DYNAMIC_DRAW);
                
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                shader_use(s);
                glBindVertexArray(vao);
                
                glDrawElements(GL_TRIANGLES, indicesLen, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);

                glfwSwapBuffers(win);
                glfwPollEvents();

        } while(u->state && !glfwWindowShouldClose(win));
        glfwTerminate();
        *retVal = 0;
        pthread_mutex_lock(&u->stateMutex);
        u->state = STATE_HALTED;
        pthread_cond_broadcast(&u->uiStateChange);
        pthread_mutex_unlock(&u->stateMutex);
        pthread_exit(retVal);
}

struct ui *ui_init(void){
        struct ui *u = malloc(sizeof(struct ui));
        if(u == NULL) return NULL;
        pthread_mutex_init(&u->dispMutex, NULL);
        pthread_mutex_init(&u->stateMutex, NULL);
        pthread_cond_init(&u->uiStateChange, NULL);
        for(int i = 0; i < 32; i++)
                for(int j = 0; j < 8; j++)
                        u->chip8Disp[i][j] = 0x0;
        u->state = 0;
        return u;
}

void ui_destroy(struct ui **u){
        pthread_mutex_destroy(&(*u)->dispMutex);
        pthread_mutex_destroy(&(*u)->stateMutex);
        pthread_cond_destroy(&(*u)->uiStateChange);
        if(*u != NULL){
                free(*u);
                *u = NULL;
        }
}

int ui_set_chip8_display(struct ui *u, uint8_t chip8Disp[32][8]){
        pthread_mutex_lock(&u->dispMutex);
        for(int i = 0; i < 32; i++)
                for(int j = 0; j < 8; j++)
                        u->chip8Disp[i][j] = chip8Disp[i][j];
        u->newData = 1;
        pthread_mutex_unlock(&u->dispMutex);
        return u->state;
}

void ui_run(struct ui *u){
        pthread_mutex_lock(&u->stateMutex);
        u->state = STATE_RUNNING;
        pthread_cond_broadcast(&u->uiStateChange);
        pthread_create(&u->tid, NULL, ui_render, (void *) u);
        pthread_mutex_unlock(&u->stateMutex);
}

void ui_halt(struct ui *u){
        int *retVal = 0;
        /* request that UI stops. The UI thread will do a pthread_cond_broadcast
         * when it actually stops */
        u->state = STATE_HALTED;
        /* Wait for the thread to stop */
        pthread_join(u->tid, (void **) &retVal);

        free(retVal);
}
