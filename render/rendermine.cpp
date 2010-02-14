/*
THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
COPYRIGHT 1993-1999 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "descent.h"
#include "error.h"
#include "gamefont.h"
#include "texmap.h"
#include "rendermine.h"
#include "fastrender.h"
#include "rendershadows.h"
#include "transprender.h"
#include "renderthreads.h"
#include "glare.h"
#include "radar.h"
#include "objrender.h"
#include "textures.h"
#include "screens.h"
#include "segpoint.h"
#include "texmerge.h"
#include "physics.h"
#include "gameseg.h"
#include "light.h"
#include "dynlight.h"
#include "headlight.h"
#include "newdemo.h"
#include "automap.h"
#include "key.h"
#include "u_mem.h"
#include "kconfig.h"
#include "mouse.h"
#include "interp.h"
#include "ogl_lib.h"
#include "ogl_color.h"
#include "ogl_render.h"
#include "ogl_fastrender.h"
#include "cockpit.h"
#include "input.h"
#include "shadows.h"
#include "textdata.h"
#include "sparkeffect.h"
#include "createmesh.h"
#include "systemkeys.h"

#undef DBG
#define DBG 1

//------------------------------------------------------------------------------

#define LMAP_LIGHTADJUST	1

#define bPreDrawSegs			0

#define OLD_SEGLIST			1
#define SORT_RENDER_SEGS	0

// ------------------------------------------------------------------------------

#define CLEAR_WINDOW	0

int	nClearWindow = 0; //2	// 1 = Clear whole background tPortal, 2 = clear view portals into rest of world, 0 = no clear

void RenderSkyBox (int nWindow);

//------------------------------------------------------------------------------

extern int bLog;

CCanvas *reticleCanvas = NULL;

void _CDECL_ FreeReticleCanvas (void)
{
if (reticleCanvas) {
	PrintLog ("unloading reticle data\n");
	reticleCanvas->Destroy ();
	reticleCanvas = NULL;
	}
}

//------------------------------------------------------------------------------

#if 0

// Draw the reticle in 3D for head tracking
void Draw3DReticle (fix xStereoSeparation)
{
	g3sPoint 	reticlePoints [4];
	tUVL			tUVL [4];
	g3sPoint		*pointList [4];
	int 			i;
	CFixVector	v1, v2;
	int			saved_interp_method;

//	if (!transformation.m_info.bUsePlayerHeadAngles) return;

for (i = 0; i < 4; i++) {
	reticlePoints [i].p3_index = -1;
	pointList [i] = reticlePoints + i;
	tUVL [i].l = MAX_LIGHT;
	}
tUVL [0].u =
tUVL [0].v =
tUVL [1].v =
tUVL [3].u = 0;
tUVL [1].u =
tUVL [2].u =
tUVL [2].v =
tUVL [3].v = I2X (1);

v1 = gameData.objs.viewerP->info.position.vPos + gameData.objs.viewerP->info.position.mOrient.FVec () * (I2X (4));
v1 += gameData.objs.viewerP->info.position.mOrient.RVec () * xStereoSeparation;
v2 = v1 + gameData.objs.viewerP->info.position.mOrient.RVec () * (-I2X (1));
v2 += gameData.objs.viewerP->info.position.mOrient.UVec () * (I2X (1));
G3TransformAndEncodePoint(&reticlePoints [0], v2);
v2 = v1 + gameData.objs.viewerP->info.position.mOrient.RVec () * (+I2X (1));
v2 += gameData.objs.viewerP->info.position.mOrient.UVec () * (I2X (1));
G3TransformAndEncodePoint(&reticlePoints [1], v2);
v2 = v1 + gameData.objs.viewerP->info.position.mOrient.RVec () * (+I2X (1));
v2 += gameData.objs.viewerP->info.position.mOrient.UVec () * (-I2X (1));
G3TransformAndEncodePoint(&reticlePoints [2], v2);
v2 = v1 + gameData.objs.viewerP->info.position.mOrient.RVec () * (-I2X (1));
v2 += gameData.objs.viewerP->info.position.mOrient.UVec () * (-I2X (1));
G3TransformAndEncodePoint(&reticlePoints [3], v2);

if ( reticleCanvas == NULL) {
	reticleCanvas = CCanvas::Create (64, 64);
	if ( !reticleCanvas)
		Error ("Couldn't allocate reticleCanvas");
	//reticleCanvas->Bitmap ().nId = 0;
	reticleCanvas->SetFlags (BM_FLAG_TRANSPARENT);
	}

CCanvas::Push ();
CCanvas::SetCurrent (reticleCanvas);
reticleCanvas->Clear (0);		// Clear to Xparent
cockpit->DrawReticle (1);
CCanvas::Pop ();

saved_interp_method = gameStates.render.nInterpolationMethod;
gameStates.render.nInterpolationMethod	= 3;		// The best, albiet slowest.
G3DrawTexPoly (4, pointList, tUVL, reticleCanvas, NULL, 1, -1);
gameStates.render.nInterpolationMethod	= saved_interp_method;
}

#endif

//------------------------------------------------------------------------------

//cycle the flashing light for when mine destroyed
void FlashFrame (void)
{
	static fixang flash_ang = 0;

if (automap.Display ())
	return;
if (!(gameData.reactor.bDestroyed || gameStates.gameplay.seismic.nMagnitude)) {
	gameStates.render.nFlashScale = I2X (1);
	return;
	}
if (gameStates.app.bEndLevelSequence)
	return;
if (paletteManager.BlueEffect () > 10)		//whiting out
	return;
//	flash_ang += FixMul(FLASH_CYCLE_RATE, gameData.time.xFrame);
if (gameStates.gameplay.seismic.nMagnitude) {
	fix xAddedFlash = abs(gameStates.gameplay.seismic.nMagnitude);
	if (xAddedFlash < I2X (1))
		xAddedFlash *= 16;
	flash_ang += (fixang) FixMul (gameStates.render.nFlashRate, FixMul(gameData.time.xFrame, xAddedFlash+I2X (1)));
	FixFastSinCos (flash_ang, &gameStates.render.nFlashScale, NULL);
	gameStates.render.nFlashScale = (gameStates.render.nFlashScale + I2X (3))/4;	//	gets in range 0.5 to 1.0
	}
else {
	flash_ang += (fixang) FixMul (gameStates.render.nFlashRate, gameData.time.xFrame);
	FixFastSinCos (flash_ang, &gameStates.render.nFlashScale, NULL);
	gameStates.render.nFlashScale = (gameStates.render.nFlashScale + I2X (1))/2;
	if (gameStates.app.nDifficultyLevel == 0)
		gameStates.render.nFlashScale = (gameStates.render.nFlashScale+I2X (3))/4;
	}
}

// ----------------------------------------------------------------------------
//	Render a face.
//	It would be nice to not have to pass in nSegment and nSide, but
//	they are used for our hideously hacked in headlight system.
//	vp is a pointer to vertex ids.
//	tmap1, tmap2 are texture map ids.  tmap2 is the pasty one.

int RenderColoredSegFace (int nSegment, int nSide, int nVertices, g3sPoint **pointList)
{
	short nConnSeg = SEGMENTS [nSegment].m_children [nSide];
	int	owner = SEGMENTS [nSegment].m_owner;
	int	special = SEGMENTS [nSegment].m_nType;

if ((gameData.app.nGameMode & GM_ENTROPY) && (extraGameInfo [1].entropy.nOverrideTextures == 2) && (owner > 0)) {
	if ((nConnSeg >= 0) && (SEGMENTS [nConnSeg].m_owner == owner))
			return 0;
	if (owner == 1)
		G3DrawPolyAlpha (nVertices, pointList, segmentColors + 1, 0, nSegment);
	else
		G3DrawPolyAlpha (nVertices, pointList, segmentColors, 0, nSegment);
	return 1;
	}
if (special == SEGMENT_IS_WATER) {
	if ((nConnSeg < 0) || (SEGMENTS [nConnSeg].m_nType != SEGMENT_IS_WATER))
		G3DrawPolyAlpha (nVertices, pointList, segmentColors + 2, 0, nSegment);
	return 1;
	}
if (special == SEGMENT_IS_LAVA) {
	if ((nConnSeg < 0) || (SEGMENTS [nConnSeg].m_nType != SEGMENT_IS_LAVA))
		G3DrawPolyAlpha (nVertices, pointList, segmentColors + 3, 0, nSegment);
	return 1;
	}
return 0;
}

//------------------------------------------------------------------------------

int RenderWall (tFaceProps *propsP, g3sPoint **pointList, int bIsMonitor)
{
	short c, nWallNum = SEGMENTS [propsP->segNum].WallNum (propsP->sideNum);
	static tRgbaColorf cloakColor = {0, 0, 0, -1};

if (IS_WALL (nWallNum)) {
	if (propsP->widFlags & (WID_CLOAKED_FLAG | WID_TRANSPARENT_FLAG)) {
		if (!bIsMonitor) {
			if (!RenderColoredSegFace (propsP->segNum, propsP->sideNum, propsP->nVertices, pointList)) {
				c = WALLS [nWallNum].cloakValue;
				if (propsP->widFlags & WID_CLOAKED_FLAG) {
					if (c < FADE_LEVELS) {
						gameStates.render.grAlpha = GrAlpha (ubyte (c));
						G3DrawPolyAlpha (propsP->nVertices, pointList, &cloakColor, 1, propsP->segNum);		//draw as flat poly
						}
					}
				else {
					if (!gameOpts->render.color.bWalls)
						c = 0;
					if (WALLS [nWallNum].hps)
						gameStates.render.grAlpha = float (WALLS [nWallNum].hps) / float (I2X (100));
					else if (IsMultiGame && gameStates.app.bHaveExtraGameInfo [1])
						gameStates.render.grAlpha = COMPETITION ? 2.0f / 3.0f : GrAlpha (FADE_LEVELS - extraGameInfo [1].grWallTransparency);
					else
						gameStates.render.grAlpha = GrAlpha (ubyte (FADE_LEVELS - extraGameInfo [0].grWallTransparency));
					if (gameStates.render.grAlpha < 1.0f) {
						tRgbaColorf wallColor;
						
						paletteManager.Game ()->ToRgbaf ((ubyte) c, wallColor); 
						G3DrawPolyAlpha (propsP->nVertices, pointList, &wallColor, 1, propsP->segNum);	//draw as flat poly
						}
					}
				}
			gameStates.render.grAlpha = 1.0f;
			return 1;
			}
		}
	else if (gameStates.app.bD2XLevel) {
		c = WALLS [nWallNum].cloakValue;
		if (c && (c < FADE_LEVELS))
			gameStates.render.grAlpha = GrAlpha (FADE_LEVELS - c);
		}
	else if (gameOpts->render.effects.bAutoTransparency && IsTransparentFace (propsP))
		gameStates.render.grAlpha = 0.8f;
	else
		gameStates.render.grAlpha = 1.0f;
	}
return 0;
}

//------------------------------------------------------------------------------

void RenderFace (tFaceProps *propsP)
{
	tFaceProps	props = *propsP;
	CBitmap  *bmBot = NULL;
	CBitmap  *bmTop = NULL;

	int			i, bIsMonitor, bIsTeleCam, bHaveMonitorBg, nCamNum, bCamBufAvail;
	g3sPoint		*pointList [8], **pp;
	CSegment		*segP = SEGMENTS + props.segNum;
	CSide			*sideP = segP->m_sides + props.sideNum;
	CCamera		*cameraP = NULL;

if (props.nBaseTex < 0)
	return;
if (gameStates.render.nShadowPass == 2) {
#if DBG_SHADOWS
	if (!bWallShadows)
		return;
#endif
	G3SetCullAndStencil (0, 0);
	RenderFaceShadow (propsP);
	G3SetCullAndStencil (1, 0);
	RenderFaceShadow (propsP);
	return;
	}
#if DBG //convenient place for a debug breakpoint
if (props.segNum == nDbgSeg && ((nDbgSide < 0) || (props.sideNum == nDbgSide)))
	props.segNum = props.segNum;
if (props.nBaseTex == nDbgBaseTex)
	props.segNum = props.segNum;
if (props.nOvlTex == nDbgOvlTex)
	props.segNum = props.segNum;
#	if 0
else
	return;
#	endif
#endif

gameData.render.vertexList = gameData.segs.fVertices.Buffer ();
Assert(props.nVertices <= 4);
for (i = 0, pp = pointList; i < props.nVertices; i++, pp++)
	*pp = gameData.segs.points + props.vp [i];
if (!(gameOpts->render.debug.bTextures || IsMultiGame))
	goto drawWireFrame;
#if 1
if (gameStates.render.nShadowBlurPass == 1) {
	G3DrawWhitePoly (props.nVertices, pointList);
	gameData.render.vertexList = NULL;
	return;
	}
#endif
SetVertexColors (&props);
if (gameStates.render.nType == 2) {
#if DBG //convenient place for a debug breakpoint
	if (props.segNum == nDbgSeg && ((nDbgSide < 0) || (props.sideNum == nDbgSide)))
		props.segNum = props.segNum;
#endif
	RenderColoredSegFace (props.segNum, props.sideNum, props.nVertices, pointList);
	gameData.render.vertexList = NULL;
	return;
	}
nCamNum = IsMonitorFace (props.segNum, props.sideNum, 0);
if ((bIsMonitor = gameStates.render.bUseCameras && (nCamNum >= 0))) {
	cameraP = cameraManager.Camera (nCamNum);
	cameraP->SetVisible (1);
	bIsTeleCam = cameraP->GetTeleport ();
#if RENDER2TEXTURE
	bCamBufAvail = cameraP->HaveBuffer (1) == 1;
#else
	bCamBufAvail = 0;
#endif
	bHaveMonitorBg = cameraP->Valid () && /*!cameraP->bShadowMap &&*/
						  (cameraP->HaveTexture () || bCamBufAvail) &&
						  (!bIsTeleCam || EGI_FLAG (bTeleporterCams, 0, 1, 0));
	}
