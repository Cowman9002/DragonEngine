#include "d_internal.h"
#include "DgnEngine/DgnEngine.h"

#include <MemLeaker/malloc.h>

#include <float.h>
#include <math.h>

Mat4x4 dgnLightingCreateDirViewMat(Vec3 dir)
{
    Mat4x4 light_rot = m3dMat4x4InitIdentity();

    Quat light_quat = m3dQuatFace(dir, (Vec3){0.0f, 1.0f, 0.0f});

    m3dMat4x4Rotate(&light_rot, m3dQuatConjugate(light_quat));

    return light_rot;
}

Mat4x4 dgnLightingCreateLightProjMat(Mat4x4 inv_view, Mat4x4 light_mat,
        float width, float height, float fov, float near, float far, float near_pull)
{
    float ratio = width / height;
    float tanHalfHFOV = tanf(fov * ratio / 2.0f);
    float tanHalfVFOV = tanf(fov / 2.0f);

    float proj_near_z = -near;
    float proj_far_z = -far;
    float proj_near_y = proj_near_z * tanHalfVFOV;
    float proj_far_y = proj_far_z   * tanHalfVFOV;
    float proj_near_x = proj_near_z * tanHalfHFOV;
    float proj_far_x = proj_far_z   * tanHalfHFOV;

    // local positions
    Vec4 frustum_corners[] =
    {
        // near
        {proj_near_x,  -proj_near_y, proj_near_z, 1.0f},
        {proj_near_x,   proj_near_y, proj_near_z, 1.0f},
        {-proj_near_x, -proj_near_y, proj_near_z, 1.0f},
        {-proj_near_x,  proj_near_y, proj_near_z, 1.0f},

        // far plane
        {proj_far_x,  -proj_far_y, proj_far_z, 1.0f},
        {proj_far_x,   proj_far_y, proj_far_z, 1.0f},
        {-proj_far_x, -proj_far_y, proj_far_z, 1.0f},
        {-proj_far_x,  proj_far_y, proj_far_z, 1.0f}
    };

    // transform to world space

    Vec4 frustum_corners_L[8];

    float minX = FLT_MAX;
    float maxX = -FLT_MAX;
    float minY = FLT_MAX;
    float maxY = -FLT_MAX;
    float minZ = FLT_MAX;
    float maxZ = -FLT_MAX;

    for (uint8_t j = 0 ; j < 8 ; j++) {

        // Transform the frustum coordinate from view to world space
        Vec4 vW = m3dMat4x4MulVec4(inv_view, frustum_corners[j]);

        // Transform the frustum coordinate from world to light space
        frustum_corners_L[j] = m3dMat4x4MulVec4(light_mat, vW);

        minX = fminf(minX, frustum_corners_L[j].x);
        maxX = fmaxf(maxX, frustum_corners_L[j].x);
        minY = fminf(minY, frustum_corners_L[j].y);
        maxY = fmaxf(maxY, frustum_corners_L[j].y);
        minZ = fminf(minZ, frustum_corners_L[j].z);
        maxZ = fmaxf(maxZ, frustum_corners_L[j].z);
    }

    return m3dMat4x4InitOrtho(maxX, minX, maxY, minY, minZ - near_pull, maxZ);
}

Mat4x4 dgnLightingCreateLightSpaceMat(DgnShadowMap shadow)
{
    return m3dMat4x4MulMat4x4(shadow.proj_mat, shadow.view_mat);
}

DgnTexture *dgnLightingCreateShadowMap(uint16_t width, uint16_t height, uint8_t light_type)
{
    DgnTexture *res = NULL;

    switch(light_type)
    {
    case DGN_LIGHT_TYPE_DIR:
        res = dgnTextureCreate(NULL, width, height,
                                    DGN_TEX_WRAP_CLAMP_TO_BOARDER, DGN_TEX_FILTER_NEAREST, DGN_FALSE,
                                    DGN_TEX_STORAGE_DEPTH, DGN_TEX_STORAGE_DEPTH,
                                    DGN_DATA_TYPE_FLOAT);
        break;
    default:
        logError("UNDEFINED VALUE", "Light type value not recognized");
    }

    dgnTextureSetBorderColor(res, 1.0f, 1.0f, 1.0f, 1.0f);

    return res;
}
