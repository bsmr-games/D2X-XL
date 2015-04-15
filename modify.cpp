
#include "stdafx.h"
#include "dle-xp-res.h"

#include <math.h>
#include "define.h"
#include "types.h"
#include "global.h"
#include "mine.h"
#include "matrix.h"
#include "FileManager.h"
#include "textures.h"
#include "PaletteManager.h"
#include "dle-xp.h"

#define CURRENT_POINT(a) ((current->Point () + (a))&0x03)

extern short nDbgSeg, nDbgSide;
extern int nDbgVertex;

//------------------------------------------------------------------------------

bool CMine::EditGeoFwd (void)
{
	CDoubleVector	vDelta;
	CSegment*		segP = current->Segment ();

if (DLE.MineView ()->GetElementMovementReference ()) {
	vDelta = DLE.MineView ()->ViewMatrix ()->Forward ();
	}
else {
	vDelta = segP->ComputeCenter (current->SideId ()) - segP->ComputeCenter (short (oppSideTable [current->SideId ()]));
	// normalize direction
	double radius = vDelta.Mag ();
	if (radius > 0.1)
		vDelta /= radius;
	else
		vDelta = segmentManager.CalcSideNormal (*current);
	}
// move on x, y, and z
 vDelta *= DLE.MineView ()->MineMoveRate ();
 MoveElements (vDelta);
 return true;
}

//------------------------------------------------------------------------------

bool CMine::EditGeoBack (void) 
{
	CSegment*	segP = current->Segment ();
	CSide*		sideP = current->Side ();
	bool			okToMove = true;

	CEdgeList	edgeList;

	ubyte			v1, v2, s1, s2;
	short			nSide = current->SideId ();
	short			nOppSide = oppSideTable [nSide];
	short			nVertices = sideP->VertexCount ();
	int			nEdges = segP->BuildEdgeList (edgeList);
	short			p1, p2;
	double		moveRate = DLE.MineView ()->MineMoveRate ();
	
if (m_selectMode == POINT_MODE)
	p1 = p2 = current->Point ();
else if (m_selectMode == LINE_MODE) {
	p1 = current->Edge ();
	p2 = (current->Edge () + 1) % nVertices;
	okToMove = (p1 != p2);
	}

for (int i = 0; (i < nEdges) && okToMove; i++) {
	edgeList.Get (i, s1, s2, v1, v2);
	if ((s1 == nSide) || (s2 == nSide) || (s1 == nOppSide) || (s2 == nOppSide))
		continue;

	if (m_selectMode == POINT_MODE) {
		if ((p1 != v1) && (p1 != v2))
			continue;
		if (Distance (*segP->Vertex (v1), *segP->Vertex (v2)) - moveRate < 0.25) 
			okToMove = false;
		break;
		}
	else if (m_selectMode == LINE_MODE) {
		if ((p1 != v1) && (p1 != v2) && (p2 != v1) && (p2 != v2))
			continue;
		if (Distance (*segP->Vertex (v1), *segP->Vertex (v2)) - moveRate < 0.25) 
			okToMove = false;
		}
	else if (m_selectMode == SIDE_MODE) {
		if (Distance (*segP->Vertex (v1), *segP->Vertex (v2)) - moveRate < 0.25) 
			okToMove = false;
		}
	}

if (!okToMove) {
	ErrorMsg ("Too small to move in that direction");
	return false;
	}

CDoubleVector vDelta;
if (DLE.MineView ()->GetElementMovementReference ())
	vDelta = DLE.MineView ()->ViewMatrix ()->Forward ();
else {
	vDelta = segP->ComputeCenter (current->SideId ()) - segP->ComputeCenter (short (oppSideTable [current->SideId ()]));
	// normalize direction
	double radius = vDelta.Mag ();
	if (radius > 0.1)
		vDelta /= radius;
	else
		vDelta = segmentManager.CalcSideNormal (*current);
	}
vDelta *= -DLE.MineView ()->MineMoveRate ();
MoveElements (vDelta);
return true;
}

//------------------------------------------------------------------------------

bool CMine::EditGeoRotRight (void)
{
return SpinSelection (-theMine->RotateRate ());
}

//------------------------------------------------------------------------------

