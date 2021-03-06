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

#include <stdio.h>		//	for printf ()
#include <stdlib.h>		// for RandShort () and qsort ()
#include <string.h>		// for memset ()

#include "descent.h"
#include "mono.h"
#include "u_mem.h"

#include "error.h"
#include "physics.h"
#include "segmath.h"

#define	PARALLAX	0		//	If !0, then special debugging for Parallax eyes enabled.

//	Length in segments of avoidance path
#define	AVOID_SEG_LENGTH	7

#if !DBG
#	define	PATH_VALIDATION	0
#else
#	define	PATH_VALIDATION	1
#endif

void AIPathSetOrientAndVel (CObject *objP, CFixVector* vGoalPoint, int32_t nTargetVisibility, CFixVector *vecToTarget);
void MaybeAIPathGarbageCollect (void);
void AICollectPathGarbage (void);
#if PATH_VALIDATION
void ValidateAllPaths (void);
int32_t ValidatePath (int32_t debugFlag, tPointSeg* pointSegP, int32_t numPoints);
#endif
bool MoveObjectToLegalSpot (CObject *objP, int32_t bMoveToCenter);

//	------------------------------------------------------------------------

void CreateRandomXlate (int8_t *xt)
{
	int32_t	i, j;
	int8_t h;

for (i = 0; i < SEGMENT_SIDE_COUNT; i++)
	xt [i] = i;
for (i = 0; i < SEGMENT_SIDE_COUNT; i++) {
	j = (RandShort () * SEGMENT_SIDE_COUNT) / (SHORT_RAND_MAX + 1);
	Assert ((j >= 0) && (j < SEGMENT_SIDE_COUNT));
	h = xt [j];
	xt [j] = xt [i];
	xt [i] = h;
	}
}

//	-----------------------------------------------------------------------------------------------------------

tPointSeg *InsertTransitPoint (tPointSeg *curSegP, tPointSeg *predSegP, tPointSeg *succSegP, uint8_t nConnSide)
{
	CFixVector	vCenter, vPoint;
	int16_t			nSegment;

vCenter = SEGMENT (predSegP->nSegment)->SideCenter (nConnSide);
vPoint = predSegP->point - vCenter;
vPoint.v.coord.x /= 16;
vPoint.v.coord.y /= 16;
vPoint.v.coord.z /= 16;
curSegP->point = vCenter - vPoint;
nSegment = FindSegByPos (curSegP->point, succSegP->nSegment, 1, 0);
if (nSegment == -1) {
#if TRACE
	console.printf (1, "Warning: point not in ANY CSegment in aipath.c/InsertCenterPoints().\n");
#endif
	curSegP->point = vCenter;
	FindSegByPos (curSegP->point, succSegP->nSegment, 1, 0);
	}
curSegP->nSegment = succSegP->nSegment;
return curSegP;
}

//	-----------------------------------------------------------------------------------------------------------

int32_t OptimizePath (tPointSeg *pointSegP, int32_t nSegs)
{
	int32_t			i, j;
	CFixVector	temp1, temp2;
	fix			dot, mag1, mag2 = 0;

for (i = 1; i < nSegs - 1; i += 2) {
	if (i == 1) {
		temp1 = pointSegP [i].point - pointSegP [i-1].point;
		mag1 = temp1.Mag();
		}
	else {
		temp1 = temp2;
		mag1 = mag2;
		}
	temp2 = pointSegP [i + 1].point - pointSegP [i].point;
	mag2 = temp1.Mag();
	dot = CFixVector::Dot (temp1, temp2);
	if (dot * 9/8 > FixMul (mag1, mag2))
		pointSegP [i].nSegment = -1;
	}
//	Now, scan for points with nSegment == -1
for (i = j = 0; i < nSegs; i++)
	if (pointSegP [i].nSegment != -1)
		pointSegP [j++] = pointSegP [i];
return j;
}

//	-----------------------------------------------------------------------------------------------------------
//	Insert the point at the center of the CSide connecting two segments between the two points.
// This is messy because we must insert into the list.  The simplest (and not too slow) way to do this is to start
// at the end of the list and go backwards.
int32_t InsertCenterPoints (tPointSeg *pointSegP, int32_t numPoints)
{
	int32_t	i, j;

for (i = 0; i < numPoints; i++) {
	j = i + 2;
	InsertTransitPoint (pointSegP + i + 1, pointSegP + i, pointSegP + j, pointSegP [j].nConnSide);
	}
return OptimizePath (pointSegP, numPoints);
}

//	-----------------------------------------------------------------------------------------------------------
//	Move points halfway to outside of segment.
static tPointSeg newPtSegs [MAX_SEGMENTS_D2X];

void MoveTowardsOutside (tPointSeg *ptSegs, int32_t *nPoints, CObject *objP, int32_t bRandom)
{
	int32_t			i, j;
	int32_t			nNewSeg;
	fix			xSegSize;
	int32_t			nSegment;
	CFixVector	a, b, c, d, e;
	CFixVector	vGoalPos;
	int32_t			count;
	int32_t			nTempSeg;
	CHitResult		hitResult;
	int32_t			nHitType;

j = *nPoints;
if (j > LEVEL_SEGMENTS)
	j = LEVEL_SEGMENTS;
for (i = 1, --j; i < j; i++) {
	nTempSeg = FindSegByPos (ptSegs [i].point, ptSegs [i].nSegment, 1, 0);
	if (nTempSeg < 0)
		break;
	ptSegs [i].nSegment = nTempSeg;
	nSegment = ptSegs [i].nSegment;

	if (i == 1) {
		a = ptSegs [i].point - ptSegs [i-1].point;
		CFixVector::Normalize (a);
		}
	else
		a = b;
	b = ptSegs [i + 1].point - ptSegs [i].point;
	c = ptSegs [i + 1].point - ptSegs [i-1].point;
	CFixVector::Normalize (b);
	if (abs (CFixVector::Dot (a, b)) > I2X (3)/4) {
		if (abs (a.v.coord.z) < I2X (1)/2) {
			if (bRandom) {
				e.v.coord.x = SRandShort () / 2;
				e.v.coord.y = SRandShort () / 2;
				e.v.coord.z = abs (e.v.coord.x) + abs (e.v.coord.y) + 1;
				CFixVector::Normalize (e);
				}
			else {
				e.v.coord.x =
				e.v.coord.y = 0;
				e.v.coord.z = I2X (1);
				}
			}
		else {
			if (bRandom) {
				e.v.coord.y = SRandShort () / 2;
				e.v.coord.z = SRandShort () / 2;
				e.v.coord.x = abs (e.v.coord.y) + abs (e.v.coord.z) + 1;
				CFixVector::Normalize (e);
				}
			else {
				e.v.coord.x = I2X (1);
				e.v.coord.y =
				e.v.coord.z = 0;
				}
			}
		}
	else {
		d = CFixVector::Cross(a, b);
		e = CFixVector::Cross(c, d);
		CFixVector::Normalize (e);
		}
#if DBG
	if (e.Mag () < I2X (1)/2)
		Int3 ();
#endif
	xSegSize = SEGMENT (nSegment)->AvgRad ();
	if (xSegSize > I2X (40))
		xSegSize = I2X (40);
	vGoalPos = ptSegs [i].point + e * (xSegSize / 4);
	count = 3;
	while (count) {
		CHitQuery hitQuery (0, &ptSegs [i].point, &vGoalPos, ptSegs [i].nSegment, objP->Index (), objP->info.xSize, objP->info.xSize);
		nHitType = FindHitpoint (hitQuery, hitResult);
		if (nHitType == HIT_NONE)
			count = 0;
		else {
			if ((count == 3) && (nHitType == HIT_BAD_P0))
				return;
			vGoalPos.v.coord.x = ((*hitQuery.p0).v.coord.x + hitResult.vPoint.v.coord.x)/2;
			vGoalPos.v.coord.y = ((*hitQuery.p0).v.coord.y + hitResult.vPoint.v.coord.y)/2;
			vGoalPos.v.coord.z = ((*hitQuery.p0).v.coord.z + hitResult.vPoint.v.coord.z)/2;
			if (!--count)	//	Couldn't move towards outside, that's ok, sometimes things can't be moved.
				vGoalPos = ptSegs [i].point;
			}
		}
	//	Only move towards outside if remained inside CSegment.
	nNewSeg = FindSegByPos (vGoalPos, ptSegs [i].nSegment, 1, 0);
	if (nNewSeg == ptSegs [i].nSegment) {
		newPtSegs [i].point = vGoalPos;
		newPtSegs [i].nSegment = nNewSeg;
		}
	else {
		newPtSegs [i].point = ptSegs [i].point;
		newPtSegs [i].nSegment = ptSegs [i].nSegment;
		}
	}
if (j > 1)
	memcpy (ptSegs + 1, newPtSegs + 1, (j - 1) * sizeof (tPointSeg));
}