else
	bIsTeleCam =
	bHaveMonitorBg =
	bCamBufAvail = 0;
if (RenderWall (&props, pointList, bIsMonitor)) {	//handle semi-transparent walls
	gameData.render.vertexList = NULL;
	return;
	}
if (props.widFlags & WID_RENDER_FLAG) {
	if (props.nBaseTex >= gameData.pig.tex.nTextures [gameStates.app.bD1Data]) {
	sideP->m_nBaseTex = 0;
	}
if (!(bHaveMonitorBg && gameOpts->render.cameras.bFitToWall)) {
	if (gameStates.render.nType == 3) {
		bmBot = bmpCorona;
		bmTop = NULL;
		props.uvls [0].u =
		props.uvls [0].v =
		props.uvls [1].v =
		props.uvls [3].u = I2X (1) / 4;
		props.uvls [1].u =
		props.uvls [2].u =
		props.uvls [2].v =
		props.uvls [3].v = I2X (3) / 4;
		}
	else if (gameOpts->ogl.bGlTexMerge) {
		bmBot = LoadFaceBitmap (props.nBaseTex, sideP->m_nFrame);
		if (props.nOvlTex)
			bmTop = LoadFaceBitmap ((short) (props.nOvlTex), sideP->m_nFrame);
		}
	else {
		if (props.nOvlTex != 0) {
			bmBot = TexMergeGetCachedBitmap (props.nBaseTex, props.nOvlTex, props.nOvlOrient);
#if DBG
			if (!bmBot)
				bmBot = TexMergeGetCachedBitmap (props.nBaseTex, props.nOvlTex, props.nOvlOrient);
#endif
			}
		else {
			bmBot = gameData.pig.tex.bitmapP + gameData.pig.tex.bmIndexP [props.nBaseTex].index;
			LoadBitmap (gameData.pig.tex.bmIndexP [props.nBaseTex].index, gameStates.app.bD1Mission);
			}
		}
	}

if (bHaveMonitorBg) {
	cameraP->GetUVL (NULL, props.uvls, NULL, NULL);
	if (bIsTeleCam) {
#if DBG
		bmBot = &cameraP->Texture ();
		gameStates.render.grAlpha = 1.0f;
#else
		bmTop = &cameraP->Texture ();
		gameStates.render.grAlpha = 0.7f;
#endif
		}
	else if (gameOpts->render.cameras.bFitToWall || (props.nOvlTex > 0))
		bmBot = &cameraP->Texture ();
	else
		bmTop = &cameraP->Texture ();
	}
SetFaceLight (&props);
#if DBG //convenient place for a debug breakpoint
if (props.segNum == nDbgSeg && props.sideNum == nDbgSide)
	props.segNum = props.segNum;
#endif
#if DBG
if (bmTop)
	fpDrawTexPolyMulti (
		props.nVertices, pointList, props.uvls,
		props.uvl_lMaps,
		bmBot, bmTop,
		NULL, //lightmaps + props.segNum * 6 + props.sideNum,
		&props.vNormal, props.nOvlOrient, !bIsMonitor || bIsTeleCam, props.segNum);
else
	fpDrawTexPolyMulti (
		props.nVertices, pointList, props.uvls, props.uvl_lMaps, bmBot, NULL,
		NULL, //lightmaps + props.segNum * 6 + props.sideNum,
		&props.vNormal, 0, !bIsMonitor || bIsTeleCam, props.segNum);
#else
fpDrawTexPolyMulti (
	props.nVertices, pointList, props.uvls,
	props.uvl_lMaps,
	bmBot, bmTop,
	NULL, //lightmaps + props.segNum * 6 + props.sideNum,
	&props.vNormal, props.nOvlOrient, !bIsMonitor || bIsTeleCam,
	props.segNum);
#endif
	}
