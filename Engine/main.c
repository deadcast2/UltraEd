#include <nusys.h>
#include <math.h>
#include "hashtable.h"
#include "actor.h"
#include "segments.h"
#include "actors.h"
#include "mappings.h"
#include "utilities.h"
#include "core.h"
#include "scripts.h"
#include "collisions.h"

#define GFX_GLIST_LEN 2048

char mem_heep[1024 * 512];
Gfx *glistp;
Gfx gfx_glist[GFX_GLIST_LEN];
struct transform world;
u16 perspNormal;
NUContData contdata[4];
int currentCamera = 0;

static Vp viewPort =
{
  SCREEN_WD * 2, SCREEN_HT * 2, G_MAXZ / 2, 0,
  SCREEN_WD * 2, SCREEN_HT * 2, G_MAXZ / 2, 0,
};

Gfx rspState[] =
{
  gsSPViewport(&viewPort),
  gsSPClearGeometryMode(0xFFFFFFFF),
  gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK),
  gsSPTexture(0, 0, 0, 0, G_OFF),
  gsSPEndDisplayList()
};

Gfx rdpState[] =
{
  gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2),
  gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
  gsDPSetScissor(G_SC_NON_INTERLACE, 0, 0, SCREEN_WD, SCREEN_HT),
  gsDPSetColorDither(G_CD_BAYER),
  gsSPEndDisplayList()
};

void rcpInit()
{
    gSPSegment(glistp++, 0, 0x0);
    gSPDisplayList(glistp++, OS_K0_TO_PHYSICAL(rspState));
    gSPDisplayList(glistp++, OS_K0_TO_PHYSICAL(rdpState));
}

void clearFramBuffer()
{
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
    gDPSetFillColor(glistp++, (GPACK_RGBA5551(207, 239, 252, 1) << 16 |
        GPACK_RGBA5551(207, 239, 252, 1)));
    gDPFillRectangle(glistp++, 0, 0, SCREEN_WD - 1, SCREEN_HT - 1);
    gDPPipeSync(glistp++);
}

void setup_world_matrix(Gfx **display_list)
{
    guPerspective(&world.projection,
        &perspNormal,
        80.0F, SCREEN_WD / SCREEN_HT,
        0.1F, 1000.0F, 1.0F);

    gSPPerspNormalize((*display_list)++, perspNormal);

    gSPMatrix((*display_list)++, OS_K0_TO_PHYSICAL(&world.projection),
        G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);

    gSPMatrix((*display_list)++, OS_K0_TO_PHYSICAL(&world.rotation),
        G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

    gSPMatrix((*display_list)++, OS_K0_TO_PHYSICAL(&world.translation),
        G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
}

void createDisplayList()
{
    glistp = gfx_glist;
    rcpInit();
    clearFramBuffer();
    setup_world_matrix(&glistp);
    _UER_Draw(&glistp);
    gDPFullSync(glistp++);
    gSPEndDisplayList(glistp++);
    nuGfxTaskStart(gfx_glist, (s32)(glistp - gfx_glist) * sizeof(Gfx),
        NU_GFX_UCODE_F3DEX, NU_SC_SWAPBUFFER);
}

void checkInputs()
{
    nuContDataGetEx(contdata, 0);
    _UER_Input(contdata);
}

void updateCamera()
{
    struct actor *camera = _UER_Actors[currentCamera];
    guTranslate(&world.translation, -camera->position->x, -camera->position->y, camera->position->z);
    guRotate(&world.rotation, camera->rotationAngle, camera->rotationAxis->x, camera->rotationAxis->y,
        -camera->rotationAxis->z);
}

void gfxCallback(int pendingGfx)
{
    checkInputs();
    updateCamera();
    _UER_Update();
    _UER_Collide();
    if (pendingGfx < 1) createDisplayList();
}

int initHeapMemory()
{
    if (InitHeap(mem_heep, sizeof(mem_heep)) == -1) return -1;
    return 0;
}

void setDefaultCamera()
{
    for (int i = 0; i < _UER_ActorCount; i++)
    {
        if (_UER_Actors[i]->type == Camera)
        {
            currentCamera = i;
            break;
        }
    }
}

void mainproc()
{
    nuGfxInit();
    nuContInit();

    if (initHeapMemory() > -1)
    {
        _UER_Load();
        setDefaultCamera();
        _UER_Mappings();
        _UER_Start();
    }

    while (1)
    {
        nuGfxFuncSet((NUGfxFunc)gfxCallback);
        nuGfxDisplayOn();
    }
}
