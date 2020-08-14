#include "d_internal.h"
#include "DgnEngine/DgnEngine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assimp/cimport.h> // Plain-C interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags

#include <MemLeaker/malloc.h>

DgnMesh* dgnMeshCreate(float vertex_data[],
    size_t vertex_data_size,
    uint32_t index_data[],
    size_t index_data_size,
    uint16_t mesh_type)
{
    uint32_t vao, vbo, ibo;

    glCall(glGenVertexArrays(1, &vao));
    glCall(glGenBuffers(1, &vbo));
    glCall(glGenBuffers(1, &ibo));

    glCall(glBindVertexArray(vao));

    // -------- Index Data
    glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
    glCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_data_size, index_data, GL_STATIC_DRAW));
    glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

    // -------- Vertex Data
    glCall(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    glCall(glBufferData(GL_ARRAY_BUFFER, vertex_data_size, vertex_data, GL_STATIC_DRAW));

    uint32_t index = 0;
    uint32_t stride = 0;
    uint64_t pointer = 0;
    uint16_t bitMask = 1;

    // Find the stride
    for(int i = 0; i < NUM_VERT_ATTRIB_INTERNAL; i++)
    {
        uint8_t i_valid = mesh_type & (1 << i);

        if(i_valid)
        {
            if(i == 1)// texcoord
            {
                stride += 2;
            }
            else
            {
                stride += 3;
            }
        }
    }

    stride *= sizeof(float);

    for(int i = 0; i < NUM_VERT_ATTRIB_INTERNAL; i++)
    {
        if(mesh_type & bitMask)
        {
            uint8_t size = 0;

            if(i == 1)// texcoord
            {
                size = 2;
            }
            else
            {
                size = 3;
            }

            // set attrib pointers using solved data
            glCall(glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, (void*)pointer));
            glCall(glEnableVertexAttribArray(index));
            // offset of next attribute start
            pointer += size * sizeof(float);
        }
        // vertex attribute locations are absolute
        index++;
        // multiplication of two starting at one is the same as setting the next bit only
        bitMask *= 2;
    }

    glCall(glBindBuffer(GL_ARRAY_BUFFER, 0));

    glCall(glBindVertexArray(0));

    DgnMesh *res = malloc(sizeof(*res));

    res->VAO = vao;
    res->VBO = vbo;
    res->IBO = ibo;
    res->length = index_data_size / sizeof(*index_data);

    return res;
}

DgnMesh *aiMeshConvert(struct aiMesh* mesh)
{
    uint8_t single_vertex_size = 0;
    uint16_t mesh_type = 0;

    if(mesh->mVertices)
    {
        single_vertex_size += 3;
        mesh_type |= DGN_VERT_ATTRIB_POSITION;
    }

    if(mesh->mTextureCoords[0])
    {
        single_vertex_size += 2;
        mesh_type |= DGN_VERT_ATTRIB_TEXCOORD;
    }

    if(mesh->mNormals)
    {
        single_vertex_size += 3;
        mesh_type |= DGN_VERT_ATTRIB_NORMAL;
    }

    if(mesh->mTangents)
    {
        single_vertex_size += 3;
        mesh_type |= DGN_VERT_ATTRIB_TANGENT;
    }

    size_t size_vertices = mesh->mNumVertices * single_vertex_size * sizeof(float);
    size_t size_indices = mesh->mNumFaces * 3 * sizeof(uint32_t);

    float *vertices = malloc(size_vertices);
    uint32_t *indices = malloc(size_indices);

    for(uint32_t v = 0, k = 0; v < mesh->mNumVertices; v++)
    {
        if(mesh_type & DGN_VERT_ATTRIB_POSITION)
        {
            vertices[k++] = mesh->mVertices[v].x;
            vertices[k++] = mesh->mVertices[v].y;
            vertices[k++] = mesh->mVertices[v].z;
        }

        if(mesh_type & DGN_VERT_ATTRIB_TEXCOORD)
        {
            vertices[k++] = mesh->mTextureCoords[0][v].x;
            vertices[k++] = mesh->mTextureCoords[0][v].y;
        }

        if(mesh_type & DGN_VERT_ATTRIB_NORMAL)
        {
            vertices[k++] = mesh->mNormals[v].x;
            vertices[k++] = mesh->mNormals[v].y;
            vertices[k++] = mesh->mNormals[v].z;
        }

        if(mesh_type & DGN_VERT_ATTRIB_TANGENT)
        {
            vertices[k++] = mesh->mTangents[v].x;
            vertices[k++] = mesh->mTangents[v].y;
            vertices[k++] = mesh->mTangents[v].z;
        }
    }

    for(uint32_t f = 0, k = 0; f < mesh->mNumFaces; f++)
    {
        struct aiFace face = mesh->mFaces[f];
        for(int i = 0; i < face.mNumIndices; i++)
        {
            indices[k++] = face.mIndices[i];
        }
    }

    DgnMesh *res = dgnMeshCreate(vertices, size_vertices, indices, size_indices, mesh_type);

    free(vertices);
    free(indices);

    return res;
}

DgnMesh **dgnMeshLoad(const char *filepath, uint16_t *out_num_meshes)
{
    const struct aiScene* scene = aiImportFile( filepath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    // If the import failed, report it
    if(!scene)
    {
        logError("MESH LOADING", aiGetErrorString());
        return NULL;
    }

    int mesh_num = 0;
    // only return the first mesh
    if(out_num_meshes == NULL)
    {
        mesh_num = 1;
    }
    else
    {
        mesh_num = scene->mNumMeshes;
        *out_num_meshes = mesh_num;
    }

    DgnMesh **res = malloc(sizeof(*res) * mesh_num);

    // Now we can access the file's contents
    for(int i = 0; i < mesh_num; i++)
    {
        res[i] = aiMeshConvert(scene->mMeshes[i]);
    }

    // We're done. Release all resources associated with this import
    aiReleaseImport( scene);
    return res;
}

void dgnMeshDestroy(DgnMesh *mesh)
{
    glCall(glBindVertexArray(0));
    glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

    glCall(glDeleteBuffers(1, &mesh->VBO));
    glCall(glDeleteBuffers(1, &mesh->IBO));
    glCall(glDeleteVertexArrays(1, &mesh->VAO));

    free(mesh);
}

void dgnMeshDestroyArr(DgnMesh **meshes, uint16_t num_meshes)
{
    for(int i = 0; i < num_meshes; i++)
    {
        dgnMeshDestroy(meshes[i]);
    }

    free(meshes);
}
