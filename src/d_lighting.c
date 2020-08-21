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

Mat4x4 dgnLightingCreateLightProjMat(DgnCamera cam, DgnShadowMap shadow, DgnFrustum frustum, float near_pull)
{
    float ratio = frustum.width / frustum.height;
    float tanHalfHFOV = tanf(frustum.fov * ratio / 2.0f);
    float tanHalfVFOV = tanf(frustum.fov / 2.0f);

    float proj_near_z = -frustum.near;
    float proj_far_z = -frustum.far;
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

    //Vec3 frustum_corners_W[8];
    Vec3 frustum_corners_L[8];
    Mat4x4 inv_view = dgnCameraGetInverseView(cam);

    for (uint8_t j = 0 ; j < 8 ; j++)
    {

        // Transform the frustum coordinate from view to world space
        Vec4 vW = m3dMat4x4MulVec4(inv_view, frustum_corners[j]);

        // Transform the frustum coordinate from world to light space
        Vec4 vL = m3dMat4x4MulVec4(shadow.view_mat, vW);
        frustum_corners_L[j] = (Vec3){vL.x, vL.y, vL.z};
        //frustum_corners_W[j] = (Vec3){vW.x, vW.y, vW.z};
    }

    /** ---- Fix Shadow Shimmering ---- **/

    /** -- Rotation shimmering -- **/

    DgnBoundingSphere sphere = dgnCollisionGenerateSphere(frustum_corners_L, 8);
    sphere.radius /= 1.414314f;
    DgnBoundingBox ortho_box;

    /** -- Position shimmering -- **/
    float rx2 = sphere.radius * 2.0f;
    Vec3 texel_world_size = {rx2 / dgnTextureGetWidth(shadow.texture),
                             rx2 / dgnTextureGetHeight(shadow.texture), 1.0f};

    sphere.center = m3dVec3DivVec3(sphere.center, texel_world_size);
    sphere.center.x = floor(sphere.center.x);
    sphere.center.y = floor(sphere.center.y);
    sphere.center = m3dVec3MulVec3(sphere.center, texel_world_size);

    ortho_box.max = m3dVec3AddVec3(sphere.center, (Vec3){sphere.radius, sphere.radius, sphere.radius});
    ortho_box.min = m3dVec3SubVec3(sphere.center, (Vec3){sphere.radius, sphere.radius, sphere.radius});

    return m3dMat4x4InitOrtho(ortho_box.max.x, ortho_box.min.x, ortho_box.max.y, ortho_box.min.y, ortho_box.min.z - near_pull, ortho_box.max.z);
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