bool CMine::EditGeoRotLeft (void)
{
return SpinSelection (theMine->RotateRate ());
}

//------------------------------------------------------------------------------

bool CMine::EditGeoUp (void) 
{
if (m_selectMode == SIDE_MODE) 
	return RotateSelection (theMine->RotateRate (), false);
if (!DLE.MineView ()->GetElementMovementReference ())
	return MovePoints (1, 0);
return MovePoints (DLE.MineView ()->ViewMatrix ()->Up ());
}

//------------------------------------------------------------------------------

bool CMine::EditGeoDown (void) 
{
if (m_selectMode == SIDE_MODE)
	return RotateSelection (-theMine->RotateRate (), false);
if (!DLE.MineView ()->GetElementMovementReference ())
	return MovePoints (0, 1);
return MovePoints (DLE.MineView ()->ViewMatrix ()->Up () * -1.0);
}

//------------------------------------------------------------------------------

bool CMine::EditGeoRight (void) 
{
if (m_selectMode == SIDE_MODE) 
	return RotateSelection (theMine->RotateRate (), true);
if (!DLE.MineView ()->GetElementMovementReference ())
	return MovePoints (current->Side ()->VertexCount () -1, 0);
return MovePoints (DLE.MineView ()->ViewMatrix ()->Left () * -1.0);
}

//------------------------------------------------------------------------------

bool CMine::EditGeoLeft (void) 
{
if (m_selectMode == SIDE_MODE) 
	return RotateSelection (-theMine->RotateRate (),true);
if (!DLE.MineView ()->GetElementMovementReference ())
	return MovePoints (0, current->Side ()->VertexCount () - 1);
return MovePoints (DLE.MineView ()->ViewMatrix ()->Left ());
}

//------------------------------------------------------------------------------

bool CMine::EditGeoGrow (void) 
{
return ResizeItem (DLE.MineView ()->MineMoveRate ());
}

//------------------------------------------------------------------------------

bool CMine::EditGeoShrink (void) 
{
return ResizeItem (-DLE.MineView ()->MineMoveRate ());
}

//------------------------------------------------------------------------------
//                    RotateSelection()
//
// ACTION - rotates a side about the opposite side.  The line is drawn
//          between the center points of lines 0 and 2.  If perpendicular
//          is true, then the lines 1 and 3 are used instead.
//------------------------------------------------------------------------------

bool CMine::RotateSelection (double angle, bool perpendicular) 
{
	int			nSegment = current->SegmentId ();
	int			nSide = current->SideId (), nOppSide = oppSideTable [nSide];
	CSegment*	segP = current->Segment ();
	CSide*		sideP = segP->Side (nSide);
	CVertex		center, oppCenter;
	ubyte			oppSideVerts [4];
	int			pts [4];
	int			i, n;

switch (m_selectMode) {
	case POINT_MODE:
		ErrorMsg ("Cannot bend a point");
		return false;

	case LINE_MODE:
		ErrorMsg ("Cannot bend a line");
		return false;

	case SIDE_MODE:	// spin side around the opposite side
		if (perpendicular) { // use lines 0 and 2
			pts [0] = 1;
			pts [1] = 2;
			pts [2] = 3;
			pts [3] = 0;
			} 
		else {             // use lines 1 and 3
			pts [0] = 0;
			pts [1] = 1;
			pts [2] = 2;
			pts [3] = 3;
			}
		// calculate center opp side line 0
		segP->CreateOppVertexIndex (nSide, oppSideVerts);
		n = segP->Side (nOppSide)->VertexCount ();
		oppCenter = Average (*segP->Vertex (oppSideVerts [pts [0] % n]),
									*segP->Vertex (oppSideVerts [pts [1] % n]));
		// calculate center opp side line 2
		center = Average (*segP->Vertex (oppSideVerts [pts [2] % n]),
									*segP->Vertex (oppSideVerts [pts [3] % n]));
		// rotate points around a line
		undoManager.Begin (__FUNCTION__, udVertices | udSegments);
		n = sideP->VertexCount ();
		for (i = 0; i < n; i++)
			segP->Vertex (sideP->VertexIdIndex (i))->Rotate (center, oppCenter, angle);
		undoManager.End (__FUNCTION__);	
		break;
	
	case SEGMENT_MODE:
		ErrorMsg ("Cannot bend a segment");
		return false;
	
	case OBJECT_MODE:
		ErrorMsg ("Cannot bend an object");
		return false;

	case BLOCK_MODE:
		ErrorMsg ("Cannot bend a block");
		return false;
	}
return true;
}

