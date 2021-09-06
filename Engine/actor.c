#include <nusys.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include "upng.h"
#include "actor.h"
#include "utilities.h"
#include "vector.h"

Actor *CActor_LoadModel(int id, const char *name, void *dataStart, void *dataEnd, double positionX, double positionY, double positionZ,
    double rotX, double rotY, double rotZ, double angle, double scaleX, double scaleY, double scaleZ,
    double centerX, double centerY, double centerZ, double radius,
    double extentX, double extentY, double extentZ, enum ColliderType collider)
{
    return CActor_LoadTexturedModel(id, name, dataStart, dataEnd,
        NULL, NULL, 0, 0, positionX, positionY, positionZ, rotX, rotY, rotZ, angle, scaleX, scaleY, scaleZ,
        centerX, centerY, centerZ, radius, extentX, extentY, extentZ, collider);
}

Actor *CActor_LoadTexturedModel(int id, const char *name, void *dataStart, void *dataEnd, void *textureStart, void *textureEnd,
    int textureWidth, int textureHeight, double positionX, double positionY, double positionZ, double rotX,
    double rotY, double rotZ, double angle, double scaleX, double scaleY, double scaleZ,
    double centerX, double centerY, double centerZ, double radius,
    double extentX, double extentY, double extentZ, enum ColliderType collider)
{
    unsigned char dataBuffer[200000];
    unsigned char textureBuffer[200000];
    int dataSize = dataEnd - dataStart;
    int textureSize = textureEnd - textureStart;
    Actor *newModel;

    // Transfer from ROM the model mesh data and texture.
    rom_2_ram(dataStart, dataBuffer, dataSize);
    rom_2_ram(textureStart, textureBuffer, textureSize);

    newModel = (Actor *)malloc(sizeof(Actor));
    newModel->id = id;
    newModel->name = name;
    newModel->visible = 1;
    newModel->type = Model;
    newModel->collider = collider;
    newModel->parent = NULL;
    newModel->children = NULL;
    newModel->start = NULL;
    newModel->update = NULL;
    newModel->input = NULL;
    newModel->collide = NULL;
    newModel->texture = NULL;
    newModel->textureWidth = textureWidth;
    newModel->textureHeight = textureHeight;

    newModel->originalCenter.x = newModel->center.x = centerX;
    newModel->originalCenter.y = newModel->center.y = centerY;
    newModel->originalCenter.z = newModel->center.z = -centerZ;
    newModel->originalRadius = newModel->radius = radius;

    newModel->originalExtents.x = newModel->extents.x = extentX;
    newModel->originalExtents.y = newModel->extents.y = extentY;
    newModel->originalExtents.z = newModel->extents.z = extentZ;

    // Read how many vertices for this mesh.
    int vertexCount = 0;
    char *line = (char *)strtok(dataBuffer, "\n");
    sscanf(line, "%i", &vertexCount);
    newModel->mesh.vertices = (Vtx *)malloc(vertexCount * sizeof(Vtx));
    newModel->mesh.vertexCount = vertexCount;

    // Gather all of the X, Y, and Z vertex info.
    for (int i = 0; i < vertexCount; i++)
    {
        double x, y, z, r, g, b, a, s, t;
        line = (char *)strtok(NULL, "\n");
        sscanf(line, "%lf %lf %lf %lf %lf %lf %lf %lf %lf", &x, &y, &z, &r, &g, &b, &a, &s, &t);

        newModel->mesh.vertices[i].v.ob[0] = x * SCALE_FACTOR;
        newModel->mesh.vertices[i].v.ob[1] = y * SCALE_FACTOR;
        newModel->mesh.vertices[i].v.ob[2] = -z * SCALE_FACTOR;
        newModel->mesh.vertices[i].v.flag = 0;
        newModel->mesh.vertices[i].v.tc[0] = (int)(s * textureWidth) << 5;
        newModel->mesh.vertices[i].v.tc[1] = (int)(t * textureHeight) << 5;
        newModel->mesh.vertices[i].v.cn[0] = r * 255;
        newModel->mesh.vertices[i].v.cn[1] = g * 255;
        newModel->mesh.vertices[i].v.cn[2] = b * 255;
        newModel->mesh.vertices[i].v.cn[3] = a * 255;
    }

    // Entire axis can't be zero or it won't render.
    if (rotX == 0.0 && rotY == 0.0 && rotZ == 0.0) rotZ = 1;

    newModel->position.x = positionX;
    newModel->position.y = positionY;
    newModel->position.z = -positionZ;
    newModel->scale.x = scaleX;
    newModel->scale.y = scaleY;
    newModel->scale.z = scaleZ;
    newModel->rotationAxis.x = rotX;
    newModel->rotationAxis.y = rotY;
    newModel->rotationAxis.z = -rotZ;
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

void CActor_Draw(Actor *model, Gfx **displayList)
{
    if (!model->visible) return;

    const int invertScalar = model->type == Camera ? -1 : 1;

    guTranslate(&model->transform.translation, model->position.x * SCALE_FACTOR, model->position.y * SCALE_FACTOR, invertScalar * model->position.z * SCALE_FACTOR);

    guRotate(&model->transform.rotation, model->rotationAngle, invertScalar * model->rotationAxis.x, invertScalar * model->rotationAxis.y, model->rotationAxis.z);

    guScale(&model->transform.scale, model->scale.x, model->scale.y, model->scale.z);

    gSPMatrix((*displayList)++, OS_K0_TO_PHYSICAL(&model->transform.translation),
        G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);

    gSPMatrix((*displayList)++, OS_K0_TO_PHYSICAL(&model->transform.rotation),
        G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    gSPMatrix((*displayList)++, OS_K0_TO_PHYSICAL(&model->transform.scale),
        G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    gDPPipeSync((*displayList)++);

    if (model->type == Model)
    {
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
        int remainingVertices = model->mesh.vertexCount;
        const int size = 30;

        // Send vertex data in batches.
        while (remainingVertices > 0)
        {
            int take = remainingVertices >= size ? size : remainingVertices;
            gSPVertex((*displayList)++, &(model->mesh.vertices[skip]), take, 0);
            gDPPipeSync((*displayList)++);

            for (int i = 0; i < take / 3; i++)
            {
                gSP1Triangle((*displayList)++, i * 3, i * 3 + 1, i * 3 + 2, 0);
            }

            remainingVertices -= take;
            skip += take;
        }
    }

    if (model->children != NULL)
    {
        for (int i = 0; i < vector_size(model->children); i++)
        {
            CActor_Draw(vector_get(model->children, i), displayList);
        }
    }

    gSPPopMatrix((*displayList)++, G_MTX_MODELVIEW);
}

Actor *CActor_CreateCamera(int id, const char *name, double positionX, double positionY, double positionZ,
    double rotX, double rotY, double rotZ, double angle,
    double centerX, double centerY, double centerZ, double radius,
    double extentX, double extentY, double extentZ, enum ColliderType collider)
{
    Actor *camera = (Actor *)malloc(sizeof(Actor));
    camera->id = id;
    camera->name = name;
    camera->visible = 1;
    camera->type = Camera;
    camera->collider = collider;
    camera->parent = NULL;
    camera->children = NULL;
    camera->start = NULL;
    camera->update = NULL;
    camera->input = NULL;
    camera->collide = NULL;

    camera->originalCenter.x = camera->center.x = centerX;
    camera->originalCenter.y = camera->center.y = centerY;
    camera->originalCenter.z = camera->center.z = -centerZ;
    camera->originalRadius = camera->radius = radius;

    camera->originalExtents.x = camera->extents.x = extentX;
    camera->originalExtents.y = camera->extents.y = extentY;
    camera->originalExtents.z = camera->extents.z = extentZ;

    if (rotX == 0.0 && rotY == 0.0 && rotZ == 0.0) rotZ = 1;
    camera->position.x = positionX;
    camera->position.y = positionY;
    camera->position.z = positionZ;
    camera->scale.x = 1.0f;
    camera->scale.y = 1.0f;
    camera->scale.z = 1.0f;
    camera->rotationAxis.x = rotX;
    camera->rotationAxis.y = rotY;
    camera->rotationAxis.z = rotZ;
    camera->rotationAngle = angle;

    return camera;
}

void CActor_LinkChildToParent(vector actors, Actor *child, Actor *parent)
{
    if (parent && child)
    {
        if (parent->children == NULL)
        {
            parent->children = vector_create();
        }

        vector_add(parent->children, child);

        child->parent = parent;
    }
}

Vector3 CActor_GetPosition(Actor *Actor)
{
    if (Actor == NULL)
        return (Vector3) { 0, 0, 0 };

    if (Actor->parent == NULL)
    {
        const int invertScalar = Actor->type == Camera ? -1 : 1;

        return (Vector3) { Actor->position.x, Actor->position.y, invertScalar * Actor->position.z };
    }

    return vec3_mul_mat(Actor->position, CActor_GetMatrix(Actor->parent));
}

Mtx CActor_GetMatrix(Actor *Actor)
{
    if (Actor == NULL)
    {
        Mtx mat;
        guMtxIdent(&mat);
        return mat;
    }

    Mtx combined;
    guMtxCatL(&Actor->transform.scale, &Actor->transform.rotation, &combined);

    // Need to rebuild translation matrix since the actor's stored one is scaled up.
    Mtx translation;
    const int invertScalar = Actor->type == Camera ? -1 : 1;

    guTranslate(&translation, Actor->position.x, Actor->position.y, invertScalar * Actor->position.z);
    guMtxCatL(&combined, &translation, &combined);

    if (Actor->parent == NULL)
    {
        return combined;
    }

    return mat_mul_mat(combined, CActor_GetMatrix(Actor->parent));
}

void CActor_UpdateAABB(Actor *Actor)
{
    float center[3] = { Actor->center.x, Actor->center.y, Actor->center.z };
    const float originalCenter[3] = { Actor->originalCenter.x, Actor->originalCenter.y, Actor->originalCenter.z };

    float extents[3] = { Actor->extents.x, Actor->extents.y, Actor->extents.z };
    const float originalExtents[3] = { Actor->originalExtents.x, Actor->originalExtents.y, Actor->originalExtents.z };

    float mat[4][4];
    Mtx fixedMat = CActor_GetMatrix(Actor);
    guMtxL2F(mat, &fixedMat);

    for (int i = 0; i < 3; i++)
    {
        center[i] = 0.0f;
        extents[i] = 0.0f;

        for (int j = 0; j < 3; j++)
        {
            center[i] += mat[j][i] * originalCenter[j];
            extents[i] += fabs(mat[j][i]) * originalExtents[j];
        }
    }

    Actor->center = (Vector3) { center[0], center[1], center[2] };
    Actor->extents = (Vector3) { extents[0], extents[1], extents[2] };
}

void CActor_UpdateSphere(Actor *Actor)
{
    Actor->center = vec3_mul_mat(Actor->originalCenter, Actor->transform.rotation);

    // Scale the calculated radius using the largest scale value of the actor.
    float scaleComps[3] = { fabs(Actor->scale.x), fabs(Actor->scale.y), fabs(Actor->scale.z) };
    float largestScale = 0;

    for (int i = 0; i < 3; i++)
    {
        if (scaleComps[i] > largestScale)
            largestScale = scaleComps[i];
    }

    Actor->radius = largestScale * Actor->originalRadius;
}