//	-----------------------------------------------------------------------------------------------------------
//	Create a path from objP->Position () to the center of nEndSeg.
//	Return a list of (segment_num, point_locations) at pointSegP
//	Return number of points in *numPoints.
//	if nMaxDepth == -1, then there is no maximum depth.
//	If unable to create path, return -1, else return 0.
//	If randomFlag !0, then introduce randomness into path by looking at sides in random order.  This means
//	that a path between two segments won't always be the same, unless it is unique.p.
//	If bSafeMode is set, then additional points are added to "make sure" that points are reachable.p.  I would
//	like to say that it ensures that the CObject can move between the points, but that would require knowing what
//	the CObject is (which isn't passed, right?) and making fvi calls (slow, right?).  So, consider it the more_or_less_safeFlag.
//	If nEndSeg == -2, then end seg will never be found and this routine will drop out due to depth (xProbably called by CreateNSegmentPath).
int32_t CreatePathPoints (CObject *objP, int32_t nStartSeg, int32_t nEndSeg, tPointSeg *pointSegP, int16_t *numPoints,
							 int32_t nMaxDepth, int32_t bRandom, int32_t bSafeMode, int32_t nAvoidSeg)
{
	int16_t					nCurSeg;
	int16_t					nSide, hSide;
	int32_t					qTail = 0, qHead = 0;
	int32_t					h, i, j;
	int8_t					bVisited [MAX_SEGMENTS_D2X];
	segQueueEntry		segmentQ [MAX_SEGMENTS_D2X];
	int16_t					depth [MAX_SEGMENTS_D2X];
	int32_t					nCurDepth;
	int8_t					randomXlate [SEGMENT_SIDE_COUNT];
	tPointSeg*			origPointSegs = pointSegP;
	int32_t					lNumPoints;
	CSegment*			segP;
	CFixVector			vCenter;
	int32_t					nParentSeg, nDestSeg;
	CHitResult				hitResult;
	int32_t					hitType;
	int32_t					bAvoidTarget;

#if PATH_VALIDATION
ValidateAllPaths ();
#endif

if ((objP->info.nType == OBJ_ROBOT) && (objP->cType.aiInfo.behavior == AIB_RUN_FROM) && (nAvoidSeg != -32767)) {
	bRandom = 1;
	nAvoidSeg = TARGETOBJ->info.nSegment;
	}
bAvoidTarget = TARGETOBJ->info.nSegment == nAvoidSeg;
if (nMaxDepth == -1)
	nMaxDepth = MAX_PATH_LENGTH;
lNumPoints = 0;
memset (bVisited, 0, sizeof (bVisited [0]) * gameData.segData.nSegments);
memset (depth, 0, sizeof (depth [0]) * gameData.segData.nSegments);
//	If there is a CSegment we're not allowed to visit, mark it.
if (nAvoidSeg != -1) {
	Assert (nAvoidSeg <= gameData.segData.nLastSegment);
	if ((nStartSeg != nAvoidSeg) && (nEndSeg != nAvoidSeg)) {
		bVisited [nAvoidSeg] = 1;
		depth [nAvoidSeg] = 0;
		}
	}

nCurSeg = nStartSeg;
bVisited [nCurSeg] = 1;
nCurDepth = 0;

#if DBG
if (objP->Index () == nDbgObj)
	BRP;
#endif
if (bRandom)
	CreateRandomXlate (randomXlate);
nCurSeg = nStartSeg;
bVisited [nCurSeg] = 1;
while (nCurSeg != nEndSeg) {
	segP = SEGMENT (nCurSeg);
	if (bRandom && (RandShort () < 8192))	//create a different xlate at random time intervals
		CreateRandomXlate (randomXlate);

	for (nSide = 0; nSide < SEGMENT_SIDE_COUNT; nSide++) {
		hSide = bRandom ? randomXlate [nSide] : nSide;
		if (!IS_CHILD (segP->m_children [hSide]))
			continue;
		if (!((segP->IsPassable (hSide, NULL) & WID_PASSABLE_FLAG) ||
			  (AIDoorIsOpenable (objP, segP, hSide))))
			continue;
		nDestSeg = segP->m_children [hSide];
		if (bVisited [nDestSeg])
			continue;
		if (bAvoidTarget && ((nCurSeg == nAvoidSeg) || (nDestSeg == nAvoidSeg))) {
			vCenter = segP->SideCenter (hSide);
			CHitQuery hitQuery (0, &objP->Position (), &vCenter, objP->info.nSegment, objP->Index (), objP->info.xSize, objP->info.xSize);
			hitType = FindHitpoint (hitQuery, hitResult);
			if (hitType != HIT_NONE)
				continue;
			}
		if (nDestSeg < 0)
			continue;
		if (nCurSeg < 0)
			continue;
		segmentQ [qTail].start = nCurSeg;
		segmentQ [qTail].end = nDestSeg;
		segmentQ [qTail].nConnSide = (uint8_t) hSide;
		bVisited [nDestSeg] = 1;
		depth [qTail++] = nCurDepth + 1;
		if (depth [qTail-1] == nMaxDepth) {
			nEndSeg = segmentQ [qTail-1].end;
			goto pathTooLong;
			}	// end if (depth [...
		}	//	for (nSide.p...

	if (qHead >= qTail) {
		//	Couldn't get to goal, return a path as far as we got, which is probably acceptable to the unparticular caller.
		nEndSeg = segmentQ [qTail-1].end;
		break;
		}
	nCurSeg = segmentQ [qHead].end;
	nCurDepth = depth [qHead];
	qHead++;

pathTooLong: ;
	}	//	while (nCurSeg ...
//	Set qTail to the CSegment which ends at the goal.
while (segmentQ [--qTail].end != nEndSeg)
	if (qTail < 0) {
		*numPoints = lNumPoints;
		return -1;
		}
for (i = qTail; i >= 0; ) {
	nParentSeg = segmentQ [i].start;
	lNumPoints++;
	if (nParentSeg == nStartSeg)
		break;
	while (segmentQ [--i].end != nParentSeg)
		Assert (i >= 0);
	}

if (bSafeMode && ((pointSegP - gameData.ai.routeSegs) + 2 * lNumPoints + 1 >= LEVEL_POINT_SEGS)) {
	//	Ouch! Cannot insert center points in path.  So return unsafe path.
#if TRACE
	console.printf (CON_DBG, "Resetting all paths because of bSafeMode.p.\n");
#endif
	AIResetAllPaths ();
	*numPoints = lNumPoints;
	return -1;
	}
pointSegP->nSegment = nStartSeg;
pointSegP->point = SEGMENT (nStartSeg)->Center ();
if (bSafeMode)
	lNumPoints *= 2;
j = lNumPoints++;
h = bSafeMode + 1;
for (i = qTail; i >= 0; j -= h) {
	nDestSeg = segmentQ [i].end;
	nParentSeg = segmentQ [i].start;
	pointSegP [j].nSegment = nDestSeg;
	pointSegP [j].point = SEGMENT (nDestSeg)->Center ();
	pointSegP [j].nConnSide = segmentQ [i].nConnSide;
	if (nParentSeg == nStartSeg)
		break;
	while (segmentQ [--i].end != nParentSeg)
		Assert (qTail >= 0);
	}
if (bSafeMode) {
	for (i = 0; i < lNumPoints - 1; i = j) {
		j = i + 2;
		InsertTransitPoint (pointSegP + i + 1, pointSegP + i, pointSegP + j, pointSegP [j].nConnSide);
		}
	lNumPoints = OptimizePath (pointSegP, lNumPoints);
	}
pointSegP += lNumPoints;

#if PATH_VALIDATION
ValidatePath (2, origPointSegs, lNumPoints);
#endif

#if PATH_VALIDATION
ValidatePath (3, origPointSegs, lNumPoints);
#endif

// -- MK, 10/30/95 -- This code causes apparent discontinuities in the path, moving a point
//	into a new CSegment.  It is not necessarily bad, but it makes it hard to track down actual
//	discontinuity xProblems.
if ((objP->info.nType == OBJ_ROBOT) && ROBOTINFO (objP)->companion)
	MoveTowardsOutside (origPointSegs, &lNumPoints, objP, 0);

#if PATH_VALIDATION
ValidatePath (4, origPointSegs, lNumPoints);
#endif

*numPoints = lNumPoints;
return 0;
}

