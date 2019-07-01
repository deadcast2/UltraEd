#ifndef _ACTOR_H_
#define _ACTOR_H_

#include <nusys.h>
#include "upng.h"

#define SCREEN_WD 320
#define SCREEN_HT 240

enum actorType { Model, Camera };

struct transform 
{
    Mtx projection;
    Mtx translation;
    Mtx scale;
    Mtx rotation;
};

struct actor 
{
    enum actorType type;
    struct mesh *mesh;
    unsigned short *texture;
    double rotationAngle;
    double radius;
    int visible;
    struct vector3 *position;
    struct vector3 *rotationAxis;
    struct vector3 *scale;
    struct transform transform;
};

struct vector3 
{
    double x, y, z;
};

struct mesh 
{
    int vertex_count;
    Vtx *vertices;
};

struct actor *load_model(void *data_start, void *data_end, double positionX, double positionY, double positionZ,
    double rotX, double rotY, double rotZ, double angle, double scaleX, double scaleY, double scaleZ, double radius);

struct actor *load_model_with_texture(void *data_start, void *data_end,
    void *texture_start, void *texture_end,
    double positionX, double positionY, double positionZ,
    double rotX, double rotY, double rotZ, double angle,
    double scaleX, double scaleY, double scaleZ, double radius);

struct actor *create_camera(double positionX, double positionY, double positionZ,
    double rotX, double rotY, double rotZ, double angle, double radius);

#endif
