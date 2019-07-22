#include <nusys.h>
#include "upng.h"
#include "actor.h"

#define RGB5551(R, G, B) (((R >> 3) << 11) | ((G >> 3) << 6) | ((B >> 3) << 1) | 1)

void rom_2_ram(void *from_addr, void *to_addr, s32 seq_size)
{
    // If size is odd-numbered, cannot send over PI, so make it even.
    if (seq_size & 0x00000001) seq_size++;
    nuPiReadRom((u32)from_addr, to_addr, seq_size);
}

unsigned short *image_24_to_16(const unsigned char *data, const int size_x, const int size_y)
{
    int x, y;
    unsigned short *temp = (unsigned short*)malloc(size_x * size_y * 2);

    for (y = 0; y < size_y; y++) 
    {
        for (x = 0; x < size_x; x++) 
        {
            temp[x + y * size_y] = RGB5551(data[x * 3 + y * size_y * 3],
                data[x * 3 + y * size_y * 3 + 1],
                data[x * 3 + y * size_y * 3 + 2]);
        }
    }

    return temp;
}

struct actor *load_model(void *data_start, void *data_end, double positionX, double positionY, double positionZ,
    double rotX, double rotY, double rotZ, double angle, double scaleX, double scaleY, double scaleZ, 
    double centerX, double centerY, double centerZ, double radius)
{
    return load_model_with_texture(data_start, data_end,
        NULL, NULL, positionX, positionY, positionZ, rotX, rotY, rotZ, angle, scaleX, scaleY, scaleZ, 
        centerX, centerY, centerZ, radius);
}

struct actor *load_model_with_texture(void *data_start, void *data_end, void *texture_start, void *texture_end,
    double positionX, double positionY, double positionZ, double rotX, double rotY, double rotZ, double angle,
    double scaleX, double scaleY, double scaleZ, 
    double centerX, double centerY, double centerZ, double radius)
{
    unsigned char data_buffer[200000];
    unsigned char texture_buffer[20000];
    char *line;
    int data_size = data_end - data_start;
    int texture_size = texture_end - texture_start;
    int i = 0;
    int vertex_count = 0;
    struct actor *new_model;
    upng_t *png;

    // Transfer from ROM the model mesh data and texture.
    rom_2_ram(data_start, data_buffer, data_size);
    rom_2_ram(texture_start, texture_buffer, texture_size);

    new_model = (struct actor*)malloc(sizeof(struct actor));
    new_model->mesh = (struct mesh*)malloc(sizeof(struct mesh));
    new_model->position = (struct vector3*)malloc(sizeof(struct vector3));
    new_model->rotationAxis = (struct vector3*)malloc(sizeof(struct vector3));
    new_model->scale = (struct vector3*)malloc(sizeof(struct vector3));
    new_model->visible = 1;
    new_model->type = Model;

    new_model->center = (struct vector3*)malloc(sizeof(struct vector3));
    new_model->center->x = centerX;
    new_model->center->y = centerY;
    new_model->center->z = centerZ;
    new_model->radius = radius;

    // Read how many vertices for this mesh.
    line = (char*)strtok(data_buffer, "\n");
    sscanf(line, "%i", &vertex_count);
    new_model->mesh->vertices = (Vtx*)malloc(vertex_count * sizeof(Vtx));
    new_model->mesh->vertex_count = vertex_count;

    // Gather all of the X, Y, and Z vertex info.
    for (i = 0; i < vertex_count; i++)
    {
        double x, y, z, r, g, b, a, s, t;
        line = (char*)strtok(NULL, "\n");
        sscanf(line, "%lf %lf %lf %lf %lf %lf %lf %lf %lf", &x, &y, &z, &r, &g, &b, &a, &s, &t);

        new_model->mesh->vertices[i].v.ob[0] = x * 1000;
        new_model->mesh->vertices[i].v.ob[1] = y * 1000;
        new_model->mesh->vertices[i].v.ob[2] = -z * 1000;
        new_model->mesh->vertices[i].v.flag = 0;
        new_model->mesh->vertices[i].v.tc[0] = (int)(s * 32) << 5;
        new_model->mesh->vertices[i].v.tc[1] = (int)(t * 32) << 5;
        new_model->mesh->vertices[i].v.cn[0] = r * 255;
        new_model->mesh->vertices[i].v.cn[1] = g * 255;
        new_model->mesh->vertices[i].v.cn[2] = b * 255;
        new_model->mesh->vertices[i].v.cn[3] = a * 255;
    }

    // Entire axis can't be zero or it won't render.
    if (rotX == 0.0 && rotY == 0.0 && rotZ == 0.0) rotZ = 1;

    new_model->position->x = positionX;
    new_model->position->y = positionY;
    new_model->position->z = -positionZ;
    new_model->scale->x = scaleX * 0.001;
    new_model->scale->y = scaleY * 0.001;
    new_model->scale->z = scaleZ * 0.001;
    new_model->rotationAxis->x = rotX;
    new_model->rotationAxis->y = rotY;
    new_model->rotationAxis->z = -rotZ;
    new_model->rotationAngle = -angle;

    // Load in the png texture data.
    png = upng_new_from_bytes(texture_buffer, texture_size);
    if (png != NULL)
    {
        upng_decode(png);
        if (upng_get_error(png) == UPNG_EOK)
        {
            // Convert texture data from 24bpp to 16bpp in RGB5551 format.
            new_model->texture = image_24_to_16(upng_get_buffer(png), 32, 32);
        }
        upng_free(png);
    }

    return new_model;
}