gameStates.render.grAlpha = 1.0f;
ogl.m_states.fAlpha = 1;
	// render the CSegment the CPlayerData is in with a transparent color if it is a water or lava CSegment
	//if (nSegment == OBJECTS->nSegment)
#if DBG
if (bOutLineMode)
	DrawOutline (props.nVertices, pointList);
#endif

drawWireFrame:

if (gameOpts->render.debug.bWireFrame && !IsMultiGame)
	DrawOutline (props.nVertices, pointList);
}

fix	Tulate_min_dot = (I2X (1)/4);
//--unused-- fix	Tulate_min_ratio = (I2X (2));
fix	Min_n0_n1_dot	= (I2X (15)/16);

// -----------------------------------------------------------------------------------
//	Render a side.
//	Check for Normal facing.  If so, render faces on CSide dictated by sideP->m_nType.

#undef LMAP_LIGHTADJUST
#define LMAP_LIGHTADJUST 0

void RenderSide (CSegment *segP, short nSide)
{
	CSide			*sideP = segP->m_sides + nSide;
	tFaceProps	props;

#define	LMAP_SIZE	(1.0 / 16.0)

	static tUVL	uvl_lMaps [4] = {
	 {F2X (LMAP_SIZE), F2X (LMAP_SIZE), 0}, 
	 {F2X (1.0 - LMAP_SIZE), F2X (LMAP_SIZE), 0}, 
	 {F2X (1.0 - LMAP_SIZE), F2X (1.0 - LMAP_SIZE), 0}, 
	 {F2X (LMAP_SIZE), F2X (1.0 - LMAP_SIZE), 0}
	};

props.segNum = segP->Index ();
props.sideNum = nSide;
#if DBG
if ((props.segNum == nDbgSeg) && ((nDbgSide < 0) || (props.sideNum == nDbgSide)))
	segP = segP;
#endif
props.widFlags = segP->IsDoorWay (props.sideNum, NULL);
if (!(gameOpts->render.debug.bWalls || IsMultiGame) && IS_WALL (segP->WallNum (props.sideNum)))
	return;
switch (gameStates.render.nType) {
	case -1:
		if (!(props.widFlags & WID_RENDER_FLAG) && (SEGMENTS [props.segNum].m_nType < SEGMENT_IS_WATER))		//if (WALL_IS_DOORWAY(segP, props.sideNum) == WID_NO_WALL)
			return;
		break;
	case 0:
		if (segP->m_children [props.sideNum] >= 0) //&& IS_WALL (WallNumP (segP, props.sideNum)))
			return;
		break;
	case 1:
		if (!IS_WALL (segP->WallNum (props.sideNum)))
			return;
		break;
	case 2:
		if ((SEGMENTS [props.segNum].m_nType < SEGMENT_IS_WATER) &&
			 (SEGMENTS [props.segNum].m_owner < 1))
			return;
		break;
	case 3:
		if ((IsLight (sideP->m_nBaseTex) || (sideP->m_nOvlTex && IsLight (sideP->m_nOvlTex))))
			glareRenderer.Render (props.segNum, props.sideNum, 1, 20);
		return;
	}
if (gameStates.render.bDoLightmaps) {
		float	Xs = 8;
		float	h = 0.5f / (float) Xs;

	props.uvl_lMaps [0].u =
	props.uvl_lMaps [0].v =
	props.uvl_lMaps [1].v =
	props.uvl_lMaps [3].u = F2X (h);
	props.uvl_lMaps [1].u =
	props.uvl_lMaps [2].u =
	props.uvl_lMaps [2].v =
	props.uvl_lMaps [3].v = F2X (1-h);
	}
props.nBaseTex = sideP->m_nBaseTex;
props.nOvlTex = sideP->m_nOvlTex;
props.nOvlOrient = sideP->m_nOvlOrient;

	//	========== Mark: Here is the change...beginning here: ==========

	if (gameStates.render.bDoLightmaps) {
		memcpy (props.uvl_lMaps, uvl_lMaps, sizeof (tUVL) * 4);
		props.uvls [0].l = props.uvls [1].l = props.uvls [2].l = props.uvls [3].l = I2X (1) / 2;
		}

#if DBG //convenient place for a debug breakpoint
if (props.segNum == nDbgSeg && props.sideNum == nDbgSide)
	props.segNum = props.segNum;
if (props.nBaseTex == nDbgBaseTex)
	props.segNum = props.segNum;
if (props.nOvlTex == nDbgOvlTex)
	props.segNum = props.segNum;
#	if 0
else
	return;
#	endif
#endif

if (!FaceIsVisible (props.segNum, props.sideNum))
	return;
if (sideP->m_nType == SIDE_IS_QUAD) {
	props.vNormal = sideP->m_normals [0];
	props.nVertices = 4;
	memcpy (props.uvls, sideP->m_uvls, sizeof (tUVL) * 4);
	memcpy (props.vp, SEGMENTS [props.segNum].Corners (props.sideNum), 4 * sizeof (ushort));
	RenderFace (&props);
	}
else {
	// new code
	// non-planar faces are still passed as quads to the renderer as it will render triangles (GL_TRIANGLE_FAN) anyway
	// just need to make sure the vertices come in the proper order depending of the the orientation of the two non-planar triangles
	props.vNormal = sideP->m_normals [0] + sideP->m_normals [1];
	props.vNormal *= (I2X (1) / 2);
	props.nVertices = 4;
	if (sideP->m_nType == SIDE_IS_TRI_02) {
		memcpy (props.uvls, sideP->m_uvls, sizeof (tUVL) * 4);
		memcpy (props.vp, SEGMENTS [props.segNum].Corners (props.sideNum), 4 * sizeof (ushort));
		RenderFace (&props);
		}
	else if (sideP->m_nType == SIDE_IS_TRI_13) {	//just rendering the fan with vertex 1 instead of 0
		memcpy (props.uvls + 1, sideP->m_uvls, sizeof (tUVL) * 3);
		props.uvls [0] = sideP->m_uvls [3];
		memcpy (props.vp + 1, SEGMENTS [props.segNum].Corners (props.sideNum), 4 * sizeof (ushort));
		props.vp [0] = props.vp [4];
		RenderFace (&props);
		}
	else {
		Error("Illegal CSide nType in RenderSide, nType = %i, CSegment # = %i, CSide # = %i\n", sideP->m_nType, segP->Index (), props.sideNum);
		return;
		}
	}
}

