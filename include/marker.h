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

#ifndef _MARKER_H
#define _MARKER_H

#include "player.h"

#define	MAX_DROP_MULTI		2
#define	MAX_DROP_COOP		3
#define	MAX_DROP_SINGLE	9

// -------------------------------------------------------------

static inline int MaxDrop (void)
{
return IsMultiGame ? IsCoopGame ? MAX_DROP_COOP : MAX_DROP_MULTI : MAX_DROP_SINGLE;
}

// -------------------------------------------------------------

void DropBuddyMarker (CObject *objP);
void DropSpawnMarker (void);
void DrawMarkers (void);
void RotateMarker (int bForce);
void DeleteMarker (int bForce);
void TeleportToMarker (void);
void ClearMarkers (void);
int LastMarker (void);
void InitMarkerInput (bool bRotate = false);
void MarkerInputMessage (int key);
int SpawnMarkerIndex (int nPlayer);
CObject *SpawnMarkerObject (int nPlayer);
int IsSpawnMarkerObject (CObject *objP);
int MoveSpawnMarker (tObjTransformation *posP, short nSegment);

// -------------------------------------------------------------

#endif //_MARKER_H