//--------------------------------------------------------------------------------
//     				ResizeLine()
//
// prevent lines from being bigger than 8*20 and less than 3
//--------------------------------------------------------------------------------

bool CMine::ResizeLine (CSegment *segP, int nVertexId1, int nVertexId2, double delta, int nStage) 
{
if ((nStage < 0) || (nStage == 0)) {
	CVertex	*v1 = segP->Vertex (nVertexId1),
				*v2 = segP->Vertex (nVertexId2);
	CDoubleVector v (*v1 - *v2);
	// figure out direction to modify line
	// normalize direction
	double radius = v.Mag ();
	if (radius > (double) F1_0 - delta) 
		return false;
	if ((delta < 0.0) && (radius <= -delta * 3.0)) 
		return false;
	if (radius == 0.0)
		return false;
	v *= delta / radius;
	// update vertices
	v1->SetDelta (v);
	v.Negate ();
	v2->SetDelta (v);
	}
if ((nStage < 0) || (nStage == 1)) {
	segmentManager.UpdateTexCoords (nVertexId1, false);
	segmentManager.UpdateTexCoords (nVertexId2, false);
	}
if ((nStage < 0) || (nStage == 2)) {
	segP->Vertex (nVertexId1)->Move ();
	segP->Vertex (nVertexId2)->Move ();
	segmentManager.UpdateTexCoords (nVertexId1, true);
	segmentManager.UpdateTexCoords (nVertexId2, true);
	}
return true;
}

//***************************************************************************
//			ResizeItem()
//
// need to prevent reduction through zero
// absolute value of shorts line to size must be greater
// then incremental value if inc is negetive
//
//***************************************************************************