// -----------------------------------------------------------------------------------

static int RenderSegmentFaces (short nSegment, int nWindow)
{
	CSegment		*segP = SEGMENTS + nSegment;
	g3sCodes 	cc;
	short			nSide;

ogl.SetupTransform (0);
cc = RotateVertexList (8, segP->m_verts);
gameData.render.vertP = gameData.segs.fVertices.Buffer ();
//	return;
if (cc.ccAnd /*&& !automap.Display ()*/)	//all off screen and not rendering the automap
	return 0;
gameStates.render.nState = 0;
#if DBG //convenient place for a debug breakpoint
if (nSegment == nDbgSeg)
	nSegment = nSegment;
#endif
lightManager.SetNearestToSegment (nSegment, -1, 0, 0, 0);
for (nSide = 0; nSide < 6; nSide++) //segP->nFaces, faceP = segP->pFaces; nSide; nSide--, faceP++)
	RenderSide (segP, nSide);
ogl.ResetTransform (0);
//ogl.BindTexture (0);
return 1;
}

//------------------------------------------------------------------------------

void DoRenderObject (int nObject, int nWindow)
{
	CObject*					objP = OBJECTS + nObject;
	int						count = 0;

if (!(IsMultiGame || gameOpts->render.debug.bObjects))
	return;
Assert(nObject < LEVEL_OBJECTS);
#if 0
if (!(nWindow || gameStates.render.cameras.bActive) && (gameStates.render.nShadowPass < 2) &&
    (gameData.render.mine.bObjectRendered [nObject] == gameStates.render.nFrameFlipFlop))	//already rendered this...
	return;
#endif
if (gameData.demo.nState == ND_STATE_PLAYBACK) {
	if ((nDemoDoingLeft == 6 || nDemoDoingRight == 6) && objP->info.nType == OBJ_PLAYER) {
  		return;
		}
	}
if ((count++ > LEVEL_OBJECTS) || (objP->info.nNextInSeg == nObject)) {
	Int3();					// infinite loop detected
	objP->info.nNextInSeg = -1;		// won't this clean things up?
	return;					// get out of this infinite loop!
	}
if (RenderObject (objP, nWindow, 0)) {
	gameData.render.mine.bObjectRendered [nObject] = gameStates.render.nFrameFlipFlop;
	if (!gameStates.render.cameras.bActive) {
		tWindowRenderedData*	wrd = windowRenderedData + nWindow;
		int nType = objP->info.nType;
		if ((nType == OBJ_ROBOT) || (nType == OBJ_PLAYER) ||
			 ((nType == OBJ_WEAPON) && (WeaponIsPlayerMine (objP->info.nId) || (gameData.objs.bIsMissile [objP->info.nId] && EGI_FLAG (bKillMissiles, 0, 0, 0))))) {
			if (wrd->nObjects >= MAX_RENDERED_OBJECTS) {
				Int3();
				wrd->nObjects /= 2;
				}
			wrd->renderedObjects [wrd->nObjects++] = nObject;
			}
		}
	}
for (int i = objP->info.nAttachedObj; i != -1; i = objP->cType.explInfo.attached.nNext) {
	objP = OBJECTS + i;
	Assert (objP->info.nType == OBJ_FIREBALL);
	Assert (objP->info.controlType == CT_EXPLOSION);
	Assert (objP->info.nFlags & OF_ATTACHED);
	RenderObject (objP, nWindow, 1);
	}
}

