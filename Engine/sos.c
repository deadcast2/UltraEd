#include <nusys.h>
#include "upng.h"
#include "sos.h"

#define RGB5551(R, G, B) (((R >> 3) << 11) | ((G >> 3) << 6) | ((B >> 3) << 1) | 1)

void rom_2_ram(void *from_addr, void *to_addr, s32 seq_size) {
    // If size is odd-numbered, cannot send over PI, so make it even.
    if(seq_size & 0x00000001) seq_size++;
    nuPiReadRom((u32)from_addr, to_addr, seq_size);
}

unsigned short* image_24_to_16(const unsigned char* data,
                               const int size_x, const int size_y) {
    int x, y;
    unsigned short* temp = (unsigned short*)malloc(size_x * size_y * 2);

    for(y = 0; y < size_y; y++) {
        for(x = 0; x < size_x; x++) {
            temp[x + y * size_y] = RGB5551(data[x * 3 + y * size_y * 3],
                data[x * 3 + y * size_y * 3 + 1],
                data[x * 3 + y * size_y * 3 + 2]);
        }
    }

    return temp;
}

struct sos_model *load_sos_model(void *data_start, void *data_end,
                    void *texture_start, void *texture_end) {
    unsigned char data_buffer[200000];
    unsigned char texture_buffer[20000];
    char *line;
    int data_size = data_end - data_start;
    int texture_size = texture_end - texture_start;
    int i = 0, j = 0, mesh_count = 0;
    struct sos_model *new_model;
    upng_t* png;

    // Transfer from ROM the model mesh data and texture.
    rom_2_ram(data_start, data_buffer, data_size);
    rom_2_ram(texture_start, texture_buffer, texture_size);

    // Read how many meshes the model contains.
    //line = (char*)strtok(data_buffer, "\n");
    //sscanf(line, "%i", &mesh_count);

    new_model = (struct sos_model*)malloc(sizeof(struct sos_model));
    new_model->mesh_count = 1;
    new_model->meshes = (struct mesh*)malloc(1 * sizeof(struct mesh));
    new_model->position = (struct vector3*)malloc(sizeof(struct vector3));
    new_model->rotation = (struct vector3*)malloc(sizeof(struct vector3));
    new_model->scale = (struct vector3*)malloc(sizeof(struct vector3));

    for(i = 0; i < 1; i++) {
        int vertex_count = 0, uv_count = 0, poly_count = 0;

        // Read how many vertices for this mesh.
        line = (char*)strtok(data_buffer, "\n");
        sscanf(line, "%i", &vertex_count);
        new_model->meshes[i].vertices = (struct vector3*)malloc(vertex_count
                                            * sizeof(struct vector3));
        new_model->meshes[i].poly_count = vertex_count;

        // Gather all of the X, Y, and Z vertex info.
        for(j = 0; j < vertex_count; j++) {
            double x, y, z, s, t;
            line = (char*)strtok(NULL, "\n");
            sscanf(line, "%lf %lf %lf %lf %lf", &x, &y, &z, &s, &t);

            new_model->meshes[i].vertices[j].x = x;
            new_model->meshes[i].vertices[j].y = y;
            new_model->meshes[i].vertices[j].z = z;
            new_model->meshes[i].vertices[j].s = s;
            new_model->meshes[i].vertices[j].t = t;
        }

        // Read how many UVs for the model.
        /*line = (char*)strtok(NULL, "\n");
        sscanf(line, "%i", &uv_count);
        new_model->meshes[i].uvs = (struct vector2*)malloc(uv_count
                                        * sizeof(struct vector2));

        // Gather all of the UVs per face.
        for(j = 0; j < uv_count; j++) {
            double s, t;
            line = (char*)strtok(NULL, "\n");
            sscanf(line, "%lf %lf", &s, &t);

            new_model->meshes[i].uvs[j].x = s;
            // Invert the y coordinate so it's not upside down.
            new_model->meshes[i].uvs[j].y = 1.0 - t;
        }*/

        /*
        // Finally read all of the faces to construct.
        line = (char*)strtok(NULL, "\n");
        sscanf(line, "%d", &poly_count);
        new_model->meshes[i].poly_count = poly_count;
        new_model->meshes[i].indices = (int*)malloc(3 * poly_count * sizeof(int));

        // Gather the faces and what vertices they all need.
        for(j = 0; j < poly_count; j++) {
            int v1, v2, v3;
            line = (char*)strtok(NULL, "\n");
            sscanf(line, "%i %i %i", &v1, &v2, &v3);

            new_model->meshes[i].indices[j * 3] = v1;
            new_model->meshes[i].indices[j * 3 + 1] = v2;
            new_model->meshes[i].indices[j * 3 + 2] = v3;
        }*/

        // Post process the triangle information to get it in
        // a suitable, chunked format for the RSP.
        new_model->meshes[i].processed_vertices = (Vtx*)malloc(vertex_count * sizeof(Vtx));
        for(j = 0; j < vertex_count; j++) {
            /*struct vector3 vertex1 = new_model->meshes[i].vertices[new_model->meshes[i].indices[j * 3]];
            struct vector3 vertex2 = new_model->meshes[i].vertices[new_model->meshes[i].indices[j * 3 + 1]];
            struct vector3 vertex3 = new_model->meshes[i].vertices[new_model->meshes[i].indices[j * 3 + 2]];
            */

            struct vector3 vertex = new_model->meshes[i].vertices[j];

            new_model->meshes[i].processed_vertices[j].v.ob[0] = (int)(vertex.x * 1000);
            new_model->meshes[i].processed_vertices[j].v.ob[1] = (int)(vertex.y * 1000);
            new_model->meshes[i].processed_vertices[j].v.ob[2] = (int)(vertex.z * 1000);
            new_model->meshes[i].processed_vertices[j].v.flag = 0;
            new_model->meshes[i].processed_vertices[j].v.tc[0] = (int)(vertex.s * 32) << 5;
            new_model->meshes[i].processed_vertices[j].v.tc[1] = (int)(vertex.t * 32) << 5;
            new_model->meshes[i].processed_vertices[j].v.cn[0] = 0;
            new_model->meshes[i].processed_vertices[j].v.cn[1] = 0;
            new_model->meshes[i].processed_vertices[j].v.cn[2] = 0;
            new_model->meshes[i].processed_vertices[j].v.cn[3] = 0;

            /*
            new_model->meshes[i].processed_vertices[j * 3 + 1].v.ob[0] = vertex2.x * 1000;
            new_model->meshes[i].processed_vertices[j * 3 + 1].v.ob[1] = vertex2.y * 1000;
            new_model->meshes[i].processed_vertices[j * 3 + 1].v.ob[2] = vertex2.z * 1000;
            new_model->meshes[i].processed_vertices[j * 3 + 1].v.flag = 0;
            new_model->meshes[i].processed_vertices[j * 3 + 1].v.tc[0] = (int)(new_model->meshes[i].uvs[j * 3 + 1].x*32*2) << 5;
            new_model->meshes[i].processed_vertices[j * 3 + 1].v.tc[1] = (int)(new_model->meshes[i].uvs[j * 3 + 1].y*32*2) << 5;
            new_model->meshes[i].processed_vertices[j * 3 + 1].v.cn[0] = 0;
            new_model->meshes[i].processed_vertices[j * 3 + 1].v.cn[1] = 0;
            new_model->meshes[i].processed_vertices[j * 3 + 1].v.cn[2] = 0;
            new_model->meshes[i].processed_vertices[j * 3 + 1].v.cn[3] = 0;

            new_model->meshes[i].processed_vertices[j * 3 + 2].v.ob[0] = vertex3.x * 1000;
            new_model->meshes[i].processed_vertices[j * 3 + 2].v.ob[1] = vertex3.y * 1000;
            new_model->meshes[i].processed_vertices[j * 3 + 2].v.ob[2] = vertex3.z * 1000;
            new_model->meshes[i].processed_vertices[j * 3 + 2].v.flag = 0;
            new_model->meshes[i].processed_vertices[j * 3 + 2].v.tc[0] = (int)(new_model->meshes[i].uvs[j * 3 + 2].x*32*2) << 5;
            new_model->meshes[i].processed_vertices[j * 3 + 2].v.tc[1] = (int)(new_model->meshes[i].uvs[j * 3 + 2].y*32*2) << 5;
            new_model->meshes[i].processed_vertices[j * 3 + 2].v.cn[0] = 0;
            new_model->meshes[i].processed_vertices[j * 3 + 2].v.cn[1] = 0;
            new_model->meshes[i].processed_vertices[j * 3 + 2].v.cn[2] = 0;
            new_model->meshes[i].processed_vertices[j * 3 + 2].v.cn[3] = 0;
            */
        }
    }

    // Load in the png texture data.
    png = upng_new_from_bytes(texture_buffer, texture_size);
    if (png != NULL) {
        upng_decode(png);

        if (upng_get_error(png) == UPNG_EOK) {
            // Convert texture data from 24bpp to 16bpp in RGB5551 format.
            new_model->texture = image_24_to_16(upng_get_buffer(png), 32, 32);
        }

        upng_free(png);
    }

    return new_model;
}

