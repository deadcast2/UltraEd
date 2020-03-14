#include <nusys.h>
#include <math.h>
#include "utilities.h"
#include "hashtable.h"
#include "actor.h"
#include "collision.h"
#include "scene.h"

// User generated includes.
#include "segments.h"
#include "actors.h"
#include "mappings.h"
#include "core.h"
#include "scripts.h"
#include "collisions.h"

#define SCREEN_WD 320
#define SCREEN_HT 240
#define GFX_GLIST_LEN 2048

char mem_heep[1024 * 512];
Gfx *glistp;
Gfx gfx_glist[GFX_GLIST_LEN];
transform world;
NUContData contdata[4];

static Vp view_port =
{
  SCREEN_WD * 2, SCREEN_HT * 2, G_MAXZ / 2, 0,
  SCREEN_WD * 2, SCREEN_HT * 2, G_MAXZ / 2, 0,
};

Gfx rsp_state[] =
{
  gsSPViewport(&view_port),
  gsSPClearGeometryMode(0xFFFFFFFF),
  gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK),
  gsSPTexture(0, 0, 0, 0, G_OFF),
  gsSPEndDisplayList()
};

Gfx rdp_state[] =
{
  gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2),
  gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
  gsDPSetScissor(G_SC_NON_INTERLACE, 0, 0, SCREEN_WD, SCREEN_HT),
  gsDPSetColorDither(G_CD_BAYER),
  gsSPEndDisplayList()
};

void rcp_init()
{
    gSPSegment(glistp++, 0, 0x0);
    gSPDisplayList(glistp++, OS_K0_TO_PHYSICAL(rsp_state));
    gSPDisplayList(glistp++, OS_K0_TO_PHYSICAL(rdp_state));
}

void clear_frame_buffer()
{
    unsigned int backgroundColor = GPACK_RGBA5551(_UER_SceneBackgroundColor[0],
        _UER_SceneBackgroundColor[1], _UER_SceneBackgroundColor[2], 1);

    gDPSetDepthImage(glistp++, OS_K0_TO_PHYSICAL(nuGfxZBuffer));
    gDPSetCycleType(glistp++, G_CYC_FILL);
    gDPSetColorImage(glistp++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD,
        OS_K0_TO_PHYSICAL(nuGfxZBuffer));
    gDPSetFillColor(glistp++, (GPACK_ZDZ(G_MAXFBZ, 0) << 16 |
        GPACK_ZDZ(G_MAXFBZ, 0)));
    gDPFillRectangle(glistp++, 0, 0, SCREEN_WD - 1, SCREEN_HT - 1);
    gDPPipeSync(glistp++);

    gDPSetColorImage(glistp++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD,
        osVirtualToPhysical(nuGfxCfb_ptr));
    gDPSetFillColor(glistp++, (backgroundColor << 16 | backgroundColor));
    gDPFillRectangle(glistp++, 0, 0, SCREEN_WD - 1, SCREEN_HT - 1);
    gDPPipeSync(glistp++);
}

void setup_world_matrix(Gfx **display_list)
{
    u16 persp_normal;

    guPerspective(&world.projection,
        &persp_normal,
        80.0F, SCREEN_WD / SCREEN_HT,
        0.1F, 1000.0F, 1.0F);

    gSPPerspNormalize((*display_list)++, persp_normal);

    gSPMatrix((*display_list)++, OS_K0_TO_PHYSICAL(&world.projection),
        G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);

    gSPMatrix((*display_list)++, OS_K0_TO_PHYSICAL(&world.rotation),
        G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

    gSPMatrix((*display_list)++, OS_K0_TO_PHYSICAL(&world.translation),
        G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
}

void create_display_list()
{
    glistp = gfx_glist;
    rcp_init();
    clear_frame_buffer();
    setup_world_matrix(&glistp);
    _UER_Draw(&glistp);
    gDPFullSync(glistp++);
    gSPEndDisplayList(glistp++);
    nuGfxTaskStart(gfx_glist, (s32)(glistp - gfx_glist) * sizeof(Gfx),
        NU_GFX_UCODE_F3DEX, NU_SC_SWAPBUFFER);
}

void check_inputs()
{
    nuContDataGetEx(contdata, 0);
    _UER_Input(contdata);
}

void update_camera()
{
    actor *camera = _UER_ActiveCamera;
    if (camera != NULL)
    {
        guTranslate(&world.translation, -camera->position->x, -camera->position->y, camera->position->z);
        guRotate(&world.rotation, camera->rotationAngle, camera->rotationAxis->x, camera->rotationAxis->y,
            -camera->rotationAxis->z);
    }
}

void gfx_callback(int pendingGfx)
{
    if (pendingGfx < 1)
    {
        create_display_list();
        check_inputs();
        update_camera();
        _UER_Update();
        _UER_Collide();
    }
}

int init_heap_memory()
{
    return InitHeap(mem_heep, sizeof(mem_heep));
}

void set_default_camera()
{
    for (int i = 0; i < _UER_ActorCount; i++)
    {
        if (_UER_Actors[i]->type == Camera)
        {
            _UER_ActiveCamera = _UER_Actors[i];
            break;
        }
    }
}

void mainproc()
{
    nuGfxInit();
    nuContInit();

    osViSetMode(&osViModeTable[OS_VI_NTSC_LAN1]);
    osViSetSpecialFeatures(OS_VI_DITHER_FILTER_ON | OS_VI_GAMMA_OFF 
        | OS_VI_GAMMA_DITHER_OFF | OS_VI_DIVOT_ON);
    nuGfxDisplayOff();

    if (init_heap_memory() > -1)
    {
        _UER_Load();
        set_default_camera();
        _UER_Mappings();
        _UER_Start();
    }

    nuGfxFuncSet((NUGfxFunc)gfx_callback);
    nuGfxDisplayOn();

    while (1) { }
}