bool CMine::ResizeItem (double delta) 
{
	int			nSegment = current->SegmentId ();
	int			nSide = current->SideId ();
	CSegment*	segP = current->Segment ();
	CSide*		sideP = segP->Side (nSide);
	int			i, j, n = sideP->VertexCount ();
	ubyte			s1, s2, v1, v2;
	int			point [4];
	bool			result = false;

	CEdgeList	edgeList;

switch (m_selectMode) {
	case POINT_MODE:
		return false;

	case LINE_MODE:
		point [0] = sideP->VertexIdIndex (current->Edge ());
		point [1] = sideP->VertexIdIndex (current->Edge () + 1);
		undoManager.Begin (__FUNCTION__, udVertices | udSegments);
		result = ResizeLine (segP, point [0], point [1], delta, -1);
		undoManager.End (__FUNCTION__);
		return result;

	case SIDE_MODE:
		if (sideP->Shape () > SIDE_SHAPE_EDGE)
			return false;
		undoManager.Begin (__FUNCTION__, udVertices | udSegments);
		delta *= sqrt (2.0);
#if 0
		for (i = 0; i < n; i++)
			if (!ResizeLine (segP, sideP->VertexIdIndex (i), sideP->VertexIdIndex (i + 1)))
				result = false;
#else
		for (i = 0; i < n; i++)
			point [i] = sideP->VertexIdIndex (i);
		// enlarge the diagonals -- doesn't work for triangular sides
		for (i = 0; i < 3; i++)
			if (!(result = ResizeLine (segP, point [0], point [2], delta, i) && ((n < 4) || ResizeLine (segP, point [1], point [3], delta, i))))
				break;
#endif
		undoManager.End (__FUNCTION__);
		return result;

	case SEGMENT_MODE: {
		CVertex	maxPoint (-0x7fffffffL, -0x7fffffffL, -0x7fffffffL), 
					minPoint (0x7fffffffL, 0x7fffffffL, 0x7fffffffL), 
					center;
		for (i = 0; i < 8; i++) {
			if (segP->VertexId (i) <= MAX_VERTEX) {
				CVertex* vertP = segP->Vertex (i);
				maxPoint = Max (maxPoint, *vertP);
				minPoint = Min (minPoint, *vertP);
				}
			}
		center = Average (maxPoint, minPoint);
		undoManager.Begin (__FUNCTION__, udVertices | udSegments);
		if (delta < 0.0)
			delta = 1.0 - sqrt (3.0) / Distance (minPoint, maxPoint) * DLE.MineView ()->MineMoveRate ();
		else
			delta = 1.0 + sqrt (3.0) / Distance (minPoint, maxPoint) * DLE.MineView ()->MineMoveRate ();
		for (i = 0; i < 8; i++) {
			ushort nVertexId = segP->VertexId (i);
			if (nVertexId <= MAX_VERTEX) {
				CVertex v = vertexManager [nVertexId];
				v -= center;
				v *= delta;
				v += center;
				v -= vertexManager [nVertexId];
				vertexManager [nVertexId].SetDelta (v);
				}
			}
		for (i = 0; i < 8; i++) {
			ushort nVertexId = segP->VertexId (i);
			if (nVertexId <= MAX_VERTEX) 
				segmentManager.UpdateTexCoords (nVertexId, false);
			}
		for (i = 0; i < 8; i++) {
			ushort nVertexId = segP->VertexId (i);
			if (nVertexId <= MAX_VERTEX) {
				vertexManager [nVertexId].Move ();
				segmentManager.UpdateTexCoords (nVertexId, true);
				}
			}
#if 1
		n = segP->BuildEdgeList (edgeList);
		for (i = 0; i < n; i++) {
			edgeList.Get (i, s1, s2, v1, v2);
			if (Distance (*segP->Vertex (v1), *segP->Vertex (v2)) < 1.0) {
				undoManager.Unroll (__FUNCTION__);
				return false;
				}
			}
#endif
		undoManager.End (__FUNCTION__);
		return true;
		}

	case OBJECT_MODE:
		return false;

	case BLOCK_MODE: {
		CVertex	maxPoint (-0x7fffffffL, -0x7fffffffL, -0x7fffffffL), 
					minPoint (0x7fffffffL, 0x7fffffffL, 0x7fffffffL), 
					center;
		CVertex* vertexP = vertexManager.Vertex (0);
		for (i = vertexManager.Count (), j = 0; j < i; j++, vertexP++)
			if (vertexP->Status () & TAGGED_MASK) {
				maxPoint = Max (maxPoint, *vertexP);
				minPoint = Min (minPoint, *vertexP);
				}
		center = Average (maxPoint, minPoint);
		undoManager.Begin (__FUNCTION__, udVertices | udSegments);
		//double scale = (20.0 + delta) / 20.0;
		if (delta < 0.0)
			delta = 1.0 - sqrt (3.0) / Distance (minPoint, maxPoint) * DLE.MineView ()->MineMoveRate ();
		else
			delta = 1.0 + sqrt (3.0) / Distance (minPoint, maxPoint) * DLE.MineView ()->MineMoveRate ();
		j = vertexManager.Count ();
		vertexP = vertexManager.Vertex (0);
		for (i = 0; i < j; i++, vertexP++)
			if (vertexP->IsTagged ()) {
				CVertex v = *vertexP;
				v -= center;
				v *= delta;
				v += center;
				v -= *vertexP;
				vertexP->SetDelta (v);
				}
		vertexP = vertexManager.Vertex (0);
		for (i = 0; i < j; i++, vertexP++)
			if (vertexP->IsTagged ())
				segmentManager.UpdateTexCoords (i, false);
		vertexP = vertexManager.Vertex (0);
		for (i = 0; i < j; i++, vertexP++)
			if (vertexP->IsTagged ()) {
				vertexP->Move ();
				segmentManager.UpdateTexCoords (i, true);
				}
		undoManager.End (__FUNCTION__);
		return true;
		}
	}
return false;
}

//--------------------------------------------------------------------------------
// MovePoints()
//
// moves blocks, sides, segments, lines, and points in the direction
// of the current line.
//--------------------------------------------------------------------------------