//------------------------------------------------------------------------------

static tObjRenderListItem objRenderList [MAX_OBJECTS_D2X];

void QSortObjRenderList (int left, int right)
{
	int	l = left,
			r = right,
			m = objRenderList [(l + r) / 2].xDist;

do {
	while (objRenderList [l].xDist < m)
		l++;
	while (objRenderList [r].xDist > m)
		r--;
	if (l <= r) {
		if (l < r) {
			tObjRenderListItem h = objRenderList [l];
			objRenderList [l] = objRenderList [r];
			objRenderList [r] = h;
			}
		l++;
		r--;
		}
	} while (l <= r);
if (l < right)
	QSortObjRenderList (l, right);
if (left < r)
	QSortObjRenderList (left, r);
}

//------------------------------------------------------------------------------

int SortObjList (int nSegment)
{
	tObjRenderListItem*	pi;
	int						i, j;

if (nSegment < 0)
	nSegment = -nSegment - 1;
for (i = gameData.render.mine.renderObjs.ref [nSegment], j = 0; i >= 0; i = pi->nNextItem) {
	pi = gameData.render.mine.renderObjs.objs + i;
	objRenderList [j++] = *pi;
	}
#if 1
if (j > 1)
	QSortObjRenderList (0, j - 1);
#endif
return j;
}

//------------------------------------------------------------------------------

void RenderObjList (int nListPos, int nWindow)
{
PROF_START
	int i, j;
	int saveLinDepth = gameStates.render.detail.nMaxLinearDepth;

gameStates.render.detail.nMaxLinearDepth = gameStates.render.detail.nMaxLinearDepthObjects;
for (i = 0, j = SortObjList (gameData.render.mine.nSegRenderList [nListPos]); i < j; i++)
	DoRenderObject (objRenderList [i].nObject, nWindow);	// note link to above else
gameStates.render.detail.nMaxLinearDepth = saveLinDepth;
PROF_END(ptRenderObjects)
}

//------------------------------------------------------------------------------

void RenderSegment (int nListPos)
{
	int nSegment = (nListPos < 0) ? -nListPos - 1 : gameData.render.mine.nSegRenderList [nListPos];

if (nSegment < 0)
	return;
if (automap.Display ()) {
	if (!(automap.m_bFull || automap.m_visited [0][nSegment]))
		return;
	if (!gameOpts->render.automap.bSkybox && (SEGMENTS [nSegment].m_nType == SEGMENT_IS_SKYBOX))
		return;
	}
else {
	if (VISITED (nSegment))
		return;
	}
#if DBG
if (nSegment == nDbgSeg)
	nSegment = nSegment;
#endif
VISIT (nSegment);
if (!RenderSegmentFaces (nSegment, gameStates.render.nWindow)) {
	gameData.render.mine.nSegRenderList [nListPos] = -gameData.render.mine.nSegRenderList [nListPos] - 1;
	gameData.render.mine.bVisible [gameData.render.mine.nSegRenderList [nListPos]] = gameData.render.mine.nVisible - 1;
	return;
	}
if ((gameStates.render.nType == 0) && !automap.Display ())
	automap.m_visited [0][nSegment] = gameData.render.mine.bSetAutomapVisited;
else if ((gameStates.render.nType == 1) && (gameData.render.mine.renderObjs.ref [gameData.render.mine.nSegRenderList [nListPos]] >= 0)) {
#if DBG
	if (nSegment == nDbgSeg)
		nSegment = nSegment;
#endif
	lightManager.SetNearestStatic (nSegment, 1, 1, 0);
	gameStates.render.bApplyDynLight = (gameStates.render.nLightingMethod != 0) && (gameOpts->ogl.bObjLighting || gameOpts->ogl.bLightObjects);
	RenderObjList (nListPos, gameStates.render.nWindow);
	gameStates.render.bApplyDynLight = (gameStates.render.nLightingMethod != 0);
	//gameData.render.lights.dynamic.shader.index [0][0].nActive = gameData.render.lights.dynamic.shader.iStaticLights [0];
	}
else if (gameStates.render.nType == 2)	// render objects containing transparency, like explosions
	RenderObjList (nListPos, gameStates.render.nWindow);
}

//------------------------------------------------------------------------------
//increment counter for checking if points bTransformed
//This must be called at the start of the frame if RotateVertexList() will be used
void RenderStartFrame (void)
{
if (!++gameStates.render.nFrameCount) {		//wrap!
	gameData.render.mine.nRotatedLast.Clear (0);		//clear all to zero
	gameStates.render.nFrameCount = 1;											//and set this frame to 1
	}
}

//------------------------------------------------------------------------------

