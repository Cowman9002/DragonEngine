#include "d_internal.h"
#include "DgnEngine/DgnEngine.h"

#include <MemLeaker/malloc.h>
#include <string.h>
#include <stdio.h>

#include "c_ordered_map.h"

static OrderedMapS *s_econst_map;

static uint32_t genShaderInternal(char *data, uint16_t shader_type)
{
    if(data == NULL) return 0;

    // go through each line
    // check if first token is "econt" then:
    // realloc data to required length
    // change line to "const 'value type' 'variable name' = 'value from s_econst_map';

    // tokenize line for special parsing
    char *token = strtok(data, " \n");

    while( token != NULL )
    {
        //use token
        printf("%s\n", token);
        token = strtok(NULL, " \n");
    }

    glCall(uint32_t shader = glCreateShader(shader_type));
    glCall(glShaderSource(shader, 1, &data, NULL));
    glCall(glCompileShader(shader));

    #ifdef __DEBUG
    int32_t success;
    char buff[256];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(shader, 256, NULL, buff);
        logError("SHADER COMPILE STATUS", buff);
    }
    #endif // __DEBUG

    return shader;
}

DgnShader *dgnShaderCreate(char *vertex_code, char *geometry_code, char *fragment_code)
{
    DgnShader *res = malloc(sizeof(*res));

    if(res == NULL)
    {
        return NULL;
    }

    glCall(uint32_t program = glCreateProgram());

    uint32_t vertex   = genShaderInternal(vertex_code, GL_VERTEX_SHADER);
    uint32_t geometry = genShaderInternal(geometry_code, GL_GEOMETRY_SHADER);
    uint32_t fragment = genShaderInternal(fragment_code, GL_FRAGMENT_SHADER);

    if(vertex != 0)
    {
        glCall(glAttachShader(program, vertex));
    }

    if(geometry != 0)
    {
        glCall(glAttachShader(program, geometry));
    }

    if(fragment != 0)
    {
        glCall(glAttachShader(program, fragment));
    }

    glCall(glLinkProgram(program));

    #ifdef __DEBUG
    int32_t success;
    char buff[256];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(program, 256, NULL, buff);
        logError("SHADER LINKING STATUS", buff);
    }
    #endif // __DEBUG

    glCall(glDeleteShader(vertex));
    glCall(glDeleteShader(geometry));
    glCall(glDeleteShader(fragment));

    res->program = program;

    return res;
}

#define FILE_LOAD_ERROR 0xFFFFFFFF
#define FILE_LOAD_NULL 0xFFFFFFFE

uint32_t fileToString(char **out_str, const char *filepath)
{
    if(filepath == NULL) return FILE_LOAD_NULL;

    FILE *file = fopen(filepath, "r");

    if(!file)
    {
        logError("FILE LOADING", filepath);
        return FILE_LOAD_ERROR;
    }
    fseek(file, 0, SEEK_END);   // go to end
    int64_t length = ftell(file);// get cursor position, which is now at the end
    fseek(file, 0, SEEK_SET);   // return to beginning

    if(length < 0)
    {
        fclose(file);
        return FILE_LOAD_ERROR;
    }

    *out_str = malloc(length);

    char line[128];
    size_t offset = 0;

     while(fgets(line, 128, file) != NULL)
    {
        uint16_t line_length = strlen(line);
        memcpy((*out_str) + offset, line, line_length + 1);
        offset += line_length;
    }

    fclose(file);

    return length;
}

DgnShader *dgnShaderLoad(const char* vertex_path, const char* geometry_path, const char* fragment_path)
{
    // -------- Load the files

    char *v_code = NULL;
    char *g_code = NULL;
    char *f_code = NULL;

    if(fileToString(&v_code, vertex_path) == FILE_LOAD_ERROR)
    {
        return NULL;
    }
    if(fileToString(&g_code, geometry_path) == FILE_LOAD_ERROR)
    {
        return NULL;
    }
    if(fileToString(&f_code, fragment_path) == FILE_LOAD_ERROR)
    {
        return NULL;
    }

    DgnShader* res = dgnShaderCreate(v_code, g_code, f_code);

    free(v_code);
    free(g_code);
    free(f_code);

    return res;
}

void dgnShaderDestroy(DgnShader *shader)
{
    if(shader == NULL) return;

    glCall(glDeleteProgram(shader->program));

    free(shader);
}

int32_t dgnShaderGetUniformLoc(DgnShader *shader, const char *name)
{
    if(shader == NULL) return -1;

    glCall(int32_t location = glGetUniformLocation(shader->program, name));

    if(location == -1)
    {
        logError("UNIFORM LOCATION", name);
    }

    return location;
}

void dgnShaderUniformF(int32_t loc, float value)
{
    glUniform1f(loc, value);
}

void dgnShaderUniformI(int32_t loc, int value)
{
    glUniform1i(loc, value);
}

void dgnShaderUniformB(int32_t loc, uint8_t value)
{
    glUniform1i(loc, value);
}

void dgnShaderUniformV2(int32_t loc, Vec2 value)
{
    glUniform2f(loc, value.x, value.y);
}

void dgnShaderUniformV3(int32_t loc, Vec3 value)
{
    glUniform3f(loc, value.x, value.y, value.z);
}

void dgnShaderUniformM3x3(int32_t loc, Mat3x3 value)
{
    glUniformMatrix3fv(loc, 1, GL_TRUE, value.m[0]);
}

void dgnShaderUniformM4x4(int32_t loc, Mat4x4 value)
{
    glUniformMatrix4fv(loc, 1, GL_TRUE, value.m[0]);
}

//TODO Implement shader Econt values
void dgnShaderSetEconstI(const char *name, int value)
{
    orderedMapSInsertOrReplace(s_econst_map, name, &value, sizeof(value));
}

uint8_t dgnShaderInit_internal()
{
    s_econst_map = orderedMapSCreate();

    return s_econst_map != NULL;
}

void dgnShaderTerm_internal()
{
    orderedMapSDestroy(s_econst_map);
}