int32_t	Last_buddy_polish_path_frame;

//	-------------------------------------------------------------------------------------------------------
//	SmoothPath
//	Takes an existing path and makes it nicer.
//	Drops as many leading points as possible still maintaining direct accessibility
//	from current position to first point.
//	Will not shorten path to fewer than 3 points.
//	Returns number of points.
//	Starting position in pointSegP doesn't change.p.
//	Changed, MK, 10/18/95.  I think this was causing robots to get hung up on walls.
//				Only drop up to the first three points.
int32_t SmoothPath (CObject *objP, tPointSeg *pointSegP, int32_t numPoints)
{
#if 1
return numPoints;
#else
	int32_t			i, nFirstPoint = 0;
	CHitResult		hitResult;
	int32_t			hitType;


if (numPoints <= 4)
	return numPoints;

//	Prevent the buddy from polishing his path twice in one frame, which can cause him to get hung up.  Pretty ugly, huh?
if (ROBOTINFO (objP)->companion) {
	if (gameData.app.nFrameCount == Last_buddy_polish_path_frame)
		return numPoints;
	Last_buddy_polish_path_frame = gameData.app.nFrameCount;
	}
CHitQuery fq (0, &objP->Position (), NULL, objP->info.nSegment, objP->info.xSize, objP->info.xSize, objP->Index ());
for (i = 0; i < 2; i++) {
	fq.p1 = &pointSegP [i].point;
	hitType = FindHitpoint (&fq, &hitResult);
	if (hitType != HIT_NONE)
		break;
	nFirstPoint = i + 1;
	}
if (nFirstPoint) {
	//	Scrunch down all the pointSegP.
	for (i = nFirstPoint; i < numPoints; i++)
		pointSegP [i - nFirstPoint] = pointSegP [i];
	}
return numPoints - nFirstPoint;
#endif
}

//	-------------------------------------------------------------------------------------------------------
//	Make sure that there are connections between all segments on path.
//	Note that if path has been optimized, connections may not be direct, so this function is useless, or worse.p.
//	Return true if valid, else return false.p.

#if PATH_VALIDATION

int32_t ValidatePath (int32_t debugFlag, tPointSeg *pointSegP, int32_t numPoints)
{
	int32_t i, nCurSeg, nSide, nNextSeg;

nCurSeg = pointSegP->nSegment;
if ((nCurSeg < 0) || (nCurSeg > gameData.segData.nLastSegment)) {
#if TRACE
	console.printf (CON_DBG, "Path beginning at index %i, length=%i is bogus!\n", pointSegP-gameData.ai.routeSegs, numPoints);
#endif
	Int3 ();		//	Contact Mike: Debug trap for elusive, nasty bug.
	return 0;
	}
#if TRACE
if (debugFlag == 999)
	console.printf (CON_DBG, "That's curious...\n");
#endif
if (numPoints == 0)
	return 1;
for (i = 1; i < numPoints; i++) {
	nNextSeg = pointSegP [i].nSegment;
	if ((nNextSeg < 0) || (nNextSeg > gameData.segData.nLastSegment)) {
#if TRACE
		console.printf (CON_DBG, "Path beginning at index %i, length=%i is bogus!\n", pointSegP-gameData.ai.routeSegs, numPoints);
#endif
		Int3 ();		//	Contact Mike: Debug trap for elusive, nasty bug.
		return 0;
		}
	if (nCurSeg != nNextSeg) {
		for (nSide = 0; nSide < SEGMENT_SIDE_COUNT; nSide++)
			if (SEGMENT (nCurSeg)->m_sides [nSide].FaceCount () && (SEGMENT (nCurSeg)->m_children [nSide] == nNextSeg))
				break;
		if (nSide == SEGMENT_SIDE_COUNT) {
#if TRACE
			console.printf (CON_DBG, "Path beginning at index %i, length=%i is bogus!\n", pointSegP-gameData.ai.routeSegs, numPoints);
#endif
			Int3 ();
			return 0;
			}
		nCurSeg = nNextSeg;
		}
	}
return 1;
}

#endif

//	-----------------------------------------------------------------------------------------------------------

#if PATH_VALIDATION

void ValidateAllPaths (void)
{
	CObject*			objP;
	tAIStaticInfo*	aiP;

FORALL_ROBOT_OBJS (objP) {
	aiP = &objP->cType.aiInfo;
	if ((objP->info.controlType == CT_AI) &&
		 (aiP->nHideIndex != -1) && (aiP->nPathLength > 0) &&
		 !ValidatePath (4, &gameData.ai.routeSegs [aiP->nHideIndex], aiP->nPathLength))
		aiP->nPathLength = 0;	//	This allows people to resume without harm...
	}
}
#endif

//	-------------------------------------------------------------------------------------------------------
//	Creates a path from the OBJECTS current CSegment (objP->info.nSegment) to the specified CSegment for the CObject to
//	hide in gameData.ai.localInfo [nObject].nGoalSegment.
//	Sets	objP->cType.aiInfo.nHideIndex, 		a pointer into gameData.ai.routeSegs, the first tPointSeg of the path.
//			objP->cType.aiInfo.nPathLength, 		length of path
//			gameData.ai.freePointSegs				global pointer into gameData.ai.routeSegs array

