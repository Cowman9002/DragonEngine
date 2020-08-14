#include "d_internal.h"
#include "DgnEngine/DgnEngine.h"

Mat4x4 dgnCameraGetProjection(DgnCamera cam)
{
    return m3dMat4x4InitPerspective(cam.width, cam.height, cam.fov, cam.near_plane, cam.far_plane);
}

Mat4x4 dgnCameraGetView(DgnCamera cam)
{
    Mat4x4 pos = m3dMat4x4InitIdentity();
    Mat4x4 rot = m3dMat4x4InitIdentity();

    m3dMat4x4Translate(&pos, m3dVec3MulValue(cam.pos, -1));
    m3dMat4x4Rotate(&rot, m3dQuatConjugate(cam.rot));

    return m3dMat4x4MulMat4x4(rot, pos);
}

Mat4x4 dgnCameraGetInverseView(DgnCamera cam)
{
    Mat4x4 pos = m3dMat4x4InitIdentity();
    Mat4x4 rot = m3dMat4x4InitIdentity();

    m3dMat4x4Translate(&pos, cam.pos);
    m3dMat4x4Rotate(&rot, cam.rot);

    return m3dMat4x4MulMat4x4(pos, rot);
}
