#include "utilities.h"
#include "collision.h"

int check_collision(Actor *a, Actor *b)
{
    if (a->collider == TSphere && b->collider == TSphere)
        return sphere_sphere_collision(a, b);
    else if (a->collider == TBox && b->collider == TSphere)
        return box_sphere_collision(a, b);
    else if (a->collider == TSphere && b->collider == TBox)
        return box_sphere_collision(b, a);
    else if (a->collider == TBox && b->collider == TBox)
        return box_box_collision(a, b);
    
    return 0;
}

int sphere_sphere_collision(Actor *a, Actor *b)
{
    CActor_UpdateSphere(a);
    CActor_UpdateSphere(b);

    const float radiusSum = a->radius + b->radius;
    Vector3 aPos = vec3_add(CActor_GetPosition(a), a->center);
    Vector3 bPos = vec3_add(CActor_GetPosition(b), b->center);
    Vector3 dist = vec3_sub(aPos, bPos);

    return vec3_dot(dist, dist) <= radiusSum * radiusSum;
}

int box_box_collision(Actor *a, Actor *b)
{
    CActor_UpdateAABB(a);
    CActor_UpdateAABB(b);

    Vector3 aPos = vec3_add(CActor_GetPosition(a), a->center);
    Vector3 bPos = vec3_add(CActor_GetPosition(b), b->center);

    if (fabs(aPos.x - bPos.x) > (a->extents.x + b->extents.x)) return 0;
    if (fabs(aPos.y - bPos.y) > (a->extents.y + b->extents.y)) return 0;
    if (fabs(aPos.z - bPos.z) > (a->extents.z + b->extents.z)) return 0;

    return 1;
}

int box_sphere_collision(Actor *a, Actor *b)
{
    CActor_UpdateAABB(a);
    CActor_UpdateSphere(b);

    Vector3 aPos = vec3_add(CActor_GetPosition(a), a->center);
    Vector3 bPos = vec3_add(CActor_GetPosition(b), b->center);

    float aMin[3] = { aPos.x - a->extents.x, aPos.y - a->extents.y, aPos.z - a->extents.z };
    float aMax[3] = { aPos.x + a->extents.x, aPos.y + a->extents.y, aPos.z + a->extents.z };
    float bPosArray[3] = { bPos.x, bPos.y, bPos.z };

    float sqDist = 0.0f;
    for (int i = 0; i < 3; i++)
    {
        const float point = bPosArray[i];
        if (point < aMin[i]) sqDist += (aMin[i] - point) * (aMin[i] - point);
        if (point > aMax[i]) sqDist += (point - aMax[i]) * (point - aMax[i]);
    }

    return sqDist <= b->radius * b->radius;
}