void SetRenderView (fix xStereoSeparation, short *nStartSegP, int bOglScale)
{
	short nStartSeg;
	bool	bPlayer = gameData.objs.viewerP == gameData.objs.consoleP;

gameData.render.mine.viewerEye = gameData.objs.viewerP->info.position.vPos;
if (xStereoSeparation && bPlayer)
	gameData.render.mine.viewerEye += gameData.objs.viewerP->info.position.mOrient.RVec () * xStereoSeparation;

externalView.SetPos (NULL);
if (gameStates.render.cameras.bActive) {
	nStartSeg = gameData.objs.viewerP->info.nSegment;
	G3SetViewMatrix (gameData.render.mine.viewerEye, gameData.objs.viewerP->info.position.mOrient, gameStates.render.xZoom, bOglScale, xStereoSeparation);
	}
else {
	if (!gameStates.render.nWindow && (bPlayer))
		externalView.SetPoint (gameData.objs.viewerP);
	if ((bPlayer) && transformation.m_info.bUsePlayerHeadAngles) {
		CFixMatrix mHead = CFixMatrix::Create (transformation.m_info.playerHeadAngles);
		CFixMatrix mView = gameData.objs.viewerP->info.position.mOrient * mHead;
		G3SetViewMatrix (gameData.render.mine.viewerEye, mView, gameStates.render.xZoom, bOglScale, xStereoSeparation);
		}
	else if (gameStates.render.bRearView && (bPlayer)) {
#if 1
		CFixMatrix mView;

		mView = gameData.objs.viewerP->info.position.mOrient;
		mView.FVec ().Neg ();
		mView.RVec ().Neg ();
#else
		CFixMatrix mHead, mView;

		transformation.m_info.playerHeadAngles [PA] = 0;
		transformation.m_info.playerHeadAngles [BA] = 0x7fff;
		transformation.m_info.playerHeadAngles [HA] = 0x7fff;
		VmAngles2Matrix (&mHead, &transformation.m_info.playerHeadAngles);
		VmMatMul (&mView, &gameData.objs.viewerP->info.position.mOrient, &mHead);
#endif
		G3SetViewMatrix (gameData.render.mine.viewerEye, mView,  //gameStates.render.xZoom, bOglScale);
							  FixDiv (gameStates.render.xZoom, gameStates.zoom.nFactor), bOglScale, xStereoSeparation);
		}
	else if ((bPlayer) && (!IsMultiGame || gameStates.app.bHaveExtraGameInfo [1])) {
		gameStates.zoom.nMinFactor = I2X (gameStates.render.glAspect); 
		gameStates.zoom.nMaxFactor = gameStates.zoom.nMinFactor * 5;
		HandleZoom ();
		if ((bPlayer) &&
#if DBG
			 gameStates.render.bChaseCam) {
#else
			 gameStates.render.bChaseCam && (!IsMultiGame || IsCoopGame || (EGI_FLAG (bEnableCheats, 0, 0, 0) && !COMPETITION))) {
#endif
			externalView.GetViewPoint ();
			if (xStereoSeparation)
				gameData.render.mine.viewerEye += gameData.objs.viewerP->info.position.mOrient.RVec () * xStereoSeparation;
			G3SetViewMatrix (gameData.render.mine.viewerEye,
								  externalView.GetPos () ? externalView.GetPos ()->mOrient : gameData.objs.viewerP->info.position.mOrient,
								  gameStates.render.xZoom, bOglScale, xStereoSeparation);
			}
		else
			G3SetViewMatrix (gameData.render.mine.viewerEye, gameData.objs.viewerP->info.position.mOrient,
								  FixDiv (gameStates.render.xZoom, gameStates.zoom.nFactor), bOglScale, xStereoSeparation);
		}
	else
		G3SetViewMatrix (gameData.render.mine.viewerEye, gameData.objs.viewerP->info.position.mOrient,
							  gameStates.render.xZoom, bOglScale, xStereoSeparation);
	if (!nStartSegP)
		nStartSeg = gameStates.render.nStartSeg;
	else if (0 > (nStartSeg = FindSegByPos (gameData.render.mine.viewerEye, gameData.objs.viewerP->info.nSegment, 1, 0)))
		nStartSeg = gameData.objs.viewerP->info.nSegment;
	}
if (nStartSegP)
	*nStartSegP = nStartSeg;
}

//------------------------------------------------------------------------------

int BeginRenderMine (short nStartSeg, fix xStereoSeparation, int nWindow)
{
PROF_START
if (!nWindow)
	GetPlayerMslLock ();	// uses rendered object info from previous frame stored in windowRenderedData
if (!gameStates.render.cameras.bActive)
	windowRenderedData [nWindow].nObjects = 0;
ogl.m_states.fAlpha = FADE_LEVELS;
if (((gameStates.render.nRenderPass <= 0) && (gameStates.render.nShadowPass < 2) && (gameStates.render.nShadowBlurPass < 2)) || gameStates.render.bShadowMaps) {
	if (!automap.Display ())
		RenderStartFrame ();
	}
if ((gameStates.render.nRenderPass <= 0) && (gameStates.render.nShadowPass < 2)) {
	ogl.m_states.bUseTransform = 1;
	BuildRenderSegList (nStartSeg, nWindow);		//fills in gameData.render.mine.nSegRenderList & gameData.render.mine.nRenderSegs
	if ((gameStates.render.nRenderPass <= 0) && (gameStates.render.nShadowPass < 2)) {
		BuildRenderObjLists (gameData.render.mine.nRenderSegs);
		if (xStereoSeparation <= 0)	// Do for left eye or zero.
			SetDynamicLight ();
		}
	ogl.m_states.bUseTransform = 0;
	lightManager.Transform (0, 1);
	}
gameStates.render.bFullBright = automap.Display () && gameOpts->render.automap.bBright;
ogl.m_states.bStandardContrast = gameStates.app.bNostalgia || IsMultiGame || (ogl.m_states.nContrast == 8);
ogl.m_states.bScaleLight = EGI_FLAG (bShadows, 0, 1, 0) && (gameStates.render.nShadowPass < 3) && !FAST_SHADOWS;
gameStates.render.bUseCameras = USE_CAMERAS;
PROF_END(ptAux);
return !gameStates.render.cameras.bActive && (gameData.objs.viewerP->info.nType != OBJ_ROBOT);
}

//------------------------------------------------------------------------------

void RenderSkyBoxObjects (void)
{
PROF_START
	short		i, nObject;
	short		*segNumP;

gameStates.render.nType = 1;
for (i = gameData.segs.skybox.ToS (), segNumP = gameData.segs.skybox.Buffer (); i; i--, segNumP++)
	for (nObject = SEGMENTS [*segNumP].m_objects; nObject != -1; nObject = OBJECTS [nObject].info.nNextInSeg)
		DoRenderObject (nObject, gameStates.render.nWindow);
PROF_END(ptRenderObjects)
}

