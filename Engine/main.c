#include <nusys.h>
#include <math.h>
#include "utilities.h"
#include "hashtable.h"
#include "actor.h"
#include "collision.h"
#include "scene.h"
#include "vector.h"

// Generated includes.
#include "definitions.h"
#include "segments.h"
#include "actors.h"
#include "mappings.h"
#include "core.h"
#include "scripts.h"
#include "collisions.h"

#define SCREEN_WD 320
#define SCREEN_HT 240
#define GFX_GLIST_LEN 2048

char SystemHeap[1024 * 512];
Gfx *GfxListPointer;
Gfx GfxList[GFX_GLIST_LEN];
Transform World;
NUContData ControllerData[4];

static Vp ViewPort =
{
  SCREEN_WD * 2, SCREEN_HT * 2, G_MAXZ / 2, 0,
  SCREEN_WD * 2, SCREEN_HT * 2, G_MAXZ / 2, 0,
};

Gfx RspState[] =
{
  gsSPViewport(&ViewPort),
  gsSPClearGeometryMode(0xFFFFFFFF),
  gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK),
  gsSPTexture(0, 0, 0, 0, G_OFF),
  gsSPEndDisplayList()
};

Gfx RdpState[] =
{
  gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2),
  gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
  gsDPSetScissor(G_SC_NON_INTERLACE, 0, 0, SCREEN_WD, SCREEN_HT),
  gsDPSetColorDither(G_CD_BAYER),
  gsSPEndDisplayList()
};

void RcpInit()
{
    gSPSegment(GfxListPointer++, 0, 0x0);
    gSPDisplayList(GfxListPointer++, OS_K0_TO_PHYSICAL(RspState));
    gSPDisplayList(GfxListPointer++, OS_K0_TO_PHYSICAL(RdpState));
}

void ClearFrameBuffer()
{
    unsigned int backgroundColor = GPACK_RGBA5551(_UER_SceneBackgroundColor[0],
        _UER_SceneBackgroundColor[1], _UER_SceneBackgroundColor[2], 1);

    gDPSetDepthImage(GfxListPointer++, OS_K0_TO_PHYSICAL(nuGfxZBuffer));
    gDPSetCycleType(GfxListPointer++, G_CYC_FILL);
    gDPSetColorImage(GfxListPointer++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD,
        OS_K0_TO_PHYSICAL(nuGfxZBuffer));
    gDPSetFillColor(GfxListPointer++, (GPACK_ZDZ(G_MAXFBZ, 0) << 16 |
        GPACK_ZDZ(G_MAXFBZ, 0)));
    gDPFillRectangle(GfxListPointer++, 0, 0, SCREEN_WD - 1, SCREEN_HT - 1);
    gDPPipeSync(GfxListPointer++);

    gDPSetColorImage(GfxListPointer++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD,
        osVirtualToPhysical(nuGfxCfb_ptr));
    gDPSetFillColor(GfxListPointer++, (backgroundColor << 16 | backgroundColor));
    gDPFillRectangle(GfxListPointer++, 0, 0, SCREEN_WD - 1, SCREEN_HT - 1);
    gDPPipeSync(GfxListPointer++);
}

void SetupWorldMatrix(Gfx **display_list)
{
    u16 persp_normal;

    guPerspective(&World.projection,
        &persp_normal,
        80.0F, SCREEN_WD / SCREEN_HT,
        0.1F, 1000.0F, 1.0F);

    gSPPerspNormalize((*display_list)++, persp_normal);

    gSPMatrix((*display_list)++, OS_K0_TO_PHYSICAL(&World.projection),
        G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);

    gSPMatrix((*display_list)++, OS_K0_TO_PHYSICAL(&World.scale),
        G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

    gSPMatrix((*display_list)++, OS_K0_TO_PHYSICAL(&World.rotation),
        G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    gSPMatrix((*display_list)++, OS_K0_TO_PHYSICAL(&World.translation),
        G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
}

void Render()
{
    GfxListPointer = GfxList;

    RcpInit();
    ClearFrameBuffer();
    SetupWorldMatrix(&GfxListPointer);

    nuContDataGetEx(ControllerData, 0);
    _UER_ActorUpdate(&GfxListPointer, ControllerData);

    gDPFullSync(GfxListPointer++);
    gSPEndDisplayList(GfxListPointer++);
    nuGfxTaskStart(GfxList, (s32)(GfxListPointer - GfxList) * sizeof(Gfx), NU_GFX_UCODE_F3DEX, NU_SC_SWAPBUFFER);
}

void UpdateCamera()
{
    Actor *camera = _UER_ActiveCamera;

    if (camera != NULL)
    {
        guTranslate(&World.translation, -camera->position.x * SCALE_FACTOR, -camera->position.y * SCALE_FACTOR, camera->position.z * SCALE_FACTOR);
        guRotate(&World.rotation, camera->rotationAngle, camera->rotationAxis.x, camera->rotationAxis.y, camera->rotationAxis.z);
        guScale(&World.scale, 1, 1, 1);
    }
}

void GfxCallback(int pendingGfx)
{
    if (pendingGfx < 1)
    {
        Render();
        UpdateCamera();
        _UER_ActorCollision();
    }
}

int InitHeapMemory()
{
    return InitHeap(SystemHeap, sizeof(SystemHeap));
}

void SetDefaultCamera()
{
    for (int i = 0; i < vector_size(_UER_Actors); i++)
    {
        if (vector_get(_UER_Actors, i)->type == Camera)
        {
            _UER_ActiveCamera = vector_get(_UER_Actors, i);

            break;
        }
    }
}

void mainproc()
{
    nuGfxInit();
    nuContInit();

    osViSetMode(&osViModeTable[_UER_VIDEO_MODE]);
    osViSetSpecialFeatures(OS_VI_DITHER_FILTER_ON | OS_VI_GAMMA_OFF | OS_VI_GAMMA_DITHER_OFF | OS_VI_DIVOT_ON);
    nuGfxDisplayOff();

    if (InitHeapMemory() > -1)
    {
        _UER_Load();
        _UER_Mappings();
        _UER_Start();

        SetDefaultCamera();
    }

    nuGfxFuncSet((NUGfxFunc)GfxCallback);
    nuGfxDisplayOn();

    while (1) { }
}
