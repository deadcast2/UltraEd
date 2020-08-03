#ifndef _ACTOR_H_
#define _ACTOR_H_

#include <nusys.h>
#include "upng.h"

enum actorType { Model, Camera };

enum colliderType { None, Sphere, Box };

typedef struct transform 
{
    Mtx projection;
    Mtx translation;
    Mtx scale;
    Mtx rotation;
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

typedef struct actor 
{
    enum actorType type;
    enum colliderType collider;
    mesh *mesh;
    unsigned short *texture;
    int textureWidth;
    int textureHeight;
    double rotationAngle;
    double radius;
    int visible;
    vector3 *position;
    vector3 *rotationAxis;
    vector3 *scale;
    vector3 *center;
    vector3 *extents;
    transform transform;
} actor;

actor *loadModel(void *dataStart, void *dataEnd, double positionX, double positionY, double positionZ,
    double rotX, double rotY, double rotZ, double angle, double scaleX, double scaleY, double scaleZ, 
    double centerX, double centerY, double centerZ, double radius,
    double extentX, double extentY, double extentZ, enum colliderType collider);

actor *loadTexturedModel(void *dataStart, void *dataEnd,
    void *textureStart, void *textureEnd, int textureWidth, int textureHeight,
    double positionX, double positionY, double positionZ,
    double rotX, double rotY, double rotZ, double angle,
    double scaleX, double scaleY, double scaleZ, 
    double centerX, double centerY, double centerZ, double radius,
    double extentX, double extentY, double extentZ, enum colliderType collider);

actor *createCamera(double positionX, double positionY, double positionZ,
    double rotX, double rotY, double rotZ, double angle, 
    double centerX, double centerY, double centerZ, double radius,
    double extentX, double extentY, double extentZ, enum colliderType collider);

void modelDraw(actor *model, Gfx **displayList);

#endif