void CreatePathToTarget (CObject *objP, int32_t nMaxDepth, int32_t bSafeMode)
{
	tAIStaticInfo*	aiP = &objP->cType.aiInfo;
	tAILocalInfo*	ailP = gameData.ai.localInfo + objP->Index ();
	int32_t				nStartSeg, nEndSeg;

if (nMaxDepth == -1)
	nMaxDepth = MAX_DEPTH_TO_SEARCH_FOR_PLAYER;

ailP->timeTargetSeen = gameData.time.xGame;			//	Prevent from resetting path quickly.
ailP->nGoalSegment = gameData.ai.target.nBelievedSeg;

nStartSeg = objP->info.nSegment;
nEndSeg = ailP->nGoalSegment;

if (nEndSeg != -1) {
	CreatePathPoints (objP, nStartSeg, nEndSeg, gameData.ai.freePointSegs, &aiP->nPathLength, nMaxDepth, 1, bSafeMode, -1);
	aiP->nPathLength = SmoothPath (objP, gameData.ai.freePointSegs, aiP->nPathLength);
	aiP->nHideIndex = int32_t (gameData.ai.routeSegs.Index (gameData.ai.freePointSegs));
#if DBG
	if (aiP->nHideIndex < 0)
		aiP->nHideIndex = aiP->nHideIndex;
#endif
	aiP->nCurPathIndex = 0;
	gameData.ai.freePointSegs += aiP->nPathLength;
	if (uint32_t (gameData.ai.routeSegs.Index (gameData.ai.freePointSegs) + MAX_PATH_LENGTH * 2) > uint32_t (LEVEL_POINT_SEGS)) {
		AIResetAllPaths ();
		return;
		}
	aiP->PATH_DIR = 1;		//	Initialize to moving forward.
	// -- UNUSED!aiP->SUBMODE = AISM_GOHIDE;		//	This forces immediate movement.
	ailP->mode = AIM_FOLLOW_PATH;
	if (ailP->targetAwarenessType < PA_RETURN_FIRE)
		ailP->targetAwarenessType = 0;		//	If robot too aware of CPlayerData, will set mode to chase
	}
MaybeAIPathGarbageCollect ();
}

//	-------------------------------------------------------------------------------------------------------
//	Creates a path from the CObject's current CSegment (objP->info.nSegment) to CSegment goalseg.
void CreatePathToSegment (CObject *objP, int16_t goalseg, int32_t nMaxDepth, int32_t bSafeMode)
{
	tAIStaticInfo	*aiP = &objP->cType.aiInfo;
	tAILocalInfo		*ailP = &gameData.ai.localInfo [objP->Index ()];
	int16_t			nStartSeg, nEndSeg;

if (nMaxDepth == -1)
	nMaxDepth = MAX_DEPTH_TO_SEARCH_FOR_PLAYER;
else if (nMaxDepth > gameData.segData.nSegments)
	nMaxDepth = gameData.segData.nSegments;
ailP->timeTargetSeen = gameData.time.xGame;			//	Prevent from resetting path quickly.
ailP->nGoalSegment = goalseg;
nStartSeg = objP->info.nSegment;
nEndSeg = ailP->nGoalSegment;
if (nEndSeg != -1) {
	CreatePathPoints (objP, nStartSeg, nEndSeg, gameData.ai.freePointSegs, &aiP->nPathLength, nMaxDepth, 1, bSafeMode, -1);
	aiP->nHideIndex = int32_t (gameData.ai.routeSegs.Index (gameData.ai.freePointSegs));
#if DBG
	if (aiP->nHideIndex < 0)
		aiP->nHideIndex = aiP->nHideIndex;
#endif
	aiP->nCurPathIndex = 0;
	gameData.ai.freePointSegs += aiP->nPathLength;
	if (uint32_t (gameData.ai.routeSegs.Index (gameData.ai.freePointSegs) + MAX_PATH_LENGTH * 2) > uint32_t (LEVEL_POINT_SEGS)) {
		AIResetAllPaths ();
		return;
		}
	aiP->PATH_DIR = 1;		//	Initialize to moving forward.
	// -- UNUSED!aiP->SUBMODE = AISM_GOHIDE;		//	This forces immediate movement.
	if (ailP->targetAwarenessType < PA_RETURN_FIRE)
		ailP->targetAwarenessType = 0;		//	If robot too aware of CPlayerData, will set mode to chase
	}
MaybeAIPathGarbageCollect ();
}

//	-------------------------------------------------------------------------------------------------------
//	Creates a path from the OBJECTS current CSegment (objP->info.nSegment) to the specified CSegment for the CObject to
//	hide in gameData.ai.localInfo [nObject].nGoalSegment
//	Sets	objP->cType.aiInfo.nHideIndex, 		a pointer into gameData.ai.routeSegs, the first tPointSeg of the path.
//			objP->cType.aiInfo.nPathLength, 		length of path
//			gameData.ai.freePointSegs				global pointer into gameData.ai.routeSegs array
void CreatePathToStation (CObject *objP, int32_t nMaxDepth)
{
	tAIStaticInfo	*aiP = &objP->cType.aiInfo;
	tAILocalInfo		*ailP = &gameData.ai.localInfo [objP->Index ()];
	int32_t			nStartSeg, nEndSeg;

if (nMaxDepth == -1)
	nMaxDepth = MAX_DEPTH_TO_SEARCH_FOR_PLAYER;

ailP->timeTargetSeen = gameData.time.xGame;			//	Prevent from resetting path quickly.

nStartSeg = objP->info.nSegment;
nEndSeg = aiP->nHideSegment;


if (nEndSeg != -1) {
	CreatePathPoints (objP, nStartSeg, nEndSeg, gameData.ai.freePointSegs, &aiP->nPathLength, nMaxDepth, 1, 1, -1);
	aiP->nPathLength = SmoothPath (objP, gameData.ai.freePointSegs, aiP->nPathLength);
	aiP->nHideIndex = int32_t (gameData.ai.routeSegs.Index (gameData.ai.freePointSegs));
#if DBG
	if (aiP->nHideIndex < 0)
		aiP->nHideIndex = aiP->nHideIndex;
#endif
	aiP->nCurPathIndex = 0;
	gameData.ai.freePointSegs += aiP->nPathLength;
	if (uint32_t (gameData.ai.routeSegs.Index (gameData.ai.freePointSegs) + MAX_PATH_LENGTH * 2) > uint32_t (LEVEL_POINT_SEGS)) {
		AIResetAllPaths ();
		return;
		}
	aiP->PATH_DIR = 1;		//	Initialize to moving forward.
	ailP->mode = AIM_FOLLOW_PATH;
	if (ailP->targetAwarenessType < PA_RETURN_FIRE)
		ailP->targetAwarenessType = 0;
	}
MaybeAIPathGarbageCollect ();
}


//	-------------------------------------------------------------------------------------------------------
//	Create a path of length nPathLength for an CObject, stuffing info in aiInfo field.

static int32_t nObject = 0;

