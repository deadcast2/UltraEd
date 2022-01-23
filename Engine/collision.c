#include "math.h"
#include "collision.h"

int CCollision_Test(Actor *a, Actor *b)
{
    if (a->collider == TSphere && b->collider == TSphere)
        return CCollision_SphereSphereTest(a, b);
    else if (a->collider == TBox && b->collider == TSphere)
        return CCollision_BoxSphereTest(a, b);
    else if (a->collider == TSphere && b->collider == TBox)
        return CCollision_BoxSphereTest(b, a);
    else if (a->collider == TBox && b->collider == TBox)
        return CCollision_BoxBoxTest(a, b);
    
    return 0;
}

int CCollision_SphereSphereTest(Actor *a, Actor *b)
{
    CActor_UpdateSphere(a);
    CActor_UpdateSphere(b);

    const float radiusSum = a->radius + b->radius;
    Vector3 aPos = CMath_Vec3Add(CActor_GetPosition(a), a->center);
    Vector3 bPos = CMath_Vec3Add(CActor_GetPosition(b), b->center);
    Vector3 dist = CMath_Vec3Sub(aPos, bPos);

    return CMath_Vec3Dot(dist, dist) <= radiusSum * radiusSum;
}

int CCollision_BoxBoxTest(Actor *a, Actor *b)
{
    CActor_UpdateAABB(a);
    CActor_UpdateAABB(b);

    Vector3 aPos = CMath_Vec3Add(CActor_GetPosition(a), a->center);
    Vector3 bPos = CMath_Vec3Add(CActor_GetPosition(b), b->center);

    if (fabs(aPos.x - bPos.x) > (a->extents.x + b->extents.x)) return 0;
    if (fabs(aPos.y - bPos.y) > (a->extents.y + b->extents.y)) return 0;
    if (fabs(aPos.z - bPos.z) > (a->extents.z + b->extents.z)) return 0;

    return 1;
}

int CCollision_BoxSphereTest(Actor *a, Actor *b)
{
    CActor_UpdateAABB(a);
    CActor_UpdateSphere(b);

    Vector3 aPos = CMath_Vec3Add(CActor_GetPosition(a), a->center);
    Vector3 bPos = CMath_Vec3Add(CActor_GetPosition(b), b->center);

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
