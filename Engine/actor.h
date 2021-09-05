#ifndef _ACTOR_H_
#define _ACTOR_H_

#include <nusys.h>

#define SCALE_FACTOR 30

enum actorType { Model, Camera };

enum colliderType { None, Sphere, Box };

typedef struct transform 
{
    Mtx projection;
    Mtx scale;
    Mtx rotation;
    Mtx translation;
} transform;

typedef struct vector3
{
    double x, y, z;
} vector3;

typedef struct mesh
{
    int vertexCount;
    Vtx *vertices;
} mesh;

typedef struct _vector *vector;

typedef struct _Actor 
{
    int id;
    enum actorType type;
    enum colliderType collider;
    mesh mesh;
    unsigned short *texture;
    int textureWidth;
    int textureHeight;
    double rotationAngle;
    double radius;
    double originalRadius;
    int visible;
    vector3 position;
    vector3 rotationAxis;
    vector3 scale;
    vector3 center;
    vector3 originalCenter;
    vector3 extents;
    vector3 originalExtents;
    transform transform;
    struct _Actor *parent;
    vector children;
    void (*start)();
    void (*update)();
    void (*input)();
    void (*collide)();
} Actor;

Actor *CActor_LoadModel(int id, void *dataStart, void *dataEnd, double positionX, double positionY, double positionZ,
    double rotX, double rotY, double rotZ, double angle, double scaleX, double scaleY, double scaleZ, 
    double centerX, double centerY, double centerZ, double radius,
    double extentX, double extentY, double extentZ, enum colliderType collider);

Actor *CActor_LoadTexturedModel(int id, void *dataStart, void *dataEnd,
    void *textureStart, void *textureEnd, int textureWidth, int textureHeight,
    double positionX, double positionY, double positionZ,
    double rotX, double rotY, double rotZ, double angle,
    double scaleX, double scaleY, double scaleZ, 
    double centerX, double centerY, double centerZ, double radius,
    double extentX, double extentY, double extentZ, enum colliderType collider);

Actor *CActor_CreateCamera(int id, double positionX, double positionY, double positionZ,
    double rotX, double rotY, double rotZ, double angle, 
    double centerX, double centerY, double centerZ, double radius,
    double extentX, double extentY, double extentZ, enum colliderType collider);

void CActor_LinkChildToParent(vector actors, int childId, int parentId);

void CActor_Draw(Actor *model, Gfx **displayList);

vector3 CActor_GetPosition(Actor *Actor);

Mtx CActor_GetMatrix(Actor *Actor);

void CActor_UpdateAABB(Actor *Actor);

void CActor_UpdateSphere(Actor *Actor);

#endif
