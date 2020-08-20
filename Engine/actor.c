#include <nusys.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include "upng.h"
#include "actor.h"
#include "utilities.h"

actor *loadModel(void *dataStart, void *dataEnd, double positionX, double positionY, double positionZ,
    double rotX, double rotY, double rotZ, double angle, double scaleX, double scaleY, double scaleZ, 
    double centerX, double centerY, double centerZ, double radius,
    double extentX, double extentY, double extentZ, enum colliderType collider)
{
    return loadTexturedModel(dataStart, dataEnd,
        NULL, NULL, 0, 0, positionX, positionY, positionZ, rotX, rotY, rotZ, angle, scaleX, scaleY, scaleZ, 
        centerX, centerY, centerZ, radius, extentX, extentY, extentZ, collider);
}

actor *loadTexturedModel(void *dataStart, void *dataEnd, void *textureStart, void *textureEnd,
    int textureWidth, int textureHeight, double positionX, double positionY, double positionZ, double rotX, 
    double rotY, double rotZ, double angle, double scaleX, double scaleY, double scaleZ, 
    double centerX, double centerY, double centerZ, double radius,
    double extentX, double extentY, double extentZ, enum colliderType collider)
{
    unsigned char dataBuffer[200000];
    unsigned char textureBuffer[20000];
    int dataSize = dataEnd - dataStart;
    int textureSize = textureEnd - textureStart;
    actor *newModel;

    // Transfer from ROM the model mesh data and texture.
    rom_2_ram(dataStart, dataBuffer, dataSize);
    rom_2_ram(textureStart, textureBuffer, textureSize);

    newModel = (actor*)malloc(sizeof(actor));
    newModel->mesh = (mesh*)malloc(sizeof(mesh));
    newModel->position = (vector3*)malloc(sizeof(vector3));
    newModel->rotationAxis = (vector3*)malloc(sizeof(vector3));
    newModel->scale = (vector3*)malloc(sizeof(vector3));
    newModel->visible = 1;
    newModel->type = Model;
    newModel->collider = collider;
    newModel->texture = NULL;
    newModel->textureWidth = textureWidth;
    newModel->textureHeight = textureHeight;

    newModel->center = (vector3*)malloc(sizeof(vector3));
    newModel->center->x = centerX;
    newModel->center->y = centerY;
    newModel->center->z = centerZ;
    newModel->radius = radius;

    newModel->extents = (vector3 *)malloc(sizeof(vector3));
    newModel->extents->x = extentX;
    newModel->extents->y = extentY;
    newModel->extents->z = extentZ;

    // Read how many vertices for this mesh.
    int vertexCount = 0;
    char *line = (char*)strtok(dataBuffer, "\n");
    sscanf(line, "%i", &vertexCount);
    newModel->mesh->vertices = (Vtx*)malloc(vertexCount * sizeof(Vtx));
    newModel->mesh->vertexCount = vertexCount;

    // Gather all of the X, Y, and Z vertex info.
    for (int i = 0; i < vertexCount; i++)
    {
        double x, y, z, r, g, b, a, s, t;
        line = (char*)strtok(NULL, "\n");
        sscanf(line, "%lf %lf %lf %lf %lf %lf %lf %lf %lf", &x, &y, &z, &r, &g, &b, &a, &s, &t);

        newModel->mesh->vertices[i].v.ob[0] = x * scaleX * 100;
        newModel->mesh->vertices[i].v.ob[1] = y * scaleY * 100;
        newModel->mesh->vertices[i].v.ob[2] = -z * scaleZ * 100;
        newModel->mesh->vertices[i].v.flag = 0;
        newModel->mesh->vertices[i].v.tc[0] = (int)(s * textureWidth) << 5;
        newModel->mesh->vertices[i].v.tc[1] = (int)(t * textureHeight) << 5;
        newModel->mesh->vertices[i].v.cn[0] = r * 255;
        newModel->mesh->vertices[i].v.cn[1] = g * 255;
        newModel->mesh->vertices[i].v.cn[2] = b * 255;
        newModel->mesh->vertices[i].v.cn[3] = a * 255;
    }

    // Entire axis can't be zero or it won't render.
    if (rotX == 0.0 && rotY == 0.0 && rotZ == 0.0) rotZ = 1;

    newModel->position->x = positionX;
    newModel->position->y = positionY;
    newModel->position->z = -positionZ;
    newModel->scale->x = 0.01;
    newModel->scale->y = 0.01;
    newModel->scale->z = 0.01;
    newModel->rotationAxis->x = rotX;
    newModel->rotationAxis->y = rotY;
    newModel->rotationAxis->z = -rotZ;
    newModel->rotationAngle = -angle;

    // Load in the png texture data.
    upng_t *png = upng_new_from_bytes(textureBuffer, textureSize);
    if (png != NULL)
    {
        upng_decode(png);
        if (upng_get_error(png) == UPNG_EOK)
        {
            // Convert texture data from 24bpp to 16bpp in RGB5551 format.
            newModel->texture = image_24_to_16(upng_get_buffer(png), textureWidth, textureHeight);
        }
        upng_free(png);
    }

    return newModel;
}

