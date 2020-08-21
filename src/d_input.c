#include "d_internal.h"
#include "DGNEngine/DGNEngine.h"

#include "d_defines.h"

#include <math.h>

#include "c_linked_list.h"
#include "c_ordered_map.h"

typedef struct
{
    const char *axis;
    uint16_t positive;
    uint16_t negative;
}KeyAxis;

typedef struct
{
    const char *axis;
    uint8_t positive;
    uint8_t negative;
}ButtonAxis;

typedef struct
{
    const char *axis;
    uint8_t gamepad;
    uint8_t gp_axis_name;
    float deadzone;
}GamepadAxis;

DgnInput *s_current_input;

void dgnInputPollEvents()
{
    for(int i = 0; i <DGN_KEY_LAST; i++)
    {
        s_current_input->keys_l[i] = s_current_input->keys[i];
    }

    for(int i = 0; i <DGN_MOUSE_BUTTON_LAST; i++)
    {
        s_current_input->m_buttons_l[i] = s_current_input->m_buttons[i];
    }

    for(int i = 0; i < DGN_GAMEPAD_LAST; i++)
    {
        glfwGetGamepadState(GLFW_JOYSTICK_1 + i, &s_current_input->gp_states[i]);
    }

    s_current_input->mouse_x_d = 0;
    s_current_input->mouse_y_d = 0;

    glfwPollEvents();
}

uint8_t dgnInputGetKey(uint16_t key)
{
    return s_current_input->keys[key];
}

uint8_t dgnInputGetKeyDown(uint16_t key)
{
    return s_current_input->keys[key] && !s_current_input->keys_l[key];
}

uint8_t dgnInputGetKeyUp(uint16_t key)
{
    return !s_current_input->keys[key] && s_current_input->keys_l[key];
}

uint8_t dgnInputGetMouseButton(uint8_t button)
{
    return s_current_input->m_buttons[button];
}

uint8_t dgnInputGetMouseButtonDown(uint8_t button)
{
    return s_current_input->m_buttons[button] && !s_current_input->m_buttons_l[button];
}

uint8_t dgnInputGetMouseButtonUp(uint8_t button)
{
    return !s_current_input->m_buttons[button] && s_current_input->m_buttons_l[button];
}

int32_t dgnInputGetMouseX()
{
    return s_current_input->mouse_x;
}

int32_t dgnInputGetMouseY()
{
    return s_current_input->mouse_y;
}

float dgnInputGetMouseXDelta()
{
    return s_current_input->mouse_x_d;
}

float dgnInputGetMouseYDelta()
{
    return s_current_input->mouse_y_d;
}

float dgnInputGetGamepadAxis(uint8_t gamepad, uint8_t axis, float deadzone)
{
    if(gamepad > DGN_GAMEPAD_LAST) return 0.0f;

    float v = s_current_input->gp_states[gamepad].axes[axis];

    return fabs(v) < deadzone ? 0 : v;
}

uint8_t dgnInputGetGamepadButton(uint8_t gamepad, uint8_t button)
{
    if(gamepad > DGN_GAMEPAD_LAST) return 0.0f;

    return s_current_input->gp_states[gamepad].buttons[button];
}

/*****************************************************************/

void set_input_holder_internal(DgnInput *input)
{
     for(int i = 0; i <DGN_KEY_LAST; i++)
    {
        input->keys[i] = DGN_FALSE;
        input->keys_l[i] = DGN_FALSE;
    }

    for(int i = 0; i <DGN_MOUSE_BUTTON_LAST; i++)
    {
        input->m_buttons[i] = DGN_FALSE;
        input->m_buttons_l[i] = DGN_FALSE;
    }

    s_current_input = input;
}

void key_callback_internal(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(key != DGN_KEY_UNKNOWN)
    {
        if(action == GLFW_PRESS)
        {
            s_current_input->keys[key] = DGN_TRUE;
        }
        else if(action == GLFW_RELEASE)
        {
            s_current_input->keys[key] = DGN_FALSE;
        }
    }
}

void mouse_button_callback_internal(GLFWwindow* window, int button, int action, int mods)
{
    if(action == GLFW_PRESS)
    {
        s_current_input->m_buttons[button] = DGN_TRUE;
    }
    else if(action == GLFW_RELEASE)
    {
        s_current_input->m_buttons[button] = DGN_FALSE;
    }
}

void cursor_position_callback_internal(GLFWwindow* window, double xpos, double ypos)
{
    s_current_input->mouse_x_d = xpos - s_current_input->mouse_x;
    s_current_input->mouse_y_d = ypos - s_current_input->mouse_y;

    s_current_input->mouse_x = (int32_t)xpos;
    s_current_input->mouse_y = (int32_t)ypos;
}

void scroll_callback_internal(GLFWwindow *window, double xscroll, double yscroll)
{
    s_current_input->scroll_x = xscroll;
    s_current_input->scroll_y = yscroll;
}