void CreateNSegmentPath (CObject *objP, int32_t nPathLength, int16_t nAvoidSeg)
{
	tAIStaticInfo	*aiP = &objP->cType.aiInfo;
	tAILocalInfo		*ailP = gameData.ai.localInfo + objP->Index ();
	nObject = objP->Index ();

if (CreatePathPoints (objP, objP->info.nSegment, -2, gameData.ai.freePointSegs, &aiP->nPathLength, nPathLength, 1, 0, nAvoidSeg) == -1) {
	gameData.ai.freePointSegs += aiP->nPathLength;
	while ((CreatePathPoints (objP, objP->info.nSegment, -2, gameData.ai.freePointSegs, &aiP->nPathLength, --nPathLength, 1, 0, -1) == -1)) {
		Assert (nPathLength);
		}
	}
aiP->nHideIndex = int32_t (gameData.ai.routeSegs.Index (gameData.ai.freePointSegs));
#if DBG
if (aiP->nHideIndex < 0)
	aiP->nHideIndex = aiP->nHideIndex;
#endif
aiP->nCurPathIndex = 0;
#if PATH_VALIDATION
ValidatePath (8, gameData.ai.freePointSegs, aiP->nPathLength);
#endif
gameData.ai.freePointSegs += aiP->nPathLength;
if (uint32_t (gameData.ai.routeSegs.Index (gameData.ai.freePointSegs) + MAX_PATH_LENGTH * 2) > uint32_t (LEVEL_POINT_SEGS)) {
	AIResetAllPaths ();
	}
aiP->PATH_DIR = 1;		//	Initialize to moving forward.
ailP->mode = AIM_FOLLOW_PATH;
//	If this robot is visible (nTargetVisibility is not available) and it's running away, move towards outside with
//	randomness to prevent a stream of bots from going away down the center of a corridor.
if (gameData.ai.localInfo [objP->Index ()].nPrevVisibility) {
	if (aiP->nPathLength) {
		int32_t nPoints = aiP->nPathLength;
		MoveTowardsOutside (gameData.ai.routeSegs + aiP->nHideIndex, &nPoints, objP, 1);
		aiP->nPathLength = nPoints;
		}
	}
MaybeAIPathGarbageCollect ();
}

//	-------------------------------------------------------------------------------------------------------

void CreateNSegmentPathToDoor (CObject *objP, int32_t nPathLength, int16_t nAvoidSeg)
{
CreateNSegmentPath (objP, nPathLength, nAvoidSeg);
}

#define Int3_if (cond) if (!cond) Int3 ();

//	----------------------------------------------------------------------------------------------------

void MoveObjectToGoal (CObject *objP, CFixVector *vGoalPoint, int16_t nGoalSeg)
{
	tAIStaticInfo	*aiP = &objP->cType.aiInfo;
	int32_t			nSegment;

if (aiP->nPathLength < 2)
	return;
Assert (objP->info.nSegment != -1);
Assert (aiP->nPathLength >= 2);
if (aiP->nCurPathIndex <= 0) {
	if (aiP->behavior == AIB_STATION) {
		CreatePathToStation (objP, 15);
		return;
		}
	aiP->nCurPathIndex = 1;
	aiP->PATH_DIR = 1;
	}
else if (aiP->nCurPathIndex >= aiP->nPathLength - 1) {
	if (aiP->behavior == AIB_STATION) {
		CreatePathToStation (objP, 15);
		if (!aiP->nPathLength) {
			tAILocalInfo	*ailP = &gameData.ai.localInfo [objP->Index ()];
			ailP->mode = AIM_IDLING;
			}
		return;
		}
	Assert (aiP->nPathLength != 0);
	aiP->nCurPathIndex = aiP->nPathLength - 2;
	aiP->PATH_DIR = -1;
	}
else
	aiP->nCurPathIndex += aiP->PATH_DIR;
objP->Position () = *vGoalPoint;
nSegment = objP->FindSegment ();
#if TRACE
if (nSegment != nGoalSeg)
	console.printf (1, "Object #%i goal supposed to be in CSegment #%i, but in CSegment #%i\n", objP->Index (), nGoalSeg, nSegment);
#endif
if (nSegment == -1) {
	Int3 ();	//	Oops, CObject is not in any CSegment.
				// Contact Mike: This is impossible.p.
	//	Hack, move CObject to center of CSegment it used to be in.
	objP->Position () = SEGMENT (objP->info.nSegment)->Center ();
	}
else
	objP->RelinkToSeg (nSegment);
}

// -- too much work -- //	----------------------------------------------------------------------------------------------------------
// -- too much work -- //	Return true if the CObject the companion wants to kill is reachable.p.
// -- too much work -- int32_t attackKillObject (CObject *objP)
// -- too much work -- {
// -- too much work -- 	CObject		*kill_objp;
// -- too much work -- 	CHitResult		hitResult;
// -- too much work -- 	int32_t			fate;
// -- too much work -- 	CHitQuery	fq;
// -- too much work --
// -- too much work -- 	if (gameData.escort.nKillObject == -1)
// -- too much work -- 		return 0;
// -- too much work --
// -- too much work -- 	kill_objp = OBJECT (gameData.escort.nKillObject);
// -- too much work --
// -- too much work -- 	fq.p0						= &objP->Position ();
// -- too much work -- 	fq.startSeg				= objP->info.nSegment;
// -- too much work -- 	fq.p1						= &kill_objP->Position ();
// -- too much work -- 	fq.rad					= objP->info.xSize;
// -- too much work -- 	fq.thisObjNum			= objP->Index ();
// -- too much work -- 	fq.ignoreObjList	= NULL;
// -- too much work -- 	fq.flags					= 0;
// -- too much work --
// -- too much work -- 	fate = FindHitpoint (&fq, &hitResult);
// -- too much work --
// -- too much work -- 	if (fate == HIT_NONE)
// -- too much work -- 		return 1;
// -- too much work -- 	else
// -- too much work -- 		return 0;
// -- too much work -- }