bool CMine::MovePoints (CDoubleVector vDelta) 
{
	CSegment*		segP = current->Segment ();
	CSide*			sideP = segP->Side (current->SideId ());
	double			length = vDelta.Mag ();
	int				i;
	short				selectMode = DLE.MineView ()->Perspective () ? BLOCK_MODE : m_selectMode;


if (length < 1.0) 
	vDelta.Set (DLE.MineView ()->MineMoveRate (), 0, 0);
else
	vDelta *= ((double) DLE.MineView ()->MineMoveRate () / length);

switch (selectMode) {
	case POINT_MODE:
		undoManager.Begin (__FUNCTION__, udVertices | udSegments);
		segP->Vertex (current->SideId (), current->Point ())->SetDelta (vDelta);
		segmentManager.UpdateTexCoords (segP->VertexId (current->SideId (), current->Point ()), false);
		segP->Vertex (current->SideId (), current->Point ())->Move ();
		segmentManager.UpdateTexCoords (segP->VertexId (current->SideId (), current->Point ()), true);
		undoManager.End (__FUNCTION__);
		break;

	case LINE_MODE:
		undoManager.Begin (__FUNCTION__, udVertices | udSegments);
		segP->Vertex (current->SideId (), current->Point ())->SetDelta (vDelta);
		segP->Vertex (current->SideId (), current->Point () + 1)->SetDelta (vDelta);
		segmentManager.UpdateTexCoords (segP->VertexId (current->SideId (), current->Point ()), false);
		segmentManager.UpdateTexCoords (segP->VertexId (current->SideId (), current->Point () + 1), false);
		segP->Vertex (current->SideId (), current->Point ())->Move ();
		segP->Vertex (current->SideId (), current->Point () + 1)->Move ();
		segmentManager.UpdateTexCoords (segP->VertexId (current->SideId (), current->Point ()), true);
		segmentManager.UpdateTexCoords (segP->VertexId (current->SideId (), current->Point () + 1), true);
		undoManager.End (__FUNCTION__);
		break;

	case SIDE_MODE:
		undoManager.Begin (__FUNCTION__, udVertices | udSegments);
		for (i = 0; i < sideP->VertexCount (); i++) {
#ifdef _DEBUG
			if (i == nDbgSide)
				nDbgSide = nDbgSide;
#endif
			segP->Vertex (current->SideId (), i)->SetDelta (vDelta);
			segmentManager.UpdateTexCoords (segP->VertexId (current->SideId (), ubyte (i)), false);
			segP->Vertex (current->SideId (), i)->Move ();
			segmentManager.UpdateTexCoords (segP->VertexId (current->SideId (), ubyte (i)), true);
			}
		undoManager.End (__FUNCTION__);
		break;

	case SEGMENT_MODE:
		undoManager.Begin (__FUNCTION__, udVertices | udSegments);
		for (i = 0; i < 8; i++) {
			if (segP->VertexId (i) <= MAX_VERTEX) {
				segP->Vertex (i)->SetDelta (vDelta);
				segmentManager.UpdateTexCoords (ushort (i), false);
				segP->Vertex (i)->Move ();
				segmentManager.UpdateTexCoords (ushort (i), true);
				}
			}
		undoManager.End (__FUNCTION__);
		break;

	case OBJECT_MODE:
		undoManager.Begin (__FUNCTION__, udObjects);
		current->Object ()->Position () += vDelta;
		undoManager.End (__FUNCTION__);
		break;

	case BLOCK_MODE:
		bool bMoved = false;
		undoManager.Begin (__FUNCTION__, udVertices | udSegments);
		for (i = 0; i < MAX_VERTICES; i++) {
			if (vertexManager.Status (i) & TAGGED_MASK) {
				vertexManager.Vertex (i)->SetDelta (vDelta);
				segmentManager.UpdateTexCoords (ushort (i), false);
				vertexManager.Vertex (i)->Move ();
				segmentManager.UpdateTexCoords (ushort (i), true);
				bMoved = true;
				}
			}
		undoManager.End (__FUNCTION__);
		break;
	}
return true;
}

//--------------------------------------------------------------------------------

bool CMine::MovePoints (int pt0, int pt1) 
{
CSegment* segP = current->Segment ();
return MovePoints (*segP->Vertex (current->SideId (), current->Point () + pt1) - *segP->Vertex (current->SideId (), current->Point () + pt0));
}

