#include "d_internal.h"
#include "DgnEngine/DgnEngine.h"

#include <float.h>
#include <math.h>

DgnAABB dgnCollisionGenerateAABB(Vec3 *points, size_t points_count)
{
    DgnAABB res;

    if(points_count == 0)
    {
        res.max = (Vec3){0.0f, 0.0f, 0.0f};
        res.min = (Vec3){0.0f, 0.0f, 0.0f};
        return res;
    }
    else if(points_count == 1)
    {
        res.max = points[0];
        res.min = points[0];
        return res;
    }

    res.max = (Vec3){-FLT_MAX, -FLT_MAX, -FLT_MAX};
    res.min = (Vec3){FLT_MAX, FLT_MAX, FLT_MAX};

    for(int i = 0; i < points_count; i++)
    {
        Vec3 current = points[i];

        res.max.x = fmaxf(res.max.x, current.x);
        res.min.x = fminf(res.min.x, current.x);
        res.max.y = fmaxf(res.max.y, current.y);
        res.min.y = fminf(res.min.y, current.y);
        res.max.z = fmaxf(res.max.z, current.z);
        res.min.z = fminf(res.min.z, current.z);
    }

    return res;
}

Vec3 dgnCollisionAABBGetCenter(DgnAABB aabb)
{
    return m3dVec3DivValue(m3dVec3AddVec3(aabb.max, aabb.min), 2.0f);
}

DgnCollisionData dgnCollisionAABBPoint(DgnAABB aabb, Vec3 point)
{
    DgnCollisionData res;
    res.hit = DGN_FALSE;

    if(point.x > aabb.min.x && point.x < aabb.max.x &&
       point.y > aabb.min.y && point.y < aabb.max.y &&
       point.z > aabb.min.z && point.z < aabb.max.z)
       {
           res.hit = DGN_TRUE;
       }

    return res;
}
