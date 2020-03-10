#include "utilities.h"
#include "collision.h"

int check_collision(actor *a, actor *b)
{
    if (a->collider == Sphere && b->collider == Sphere)
        return sphere_sphere_collision(a, b);
    else if (a->collider == Box && b->collider == Sphere)
        return box_sphere_collision(a, b);
    else if (a->collider == Sphere && b->collider == Box)
        return box_sphere_collision(b, a);
    else if (a->collider == Box && b->collider == Box)
        return box_box_collision(a, b);
    
    return 0;
}

int sphere_sphere_collision(actor *a, actor *b)
{
    const float radiusSum = a->radius + b->radius;
    vector3 aPos = vec3_add(*a->position, vec3_mul_mat3x3(*a->center, a->transform.rotation));
    vector3 bPos = vec3_add(*b->position, vec3_mul_mat3x3(*b->center, b->transform.rotation));
    vector3 dist = vec3_sub(aPos, bPos);

    return vec3_dot(dist, dist) <= radiusSum * radiusSum;
}

int box_box_collision(actor *a, actor *b)
{
    vector3 aPos = vec3_add(*a->position, vec3_mul_mat3x3(*a->center, a->transform.rotation));
    vector3 bPos = vec3_add(*b->position, vec3_mul_mat3x3(*b->center, b->transform.rotation));
    
    vector3 aAxis[3] = { 
        vec3_mul_mat3x3((vector3) { 1, 0, 0 }, a->transform.rotation),
        vec3_mul_mat3x3((vector3) { 0, 1, 0 }, a->transform.rotation),
        vec3_mul_mat3x3((vector3) { 0, 0, 1 }, a->transform.rotation)
    };

    float aExt[3] = { a->extents->x, a->extents->y, a->extents->z };

    vector3 bAxis[3] = {
        vec3_mul_mat3x3((vector3) { 1, 0, 0 }, b->transform.rotation),
        vec3_mul_mat3x3((vector3) { 0, 1, 0 }, b->transform.rotation),
        vec3_mul_mat3x3((vector3) { 0, 0, 1 }, b->transform.rotation)
    };

    float bExt[3] = { b->extents->x, b->extents->y, b->extents->z };

    float ra, rb;
    float R[3][3], AbsR[3][3];
    const float EPSILON = 0.0001;

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            R[i][j] = vec3_dot(aAxis[i], bAxis[j]);

    vector3 t = vec3_sub(bPos, aPos);
    float ta[3] = { vec3_dot(t, aAxis[0]), vec3_dot(t, aAxis[1]), vec3_dot(t, aAxis[2]) };

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            AbsR[i][j] = fabs(R[i][j]) + EPSILON;

    for (int i = 0; i < 3; i++)
    {
        ra = aExt[i];
        rb = bExt[0] * AbsR[i][0] + bExt[1] * AbsR[i][1] + bExt[2] * AbsR[i][2];
        if (fabs(ta[i]) > ra + rb) return 0;
    }

    for (int i = 0; i < 3; i++)
    {
        ra = aExt[0] * AbsR[0][i] + aExt[1] * AbsR[1][i] + aExt[2] * AbsR[2][i];
        rb = bExt[i];
        if (fabs(ta[0] * R[0][i] + ta[1] * R[1][i] + ta[2] * R[2][i]) > ra + rb) return 0;
    }

    ra = aExt[1] * AbsR[2][0] + aExt[2] * AbsR[1][0];
    rb = bExt[1] * AbsR[0][2] + bExt[2] * AbsR[0][1];
    if (fabs(ta[2] * R[1][0] - ta[1] * R[2][0]) > ra + rb) return 0;

    ra = aExt[1] * AbsR[2][1] + aExt[2] * AbsR[1][1];
    rb = bExt[0] * AbsR[0][2] + bExt[2] * AbsR[0][0];
    if (fabs(ta[2] * R[1][1] - ta[1] * R[2][1]) > ra + rb) return 0;

    ra = aExt[1] * AbsR[2][2] + aExt[2] * AbsR[1][2];
    rb = bExt[0] * AbsR[0][1] + bExt[1] * AbsR[0][0];
    if (fabs(ta[2] * R[1][2] - ta[1] * R[2][2]) > ra + rb) return 0;

    ra = aExt[0] * AbsR[2][0] + aExt[2] * AbsR[0][0];
    rb = bExt[1] * AbsR[1][2] + bExt[2] * AbsR[1][1];
    if (fabs(ta[0] * R[2][0] - ta[2] * R[0][0]) > ra + rb) return 0;

    ra = aExt[0] * AbsR[2][1] + aExt[2] * AbsR[0][1];
    rb = bExt[0] * AbsR[1][2] + bExt[2] * AbsR[1][0];
    if (fabs(ta[0] * R[2][1] - ta[2] * R[0][1]) > ra + rb) return 0;

    ra = aExt[0] * AbsR[2][2] + aExt[2] * AbsR[0][2];
    rb = bExt[0] * AbsR[1][1] + bExt[1] * AbsR[1][0];
    if (fabs(ta[0] * R[2][2] - ta[2] * R[0][2]) > ra + rb) return 0;

    ra = aExt[0] * AbsR[1][0] + aExt[1] * AbsR[0][0];
    rb = bExt[1] * AbsR[2][2] + bExt[2] * AbsR[2][1];
    if (fabs(ta[1] * R[0][0] - ta[0] * R[1][0]) > ra + rb) return 0;

    ra = aExt[0] * AbsR[1][1] + aExt[1] * AbsR[0][1];
    rb = bExt[0] * AbsR[2][2] + bExt[2] * AbsR[2][0];
    if (fabs(ta[1] * R[0][1] - ta[0] * R[1][1]) > ra + rb) return 0;

    ra = aExt[0] * AbsR[1][2] + aExt[1] * AbsR[0][2];
    rb = bExt[0] * AbsR[2][1] + bExt[1] * AbsR[2][0];
    if (fabs(ta[1] * R[0][2] - ta[0] * R[1][2]) > ra + rb) return 0;

    return 1;
}

int box_sphere_collision(actor *a, actor *b)
{
    vector3 aPos = vec3_add(*a->position, vec3_mul_mat3x3(*a->center, a->transform.rotation));
    vector3 bPos = vec3_add(*b->position, vec3_mul_mat3x3(*b->center, b->transform.rotation));

    vector3 aAxis[3] = {
        vec3_mul_mat3x3((vector3) { 1, 0, 0 }, a->transform.rotation),
        vec3_mul_mat3x3((vector3) { 0, 1, 0 }, a->transform.rotation),
        vec3_mul_mat3x3((vector3) { 0, 0, 1 }, a->transform.rotation)
    };

    float aExt[3] = { a->extents->x, a->extents->y, a->extents->z };

    vector3 abDir = vec3_sub(bPos, aPos);
    vector3 closestPoint = aPos;
    for (int i = 0; i < 3; i++)
    {
        float dist = vec3_dot(abDir, aAxis[i]);
        if (dist > aExt[i]) dist = aExt[i];
        if (dist < -aExt[i]) dist = -aExt[i];
        closestPoint = vec3_add(closestPoint, vec3_mul(aAxis[i], dist));
    }

    vector3 closestDir = vec3_sub(closestPoint, bPos);
    return vec3_dot(closestDir, closestDir) <= b->radius * b->radius;
}
