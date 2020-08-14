#include "d_internal.h"
#include "DGNEngine/DGNEngine.h"

#include <MemLeaker/malloc.h>

DgnFramebuffer *dgnFramebufferCreate(DgnTexture **dst_textures, uint8_t *attachment_types, uint8_t num_textures, uint8_t flags)
{
    GLuint buffer = 0;
    glCall(glGenFramebuffers(1, &buffer));
    glCall(glBindFramebuffer(GL_FRAMEBUFFER, buffer));

    if(flags & DGN_FRAMEBUFFER_DEPTH)
    {
        GLuint depthrenderbuffer;
        glCall(glGenRenderbuffers(1, &depthrenderbuffer));
        glCall(glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer));
        glCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, dst_textures[0]->width[0], dst_textures[0]->height[0]));
        glCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer));
    }

    uint8_t c_counter = 0;
    GLenum draw_buffers[32];

    for(int i = 0; i < num_textures; i++)
    {
        uint8_t a_type = attachment_types[i];
        switch(a_type)
        {
        case DGN_FRAMEBUFFER_DEPTH:
            glCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, dst_textures[i]->texture, 0));
            break;
        case DGN_FRAMEBUFFER_COLOR:
            glCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + c_counter, dst_textures[i]->texture, 0));
            draw_buffers[c_counter] = GL_COLOR_ATTACHMENT0 + c_counter;
            c_counter++;
            break;
        default:
            logError("UNKNOWN TYPE", "framebuffer attachment.");
        }
    }

    glCall(glDrawBuffers(c_counter, draw_buffers));

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        return NULL;
    }

    glCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    DgnFramebuffer *res = malloc(sizeof(*res));
    res->buffer = buffer;

    return res;
}

void dgnFramebufferDestroy(DgnFramebuffer *buffer)
{
    glCall(glDeleteFramebuffers(1, &buffer->buffer));

    free(buffer);
}

void dgnFramebufferBind(DgnFramebuffer *buffer)
{
    if(buffer == NULL)
    {
        glCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }
    else
    {
        glCall(glBindFramebuffer(GL_FRAMEBUFFER, buffer->buffer));
    }
}
