#ifndef _ACTOR_H_
#define _ACTOR_H_

#include <nusys.h>
#include "upng.h"

#define SCREEN_WD 320
#define SCREEN_HT 240

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
    int vertex_count;
    Vtx *vertices;
} mesh;

typedef struct actor 
{
    enum actorType type;
    enum colliderType collider;
    mesh *mesh;
    unsigned short *texture;
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

actor *load_model(void *data_start, void *data_end, double positionX, double positionY, double positionZ,
    double rotX, double rotY, double rotZ, double angle, double scaleX, double scaleY, double scaleZ, 
    double centerX, double centerY, double centerZ, double radius,
    double extentX, double extentY, double extentZ, enum colliderType collider);

actor *load_model_with_texture(void *data_start, void *data_end,
    void *texture_start, void *texture_end,
    double positionX, double positionY, double positionZ,
    double rotX, double rotY, double rotZ, double angle,
    double scaleX, double scaleY, double scaleZ, 
    double centerX, double centerY, double centerZ, double radius,
    double extentX, double extentY, double extentZ, enum colliderType collider);

actor *create_camera(double positionX, double positionY, double positionZ,
    double rotX, double rotY, double rotZ, double angle, 
    double centerX, double centerY, double centerZ, double radius,
    double extentX, double extentY, double extentZ, enum colliderType collider);

void model_draw(actor *model, Gfx **display_list);

int check_collision(actor *a, actor *b);

#endif