void model_draw(struct actor *model, Gfx **display_list)
{
    int i;
    int remaining_vertices = model->mesh->vertex_count;
    int offset = 0;

    if (!model->visible) return;

    guTranslate(&model->transform.translation, model->position->x,
        model->position->y, model->position->z);

    guRotate(&model->transform.rotation, model->rotationAngle,
        model->rotationAxis->x, model->rotationAxis->y, model->rotationAxis->z);

    guScale(&model->transform.scale, model->scale->x,
        model->scale->y, model->scale->z);

    gSPMatrix((*display_list)++, OS_K0_TO_PHYSICAL(&model->transform.translation),
        G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);

    gSPMatrix((*display_list)++, OS_K0_TO_PHYSICAL(&model->transform.rotation),
        G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    gSPMatrix((*display_list)++, OS_K0_TO_PHYSICAL(&model->transform.scale),
        G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    gDPPipeSync((*display_list)++);

    gDPSetCycleType((*display_list)++, G_CYC_1CYCLE);
    gDPSetRenderMode((*display_list)++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
    gSPClearGeometryMode((*display_list)++, 0xFFFFFFFF);
    gSPSetGeometryMode((*display_list)++, G_SHADE | G_SHADING_SMOOTH | G_ZBUFFER | G_CULL_FRONT);

    if (model->texture != NULL)
    {
        gDPSetTextureFilter((*display_list)++, G_TF_BILERP);
        gDPSetCombineMode((*display_list)++, G_CC_BLENDRGBA, G_CC_BLENDRGBA);
        gDPSetTexturePersp((*display_list)++, G_TP_PERSP);
        gSPTexture((*display_list)++, 0xffff, 0xffff, 0, G_TX_RENDERTILE, G_ON);
        gDPLoadTextureBlock((*display_list)++, model->texture, G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 32, 0,
            G_TX_WRAP, G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    }

    // Send vertex data in batches of 30.
    while (remaining_vertices >= 30)
    {
        gSPVertex((*display_list)++, &(model->mesh->vertices[offset]), 30, 0);
        gDPPipeSync((*display_list)++);

        for (i = 0; i < 30 / 3; i++)
        {
            gSP1Triangle((*display_list)++, i * 3, i * 3 + 1, i * 3 + 2, 0);
        }

        remaining_vertices -= 30;
        offset += 30;
    }

    // Process last remaining vertices.
    if (remaining_vertices > 0)
    {
        gSPVertex((*display_list)++, &(model->mesh->vertices[offset]), remaining_vertices, 0);
        gDPPipeSync((*display_list)++);

        for (i = 0; i < remaining_vertices / 3; i++)
        {
            gSP1Triangle((*display_list)++, i * 3, i * 3 + 1, i * 3 + 2, 0);
        }
    }

    gSPPopMatrix((*display_list)++, G_MTX_MODELVIEW);
}

struct actor *create_camera(double positionX, double positionY, double positionZ,
    double rotX, double rotY, double rotZ, double angle, 
    double centerX, double centerY, double centerZ, double radius)
{
    struct actor *camera;
    camera = (struct actor*)malloc(sizeof(struct actor));
    camera->position = (struct vector3*)malloc(sizeof(struct vector3));
    camera->rotationAxis = (struct vector3*)malloc(sizeof(struct vector3));
    camera->visible = 1;
    camera->type = Camera;

    camera->center = (struct vector3*)malloc(sizeof(struct vector3));
    camera->center->x = centerX;
    camera->center->y = centerY;
    camera->center->z = centerZ;
    camera->radius = radius;

    if (rotX == 0.0 && rotY == 0.0 && rotZ == 0.0) rotZ = 1;
    camera->position->x = positionX;
    camera->position->y = positionY;
    camera->position->z = positionZ;
    camera->rotationAxis->x = rotX;
    camera->rotationAxis->y = rotY;
    camera->rotationAxis->z = rotZ;
    camera->rotationAngle = angle;

    return camera;
}

float dot(struct vector3 a, struct vector3 b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

int check_collision(struct actor *a, struct actor *b)
{
    float dist = 0;
    float radiusSum = 0;
    struct vector3 vectorA = *a->position;
    struct vector3 vectorB = *b->position;
    struct vector3 vectorC;

    vectorA.x += a->center->x;
    vectorA.y += a->center->y;
    vectorA.z += a->center->z;

    vectorB.x += b->center->x;
    vectorB.y += b->center->y;
    vectorB.z += b->center->z;

    vectorC.x = vectorA.x - vectorB.x;
    vectorC.y = vectorA.y - vectorB.y;
    vectorC.z = vectorA.z - vectorB.z;

    dist = dot(vectorC, vectorC);
    radiusSum = a->radius + b->radius;
    return dist <= radiusSum * radiusSum;
}