//	----------------------------------------------------------------------------------------------------------
//	Optimization: If current velocity will take robot near goal, don't change velocity
void AIFollowPath (CObject *objP, int32_t nTargetVisibility, int32_t nPrevVisibility, CFixVector *vecToTarget)
{
	tAIStaticInfo*	aiP = &objP->cType.aiInfo;

	CFixVector		vGoalPoint;
	fix				xDistToGoal;
	tRobotInfo*		botInfoP = ROBOTINFO (objP);
	int32_t				originalDir, originalIndex;
	fix				xDistToTarget;
	int16_t				nGoalSeg = -1;
	tAILocalInfo*	ailP = gameData.ai.localInfo + objP->Index ();
	fix				thresholdDistance;

#if DBG
if (objP->Index () == nDbgObj)
	BRP;
#endif
if ((aiP->nHideIndex == -1) || (aiP->nPathLength == 0)) {
	if (ailP->mode == AIM_RUN_FROM_OBJECT) {
		CreateNSegmentPath (objP, 5, -1);
		ailP->mode = AIM_RUN_FROM_OBJECT;	// restore mode that has been changed in CreateNSegmentPath!
		}
	else {
		CreateNSegmentPath (objP, 5, -1);
		}
	}

#if DBG
if (aiP->nHideIndex < 0)
	aiP->nHideIndex = aiP->nHideIndex;
#endif
if ((aiP->nPathLength > 0) && (aiP->nHideIndex + aiP->nPathLength > int32_t (gameData.ai.routeSegs.Index (gameData.ai.freePointSegs)))) {
	//	This is debugging code.p.  Figure out why garbage collection didn't compress this object's path information.
	PrintLog (0, "Error in AI path info garbage collection\n");
	AICollectPathGarbage ();
	//force_dump_aiObjects_all ("Error in AIFollowPath");
	AIResetAllPaths ();
	return;
	}

if (aiP->nPathLength < 2) {
	if ((aiP->behavior == AIB_SNIPE) || (ailP->mode == AIM_RUN_FROM_OBJECT)) {
		if (TARGETOBJ->info.nSegment == objP->info.nSegment) {
			CreateNSegmentPath (objP, AVOID_SEG_LENGTH, -1);			//	Can't avoid CSegment CPlayerData is in, robot is already in it!(That's what the -1 is for)
			//--Int3_if ((aiP->nPathLength != 0);
			}
		else {
			CreateNSegmentPath (objP, AVOID_SEG_LENGTH, TARGETOBJ->info.nSegment);
				//--Int3_if ((aiP->nPathLength != 0);
			}
		if (aiP->behavior == AIB_SNIPE) {
			if (botInfoP->thief)
				ailP->mode = AIM_THIEF_ATTACK;	//	It gets bashed in CreateNSegmentPath
			else
				ailP->mode = AIM_SNIPE_FIRE;	//	It gets bashed in CreateNSegmentPath
			}
		else {
			ailP->mode = AIM_RUN_FROM_OBJECT;	//	It gets bashed in CreateNSegmentPath
			}
		}
	else if (botInfoP->companion == 0) {
		ailP->mode = AIM_IDLING;
		aiP->nPathLength = 0;
		return;
		}
#if DBG
	if (aiP->nHideIndex < 0)
		aiP->nHideIndex = aiP->nHideIndex;
#endif
	}

int32_t i = aiP->nHideIndex + aiP->nCurPathIndex;
if (i < 0)
	xDistToGoal = 0;
else {
	vGoalPoint = gameData.ai.routeSegs [aiP->nHideIndex + aiP->nCurPathIndex].point;
	nGoalSeg = gameData.ai.routeSegs [aiP->nHideIndex + aiP->nCurPathIndex].nSegment;
	xDistToGoal = CFixVector::Dist (vGoalPoint, objP->Position ());
	}
if (gameStates.app.bPlayerIsDead)
	xDistToTarget = CFixVector::Dist (objP->Position (), gameData.objData.viewerP->Position ());
else
	xDistToTarget = CFixVector::Dist (objP->Position (), OBJPOS (TARGETOBJ)->vPos);
	//	Efficiency hack: If far away from CPlayerData, move in big quantized jumps.
if (!(nTargetVisibility || nPrevVisibility) && (xDistToTarget > I2X (200)) && !IsMultiGame) {
	if (xDistToGoal && (xDistToGoal < I2X (2))) {
		MoveObjectToGoal (objP, &vGoalPoint, nGoalSeg);
		return;
		}
	else {
		tRobotInfo	*botInfoP = ROBOTINFO (objP);
		fix	xCurSpeed = botInfoP->xMaxSpeed [gameStates.app.nDifficultyLevel] / 2;
		fix	xCoverableDist = FixMul (gameData.time.xFrame, xCurSpeed);
		// int32_t	nConnSide = ConnectedSide (objP->info.nSegment, nGoalSeg);
		//	Only move to goal if allowed to fly through the CSide.p.
		//	Buddy-bot can create paths he can't fly, waiting for player.
		// -- bah, this isn't good enough, buddy will fail to get through any door!if (WALL_IS_DOORWAY (&SEGMENTS]objP->info.nSegment], nConnSide) & WID_PASSABLE_FLAG) {
		if (!(botInfoP->companion || botInfoP->thief)) {
			if ((xCoverableDist >= xDistToGoal) || ((RandShort () >> 1) < FixDiv (xCoverableDist, xDistToGoal)))
				MoveObjectToGoal (objP, &vGoalPoint, nGoalSeg);
			return;
			}
		}
	}
//	If running from CPlayerData, only run until can't be seen.
if (ailP->mode == AIM_RUN_FROM_OBJECT) {
	if ((nTargetVisibility == 0) && (ailP->targetAwarenessType == 0)) {
		fix xVelScale = I2X (1) - gameData.time.xFrame / 2;
		if (xVelScale < I2X (1) / 2)
			xVelScale = I2X (1) / 2;
		objP->mType.physInfo.velocity *= FixMul (xVelScale, objP->DriveDamage () * 2);
		return;
		}
	else if (!(gameData.app.nFrameCount ^ ((objP->Index ()) & 0x07))) {		//	Done 1/8 frames.
		//	If CPlayerData on path (beyond point robot is now at), then create a new path.
		tPointSeg*	curPSP = &gameData.ai.routeSegs [aiP->nHideIndex];
		int16_t			nTargetSeg = TARGETOBJ->info.nSegment;
		int32_t			i;
		//	This is xProbably being done every frame, which is wasteful.
		for (i = aiP->nCurPathIndex; i < aiP->nPathLength; i++) {
			if (curPSP [i].nSegment == nTargetSeg) {
				CreateNSegmentPath (objP, AVOID_SEG_LENGTH, (nTargetSeg == objP->info.nSegment) ? -1 : nTargetSeg);
				Assert (aiP->nPathLength != 0);
				ailP->mode = AIM_RUN_FROM_OBJECT;	//	It gets bashed in CreateNSegmentPath
				break;
				}
			}
		if (nTargetVisibility) {
			ailP->targetAwarenessType = 1;
			ailP->targetAwarenessTime = I2X (1);
			}
		}
#if DBG
	if (aiP->nHideIndex < 0)
		aiP->nHideIndex = aiP->nHideIndex;
#endif
	}
if (aiP->nCurPathIndex < 0)
	aiP->nCurPathIndex = 0;
else if (aiP->nCurPathIndex >= aiP->nPathLength) {
	if (ailP->mode == AIM_RUN_FROM_OBJECT) {
		CreateNSegmentPath (objP, AVOID_SEG_LENGTH, TARGETOBJ->info.nSegment);
		ailP->mode = AIM_RUN_FROM_OBJECT;	//	It gets bashed in CreateNSegmentPath
		Assert (aiP->nPathLength != 0);
		}
	else {
		aiP->nCurPathIndex = aiP->nPathLength - 1;
		}
#if DBG
	if (aiP->nHideIndex < 0)
		aiP->nHideIndex = aiP->nHideIndex;
#endif
	}
vGoalPoint = (aiP->nHideIndex < 0) ? objP->Position () : gameData.ai.routeSegs [aiP->nHideIndex + aiP->nCurPathIndex].point;
//	If near goal, pick another goal point.
originalDir = aiP->PATH_DIR;
originalIndex = aiP->nCurPathIndex;
thresholdDistance = FixMul (objP->mType.physInfo.velocity.Mag (), gameData.time.xFrame) * 2 + I2X (2);
while (xDistToGoal < thresholdDistance) {
	//	Advance to next point on path.
	aiP->nCurPathIndex += aiP->PATH_DIR;
	//	See if next point wraps past end of path (in either direction), and if so, deal with it based on mode.p.
	if ((aiP->nCurPathIndex >= aiP->nPathLength) || (aiP->nCurPathIndex < 0)) {
		//	If mode = hiding, then stay here until get bonked or hit by player.
		// --	if (ailP->mode == AIM_BEHIND) {
		// --		ailP->mode = AIM_IDLING;
		// --		return;		// Stay here until bonked or hit by player.
		// --	} else

		//	Buddy bot.  If he's in mode to get away from CPlayerData and at end of line,
		//	if CPlayerData visible, then make a new path, else just return.
		if (botInfoP->companion) {
			if (gameData.escort.nSpecialGoal == ESCORT_GOAL_SCRAM) {
				if (nTargetVisibility) {
					CreateNSegmentPath (objP, 16 + RandShort () * 16, -1);
					aiP->nPathLength = SmoothPath (objP, &gameData.ai.routeSegs [aiP->nHideIndex], aiP->nPathLength);
					Assert (aiP->nPathLength != 0);
					ailP->mode = AIM_WANDER;	//	Special buddy mode.p.
					//--Int3_if (( (aiP->nCurPathIndex >= 0) && (aiP->nCurPathIndex < aiP->nPathLength));
					return;
					}
				else {
					ailP->mode = AIM_WANDER;	//	Special buddy mode.p.
					objP->mType.physInfo.velocity.SetZero ();
					objP->mType.physInfo.rotVel.SetZero ();
					//!!Assert ((aiP->nCurPathIndex >= 0) && (aiP->nCurPathIndex < aiP->nPathLength);
					return;
					}
				}
			}
		if (aiP->behavior == AIB_FOLLOW) {
			CreateNSegmentPath (objP, 10, TARGETOBJ->info.nSegment);
			//--Int3_if (( (aiP->nCurPathIndex >= 0) && (aiP->nCurPathIndex < aiP->nPathLength));
			}
		else if (aiP->behavior == AIB_STATION) {
			CreatePathToStation (objP, 15);
			if ((aiP->nHideSegment != gameData.ai.routeSegs [aiP->nHideIndex+aiP->nPathLength - 1].nSegment) ||
				 (aiP->nPathLength == 0)) {
				ailP->mode = AIM_IDLING;
				}
			else {
				//--Int3_if (( (aiP->nCurPathIndex >= 0) && (aiP->nCurPathIndex < aiP->nPathLength));
				}
			return;
			}
		else if (ailP->mode == AIM_FOLLOW_PATH) {
			CreatePathToTarget (objP, 10, 1);
			if ((aiP->nHideIndex+aiP->nPathLength < 1) ||
				 (aiP->nHideSegment != gameData.ai.routeSegs [aiP->nHideIndex+aiP->nPathLength - 1].nSegment)) {
				ailP->mode = AIM_IDLING;
				return;
				}
			else {
				//--Int3_if (( (aiP->nCurPathIndex >= 0) && (aiP->nCurPathIndex < aiP->nPathLength));
				}
			}
		else if (ailP->mode == AIM_RUN_FROM_OBJECT) {
			CreateNSegmentPath (objP, AVOID_SEG_LENGTH, TARGETOBJ->info.nSegment);
			ailP->mode = AIM_RUN_FROM_OBJECT;	//	It gets bashed in CreateNSegmentPath
			if (aiP->nPathLength < 1) {
				CreateNSegmentPath (objP, AVOID_SEG_LENGTH, TARGETOBJ->info.nSegment);
				ailP->mode = AIM_RUN_FROM_OBJECT;	//	It gets bashed in CreateNSegmentPath
				if (aiP->nPathLength < 1) {
					aiP->behavior = AIB_NORMAL;
					ailP->mode = AIM_IDLING;
					return;
					}
				}
			//--Int3_if (( (aiP->nCurPathIndex >= 0) && (aiP->nCurPathIndex < aiP->nPathLength));
			}
		else {
			//	Reached end of the line.p.  First see if opposite end point is reachable, and if so, go there.p.
			//	If not, turn around.
			int32_t			nOppositeEndIndex;
			CFixVector	*vOppositeEndPoint;
			CHitResult		hitResult;
			int32_t			fate;

			// See which end we're nearer and look at the opposite end point.
			if (abs (aiP->nCurPathIndex - aiP->nPathLength) < aiP->nCurPathIndex) {
				//	Nearer to far end (ie, index not 0), so try to reach 0.
				nOppositeEndIndex = 0;
				}
			else {
				//	Nearer to 0 end, so try to reach far end.
				nOppositeEndIndex = aiP->nPathLength-1;
				}
			//--Int3_if (( (nOppositeEndIndex >= 0) && (nOppositeEndIndex < aiP->nPathLength));
			vOppositeEndPoint = &gameData.ai.routeSegs [aiP->nHideIndex + nOppositeEndIndex].point;

			CHitQuery hitQuery (0, &objP->Position (), vOppositeEndPoint, objP->info.nSegment, objP->Index (), objP->info.xSize, objP->info.xSize);
			fate = FindHitpoint (hitQuery, hitResult);
			if (fate != HIT_WALL) {
				//	We can be circular! Do it!
				//	Path direction is unchanged.
				aiP->nCurPathIndex = nOppositeEndIndex;
				}
			else {
				aiP->PATH_DIR = -aiP->PATH_DIR;
				aiP->nCurPathIndex += aiP->PATH_DIR;
				}
				//--Int3_if (( (aiP->nCurPathIndex >= 0) && (aiP->nCurPathIndex < aiP->nPathLength));
			}
		break;
		}
	else {
		vGoalPoint = gameData.ai.routeSegs [aiP->nHideIndex + aiP->nCurPathIndex].point;
		xDistToGoal = CFixVector::Dist(vGoalPoint, objP->Position ());
		}
	//	If went all the way around to original point, in same direction, then get out of here!
	if ((aiP->nCurPathIndex == originalIndex) && (aiP->PATH_DIR == originalDir)) {
		CreatePathToTarget (objP, 3, 1);
		break;
		}
	}	//	end while
//	Set velocity (objP->mType.physInfo.velocity) and orientation (objP->info.position.mOrient) for this CObject.
AIPathSetOrientAndVel (objP, &vGoalPoint, nTargetVisibility, vecToTarget);
}