//------------------------------------------------------------------------------

void RenderSkyBox (int nWindow)
{
PROF_START
if (gameStates.render.bHaveSkyBox && (!automap.Display () || gameOpts->render.automap.bSkybox)) {
	ogl.SetDepthWrite (true);
	RenderSkyBoxFaces ();
	RenderSkyBoxObjects ();
	}
PROF_END(ptRenderPass)
}

//------------------------------------------------------------------------------

void RenderMineObjects (int nType)
{
	int	nListPos, nSegLights = 0;
	short	nSegment;

#if DBG
if (!gameOpts->render.debug.bObjects)
	return;
#endif
if (nType != 1)
	return;
gameStates.render.nState = 1;
for (nListPos = gameData.render.mine.nRenderSegs; nListPos; ) {
	nSegment = gameData.render.mine.nSegRenderList [--nListPos];
	if (nSegment < 0) {
		if (nSegment == -0x7fff)
			continue;
		nSegment = -nSegment - 1;
		}
#if DBG
	if (nSegment == nDbgSeg)
		nSegment = nSegment;
#endif
	if (0 > gameData.render.mine.renderObjs.ref [nSegment]) 
		continue;
#if DBG
	if (nSegment == nDbgSeg)
		nSegment = nSegment;
#endif
	if (nType == 1) {	// render opaque objects
#if DBG
		if (nSegment == nDbgSeg)
			nSegment = nSegment;
#endif
		if (gameStates.render.bUseDynLight && !gameStates.render.bQueryCoronas) {
			nSegLights = lightManager.SetNearestToSegment (nSegment, -1, 0, 1, 0);
			lightManager.SetNearestStatic (nSegment, 1, 1, 0);
			gameStates.render.bApplyDynLight = gameOpts->ogl.bLightObjects;
			}
		else
			gameStates.render.bApplyDynLight = 0;
		RenderObjList (nListPos, gameStates.render.nWindow);
		if (gameStates.render.bUseDynLight && !gameStates.render.bQueryCoronas) {
			lightManager.ResetNearestStatic (nSegment, 0);
			}
		gameStates.render.bApplyDynLight = (gameStates.render.nLightingMethod != 0);
		//lightManager.Index (0)[0].nActive = gameData.render.lights.dynamic.shader.iStaticLights [0];
		}
	else if (nType == 2)	// render objects containing transparency, like explosions
		RenderObjList (nListPos, gameStates.render.nWindow);
	}	
gameStates.render.nState = 0;
}

//------------------------------------------------------------------------------

inline int RenderSegmentList (int nType, int bFrontToBack)
{
PROF_START
gameStates.render.nType = nType;
if (!(EGI_FLAG (bShadows, 0, 1, 0) && FAST_SHADOWS && !gameOpts->render.shadows.bSoft && (gameStates.render.nShadowPass >= 2))) {
	BumpVisitedFlag ();
	RenderFaceList (nType, bFrontToBack);
	ogl.ClearError (0);
	}
RenderMineObjects (nType);
lightManager.ResetAllUsed (1, 0);
if (gameStates.app.bMultiThreaded)
	lightManager.ResetAllUsed (1, 1);
ogl.ClearError (0);
PROF_END(ptRenderPass)
return 1;
}

//------------------------------------------------------------------------------

static void SetupMineRenderer (void)
{
#if DBG
if (gameStates.app.bNostalgia) {
	gameOptions [1].render.debug.bWireFrame = 0;
	gameOptions [1].render.debug.bTextures = 1;
	gameOptions [1].render.debug.bObjects = 1;
	gameOptions [1].render.debug.bWalls = 1;
	gameOptions [1].render.debug.bDynamicLight = 1;
	}
#endif

#if 0
if (gameStates.app.bNostalgia > 1)
	gameStates.render.nLightingMethod =
	gameStates.render.bPerPixelLighting = 0;
else if (!(lightmapManager.HaveLightmaps ()))
	gameStates.render.bPerPixelLighting = 0;
else {
	if (gameStates.render.nLightingMethod == 2)
		gameStates.render.bPerPixelLighting = 2;
	else if ((gameStates.render.nLightingMethod == 1) && gameOpts->render.bUseLightmaps)
		gameStates.render.bPerPixelLighting = 1;
	else
		gameStates.render.bPerPixelLighting = 0;
	}
#endif

ogl.m_states.bHaveDepthBuffer =
gameData.render.nUsedFaces =
gameData.render.nTotalFaces =
gameData.render.nTotalObjects =
gameData.render.nTotalSprites =
gameData.render.nTotalLights =
gameData.render.nMaxLights =
gameData.render.nStateChanges =
gameData.render.nShaderChanges = 0;
SetFaceDrawer (-1);
gameData.render.vertColor.bNoShadow = !FAST_SHADOWS && (gameStates.render.nShadowPass == 4);
gameData.render.vertColor.bDarkness = IsMultiGame && gameStates.app.bHaveExtraGameInfo [1] && extraGameInfo [IsMultiGame].bDarkness;
gameStates.render.bApplyDynLight =
gameStates.render.bUseDynLight = SHOW_DYN_LIGHT;
if (!EGI_FLAG (bPowerupLights, 0, 0, 0))
	gameData.render.nPowerupFilter = 0;
else if (gameStates.render.bPerPixelLighting == 2)
	gameData.render.nPowerupFilter = 1;
else
	gameData.render.nPowerupFilter = 2;
gameStates.render.bDoCameras = extraGameInfo [0].bUseCameras &&
									    (!IsMultiGame || (gameStates.app.bHaveExtraGameInfo [1] && extraGameInfo [1].bUseCameras)) &&
										 !gameStates.render.cameras.bActive;
gameStates.render.bDoLightmaps = 0;
}

//------------------------------------------------------------------------------