/***************************************************************************
				MoveElements()
***************************************************************************/

bool CMine::MoveElements (CDoubleVector vDelta) 
{
	int				nSegment = current->SegmentId ();
	int				nSide = current->SideId ();
	int				nPoint = current->Point ();
	int				nLine = current->Edge ();
	int				i;
	CSegment*		segP = current->Segment ();
	CSide*			sideP = current->Side ();
	CGameObject*	objP;
	short				selectMode = DLE.MineView ()->Perspective () ? BLOCK_MODE : m_selectMode;

switch (selectMode) {
	case POINT_MODE:
		undoManager.Begin (__FUNCTION__, udVertices | udSegments);
		segP->Vertex (sideP->VertexIdIndex (nPoint))->SetDelta (vDelta);
		segmentManager.UpdateTexCoords (segP->VertexId (nSide, nPoint), false);
		segP->Vertex (sideP->VertexIdIndex (nPoint))->Move ();
		segmentManager.UpdateTexCoords (segP->VertexId (nSide, nPoint), true);
		undoManager.End (__FUNCTION__);
		break;

	case LINE_MODE:
		undoManager.Begin (__FUNCTION__, udVertices | udSegments);
		segP->Vertex (sideP->VertexIdIndex (nLine))->SetDelta (vDelta);
		segP->Vertex (sideP->VertexIdIndex (nLine + 1))->SetDelta (vDelta);
		segmentManager.UpdateTexCoords (segP->VertexId (nSide, nLine), false);
		segmentManager.UpdateTexCoords (segP->VertexId (nSide, nLine + 1), false);
		segP->Vertex (sideP->VertexIdIndex (nLine))->Move ();
		segP->Vertex (sideP->VertexIdIndex (nLine + 1))->Move ();
		segmentManager.UpdateTexCoords (segP->VertexId (nSide, nLine), true);
		segmentManager.UpdateTexCoords (segP->VertexId (nSide, nLine + 1), true);
		undoManager.End (__FUNCTION__);
		break;

	case SIDE_MODE:
		undoManager.Begin (__FUNCTION__, udVertices | udSegments);
		for (i = 0; i < sideP->VertexCount (); i++)
			segP->Vertex (sideP->VertexIdIndex (i))->SetDelta (vDelta);
		for (i = 0; i < sideP->VertexCount (); i++)
			segmentManager.UpdateTexCoords (segP->VertexId (nSide, i), false, current->SegmentId (), current->SideId ());
		for (i = 0; i < sideP->VertexCount (); i++) {
			segP->Vertex (sideP->VertexIdIndex (i))->Move ();
			segmentManager.UpdateTexCoords (segP->VertexId (nSide, i), true, current->SegmentId (), current->SideId ());
			}
		undoManager.End (__FUNCTION__);
		break;

	case SEGMENT_MODE:
		undoManager.Begin (__FUNCTION__, udVertices | udObjects);
		for (i = 0; i < 8; i++) 
			if (segP->VertexId (i) <= MAX_VERTEX)
				segP->Vertex (i)->SetDelta (vDelta);
		for (i = 0; i < 8; i++) 
			if (segP->VertexId (i) <= MAX_VERTEX)
				segmentManager.UpdateTexCoords (segP->VertexId (i), false, current->SegmentId ());
		for (i = 0; i < 8; i++) 
			if (segP->VertexId (i) <= MAX_VERTEX) {
				segP->Vertex (i)->Move ();
				segmentManager.UpdateTexCoords (segP->VertexId (i), true, current->SegmentId ());
				}
		objP = objectManager.Object (0);
		for (i = objectManager.Count (); i; i--, objP++)
			if (objP->m_info.nSegment == nSegment)
				objP->Position () += vDelta;
		undoManager.End (__FUNCTION__);
		break;

	case OBJECT_MODE:
		undoManager.Begin (__FUNCTION__, udObjects);
		current->Object ()->Position () += vDelta;
		undoManager.End (__FUNCTION__);
		break;
	break;

	case BLOCK_MODE:
		undoManager.Begin (__FUNCTION__, udObjects);
		CGameObject *objP = objectManager.Object (0);
		for (i = 0; i < MAX_VERTICES; i++)
			if (vertexManager.Status (i) & TAGGED_MASK) 
				vertexManager.Vertex (i)->SetDelta (vDelta);
		for (i = 0; i < MAX_VERTICES; i++)
			if (vertexManager.Status (i) & TAGGED_MASK) 
				segmentManager.UpdateTexCoords (i, false, 0x7FFF);
		for (i = 0; i < MAX_VERTICES; i++)
			if (vertexManager.Status (i) & TAGGED_MASK) {
				vertexManager.Vertex (i)->Move ();
				segmentManager.UpdateTexCoords (i, true, 0x7FFF);
				}
		for (i = objectManager.Count (); i; i--, objP++)
			if (objP->m_info.nSegment >= 0)
				if (objP->Segment ()->IsTagged ())
					objP->Position () += vDelta;
		undoManager.End (__FUNCTION__);
		break;
	}
return true;
}

