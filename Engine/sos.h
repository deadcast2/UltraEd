#ifndef _SOS_H_
#define _SOS_H_

#include <nusys.h>
#include "upng.h"

#define SCREEN_WD 320
#define SCREEN_HT 240

struct transform {
  Mtx projection;
  Mtx translation;
  Mtx scale;
  Mtx rotation_x;
  Mtx rotation_y;
  Mtx rotation_z;
};

struct sos_model {
    struct mesh *meshes;
    int mesh_count;
    unsigned short *texture;
    struct vector3 *position;
    struct vector3 *rotation;
    struct vector3 *scale;
    struct transform transform;
};

struct vector2 {
    double x, y;
};

struct vector3 {
    double x, y, z;
};

struct mesh {
    struct vector3 *vertices;
    struct vector2 *uvs;
    int *indices;
    int poly_count;

    // What the RSP gets sent.
    Vtx *processed_vertices;
};

#endif