static void ComputeMineLighting (short nStartSeg, fix xStereoSeparation, int nWindow)
{
ogl.m_states.fLightRange = fLightRanges [IsMultiGame ? 1 : extraGameInfo [IsMultiGame].nLightRange];
if ((gameStates.render.nRenderPass <= 0) && (gameStates.render.nShadowPass < 2)) {
	gameData.render.mine.bSetAutomapVisited = BeginRenderMine (nStartSeg, xStereoSeparation, nWindow);

	if (xStereoSeparation <= 0) {
		gameStates.render.nThreads = GetNumThreads ();
		lightManager.ResetSegmentLights ();
		if (gameStates.render.bPerPixelLighting || (CountRenderFaces () < 16) || (gameStates.app.nThreads < 2)
#ifndef _OPENMP
			 || !RunRenderThreads (rtComputeFaceLight, gameStates.app.nThreads)
#endif
			) {
			gameStates.render.nThreads = 1;
			if (gameStates.render.bTriangleMesh || !gameStates.render.bApplyDynLight || (gameData.render.mine.nRenderSegs < gameData.segs.nSegments))
				ComputeFaceLight (0, gameData.render.mine.nRenderSegs, 0);
			else if (gameStates.app.bEndLevelSequence < EL_OUTSIDE)
				ComputeFaceLight (0, gameData.segs.nFaces, 0);
			else
				ComputeFaceLight (0, gameData.segs.nSegments, 0);
			}
#ifdef _OPENMP
		else {
				int	nStart, nEnd, nMax;

			if (gameStates.render.bTriangleMesh || !gameStates.render.bApplyDynLight || (gameData.render.mine.nRenderSegs < gameData.segs.nSegments))
				nMax = gameData.render.mine.nRenderSegs;
			else if (gameStates.app.bEndLevelSequence < EL_OUTSIDE)
				nMax = gameData.segs.nFaces;
			else
				nMax = gameData.segs.nSegments;
			if (gameStates.app.nThreads & 1) {
				#pragma omp parallel
					{
					#pragma omp for private (nStart, nEnd)
					for (int i = 0; i < gameStates.app.nThreads; i++) {
						ComputeThreadRange (i, nMax, nStart, nEnd);
						ComputeFaceLight (nStart, nEnd, i);
						}
					}
				}
			else {
				int	nPivot = gameStates.app.nThreads / 2;
				#pragma omp parallel
					{
					#pragma omp for private (nStart, nEnd)
					for (int i = 0; i < gameStates.app.nThreads; i++) {
						if (i < nPivot) {
							ComputeThreadRange (i, tiRender.nMiddle, nStart, nEnd, nPivot);
							ComputeFaceLight (nStart, nEnd, i);
							}
						else {
							ComputeThreadRange (i - nPivot, nMax - tiRender.nMiddle, nStart, nEnd, nPivot);
							ComputeFaceLight (nStart + tiRender.nMiddle, nEnd + tiRender.nMiddle, i);
							}
						}
					}
				}
			}
#endif //_OPENMP
		if ((gameStates.render.bPerPixelLighting == 2) && !gameData.app.nFrameCount)
			meshBuilder.BuildVBOs ();
		gameStates.render.bHeadlights = gameOpts->ogl.bHeadlight && lightManager.Headlights ().nLights && 
												  !(gameStates.render.bFullBright || automap.Display ());
		}
	transparencyRenderer.InitBuffer (gameData.render.zMin, gameData.render.zMax);
	}
}

//------------------------------------------------------------------------------

void RenderEffects (int nWindow)
{
	int bLightnings, bParticles, bSparks;

PROF_START
#if UNIFY_THREADS
WaitForRenderThreads ();
#else
WaitForEffectsThread ();
#endif
if (automap.Display ()) {
	bLightnings = gameOpts->render.automap.bLightnings;
	bParticles = gameOpts->render.automap.bParticles;
	bSparks = gameOpts->render.automap.bSparks;
	}
else {
	bSparks = (gameOptions [0].render.nQuality > 0);
	bLightnings = (!nWindow || gameOpts->render.lightning.bAuxViews) && 
					  (!gameStates.render.cameras.bActive || gameOpts->render.lightning.bMonitors);
	bParticles = (!nWindow || gameOpts->render.particles.bAuxViews) &&
					 (!gameStates.render.cameras.bActive || gameOpts->render.particles.bMonitors);
	}
if (bSparks) {
	SEM_ENTER (SEM_SPARKS)
	//PrintLog ("RenderEnergySparks\n");
	sparkManager.Render ();
	//SEM_LEAVE (SEM_SPARKS)
	}
if (bParticles) {
	SEM_ENTER (SEM_SMOKE)
	//PrintLog ("RenderSmoke\n");
	particleManager.Cleanup ();
	particleManager.Render ();
	//SEM_LEAVE (SEM_SMOKE)
	}
if (bLightnings) {
	SEM_ENTER (SEM_LIGHTNING)
	//PrintLog ("RenderLightnings\n");
	lightningManager.Render ();
	}
//PrintLog ("transparencyRenderer.Render\n");
if (bLightnings)
	SEM_LEAVE (SEM_LIGHTNING)
transparencyRenderer.Render ();
#if 1
if (bParticles)
	SEM_LEAVE (SEM_SMOKE)
if (bSparks)
	SEM_LEAVE (SEM_SPARKS)
#endif
PROF_END(ptEffects)
}

//------------------------------------------------------------------------------
//renders onto current canvas

extern int bLog;

void RenderMine (short nStartSeg, fix xStereoSeparation, int nWindow)
{
PROF_START
SetupMineRenderer ();
PROF_END(ptAux)
ComputeMineLighting (nStartSeg, xStereoSeparation, nWindow);
RenderSegmentList (0, 1);	// render opaque geometry
RenderSegmentList (1, 1);	// render objects
if (!EGI_FLAG (bShadows, 0, 1, 0) || (gameStates.render.nShadowPass == 1)) {
	if (!gameData.app.nFrameCount || gameData.render.nColoredFaces) {
		ogl.SetDepthMode (GL_LEQUAL);
		RenderSegmentList (2, 1);	// render transparent geometry
		ogl.SetDepthMode (GL_LESS);
		}
#if 0
	RenderEffects (nWindow);
#endif
	if (!gameStates.app.bNostalgia &&
		 (!automap.Display () || gameOpts->render.automap.bCoronas) && gameOpts->render.effects.bEnabled && gameOpts->render.coronas.bUse) {
 		ogl.SetTextureUsage (true);
		ogl.SetBlending (true);
		ogl.SetBlendMode (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		ogl.SetDepthMode (GL_LEQUAL);
		if (!nWindow) {
			ogl.SetDepthWrite (false);
			RenderSegmentList (3, 1);
			ogl.SetDepthWrite (true);
			}
		ogl.SetDepthMode (GL_LESS);
		ogl.SetTextureUsage (false);
		}
	ogl.SetDepthMode (GL_LESS);
	}
gameData.app.nMineRenderCount++;
PROF_END(ptRenderMine);
}

//------------------------------------------------------------------------------
// eof
