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
  int i = 0;
  int vertex_count = 0;
  struct sos_model *new_model;
  upng_t* png;
  
  // Transfer from ROM the model mesh data and texture.
  rom_2_ram(data_start, data_buffer, data_size);
  rom_2_ram(texture_start, texture_buffer, texture_size);
  
  new_model = (struct sos_model*)malloc(sizeof(struct sos_model));
  new_model->mesh = (struct mesh*)malloc(sizeof(struct mesh));
  new_model->position = (struct vector3*)malloc(sizeof(struct vector3));
  new_model->rotation = (struct vector3*)malloc(sizeof(struct vector3));
  new_model->scale = (struct vector3*)malloc(sizeof(struct vector3));
  
  // Read how many vertices for this mesh.
  line = (char*)strtok(data_buffer, "\n");
  sscanf(line, "%i", &vertex_count);
  new_model->mesh->vertices = (Vtx*)malloc(vertex_count * sizeof(Vtx));
  new_model->mesh->vertex_count = vertex_count;
  
  // Gather all of the X, Y, and Z vertex info.
  for(i = 0; i < vertex_count; i++) {
    double x, y, z, s, t;
    line = (char*)strtok(NULL, "\n");
    sscanf(line, "%lf %lf %lf %lf %lf", &x, &y, &z, &s, &t);
    
    new_model->mesh->vertices[i].v.ob[0] = x * 1000;
    new_model->mesh->vertices[i].v.ob[1] = y * 1000;
    new_model->mesh->vertices[i].v.ob[2] = z * 1000;
    new_model->mesh->vertices[i].v.flag = 0;
    new_model->mesh->vertices[i].v.tc[0] = (int)(s * 32) << 5;
    new_model->mesh->vertices[i].v.tc[1] = (int)(t * 32) << 5;
    new_model->mesh->vertices[i].v.cn[0] = 0;
    new_model->mesh->vertices[i].v.cn[1] = 0;
    new_model->mesh->vertices[i].v.cn[2] = 0;
    new_model->mesh->vertices[i].v.cn[3] = 0;
  }

  new_model->position->x = 0;
  new_model->position->y = -1;
  new_model->position->z = -5;
  new_model->scale->x = 0.001;
  new_model->scale->y = 0.001;
  new_model->scale->z = 0.001;
  new_model->rotation->x = 0;
  new_model->rotation->y = 0;
  new_model->rotation->z = 0;
  
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
  int i;
  int remaining_vertices = model->mesh->vertex_count;
  int offset = 0;
  
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
  
  // Send vertex data in batches of 30.
  while(remaining_vertices >= 30) {
    gSPVertex((*display_list)++, &(model->mesh->vertices[offset]), 30, 0);
    gDPPipeSync((*display_list)++);
    
    for(i = 0; i < 30 / 3; i++) {
      gSP1Triangle((*display_list)++, i * 3, i * 3 + 1, i * 3 + 2, 0);
    }
    
    remaining_vertices -= 30;
    offset += 30;
  }
  
  // Process last remaining vertices.
  if(remaining_vertices > 0) {
    gSPVertex((*display_list)++, &(model->mesh->vertices[offset]), remaining_vertices, 0);
    gDPPipeSync((*display_list)++);
    
    for(i = 0; i < remaining_vertices / 3; i++) {
      gSP1Triangle((*display_list)++, i * 3, i * 3 + 1, i * 3 + 2, 0);
    }
  }
  
  gSPPopMatrix((*display_list)++, G_MTX_MODELVIEW);
}
