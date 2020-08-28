#include "d_internal.h"
#include "DgnEngine/DgnEngine.h"

#include <float.h>
#include <math.h>

DgnBoundingBox dgnCollisionGenerateBox(Vec3 *points, size_t points_count)
{
    DgnBoundingBox res;

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

DgnBoundingSphere dgnCollisionGenerateSphere(Vec3 *points, size_t points_count)
{
    DgnBoundingSphere res;

    Vec3 points_summed = {0.0f, 0.0f, 0.0f};
    float d = -FLT_MAX;

    for(int i = 0; i < points_count; i++)
    {
        points_summed = m3dVec3AddVec3(points_summed, points[i]);
        for(int j = 0; j < i; j++)
        {
            float dist = m3dVec3Distance(points[i], points[j]);
            d = fmaxf(dist, d);
        }
    }

    res.radius = d / 2.0f;
    res.center = m3dVec3DivValue(points_summed, points_count);

    return res;
}

DgnPlane dgnCollisionGeneratePlane(Vec3 p1, Vec3 p2, Vec3 p3)
{
    DgnPlane res;
    res.normal = m3dVec3Normalized(m3dVec3Cross(m3dVec3SubVec3(p2, p1), m3dVec3SubVec3(p3, p1)));
    res.distance = m3dVec3Dot(res.normal, p1);
    return res;
}

DgnBoundingSphere dgnCollisionSphereFromBox(DgnBoundingBox box)
{
    DgnBoundingSphere res;
    res.center = dgnCollisionBoxGetCenter(box);
    res.radius = m3dVec3Length(m3dVec3SubVec3(box.max, res.center));

    return res;
}

DgnCollisionData dgnCollisionBoxPoint(DgnBoundingBox box, Vec3 point)
{
    DgnCollisionData res;
    res.hit = DGN_FALSE;

    if(point.x > box.min.x && point.x < box.max.x &&
       point.y > box.min.y && point.y < box.max.y &&
       point.z > box.min.z && point.z < box.max.z)
       {
           res.hit = DGN_TRUE;
       }

    return res;
}

DgnCollisionData dgnCollisionBoxBox(DgnBoundingBox a, DgnBoundingBox b)
{
    DgnCollisionData res;
    res.hit = DGN_FALSE;

    if(a.min.x <= b.max.x && a.max.x >= b.min.x &&
       a.min.y <= b.max.y && a.max.y >= b.min.y &&
       a.min.z <= b.max.z && a.max.z >= b.min.z)
       {
           res.hit = DGN_TRUE;
       }

    return res;
}
DgnCollisionData dgnCollisionBoxSphere(DgnBoundingBox box, DgnBoundingSphere sphere)
{
    DgnCollisionData res;
    res.hit = DGN_FALSE;

    // get box closest point to sphere center by clamping
    Vec3 nearest_point;
    nearest_point.x = fmaxf(box.min.x, fminf(sphere.center.x, box.max.x));
    nearest_point.y = fmaxf(box.min.y, fminf(sphere.center.y, box.max.y));
    nearest_point.z = fmaxf(box.min.z, fminf(sphere.center.z, box.max.z));

    // this is the same as isPointInsideSphere
    float dist = m3dVec3Distance(sphere.center, nearest_point);

    res.hit = dist <= sphere.radius;

    return res;
}

DgnCollisionData dgnCollisionSphereSphere(DgnBoundingSphere a, DgnBoundingSphere b)
{
    DgnCollisionData res;
    res.hit = DGN_FALSE;

    float dist = m3dVec3Distance(a.center, b.center);

    res.hit = dist <= (a.radius + b.radius);

    return res;
}

DgnCollisionData dgnCollisionSpherePoint(DgnBoundingSphere sphere, Vec3 point)
{
    DgnCollisionData res;
    res.hit = DGN_FALSE;

    float dist = m3dVec3Distance(sphere.center, point);

    res.hit = dist <= sphere.radius;

    return res;
}

DgnCollisionData dgnCollisionPlanePoint(DgnPlane plane, Vec3 point)
{
    DgnCollisionData res;
    res.hit = DGN_FALSE;

    float distance = dgnCollisionDistFromPlane(point, plane);

    res.hit = abs(distance) < 0.001;

    return res;
}

DgnCollisionData dgnCollisionLinePoint(DgnLine line, Vec3 point)
{
    DgnCollisionData res;
    res.hit = DGN_FALSE;

    float m = (line.p2.y - line.p1.y) / (line.p2.x - line.p1.x);
    float b = line.p1.y - m * line.p1.x;

    float pass = point.y - (m * point.x + b);

    if (abs(pass) < 0.001f)
    {
        res.hit = DGN_TRUE;
    }

    return res;
}

//https://gdbooks.gitbooks.io/3dcollisions/content/Chapter4/point_in_triangle.html
DgnCollisionData dgnCollisionTrianglePoint(DgnTriangle tri, Vec3 point)
{
    DgnCollisionData res;
    res.hit = DGN_FALSE;

    // move triangle so point is triangle's origin
    tri.p1 = m3dVec3SubVec3(tri.p1, point);
    tri.p2 = m3dVec3SubVec3(tri.p2, point);
    tri.p3 = m3dVec3SubVec3(tri.p3, point);

    // u = normal of PBC
    // v = normal of PCA
    // w = normal of PAB

    Vec3 u = m3dVec3Cross(tri.p2, tri.p3);
    Vec3 v = m3dVec3Cross(tri.p3, tri.p1);
    Vec3 w = m3dVec3Cross(tri.p1, tri.p2);

    // Test to see if the normals are facing
    // the same direction
    if (m3dVec3Dot(u, v) < 0.0f)
    {
        res.hit = DGN_FALSE;
    }
    else if (m3dVec3Dot(u, w) < 0.0f)
    {
        res.hit = DGN_FALSE;
    }
    else
    {
        res.hit = DGN_TRUE;
    }

    return res;
}

DgnCollisionData dgnCollisionTriangleSphere(DgnTriangle tri, DgnBoundingSphere sphere)
{
    DgnCollisionData res;
    res.hit = DGN_FALSE;

    Vec3 nearest_point = dgnCollisionNearestPointTriangle(sphere.center, tri);

    float dist = m3dVec3Distance(sphere.center, nearest_point);

    res.hit = dist <= sphere.radius;

    return res;
}

float dgnCollisionDistFromPlane(Vec3 point, DgnPlane plane)
{
    return m3dVec3Dot(point, plane.normal) - plane.distance;
}

Vec3 dgnCollisionNearestPointPlane(Vec3 point, DgnPlane plane)
{
    float dist = dgnCollisionDistFromPlane(point, plane);

    return m3dVec3SubVec3(point, m3dVec3MulValue(plane.normal, dist));
}

Vec3 dgnCollisionNearestPointLine(Vec3 point, DgnLine line)
{
    Vec3 l = m3dVec3SubVec3(line.p2, line.p1);
    Vec3 p = m3dVec3SubVec3(point, line.p1);
    float d = m3dVec3Dot(l, p) / m3dVec3Dot(l, l);
    d = m3d1DClamp(d, 0.0f, 1.0f);

    return m3dVec3AddVec3(line.p1, m3dVec3MulValue(l, d));
}

Vec3 dgnCollisionNearestPointTriangle(Vec3 point, DgnTriangle tri)
{
    // convert to plane and find nearest point on plane
    DgnPlane plane = dgnCollisionGeneratePlane(tri.p1, tri.p2, tri.p3);
    Vec3 nearest_point = dgnCollisionNearestPointPlane(point, plane);
    // check if point is inside plane, if so, return that point
    if(dgnCollisionTrianglePoint(tri, nearest_point).hit)
    {
        return nearest_point;
    }
    // find the nearest point on each of the edges
    DgnLine edge1;
    DgnLine edge2;
    DgnLine edge3;

    edge1.p1 = tri.p1;
    edge1.p2 = tri.p2;

    edge2.p1 = tri.p1;
    edge2.p2 = tri.p3;

    edge3.p1 = tri.p2;
    edge3.p2 = tri.p3;

    Vec3 ep1 = dgnCollisionNearestPointLine(point, edge1);
    Vec3 ep2 = dgnCollisionNearestPointLine(point, edge2);
    Vec3 ep3 = dgnCollisionNearestPointLine(point, edge3);

    // return the point with the smallest distance from point
    float dist1 = m3dVec3LengthSqr(m3dVec3SubVec3(point, ep1));
    float dist2 = m3dVec3LengthSqr(m3dVec3SubVec3(point, ep2));
    float dist3 = m3dVec3LengthSqr(m3dVec3SubVec3(point, ep3));

    float min_dist = fminf(fminf(dist1, dist2), dist3);

    if(min_dist == dist1)
    {
        return ep1;
    }
    else if(min_dist == dist2)
    {
        return ep2;
    }
    else
    {
        return ep3;
    }
}

Vec3 dgnCollisionBoxGetCenter(DgnBoundingBox box)
{
    return m3dVec3DivValue(m3dVec3AddVec3(box.max, box.min), 2.0f);
}

Mat4x4 dgnCollisionBoxGetModel(DgnBoundingBox box)
{
    Mat4x4 pos_mat = m3dMat4x4InitIdentity();
    Mat4x4 scale_mat = m3dMat4x4InitIdentity();

    m3dMat4x4Translate(&pos_mat, dgnCollisionBoxGetCenter(box));
    m3dMat4x4Scale(&scale_mat, m3dVec3SubVec3(box.max, box.min));

    return m3dMat4x4MulMat4x4(pos_mat, scale_mat);
}

Mat4x4 dgnCollisionSphereGetModel(DgnBoundingSphere sphere)
{
    Mat4x4 pos_mat = m3dMat4x4InitIdentity();
    Mat4x4 scale_mat = m3dMat4x4InitIdentity();

    m3dMat4x4Translate(&pos_mat, sphere.center);
    m3dMat4x4Scale(&scale_mat, (Vec3){sphere.radius, sphere.radius, sphere.radius});

    return m3dMat4x4MulMat4x4(pos_mat, scale_mat);
}
