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

#ifndef _FASTRENDER_H
#define _FASTRENDER_H

#include "descent.h"
#include "endlevel.h"

//------------------------------------------------------------------------------

void QSortFaces (int left, int right);
int SetupDepthBuffer (int nType);
int SetupCoronas (void);
void RenderFaceList (int nType, int bFrontToBack);
void ComputeDynamicQuadLight (int nStart, int nEnd, int nThread);
void ComputeDynamicTriangleLight (int nStart, int nEnd, int nThread);
void ComputeDynamicFaceLight (int nStart, int nEnd, int nThread);
void ComputeStaticFaceLight (int nStart, int nEnd, int nThread);
void ComputeMineLighting (short nStartSeg, fix xStereoSeparation, int nWindow);
void UpdateSlidingFaces (void);
int CountRenderFaces (void);
void GetRenderVertices (void);
void RenderMineObjects (int nType);
void RenderSkyBoxFaces (void);
int BeginRenderFaces (int nType, int bDepthOnly);
void EndRenderFaces (int bDepthOnly);
short BuildFaceLists (void);

//------------------------------------------------------------------------------

static inline void ComputeFaceLight (int nStart, int nEnd, int nThread)
{
if (gameStates.render.bApplyDynLight && (gameStates.app.bEndLevelSequence < EL_OUTSIDE)) {
	if (gameStates.render.bTriangleMesh)
		ComputeDynamicTriangleLight (nStart, nEnd, nThread);
	else if (gameData.render.mine.nRenderSegs < gameData.segs.nSegments)
		ComputeDynamicQuadLight (nStart, nEnd, nThread);
	else
		ComputeDynamicFaceLight (nStart, nEnd, nThread);
	}
else
	ComputeStaticFaceLight (nStart, nEnd, nThread);
}

//------------------------------------------------------------------------------

#endif /* _FASTRENDER_H */
