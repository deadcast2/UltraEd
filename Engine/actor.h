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

typedef struct actor 
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
    struct actor *parent;
    vector children;
    void (*start)();
    void (*update)();
    void (*input)();
    void (*collide)();
} actor;

actor *loadModel(int id, void *dataStart, void *dataEnd, double positionX, double positionY, double positionZ,
    double rotX, double rotY, double rotZ, double angle, double scaleX, double scaleY, double scaleZ, 
    double centerX, double centerY, double centerZ, double radius,
    double extentX, double extentY, double extentZ, enum colliderType collider);

actor *loadTexturedModel(int id, void *dataStart, void *dataEnd,
    void *textureStart, void *textureEnd, int textureWidth, int textureHeight,
    double positionX, double positionY, double positionZ,
    double rotX, double rotY, double rotZ, double angle,
    double scaleX, double scaleY, double scaleZ, 
    double centerX, double centerY, double centerZ, double radius,
    double extentX, double extentY, double extentZ, enum colliderType collider);

actor *createCamera(int id, double positionX, double positionY, double positionZ,
    double rotX, double rotY, double rotZ, double angle, 
    double centerX, double centerY, double centerZ, double radius,
    double extentX, double extentY, double extentZ, enum colliderType collider);

void linkChildToParent(vector actors, int childId, int parentId);

void modelDraw(actor *model, Gfx **displayList);

vector3 CActor_GetPosition(actor *actor);

Mtx CActor_GetMatrix(actor *actor);

void CActor_UpdateAABB(actor *actor);

void CActor_UpdateSphere(actor *actor);

#endif
