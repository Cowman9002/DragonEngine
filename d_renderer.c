#include "d_internal.h"
#include "DGNEngine/DGNEngine.h"

#include "d_defines.h"

#include <MemLeaker/malloc.h>

static unsigned int s_clear_flags;
static uint8_t s_render_mode = DGN_DRAW_MODE_TRIANGLES;
static uint32_t s_size_bound_mesh = 0;

static DgnMesh *s_skybox_mesh = 0;
static DgnShader *s_skybox_shader = 0;
static DgnMesh *s_screen_mesh = 0;

uint8_t genSkyboxMeshInternal();
uint8_t genScreenMeshInternal();

uint8_t dgnRendererInitialize()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        return DGN_FALSE;
    }

    ASSERT_RETURN(genSkyboxMeshInternal());
    ASSERT_RETURN(genScreenMeshInternal());

    return DGN_TRUE;
}

void dgnRendererTerminate()
{
    dgnMeshDestroy(s_skybox_mesh);
    dgnShaderDestroy(s_skybox_shader);
}

void dgnRendererClear()
{
    glCall(glClear(s_clear_flags));
}

void dgnRendererBindMesh(DgnMesh* mesh)
{
    if(mesh == NULL)
    {
        glCall(glBindVertexArray(0));
        glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        s_size_bound_mesh = 0;
    }
    else
    {
        glCall(glBindVertexArray(mesh->VAO));
        glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->IBO));
        s_size_bound_mesh = mesh->length;
    }
}

void dgnRendererBindShader(DgnShader* shader)
{
    if(shader == NULL)
    {
        glCall(glUseProgram(0));
    }
    else
    {
        glCall(glUseProgram(shader->program));
    }
}

void bindTextureInternal(GLenum type, DgnTexture *texture, uint8_t slot)
{
    glCall(glActiveTexture(GL_TEXTURE0 + slot));

    if(texture == NULL)
    {
        glCall(glBindTexture(type, 0));
    }
    else
    {
        glCall(glBindTexture(type, texture->texture));
    }
}

void dgnRendererBindTexture(DgnTexture *texture,  uint8_t slot)
{
    bindTextureInternal(GL_TEXTURE_2D, texture, slot);
}

void dgnRendererBindCubemap(DgnTexture *texture, uint8_t slot)
{
    bindTextureInternal(GL_TEXTURE_CUBE_MAP, texture, slot);
}

void dgnRendererDrawMesh()
{
    glCall(glDrawElements(s_render_mode, s_size_bound_mesh, GL_UNSIGNED_INT, NULL));
}

void dgnRendererDrawSkybox(DgnTexture *texture, DgnShader *shader, int32_t uniform_loc, Mat4x4 view_projection_mat)
{
    dgnRendererBindMesh(s_skybox_mesh);
    dgnRendererDrawMesh();
    dgnRendererBindMesh(0);
}

void dgnRendererDrawScreenTexture()
{
    dgnRendererBindMesh(s_screen_mesh);

    dgnRendererDrawMesh();

    dgnRendererBindMesh(0);
}

void dgnRendererSetDepthTest(uint16_t func)
{
    glDepthFunc(func);
}

void dgnRendererSetClearColor(float red, float green, float blue)
{
    glCall(glClearColor(red, green, blue, 1.0f));
}

void dgnRendererSetVsync(uint8_t sync)
{
    glfwSwapInterval(sync);
}

void dgnRendererSetDrawMode(uint8_t mode)
{
    s_render_mode = mode;
}

void dgnRendererSetViewport(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    glCall(glViewport(x, y, width, height));
}

void dgnRendererSetCullFace(uint8_t face)
{
    glCall(glCullFace(GL_FRONT + face));
}

void dgnRendererSetWinding(uint8_t face)
{
    glCall(glFrontFace(GL_CW + face));
}

void dgnRendererSetAlphaBlend(uint16_t sfactor, uint16_t dfactor)
{
    glBlendFunc(sfactor, dfactor);
}

void dgnRendererEnableClearFlag(unsigned int flag)
{
    s_clear_flags |= flag;
}

void dgnRendererDisableClearFlag(unsigned int flag)
{
    s_clear_flags &= ~flag;
}

void dgnRendererEnableFlag(uint16_t value)
{
    glCall(glEnable(value));
}

void dgnRendererDisableFlag(uint16_t value)
{
    glCall(glDisable(value));
}

void dgnRendererSetupShadow(DgnShadowMap shadow, DgnShader *shader, int32_t uniform_loc, Mat4x4 light_view_mat)
{
    dgnFramebufferBind(shadow.framebuffer);
    dgnRendererClear();
    dgnRendererSetViewport(0, 0, shadow.texture->width[0], shadow.texture->height[0]);
    dgnRendererSetCullFace(DGN_FACE_FRONT);
    dgnRendererSetDepthTest(DGN_DEPTH_PASS_LESS);

    dgnRendererBindShader(shader);

    dgnShaderUniformM4x4(uniform_loc, light_view_mat);
}

/** -------------------------------------------------*/

uint8_t genSkyboxMeshInternal()
{
    float skybox_vertices[] =
    {
        -1.000000, -1.000000,  1.000000,
        -1.000000,  1.000000,  1.000000,
        -1.000000, -1.000000, -1.000000,
        -1.000000,  1.000000, -1.000000,
         1.000000, -1.000000,  1.000000,
         1.000000,  1.000000,  1.000000,
         1.000000, -1.000000, -1.000000,
         1.000000,  1.000000, -1.000000
    };

    unsigned skybox_indices[] =
    {
       2, 1, 0,
       6, 3, 2,
       4, 7, 6,
       0, 5, 4,
       0, 6, 2,
       5, 3, 7,
       2, 3, 1,
       6, 7, 3,
       4, 5, 7,
       0, 1, 5,
       0, 4, 6,
       5, 1, 3
    };

    s_skybox_mesh = dgnMeshCreate(skybox_vertices, sizeof(skybox_vertices),
                                  skybox_indices, sizeof(skybox_indices),
                                  DGN_VERT_ATTRIB_POSITION);

    return s_skybox_mesh != NULL;
}

uint8_t genScreenMeshInternal()
{
    float quad_verts[] =
    {
         0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
         1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, 0.0f, 0.0f, 1.0f, 0.0f
    };

    unsigned quad_indices[] =
    {
        0, 2, 1,
        0, 3, 2
    };

    s_screen_mesh = dgnMeshCreate(quad_verts, sizeof(quad_verts), quad_indices, sizeof(quad_indices),
                                       DGN_VERT_ATTRIB_POSITION | DGN_VERT_ATTRIB_TEXCOORD);

    return s_screen_mesh != NULL;
}
