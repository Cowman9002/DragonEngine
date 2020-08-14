#include "d_internal.h"
#include "DGNEngine/DGNEngine.h"

#include <stdio.h>

#include <MemLeaker/malloc.h>

uint8_t dgnWindowCreate(DgnWindow **out_window, uint16_t width, uint16_t height, const char* title)
{
    if(glfwInit() == GLFW_FALSE)
    {
        return DGN_FALSE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
    if(window == NULL)
    {
        return DGN_FALSE;
    }

    DgnInput* new_input = malloc(sizeof(*new_input));
    new_input->keys = malloc(sizeof(*new_input->keys) * DGN_KEY_LAST);
    new_input->keys_l = malloc(sizeof(*new_input->keys_l) * DGN_KEY_LAST);
    new_input->m_buttons = malloc(sizeof(*new_input->m_buttons) * DGN_MOUSE_BUTTON_LAST);
    new_input->m_buttons_l = malloc(sizeof(*new_input->m_buttons_l) * DGN_MOUSE_BUTTON_LAST);
    new_input->gp_states = malloc(sizeof(*new_input->gp_states) * DGN_GAMEPAD_LAST);

    for(int i = 0; i < DGN_GAMEPAD_LAST; i++)
    {
        int jid = GLFW_JOYSTICK_1 + i;
        if (glfwJoystickIsGamepad(jid))
        {
            logMessage("%s Connected to port %u\n", glfwGetGamepadName(jid), jid);
        }
    }

    *out_window = malloc(sizeof(**out_window));

    (*out_window)->native_window  = window;
    (*out_window)->width          = width;
    (*out_window)->height         = height;
    (*out_window)->title          = title;
    (*out_window)->frame_count    = 0;
    (*out_window)->input          = new_input;
    (*out_window)->time_1         = glfwGetTime();

    glfwSetKeyCallback(window, key_callback_internal);
    glfwSetMouseButtonCallback(window, mouse_button_callback_internal);
    glfwSetCursorPosCallback(window, cursor_position_callback_internal);
    glfwSetScrollCallback(window, scroll_callback_internal);

    return DGN_TRUE;
}

void dgnWindowDestroy(DgnWindow *window)
{
    glfwDestroyWindow(window->native_window);

    free(window->input->keys);
    free(window->input->keys_l);
    free(window->input->m_buttons);
    free(window->input->m_buttons_l);
    free(window->input->gp_states);
    free(window->input);

    free(window);
}

void dgnWindowMakeCurrent(DgnWindow *window)
{
    glfwMakeContextCurrent(window->native_window);
    set_input_holder_internal(window->input);
}

uint8_t dgnWindowShouldClose(DgnWindow *window)
{
    return glfwWindowShouldClose(window->native_window);
}

void dgnWindowSwapBuffers(DgnWindow *window)
{
    glfwSwapBuffers(window->native_window);
    window->frame_count++;
    double time = glfwGetTime();
    window->delta = time - window->time_1;
    window->time_1 = time;
}

uint16_t dgnWindowGetWidth(DgnWindow *window)
{
    return window->width;
}

uint16_t dgnWindowGetHeight(DgnWindow *window)
{
    return window->height;
}

const char* dgnWindowGetTitle(DgnWindow *window)
{
    return window->title;
}

uint64_t dgnWindowGetFrameCount(DgnWindow *window)
{
    return window->frame_count;
}

double dgnWindowGetDelta(DgnWindow *window)
{
    return window->delta;
}

void dgnWindowSetRawCursorMode(DgnWindow *window, uint8_t enabled)
{
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(window->native_window, GLFW_RAW_MOUSE_MOTION, enabled);
    }
}

void dgnWindowSetCursorMode(DgnWindow *window, uint32_t cursor_mode)
{
    glfwSetInputMode(window->native_window, GLFW_CURSOR, cursor_mode);
}

void dgnWindowSetVsync(uint8_t sync)
{
    glfwSwapInterval(sync);
}

void dgnWindowSetWidth(DgnWindow *window, uint16_t new_width)
{
    window->width = new_width;
    glfwSetWindowSize(window->native_window, new_width, window->height);
}

void dgnWindowSetHeight(DgnWindow *window, uint16_t new_height)
{
    window->height = new_height;
    glfwSetWindowSize(window->native_window, window->width, new_height);
}

void dgnWindowSetSize(DgnWindow* window, uint16_t new_width, uint16_t new_height)
{
    window->width = new_width;
    window->height = new_height;
    glfwSetWindowSize(window->native_window, new_width, new_height);
}

void dgnWindowSetTitle(DgnWindow *window, const char* new_title)
{
    window->title = new_title;
    glfwSetWindowTitle(window->native_window, new_title);
}