/***************************************************************************
			    SpinSelection()

  ACTION - Spins a side, segment, or object the amount specified.

***************************************************************************/

class CVertexSpinner {
	public:
	short				m_nSegment;
	short				m_nSide;
	short				m_nOppSide;
	CVertex			m_vCenter, m_vOppCenter, m_vNormal;
	double			m_angle;

	explicit CVertexSpinner (double angle = 0.0) : m_angle (angle) {}
	void Setup (void);
	void SetDelta (ushort nVertex);
	void UpdateTexCoords (ushort nVertexId, bool bMove);
	void Move (ushort nVertexId);
};

// -----------------------------------------------------------------------------

void CVertexSpinner::Setup (void)
{
m_nSegment = current->SegmentId ();
m_nSide = current->SideId ();
m_nOppSide = oppSideTable [m_nSide];
}

// -----------------------------------------------------------------------------

void CVertexSpinner::SetDelta (ushort nVertexId)
{
if (nVertexId <= MAX_VERTEX) {
	CVertex v = vertexManager [nVertexId];
	v.Rotate (m_vCenter, m_vOppCenter, m_angle);
	v -= vertexManager [nVertexId];
	vertexManager [nVertexId].SetDelta (v);
	}
}

// -----------------------------------------------------------------------------

void CVertexSpinner::UpdateTexCoords (ushort nVertexId, bool bMove)
{
if (nVertexId <= MAX_VERTEX)
	segmentManager.UpdateTexCoords (nVertexId, bMove, m_nSegment, m_nSide);
}

// -----------------------------------------------------------------------------