//	----------------------------------------------------------------------------------------------------------

class CObjPath {
	public:
		int16_t	nStart, nObject;
		bool operator< (CObjPath& other) { return nStart < other.nStart; }
		bool operator> (CObjPath& other) { return nStart > other.nStart; }
};

//	----------------------------------------------------------------------------------------------------------
//	Set orientation matrix and velocity for objP based on its desire to get to a point.
void AIPathSetOrientAndVel (CObject *objP, CFixVector *vGoalPoint, int32_t nTargetVisibility, CFixVector *vecToTarget)
{
	CFixVector	vCurVel = objP->mType.physInfo.velocity;
	CFixVector	vNormCurVel;
	CFixVector	vNormToGoal;
	CFixVector	vCurPos = objP->Position ();
	CFixVector	vNormFwd;
	fix			xSpeedScale;
	fix			xMaxSpeed;
	fix			dot;
	tRobotInfo	*botInfoP = ROBOTINFO (objP);

//	If evading CPlayerData, use highest difficulty level speed, plus something based on diff level
xMaxSpeed = FixMul (botInfoP->xMaxSpeed [gameStates.app.nDifficultyLevel], 2 * objP->DriveDamage ());
if ((gameData.ai.localInfo [objP->Index ()].mode == AIM_RUN_FROM_OBJECT) || (objP->cType.aiInfo.behavior == AIB_SNIPE))
	xMaxSpeed = 3 * xMaxSpeed / 2;
vNormToGoal = *vGoalPoint - vCurPos;
CFixVector::Normalize (vNormToGoal);
vNormCurVel = vCurVel;
CFixVector::Normalize (vNormCurVel);
vNormFwd = objP->info.position.mOrient.m.dir.f;
CFixVector::Normalize (vNormFwd);
dot = CFixVector::Dot (vNormToGoal, vNormFwd);
//	If very close to facing opposite desired vector, perturb vector
if (dot < -I2X (15) / 16)
	vNormCurVel = vNormToGoal;
else
	vNormCurVel += vNormToGoal / (I2X (1) / 2);
CFixVector::Normalize (vNormCurVel);
//	Set speed based on this robot nType's maximum allowed speed and how hard it is turning.
//	How hard it is turning is based on the dot product of (vector to goal) and (current velocity vector)
//	Note that since I2X (3)/4 is added to dot product, it is possible for the robot to back up.
//	Set speed and orientation.
if (dot < 0)
	dot /= -4;

//	If in snipe mode, can move fast even if not facing that direction.
if ((objP->cType.aiInfo.behavior == AIB_SNIPE) && (dot < I2X (1)/2))
	dot = (dot + I2X (1)) / 2;
xSpeedScale = FixMul (xMaxSpeed, dot);
vNormCurVel *= xSpeedScale;
objP->mType.physInfo.velocity = vNormCurVel;
if ((gameData.ai.localInfo [objP->Index ()].mode == AIM_RUN_FROM_OBJECT) || (botInfoP->companion == 1) || (objP->cType.aiInfo.behavior == AIB_SNIPE)) {
	if (gameData.ai.localInfo [objP->Index ()].mode == AIM_SNIPE_RETREAT_BACKWARDS) {
		if ((nTargetVisibility) && (vecToTarget != NULL))
			vNormToGoal = *vecToTarget;
		else
			vNormToGoal.Neg ();
		}
	AITurnTowardsVector (&vNormToGoal, objP, botInfoP->turnTime [NDL-1]/2);
	}
else
	AITurnTowardsVector (&vNormToGoal, objP, botInfoP->turnTime [gameStates.app.nDifficultyLevel]);
}