void sos_draw(struct sos_model *model, Gfx **display_list) {
    int i, j;

    guTranslate(&model->transform.translation, model->position->x,
                model->position->y, model->position->z);

    guRotate(&model->transform.rotation_x, model->rotation->x, 1.0F, 0.0F, 0.0F);

    guRotate(&model->transform.rotation_y, model->rotation->y, 0.0F, 1.0F, 0.0F);

    guRotate(&model->transform.rotation_z, model->rotation->z, 0.0F, 0.0F, 1.0F);

    guScale(&model->transform.scale, model->scale->x,
            model->scale->y, model->scale->z);

    gSPMatrix((*display_list)++, OS_K0_TO_PHYSICAL(&model->transform.translation),
        G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);

    gSPMatrix((*display_list)++, OS_K0_TO_PHYSICAL(&model->transform.rotation_x),
        G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    gSPMatrix((*display_list)++, OS_K0_TO_PHYSICAL(&model->transform.rotation_y),
        G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    gSPMatrix((*display_list)++, OS_K0_TO_PHYSICAL(&model->transform.rotation_z),
        G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    gSPMatrix((*display_list)++, OS_K0_TO_PHYSICAL(&model->transform.scale),
        G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    gDPPipeSync((*display_list)++);

    gDPSetCycleType((*display_list)++, G_CYC_1CYCLE);
    gDPSetRenderMode((*display_list)++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
    gSPClearGeometryMode((*display_list)++, 0xFFFFFFFF);
    gSPSetGeometryMode((*display_list)++, G_SHADE | G_SHADING_SMOOTH | G_ZBUFFER | G_CULL_BACK);
    gDPSetTextureFilter((*display_list)++, G_TF_BILERP);
    gDPSetCombineMode((*display_list)++, G_CC_BLENDRGBA, G_CC_BLENDRGBA);
    gDPSetTexturePersp((*display_list)++, G_TP_PERSP);
    gSPTexture((*display_list)++, 0xffff, 0xffff, 0, G_TX_RENDERTILE, G_ON);
    gDPLoadTextureBlock((*display_list)++, model->texture, G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 32, 0,
        G_TX_WRAP, G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

    // Draw the triangles.
    for(i = 0; i < 1; i++) {
        int remaining_vertices = model->meshes[i].poly_count;
        int offset = 0;

        // Send vertex data in batches of 30.
        while(remaining_vertices >= 30) {
            gSPVertex((*display_list)++, &(model->meshes[i].processed_vertices[offset]), 30, 0);
            gDPPipeSync((*display_list)++);

            for(j = 0; j < 30 / 3; j++) {
                gSP1Triangle((*display_list)++, j * 3, j * 3 + 1, j * 3 + 2, 0);
            }

            remaining_vertices -= 30;
            offset += 30;
        }

        // Process last remaining vertices.
        if(remaining_vertices > 0) {
            gSPVertex((*display_list)++, &(model->meshes[i].processed_vertices[offset]), remaining_vertices, 0);
            gDPPipeSync((*display_list)++);

            for(j = 0; j < remaining_vertices / 3; j++) {
                gSP1Triangle((*display_list)++, j * 3, j * 3 + 1, j * 3 + 2, 0);
            }
        }
    }

    gSPPopMatrix((*display_list)++, G_MTX_MODELVIEW);
}