void CVertexSpinner::Move (ushort nVertexId)
{
if (nVertexId <= MAX_VERTEX)
	vertexManager [nVertexId].Move ();
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

bool CMine::SpinSelection (double angle) 
{
	CSegment*		segP = current->Segment ();
	CSide*			sideP = current->Side ();
	CGameObject*	objP;
	CDoubleMatrix* orient;
	CVertexSpinner	spinner (angle);
	int				i, j;
	short				selectMode = DLE.MineView ()->Perspective () ? BLOCK_MODE : m_selectMode;

spinner.Setup ();
/* calculate segment pointer */
switch (selectMode) {
	case POINT_MODE:
		ErrorMsg ("Cannot spin a point");
		return false;
	
	case LINE_MODE:
		ErrorMsg ("Cannot spin a line");
		return false;
	
	case SIDE_MODE: // spin side around its center in the plane of the side
		// calculate center of current side
		spinner.m_vCenter = segP->ComputeCenter (spinner.m_nSide);
		// calculate orthogonal vector from lines which intersect point 0
		//       |x  y  z |
		// AxB = |ax ay az| = x(aybz-azby), y(azbx-axbz), z(axby-aybx)
		//       |bx by bz|
		if (DLE.MineView ()->GetElementMovementReference ())
			spinner.m_vOppCenter = spinner.m_vCenter + DLE.MineView ()->ViewMatrix ()->Forward ();
		else {
			segP->ComputeNormals (spinner.m_nSide);
			spinner.m_vNormal = sideP->Normal ();
			spinner.m_vOppCenter = spinner.m_vCenter + spinner.m_vNormal;
			}
		/* rotate points around a line */
		undoManager.Begin (__FUNCTION__, udVertices | udSegments);
		j = sideP->VertexCount ();
		for (i = 0; i < j; i++) 
			spinner.SetDelta (segP->VertexId (current->SideId (), i));
		for (i = 0; i < j; i++) 
			spinner.UpdateTexCoords (segP->VertexId (current->SideId (), i), false);
		for (i = 0; i < j; i++) {
			spinner.Move (segP->VertexId (current->SideId (), i));
			spinner.UpdateTexCoords (segP->VertexId (current->SideId (), i), true);
			}
		undoManager.End (__FUNCTION__);
		break;

	case SEGMENT_MODE:	// spin segment around the spinner.m_vCenter of the segment using screen's perspective
		spinner.m_vCenter = segP->ComputeCenter ();
		spinner.m_vOppCenter = DLE.MineView ()->GetElementMovementReference () ? DLE.MineView ()->ViewMatrix ()->Forward () : segP->ComputeCenter ((short) oppSideTable [spinner.m_nSide]);

		// rotate points about a point
		undoManager.Begin (__FUNCTION__, udVertices | udSegments);
		for (i = 0; i < 8; i++)
			spinner.SetDelta (segP->VertexId (i));
		for (i = 0; i < 8; i++)
			spinner.UpdateTexCoords (segP->VertexId (i), false);
		for (i = 0; i < 8; i++) {
			spinner.Move (segP->VertexId (i));
			spinner.UpdateTexCoords (segP->VertexId (i), true);
			}
		undoManager.End (__FUNCTION__);
		break;

	case OBJECT_MODE:	// spin object vector
		undoManager.Begin  (__FUNCTION__, udObjects);
		orient = (current->ObjectId () == objectManager.Count ()) ? &objectManager.SecretOrient () : &current->Object ()->Orient ();
		switch (spinner.m_nSide) {
			case 0:
				orient->Rotate (angle, 'x');
				break;
			case 2:
				orient->Rotate (-angle, 'x');
				break;
			case 1:
				orient->Rotate (-angle, 'y');
				break;
			case 3:
				orient->Rotate (angle, 'y');
				break;
			case 4:
				orient->Rotate (angle, 'z');
				break;
			case 5:
				orient->Rotate (-angle, 'z');
				break;
			}
		undoManager.End (__FUNCTION__);
		break;

	case BLOCK_MODE:
		spinner.m_vCenter = segP->ComputeCenter ();
		if (DLE.MineView ()->GetElementMovementReference ()) {
			spinner.m_vOppCenter = spinner.m_vCenter + DLE.MineView ()->ViewMatrix ()->Forward ();
			CVertex& v = segP->ComputeCenter ();
			CVertex v1 = spinner.m_vCenter - v;
			CVertex v2 = spinner.m_vOppCenter - v;
			if (Dot (v1, v2) < 0.0)
				spinner.m_vOppCenter = spinner.m_vCenter - DLE.MineView ()->ViewMatrix ()->Forward ();
			}
		else
			spinner.m_vOppCenter = segP->ComputeCenter ((short) oppSideTable [spinner.m_nSide]);
		// rotate points about a point
		undoManager.Begin  (__FUNCTION__, udVertices | udObjects);
		for (i = 0; i < vertexManager.Count (); i++)
			if (vertexManager.Status (i) & TAGGED_MASK)
				spinner.SetDelta (i);
		for (i = 0; i < vertexManager.Count (); i++)
			if (vertexManager.Status (i) & TAGGED_MASK)
				spinner.UpdateTexCoords (i, false);
		for (i = 0; i < vertexManager.Count (); i++)
			if (vertexManager.Status (i) & TAGGED_MASK) {
				spinner.Move (i);
				spinner.UpdateTexCoords (i, true);
				}
		// rotate Objects () within marked segments
		objP = objectManager.Object (0);
		for (i = objectManager.Count (); i; i--, objP++)
			if (objP->Segment ()->IsTagged ())
				objP->Position ().Rotate (spinner.m_vCenter, spinner.m_vOppCenter, angle);
		undoManager.End (__FUNCTION__);
		break;
	}
return true;
}

// -----------------------------------------------------------------------------
// eof modify.cpp