int32_t	nLastFrameGarbageCollected = 0;

//	----------------------------------------------------------------------------------------------------------
//	Garbage colledion -- Free all unused records in gameData.ai.routeSegs and compress all paths.
void AICollectPathGarbage (void)
{
#if DBG
	int32_t					i;
#endif
	int32_t					nFreeIndex = 0;
	int32_t					nObjects = 0;
	int32_t					nObject;
	int32_t					nObjIdx, nOldIndex;
	CObject*				objP;
	tAIStaticInfo*		aiP;
	CStaticArray<CObjPath, MAX_OBJECTS_D2X>	objectList;

nLastFrameGarbageCollected = gameData.app.nFrameCount;
#if PATH_VALIDATION
ValidateAllPaths ();
#endif
	//	Create a list of OBJECTS which have paths of length 1 or more.p.
FORALL_ROBOT_OBJS (objP) {
	if ((objP->info.controlType == CT_AI) || (objP->info.controlType == CT_MORPH)) {
		aiP = &objP->cType.aiInfo;
		if (aiP->nPathLength > 0) {
			if (aiP->nHideIndex < 0)
#if DBG
				aiP->nHideIndex = aiP->nHideIndex;
#else
				aiP->nPathLength = 0;
#endif
			else {	
				objectList [nObjects].nStart = aiP->nHideIndex;
				objectList [nObjects].nObject = objP->Index ();
				nObjects++;
				}
			}
		}
	}

if (nObjects > 0) {
	if (nObjects > 1) 
		objectList.SortAscending (0, nObjects - 1);
	for (nObjIdx = 0; nObjIdx < nObjects; nObjIdx++) {
		nObject = objectList [nObjIdx].nObject;
		objP = OBJECT (nObject);
		aiP = &objP->cType.aiInfo;
#if DBG
		if (aiP->nHideIndex < 0)
			aiP->nHideIndex = aiP->nHideIndex;
#endif
		nOldIndex = aiP->nHideIndex;
		aiP->nHideIndex = nFreeIndex;
#if DBG
		for (i = 0; i < aiP->nPathLength; i++)
			gameData.ai.routeSegs [nFreeIndex + i] = gameData.ai.routeSegs [nOldIndex + i];
		nFreeIndex += i;
#else
		memmove (&gameData.ai.routeSegs [nFreeIndex], &gameData.ai.routeSegs [nOldIndex], aiP->nPathLength * sizeof (tPointSeg));
		nFreeIndex += aiP->nPathLength;
#endif
		}
	}
gameData.ai.freePointSegs = gameData.ai.routeSegs + nFreeIndex;

#if DBG
FORALL_ROBOT_OBJS (objP)
	if (objP->info.controlType == CT_AI) {
		aiP = &objP->cType.aiInfo; 
		if ((aiP->nHideIndex + aiP->nPathLength > int32_t (gameData.ai.routeSegs.Index (gameData.ai.freePointSegs))) && (aiP->nPathLength > 0))
			Int3 ();		//	Contact Mike: Debug trap for nasty, elusive bug.
		}
#	if PATH_VALIDATION
ValidateAllPaths ();
#	endif
#endif
}

//	-----------------------------------------------------------------------------
//	Do garbage collection if not been done for awhile, or things getting really critical.
void MaybeAIPathGarbageCollect (void)
{
	int32_t i = gameData.ai.routeSegs.Index (gameData.ai.freePointSegs);

if (i > LEVEL_POINT_SEGS - MAX_PATH_LENGTH) {
	if (nLastFrameGarbageCollected + 1 >= gameData.app.nFrameCount) {
		//	This is kind of bad.  Garbage collected last frame or this frame.p.
		//	Just destroy all paths.  Too bad for the robots.  They are memory wasteful.
		AIResetAllPaths ();
#if TRACE
		console.printf (1, "Warning: Resetting all paths.  gameData.ai.routeSegs buffer nearly exhausted.\n");
#endif
		}
	else {
			//	We are really close to full, but didn't just garbage collect, so maybe this is recoverable.p.
#if TRACE
		console.printf (1, "Warning: Almost full garbage collection being performed: ");
#endif
		AICollectPathGarbage ();
#if TRACE
		console.printf (1, "Free records = %i/%i\n", LEVEL_POINT_SEGS - gameData.ai.routeSegs.Index (gameData.ai.freePointSegs), LEVEL_POINT_SEGS);
#endif
		}
	}
else if (i > 3 * LEVEL_POINT_SEGS / 4) {
	if (nLastFrameGarbageCollected + 16 < gameData.app.nFrameCount) {
		AICollectPathGarbage ();
		}
	}
else if (i > LEVEL_POINT_SEGS / 2) {
	if (nLastFrameGarbageCollected + 256 < gameData.app.nFrameCount) {
		AICollectPathGarbage ();
		}
	}
}

//	-----------------------------------------------------------------------------
//	Reset all paths.  Do garbage collection.
//	Should be called at the start of each level.
void AIResetAllPaths (void)
{
	CObject*	objP = OBJECTS.Buffer ();

FORALL_OBJS (objP)
	if (objP->info.controlType == CT_AI) {
		objP->cType.aiInfo.nHideIndex = -1;
		objP->cType.aiInfo.nPathLength = 0;
		}
AICollectPathGarbage ();
}

//	---------------------------------------------------------------------------------------------------------
//	Probably called because a robot bashed a CWall, getting a bunch of retries.
//	Try to resume path.
void AttemptToResumePath (CObject *objP)
{
	//int32_t				nObject = objP->Index ();
	tAIStaticInfo		*aiP = &objP->cType.aiInfo;
//	int32_t				nGoalSegnum, object_segnum,
//	int32_t				nAbsIndex, nNewPathIndex;

if ((aiP->behavior == AIB_STATION) && (ROBOTINFO (objP)->companion != 1))
	if (RandShort () > 8192) {
		tAILocalInfo *ailP = &gameData.ai.localInfo [objP->Index ()];

		aiP->nHideSegment = objP->info.nSegment;
//Int3 ();
		ailP->mode = AIM_IDLING;
#if TRACE
		console.printf (1, "Note: Bashing hide CSegment of robot %i to current CSegment because he's lost.\n", objP->Index ());
#endif
		}
#if 0
nAbsIndex = aiP->nHideIndex+aiP->nCurPathIndex;
nNewPathIndex = aiP->nCurPathIndex - aiP->PATH_DIR;
if ((nNewPathIndex >= 0) && (nNewPathIndex < aiP->nPathLength)) {
	aiP->nCurPathIndex = nNewPathIndex;
	}
else 
#endif
	{
	//	At end of line and have nowhere to go.
	MoveTowardsSegmentCenter (objP);
	if (!MoveObjectToLegalSpot (objP, 1))
		MoveObjectToLegalSpot (objP, 0);
	CreatePathToStation (objP, 15);
	}
}

//	----------------------------------------------------------------------------------------------------------