void modelDraw(actor *model, Gfx **displayList)
{
    if (!model->visible) return;

    guTranslate(&model->transform.translation, model->position->x,
        model->position->y, model->position->z);

    guRotate(&model->transform.rotation, model->rotationAngle,
        model->rotationAxis->x, model->rotationAxis->y, model->rotationAxis->z);

    guScale(&model->transform.scale, model->scale->x,
        model->scale->y, model->scale->z);

    gSPMatrix((*displayList)++, OS_K0_TO_PHYSICAL(&model->transform.translation),
        G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);

    gSPMatrix((*displayList)++, OS_K0_TO_PHYSICAL(&model->transform.rotation),
        G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    gSPMatrix((*displayList)++, OS_K0_TO_PHYSICAL(&model->transform.scale),
        G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    gDPPipeSync((*displayList)++);

    gDPSetCycleType((*displayList)++, G_CYC_1CYCLE);
    gDPSetRenderMode((*displayList)++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
    gSPClearGeometryMode((*displayList)++, 0xFFFFFFFF);
    gSPSetGeometryMode((*displayList)++, G_SHADE | G_SHADING_SMOOTH | G_ZBUFFER | G_CULL_FRONT);

    if (model->texture != NULL)
    {
        gSPTexture((*displayList)++, 0xffff, 0xffff, 0, G_TX_RENDERTILE, G_ON);
        gDPSetTextureFilter((*displayList)++, G_TF_BILERP);
        gDPSetTexturePersp((*displayList)++, G_TP_PERSP);
        gDPSetCombineMode((*displayList)++, G_CC_MODULATERGB, G_CC_MODULATERGB);
        gDPLoadTextureBlock((*displayList)++, model->texture, G_IM_FMT_RGBA, G_IM_SIZ_16b, 
            model->textureWidth, model->textureHeight, 0,
            G_TX_WRAP, G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    }
    else
    {
        gDPSetCombineMode((*displayList)++, G_CC_SHADE, G_CC_SHADE);
    }

    int skip = 0;
    int remainingVertices = model->mesh->vertexCount;
    const int size = 30;

    // Send vertex data in batches.
    while (remainingVertices > 0)
    {
        int take = remainingVertices >= size ? size : remainingVertices;
        gSPVertex((*displayList)++, &(model->mesh->vertices[skip]), take, 0);
        gDPPipeSync((*displayList)++);

        for (int i = 0; i < take / 3; i++)
        {
            gSP1Triangle((*displayList)++, i * 3, i * 3 + 1, i * 3 + 2, 0);
        }

        remainingVertices -= take;
        skip += take;
    }

    gSPPopMatrix((*displayList)++, G_MTX_MODELVIEW);
}

actor *createCamera(double positionX, double positionY, double positionZ,
    double rotX, double rotY, double rotZ, double angle, 
    double centerX, double centerY, double centerZ, double radius,
    double extentX, double extentY, double extentZ, enum colliderType collider)
{
    actor *camera = (actor*)malloc(sizeof(actor));
    camera->position = (vector3*)malloc(sizeof(vector3));
    camera->rotationAxis = (vector3*)malloc(sizeof(vector3));
    camera->visible = 1;
    camera->type = Camera;
    camera->collider = collider;

    camera->center = (vector3*)malloc(sizeof(vector3));
    camera->center->x = centerX;
    camera->center->y = centerY;
    camera->center->z = centerZ;
    camera->radius = radius;

    camera->extents = (vector3 *)malloc(sizeof(vector3));
    camera->extents->x = extentX;
    camera->extents->y = extentY;
    camera->extents->z = extentZ;

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
