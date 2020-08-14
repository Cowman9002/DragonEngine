#ifndef D_INTERNAL_H
#define D_INTERNAL_H

#include <m3d/m3d.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define NUM_VERT_ATTRIB_INTERNAL 5

typedef struct
{
    uint8_t *keys;
    uint8_t *keys_l;

    uint8_t *m_buttons;
    uint8_t *m_buttons_l;

    int32_t mouse_x;
    int32_t mouse_y;

    float mouse_x_d;
    float mouse_y_d;

    float scroll_x;
    float scroll_y;

    GLFWgamepadstate *gp_states;
}DgnInput;

typedef struct
{
    GLFWwindow* native_window;
    const char* title;
    DgnInput *input;

    uint16_t width;
    uint16_t height;
    uint64_t frame_count;

    double time_1;
    double delta;

}DgnWindow;

typedef struct
{
    uint32_t VAO;
    uint32_t VBO;
    uint32_t IBO;
    uint32_t length;
}DgnMesh;

typedef struct
{
    uint32_t program;
}DgnShader;

typedef struct
{
    uint32_t texture;
    uint16_t *width;
    uint16_t *height;
    uint8_t mipmapped;
}DgnTexture;

typedef struct
{
    uint32_t buffer;
}DgnFramebuffer;

void set_input_holder_internal(DgnInput *input);
void key_callback_internal(GLFWwindow *window, int key, int scancode, int action, int mods);
void cursor_position_callback_internal(GLFWwindow *window, double xpos, double ypos);
void mouse_button_callback_internal(GLFWwindow *window, int button, int action, int mods);
void scroll_callback_internal(GLFWwindow *window, double xscroll, double yscroll);

#ifdef __DEBUG
#include <stdio.h>

void clearGLErrorsInternal();
uint8_t checkGLErrorsInternal();

void printDebugDataInternal(const char* file, uint32_t line);
void logErrorInternal(const char* error, const char* message, const char* file, uint32_t line);

#define glCall(func) clearGLErrorsInternal(); func; if(!checkGLErrorsInternal()) printDebugDataInternal(__FILE__, __LINE__)
#define logError(error, message) logErrorInternal(error, message, __FILE__, __LINE__)
#define logMessage(str, ...) printf(str, __VA_ARGS__)
#else
#define glCall(func) func
#define logError(error, message)
#endif // __DEBUG

#endif // D_INTERNAL_H

