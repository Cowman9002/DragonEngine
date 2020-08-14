#include "d_internal.h"
#include "DgnEngine/DgnEngine.h"

#include "lodepng.h"

#include <MemLeaker/malloc.h>
#include <string.h>
#include <stdio.h>

static void setWrapInternal(GLenum image_type, uint8_t wrap_mode)
{
    switch (wrap_mode)
    {
    case DGN_TEX_WRAP_CLAMP_TO_BOARDER:
        glCall(glTexParameterf(image_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
        glCall(glTexParameterf(image_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
        break;
    case DGN_TEX_WRAP_CLAMP_TO_EDGE:
        glCall(glTexParameterf(image_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        glCall(glTexParameterf(image_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        break;
    case DGN_TEX_WRAP_MIRROR:
        glCall(glTexParameterf(image_type, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
        glCall(glTexParameterf(image_type, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));
        break;
    case DGN_TEX_WRAP_REPEAT:
    default:
        glCall(glTexParameterf(image_type, GL_TEXTURE_WRAP_S, GL_REPEAT));
        glCall(glTexParameterf(image_type, GL_TEXTURE_WRAP_T, GL_REPEAT));
        break;
    }
}

static void setFilterInternal(GLenum image_type, uint8_t filter_mode, uint8_t mipmapped)
{
    switch (filter_mode)
    {
    case DGN_TEX_FILTER_NEAREST:
        if(mipmapped)
        {
            glCall(glTexParameterf(image_type, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST));
        }
        else
        {
            glCall(glTexParameterf(image_type, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        }
        glCall(glTexParameterf(image_type, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        break;
    case DGN_TEX_FILTER_BILINEAR:
        if(mipmapped)
        {
            glCall(glTexParameterf(image_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST));
        }
        else
        {
            glCall(glTexParameterf(image_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        }
        glCall(glTexParameterf(image_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        break;
    case DGN_TEX_FILTER_TRILINEAR:
    default:
        if(mipmapped)
        {
            glCall(glTexParameterf(image_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
        }
        else
        {
            glCall(glTexParameterf(image_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        }
        glCall(glTexParameterf(image_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        break;
    }
}

DgnTexture *dgnTextureCreate(
    uint8_t *data,
    uint32_t width,
    uint32_t height,
    uint8_t wrapping,
    uint8_t filtering,
    uint8_t mipmapped,
    uint16_t storage_type,
    uint16_t internal_type,
    uint16_t data_type)
{
    glCall(glActiveTexture(GL_TEXTURE0));

    uint32_t tex;
    glCall(glGenTextures(1, &tex));

    glCall(glBindTexture(GL_TEXTURE_2D, tex));

    setWrapInternal(GL_TEXTURE_2D, wrapping);
    setFilterInternal(GL_TEXTURE_2D, filtering, mipmapped);

    glCall(glTexImage2D(GL_TEXTURE_2D, 0, internal_type, width, height, 0, storage_type, data_type, data));

    if(mipmapped)
    {
        glCall(glGenerateMipmap(GL_TEXTURE_2D));
    }

    glCall(glBindTexture(GL_TEXTURE_2D, 0));

    DgnTexture *res = malloc(sizeof(*res));

    res->texture = tex;
    res->mipmapped = mipmapped;
    res->width = malloc(sizeof(*res->width));
    res->height = malloc(sizeof(*res->height));

    res->width[0] = width;
    res->height[0] = height;

    return res;
}


DgnTexture *dgnCubemapCreate(
    uint8_t *data[6],
    uint32_t *width,
    uint32_t *height,
    uint8_t wrapping,
    uint8_t filtering,
    uint16_t storage_type)
{
    glCall(glActiveTexture(GL_TEXTURE0));

    uint32_t tex;
    glCall(glGenTextures(1, &tex));

    glCall(glBindTexture(GL_TEXTURE_CUBE_MAP, tex));

    setWrapInternal(GL_TEXTURE_CUBE_MAP, wrapping);
    setFilterInternal(GL_TEXTURE_CUBE_MAP, filtering, DGN_TRUE);

    for(int i = 0; i < 6; i++)
    {
        glCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, storage_type, width[i], height[i], 0, GL_RGBA, GL_UNSIGNED_BYTE, data[i]));
    }

    glCall(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));

    glCall(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));

    DgnTexture *res = malloc(sizeof(*res));

    res->texture = tex;
    res->mipmapped = DGN_TRUE;
    res->width = malloc(sizeof(*res->width) * 6);
    res->height = malloc(sizeof(*res->height) * 6);

    memcpy(res->width, width, 6 * sizeof(*width));
    memcpy(res->height, width, 6 * sizeof(*height));

    return res;
}

DgnTexture *dgnTextureLoad(const char *filepath, uint8_t wrapping, uint8_t filtering, uint8_t mipmapped, uint16_t storage_type)
{
    uint8_t *pixels;
    uint32_t width, height;

    unsigned error = lodepng_decode32_file(&pixels, &width, &height, filepath);
    if(error )
    {
        char str[256];
        sprintf_s(str, 256, "%s\n\tFile: %s", lodepng_error_text(error), filepath);
        logError("PNG LOADING", str);
        return NULL;
    }

    return dgnTextureCreate(pixels, width, height, wrapping, filtering, mipmapped, DGN_TEX_STORAGE_RGBA, storage_type, DGN_DATA_TYPE_UBYTE);
}

DgnTexture *dgnCubemapLoad(const char *filepath[6], uint8_t wrapping, uint8_t filtering, uint16_t storage_type)
{
    uint8_t *pixels[6];
    uint32_t width[6], height[6];

    for(int i = 0; i < 6; i++)
    {
        unsigned error = lodepng_decode32_file(&pixels[i], &width[i], &height[i], filepath[i]);
        if(error)
        {
            char str[256];
            sprintf_s(str, 256, "%s\n\tFile: %s", lodepng_error_text(error), filepath[i]);
            logError("PNG LOADING", str);
            return NULL;
        }
    }

    return dgnCubemapCreate(pixels, width, height, wrapping, filtering, storage_type);
}

void dgnTextureDestroy(DgnTexture *texture)
{
    glCall(glDeleteTextures(1, &texture->texture));

    free(texture->width);
    free(texture->height);
    free(texture);
}

void dgnTextureSetWrap(DgnTexture *texture, uint8_t wrap_mode)
{
    glCall(glBindTexture(GL_TEXTURE_2D, texture->texture));
    setWrapInternal(GL_TEXTURE_2D, wrap_mode);
    glCall(glBindTexture(GL_TEXTURE_2D, 0));
}

void dgnTextureSetFilter(DgnTexture *texture, uint8_t filter_mode)
{
    glCall(glBindTexture(GL_TEXTURE_2D, texture->texture));
    setFilterInternal(GL_TEXTURE_2D, filter_mode, texture->mipmapped);
    glCall(glBindTexture(GL_TEXTURE_2D, 0));
}

void dgnTextureSetBorderColor(DgnTexture *texture, float r, float g, float b, float a)
{
    float color[] = {r, g, b, a};

    glCall(glBindTexture(GL_TEXTURE_2D, texture->texture));
    glCall(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color));
    glCall(glBindTexture(GL_TEXTURE_2D, 0));
}
