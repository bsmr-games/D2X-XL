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

#ifdef _WIN32
#	include <windows.h>
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "pstypes.h"
#include "strutil.h"
#include "descent.h"
#include "gr.h"
#include "u_mem.h"
#include "iff.h"
#include "mono.h"
#include "error.h"
#include "sounds.h"
#include "songs.h"
#include "loadgamedata.h"
#include "bmread.h"
#include "loadgeometry.h"
#include "hash.h"
#include "args.h"
#include "palette.h"
#include "gamefont.h"
#include "rle.h"
#include "screens.h"
#include "piggy.h"
#include "loadgeometry.h"
#include "textures.h"
#include "texmerge.h"
#include "paging.h"
#include "game.h"
#include "text.h"
#include "cfile.h"
#include "menu.h"
#include "byteswap.h"
#include "findfile.h"
#include "makesig.h"
#include "effects.h"
#include "wall.h"
#include "weapon.h"
#include "error.h"
#include "grdef.h"
#include "gamepal.h"
#include "hiresmodels.h"
#include "pcx.h"

//#define NO_DUMP_SOUNDS        1   //if set, dump bitmaps but not sounds

#if DBG
#	define PIGGY_MEM_QUOTA	4
#else
#	define PIGGY_MEM_QUOTA	8
#endif

const char* szPowerupTextures [] = {
	// guns
	"fusion#",
	"gauss#",
	"helix#",
	"laser#",
	"omega#",
	"plasma#",
	"phoenix#",
	"spread#",
	"suprlasr#",
	"vulcan#",
	// ammo
	"vammo#",
	// equipment
	"aftrbrnr#",
	"allmap#",
	"ammorack#",
	"convert#",
	"headlite#",
	"quad#",
	// mines
	"pbombs#",
	"spbombs#",
	"pbomb#",
	"spbomb#"
	};

const char *szAddonTextures [MAX_ADDON_BITMAP_FILES] = {
	"slowmotion#0",
	"bullettime#0",
	"targ01b#0-green",
	"targ01b#1-green",
	"targ02b#0-green",
	"targ02b#1-green",
	"targ02b#2-green",
	"targ03b#0-green",
	"targ03b#1-green",
	"targ03b#2-green",
	"targ03b#3-green",
	"targ03b#4-green",
	"targ01b#0-red",
	"targ01b#1-red",
	"targ02b#0-red",
	"targ02b#1-red",
	"targ02b#2-red",
	"targ03b#0-red",
	"targ03b#1-red",
	"targ03b#2-red",
	"targ03b#3-red",
	"targ03b#4-red"
	};

static int16_t d2OpaqueDoors [] = {
	440, 451, 463, 477, /*483,*/ 488,
	500, 508, 523, 550, 556, 564, 572, 579, 585, 593, 536,
	600, 608, 615, 628, 635, 6
, 649, 664, /*672,*/ 687,
	702, 717, 725, 731, 738, 745, 754, 763, 772, 780, 790,
	806, 817, 827, 838, 849, /*858,*/ 863, 871, 886,
	901,
	-1};

#define RLE_REMAP_MAX_INCREASE 132 /* is enough for d1 pc registered */

#if DBG
#	define PIGGY_BUFFER_SIZE ((uint32_t) (512*1024*1024))
#else
#	define PIGGY_BUFFER_SIZE ((uint32_t) 0x7fffffff)
#endif
#define PIGGY_SMALL_BUFFER_SIZE (16*1024*1024)

#define DBM_FLAG_ABM    64 // animated bitmap
#define DBM_NUM_FRAMES  63

#define BM_FLAGS_TO_COPY (BM_FLAG_TRANSPARENT | BM_FLAG_SUPER_TRANSPARENT | BM_FLAG_NO_LIGHTING | BM_FLAG_RLE | BM_FLAG_RLE_BIG)

extern uint8_t bBigPig;

//------------------------------------------------------------------------------

#if DBG
typedef struct tTrackedBitmaps {
	CBitmap*	bmP;
	int32_t		nSize;
} tTrackedBitmaps;

tTrackedBitmaps	trackedBitmaps [100000];
int32_t					nTrackedBitmaps = 0;
#endif

void UseBitmapCache (CBitmap *bmP, int32_t nSize)
{
if ((nSize > 0) || (uint32_t (-nSize) < bitmapCacheUsed))
	bitmapCacheUsed += nSize;
else
	bitmapCacheUsed = 0;
#if DBG
if (nSize < 0) {
	for (int32_t i = 0; i < nTrackedBitmaps; i++)
		if (trackedBitmaps [i].bmP == bmP) {
			//CBP (trackedBitmaps [i].nSize != -nSize);
			if (i < --nTrackedBitmaps)
				trackedBitmaps [i] = trackedBitmaps [nTrackedBitmaps];
			trackedBitmaps [nTrackedBitmaps].bmP = NULL;
			trackedBitmaps [nTrackedBitmaps].nSize = 0;
			break;
			}
	}
else {
	trackedBitmaps [nTrackedBitmaps].bmP = bmP;
	trackedBitmaps [nTrackedBitmaps++].nSize = nSize;
	}
#endif
}

//------------------------------------------------------------------------------

int32_t IsOpaqueDoor (int32_t i)
{
	int16_t	*p;

if (i >= 0)
	for (p = d2OpaqueDoors; *p >= 0; p++)
		if (i == gameData.pig.tex.bmIndexP [*p].index)
			return 1;
return 0;
}

//------------------------------------------------------------------------------

#if TEXTURE_COMPRESSION

int32_t CBitmap::SaveS3TC (const char *pszFolder, const char *pszFile)
{
	CFile		cf;
	char		szFilename [FILENAME_LEN], szFolder [FILENAME_LEN];

if (!m_info.compressed.bCompressed)
	return 0;

if (!pszFolder)
	pszFolder = gameFolders.game.szData [0];
CFile::SplitPath (pszFile, NULL, szFilename, NULL);
sprintf (szFolder, "%s/dxt/", pszFolder);
strcat (szFilename, ".dxt");
if (cf.Exist (szFilename, pszFolder, 0))
	return 1;
if (!cf.Open (szFilename, szFolder, "wb", 0))
	return 0;

m_info.compressed.nBufSize = m_info.compressed.buffer.Size ();
if ((cf.Write (&m_info.props.w, sizeof (m_info.props.w), 1) != 1) ||
	 (cf.Write (&m_info.props.h, sizeof (m_info.props.h), 1) != 1) ||
	 (cf.Write (&m_info.compressed.nFormat, sizeof (m_info.compressed.nFormat), 1) != 1) ||
    (cf.Write (&m_info.compressed.nBufSize, sizeof (m_info.compressed.nBufSize), 1) != 1) ||
    (m_info.compressed.buffer.Write (cf, m_info.compressed.nBufSize) != m_info.compressed.nBufSize)) {
	cf.Close ();
	return 0;
	}
return !cf.Close ();
}

//------------------------------------------------------------------------------

int32_t CBitmap::ReadS3TC (const char *pszFolder, const char *pszFile)
{
	CFile		cf;
	char		szFilename [FILENAME_LEN], szFolder [FILENAME_LEN];

if (!ogl.m_features.bTextureCompression)
	return 0;
if (!m_info.compressed.bCompressed)
	return 0;

if (!pszFolder)
	pszFolder = gameFolders.game.szData [0];
CFile::SplitPath (pszFile, NULL, szFilename, NULL);
sprintf (szFolder, "%s/dxt/", pszFolder);
strcat (szFilename, ".dxt");
if (cf.Exist (szFilename, pszFolder, 0))
	return 1;
if (!cf.Open (szFilename, szFolder, "rb", 0))
	return 0;

if ((cf.Read (&m_info.props.w, sizeof (m_info.props.w), 1) != 1) ||
	 (cf.Read (&m_info.props.h, sizeof (m_info.props.h), 1) != 1) ||
	 (cf.Read (&m_info.compressed.nFormat, sizeof (m_info.compressed.nFormat), 1) != 1) ||
	 (cf.Read (&m_info.compressed.nBufSize, sizeof (m_info.compressed.nBufSize), 1) != 1)) {
	cf.Close ();
	return 0;
	}
if (!m_info.compressed.buffer.Resize (m_info.compressed.nBufSize)) {
	cf.Close ();
	return 0;
	}
if (m_info.compressed.buffer.Read (cf, m_info.compressed.nBufSize) != m_info.compressed.nBufSize) {
	cf.Close ();
	return 0;
	}
cf.Close ();
m_info.compressed.bCompressed = true;
return 1;
}

#endif

//------------------------------------------------------------------------------

int32_t FindTextureByIndex (int32_t nIndex)
{
	int32_t	i, j = gameData.pig.tex.nBitmaps [gameStates.app.bD1Mission];

for (i = 0; i < j; i++)
	if (gameData.pig.tex.bmIndexP [i].index == nIndex)
		return i;
return -1;
}

//------------------------------------------------------------------------------

#define	CHANGING_TEXTURE(_eP)	\
			(((_eP)->changing.nWallTexture >= 0) ? \
			 (_eP)->changing.nWallTexture : \
			 (_eP)->changing.nObjectTexture)

tEffectInfo *FindEffect (tEffectInfo *effectInfoP, int32_t nTexture)
{
	int32_t			h, i, j;
	tBitmapIndex*	frameP;

if (effectInfoP)
	i = (int32_t) (++effectInfoP - gameData.effects.effectP);
else {
	effectInfoP = gameData.effects.effectP.Buffer ();
	i = 0;
	}
for (h = gameData.effects.nEffects [gameStates.app.bD1Data]; i < h; i++, effectInfoP++) {
	for (j = effectInfoP->animationInfo.nFrameCount, frameP = effectInfoP->animationInfo.frames; j > 0; j--, frameP++)
		if (frameP->index == nTexture) {
#if 0
			int32_t t = FindTextureByIndex (nTexture);
			if (t >= 0)
				effectInfoP->changing.nWallTexture = t;
#endif
			return effectInfoP;
			}
	}
return NULL;
}

//------------------------------------------------------------------------------

tAnimationInfo *FindAnimation (int32_t nTexture)
{
	int32_t	h, i, j;
	tAnimationInfo *animInfoP = gameData.effects.animations [0].Buffer ();

for (i = gameData.effects.nClips [0]; i; i--, animInfoP++) {
	for (h = animInfoP->nFrameCount, j = 0; j < h; j++)
		if (animInfoP->frames [j].index == nTexture) {
			return animInfoP;
			}
	}
return NULL;
}

//------------------------------------------------------------------------------

tWallEffect *FindWallEffect (int32_t nTexture)
{
	int32_t	h, i, j;
	tWallEffect *wallEffectP = gameData.wallData.animP.Buffer ();

for (i = gameData.wallData.nAnims [gameStates.app.bD1Data]; i; i--, wallEffectP++)
	for (h = wallEffectP->nFrameCount, j = 0; j < h; j++)
		if (gameData.pig.tex.bmIndexP [wallEffectP->frames [j]].index == nTexture)
			return wallEffectP;
return NULL;
}

//------------------------------------------------------------------------------

void UnloadHiresAnimations (void)
{
for (int32_t bD1 = 0; bD1 < 2; bD1++) {
	CBitmap*	bmP = gameData.pig.tex.bitmaps [bD1].Buffer ();
	for (int32_t i = gameData.pig.tex.nBitmaps [bD1]; i; i--, bmP++) {
		bmP->FreeHiresAnimation (bD1);
		}
	}
}

//------------------------------------------------------------------------------

void UnloadTextures (void)
{
	int32_t		i, bD1;
	CBitmap	*bmP;

#if TRACE
console.printf (CON_VERBOSE, "Unloading textures\n");
#endif
gameData.pig.tex.bPageFlushed++;
TexMergeClose ();
RLECacheFlush ();
for (bD1 = 0; bD1 < 2; bD1++) {
	bitmapCacheNext [bD1] = 0;
	for (i = 0, bmP = gameData.pig.tex.bitmaps [bD1].Buffer ();
		  i < gameData.pig.tex.nBitmaps [bD1];
		  i++, bmP++) {
#if DBG
		if (i == nDbgTexture)
			BRP;
#endif
		if (bitmapOffsets [bD1][i] > 0) { // only page out bitmaps read from disk
			bmP->AddFlags (BM_FLAG_PAGED_OUT);
			gameData.pig.tex.bitmaps [bD1][i].Unload (i, bD1);
			}
		}
	}
for (i = 0; i < MAX_ADDON_BITMAP_FILES; i++)
	gameData.pig.tex.addonBitmaps [i].Unload (i, 0);
}

//------------------------------------------------------------------------------

void ResetTextureFlags (void)
{
	int32_t		i, bD1;

#if TRACE
console.printf (CON_VERBOSE, "Resetting texture flags\n");
#endif
for (bD1 = 0; bD1 < 2; bD1++)
	for (i = 0; i < gameData.pig.tex.nBitmaps [bD1]; i++)
		gameData.pig.tex.bitmapFlags [bD1][i] &= ~BM_FLAG_TGA;
}

//------------------------------------------------------------------------------

void GetFlagData (const char *bmName, int32_t nIndex)
{
	int32_t		i;
	tFlagData*	pf;

if (strstr (bmName, "flag01#0") == bmName)
	i = 0;
else if (strstr (bmName, "flag02#0") == bmName)
	i = 1;
else
	return;
pf = gameData.pig.flags + i;
pf->bmi.index = nIndex;
pf->animInfoP = FindAnimation (nIndex);
pf->animState.nClipIndex = gameData.objData.pwrUp.info [46 + i].nClipIndex;	//46 is the blue flag powerup
pf->animState.xFrameTime = gameData.effects.animations [0][pf->animState.nClipIndex].xFrameTime;
pf->animState.nCurFrame = 0;
}

//------------------------------------------------------------------------------

int32_t IsAnimatedTexture (int16_t nTexture)
{
return (nTexture > 0) && (strchr (gameData.pig.tex.bitmapFiles [gameStates.app.bD1Mission][gameData.pig.tex.bmIndexP [nTexture].index].name, '#') != NULL);
}

//------------------------------------------------------------------------------

static int32_t BestShrinkFactor (CBitmap *bmP, int32_t nShrinkFactor)
{
	int32_t	nBaseSize, nTargetSize, nBaseFactor;

if ((bmP->Width () != Pow2ize (bmP->Width (), 65536)) || (bmP->Height () != Pow2ize (bmP->Height (), 65536)))
	return 1;
#if 0
if (bmP->Width () >= 2 * bmP->Width ()) {
#endif
	nBaseSize = bmP->Width ();
	nTargetSize = 512 / nShrinkFactor;
	nBaseFactor = (3 * nBaseSize / 2) / nTargetSize;
#if 0
	}
else {
	nBaseSize = bmP->Width () * bmP->Width ();
	nTargetSize = (512 * 512) / (nShrinkFactor * nShrinkFactor);
	nBaseFactor = (int32_t) sqrt ((double) (3 * nBaseSize / 2) / nTargetSize);
	}
#endif
if (!nBaseFactor)
	return 1;
if (nBaseFactor > nShrinkFactor)
	return nShrinkFactor;
for (nShrinkFactor = 1; nShrinkFactor <= nBaseFactor; nShrinkFactor *= 2)
	;
return nShrinkFactor / 2;
}

//------------------------------------------------------------------------------

int32_t ReadBitmap (CFile* cfP, CBitmap* bmP, int32_t nSize, bool bD1)
{
nDescentCriticalError = 0;
if (bmP->Flags () & BM_FLAG_RLE) {
	int32_t nSize = cfP->ReadInt () - 4;
	if (nDescentCriticalError) {
		PiggyCriticalError ();
		return -1;
		}

#if DBG
	if (nSize > int32_t (bmP->Size ()))
		bmP->Resize (nSize);
#endif
	if (bmP->Read (*cfP, nSize, 0) != uint32_t (nSize))
		return -2;
	nSize = bmP->RLEExpand (NULL, IsMacDataFile (cfP, bD1));
	}
else {
	if (bmP->Read (*cfP, bmP->FrameSize ()) != (size_t) bmP->FrameSize ())
		return -2;
	if (IsMacDataFile (cfP, bD1))
		bmP->SwapTransparencyColor ();
	}


if (bD1)
	bmP->SetPalette (paletteManager.D1 (), TRANSPARENCY_COLOR, SUPER_TRANSP_COLOR);
else
	bmP->SetPalette (paletteManager.Game (), TRANSPARENCY_COLOR, SUPER_TRANSP_COLOR);
bmP->SetTranspType ((bmP->Flags () | (BM_FLAG_TRANSPARENT | BM_FLAG_SUPER_TRANSPARENT)) ? 3 : 0);
return 1;
}

//------------------------------------------------------------------------------

static int32_t ReadLoresBitmap (CBitmap* bmP, int32_t nIndex, int32_t bD1)
{
nDescentCriticalError = 0;

if (gameStates.app.nLogLevel > 1)
	PrintLog (0, "loading lores texture '%s'\n", bmP->Name ());
CFile* cfP = cfPiggy + bD1;
if (!cfP->File ())
	PiggyInitPigFile (NULL);
int32_t nOffset = bitmapOffsets [bD1][nIndex];
if (cfP->Seek (nOffset, SEEK_SET))
	throw (EX_IO_ERROR);
bmP->CreateBuffer ();
if (!bmP->Buffer () || (bitmapCacheUsed > bitmapCacheSize))
	throw (EX_OUT_OF_MEMORY);
bmP->SetFlags (gameData.pig.tex.bitmapFlags [bD1][nIndex]);
if (0 > ReadBitmap (cfP, bmP, bmP->FrameSize (), bD1 != 0)) 
	throw (EX_IO_ERROR);
UseBitmapCache (bmP, int32_t (bmP->FrameSize ()));
return 1;
}

//------------------------------------------------------------------------------

static bool IsCockpit (const char* bmName)
{
return (strstr (bmName, "cockpit") == bmName) || (strstr (bmName, "status") == bmName);
}

//------------------------------------------------------------------------------

static bool IsPowerup (const char* bmName)
{
	static const char* szNames [] = {
		"invuln",
		"pwr",
		"flare"
		};

if (!strstr (bmName, "#0"))
	return false;
for (int32_t i = 0, j = sizeofa (szNames); i < j; i++)
	if (strstr (bmName, szNames [i]) == bmName)
		return true;
return false;
}

//------------------------------------------------------------------------------
// returns not just weapons, but all sprites there is a 3D model available for.

static bool IsWeapon (const char* bmName)
{
	static const char* szNames [] = {
		"slowmotion",
		"bullettime",
		"cmissil",
		"erthshkr",
		"hmissil",
		"key01",
		"key02",
		"key03",
		"merc",
		"mmissil",
		"scmiss",
		"shmiss",
		"smissil",
		"hostage"
		};

	static const int nModels [] = { -1, -1, -1, -1, -1, BLUEKEY_MODEL, GOLDKEY_MODEL, REDKEY_MODEL, -1, -1, -1, -1, -1, HOSTAGE_MODEL};
		

if (!strchr (bmName, '#'))
	return false;
for (int32_t i = 0, j = sizeofa (szNames); i < j; i++)
	if ((strstr (bmName, szNames [i]) == bmName) && ((nModels [i] < 0) || HaveReplacementModel (nModels [i])))
		return true;
return false;
}

//------------------------------------------------------------------------------

static void MakeBitmapFilenames (const char* bmName, char* rootFolder, char* cacheFolder, char* fn, char* fnShrunk, int32_t nShrinkFactor)
{
if (!*rootFolder)
	*fn = *fnShrunk = '\0';
else {
	CFile		cf;
	time_t	tBase, tShrunk;

	sprintf (fn, "%s%s.tga", rootFolder, bmName);
	tBase = cf.Date (fn, "", 0);
	if (tBase < 0)
		*fn = *fnShrunk = '\0';
	else if (*cacheFolder && gameStates.app.bCacheTextures && (nShrinkFactor > 1)) {
		sprintf (fnShrunk, "%s%d/%s.tga", cacheFolder, 512 / nShrinkFactor, bmName);
		tShrunk = cf.Date (fnShrunk, "", 0);
		if (tShrunk < tBase)
			*fnShrunk = '\0';
		}
	else
		*fnShrunk = '\0';
	}
}

//------------------------------------------------------------------------------

static int32_t SearchHiresBitmap (char fn [6][FILENAME_LEN])
{
	CFile	cf;

for (int32_t i = 0; i < 6; i++) {
	//PrintLog (0, "   Looking for '%s'\n", fn [i]);
	if (*fn [i] && cf.Open (fn [i], "", "rb", 0)) {
		cf.Close ();
		return i;
		}
	}
return -1;
}

//------------------------------------------------------------------------------

static int32_t ShrinkFactor (const char* bmName)
{
int32_t nShrinkFactor = 8 >> Min (gameOpts->render.textures.nQuality, gameStates.render.nMaxTextureQuality);
if (nShrinkFactor < 4) {
	if (nShrinkFactor == 1)
		nShrinkFactor = 2;	// cap texture quality at 256x256 (x frame#)
	else if (IsPowerup (bmName) || IsWeapon (bmName))	// force downscaling of powerup hires textures
		nShrinkFactor <<= 1;
	}
return nShrinkFactor;
}

//------------------------------------------------------------------------------

static char* FindHiresBitmap (const char* bmName, int32_t& nFile, int32_t bD1, int32_t bShrink = 1)
{
// Low res animations are realized by several textures bearing the frame number after a '#' sign in the filename.
// Since hires animations contain all frames in a single texture, the hires animation only needs to be loaded
// for the first low res animation frame texture ("name#0"). So skip all textures that have a '#' sign in their 
// name that is not followed by a single '0' character.
//const char* p = strchr (bmName, '#');
//if (p &&  ((p [1] != '0') || (p [2] != '\0')))
//	return NULL;

	static char	fn [6][FILENAME_LEN];
	char	baseName [FILENAME_LEN];

strcpy (baseName, bmName);
for (int32_t i = 0; i < 2; i++) {
#if 1
	if (i) {
		char* p = strchr (baseName, '#');
		if (p)
			strcpy (p + 1, "0");
		}
#endif
	if (*gameFolders.mods.szTextures [0]) {
		sprintf (gameFolders.mods.szTextures [1], "%s%s", gameFolders.mods.szTextures [0], LevelFolder (missionManager.nCurrentLevel));
		sprintf (gameFolders.var.szTextures [4], "%s%s", gameFolders.var.szTextures [3], gameFolders.mods.szLevel); // e.g. /var/cache/d2x-xl/mods/mymod/textures/level01
		}
	else
		*gameFolders.mods.szTextures [1] =
		*gameFolders.var.szTextures [4] = '\0';

	int32_t nShrinkFactor = bShrink ? ShrinkFactor (baseName) : 1;

	MakeBitmapFilenames (baseName, gameFolders.mods.szTextures [1], gameFolders.var.szTextures [4], fn [1], fn [0], nShrinkFactor);
	MakeBitmapFilenames (baseName, gameFolders.mods.szTextures [0], gameFolders.var.szTextures [3], fn [3], fn [2], nShrinkFactor);
	MakeBitmapFilenames (baseName, gameFolders.game.szTextures [bD1 + 1], gameFolders.var.szTextures [bD1 + 1], fn [5], fn [4], nShrinkFactor);

	if (0 < (nFile = SearchHiresBitmap (fn)))
		return fn [nFile];
	}
return NULL;
}

//------------------------------------------------------------------------------

static int32_t HaveHiresAnimation (const char* bmName, int32_t bD1)
{
	char			szAnim [FILENAME_LEN];
	const char*	ps = strchr (bmName, '#');

if (!ps)
	return -1;
int32_t l = int32_t (ps - bmName + 1);
strncpy (szAnim, bmName, l);
szAnim [l] = '0';
szAnim [l + 1] = '\0';
int32_t nFile;
if (FindHiresBitmap (szAnim, nFile, bD1))
	return 1;
if (szAnim [l - 2] == 'b')
	return 0;
strcpy (szAnim + l - 1, "b#0");
return FindHiresBitmap (szAnim, nFile, bD1) != NULL;
}

//------------------------------------------------------------------------------

static bool HaveHiresBitmap (const char* bmName, int32_t bD1)
{
if (gameStates.app.bNostalgia)
	return 0;
if (!gameOpts->render.textures.bUseHires [0])
	return 0;

	int32_t i = HaveHiresAnimation (bmName, bD1);

if (i >= 0)
	return i > 0;

char szFile [FILENAME_LEN];
const char* ps = strstr (bmName, " ");
if (ps) {
	strncpy (szFile, bmName, ps - bmName);
	strcat (szFile, "b");
	strcat (szFile, ps);
	}
else {
	strcpy (szFile, bmName);
	strcat (szFile, "b");
	}
int32_t nFile;
return FindHiresBitmap (szFile, nFile, bD1) != NULL;
}

//------------------------------------------------------------------------------

static bool HaveHiresModel (const char* bmName)
{
if (gameStates.app.bStandalone) {
	for (int32_t i = 0; i < (int32_t) sizeofa (szPowerupTextures); i++)
		if (strstr (bmName, szPowerupTextures [i]))
			return true;
	}
return false;
}

//------------------------------------------------------------------------------

int32_t ReadHiresBitmap (CBitmap* bmP, const char* bmName, int32_t nIndex, int32_t bD1)
{
#if DBG
if (strstr (bmName, "hostage#0"))
	BRP;
#endif

if (gameOpts->Use3DPowerups () && IsWeapon (bmName) && !gameStates.app.bHaveMod)
	return -1;

int32_t	nFile;
char* pszFile = FindHiresBitmap (bmName, nFile, bD1, nIndex != 0x7FFFFFFF);

if (!pszFile)
	return (nIndex < 0) ? -1 : 0;

#if DBG
if (!strcmp (bmName, "hostage#0"))
	BRP;
const char* s = strchr (bmName, '#');
if (s && (s [1] != '0'))
	BRP;
#endif

if (nFile < 2)	//was level specific mod folder
	MakeTexSubFolders (gameFolders.var.szTextures [4]);

CBitmap*	altBmP;
if (nIndex < 0) 
	altBmP = &gameData.pig.tex.addonBitmaps [-nIndex - 1];
else if (nIndex == 0x7FFFFFFF)
	altBmP = bmP;
else
	altBmP = &gameData.pig.tex.altBitmaps [bD1][nIndex];

CTGA tga (altBmP);

#if DBG
if (strstr (pszFile, "metl086"))
	pszFile = pszFile;
#endif
if (!tga.Read (pszFile, ""))
	throw (EX_OUT_OF_MEMORY);

if (strstr (pszFile, "omegblob#") && strstr (pszFile, "/mods/") && !strstr (pszFile, "/mods/descent2"))
	gameStates.render.bOmegaModded = 1;
else if (strstr (pszFile, "plasblob#") && strstr (pszFile, "/mods/") && !strstr (pszFile, "/mods/descent2"))
	gameStates.render.bPlasmaModded = 1;
altBmP->SetType (BM_TYPE_ALT);
altBmP->SetName (bmName);
altBmP->SetKey (nIndex);
if (nIndex != 0x7FFFFFFF) {
	bmP->SetOverride (altBmP);
	bmP = altBmP;
	}
bmP->DelFlags (BM_FLAG_RLE);

int32_t nSize = int32_t (bmP->Size ());
int32_t nFrames = (bmP->Height () % bmP->Width ()) ? 1 : bmP->Height () / bmP->Width ();
bmP->SetFrameCount (uint8_t (nFrames));

if ((nIndex >= 0) && (nIndex < 0x7FFFFFFF)) {	// replacement texture for a lores game texture
	if (bmP->Height () > bmP->Width ()) {
		tEffectInfo	*effectInfoP = NULL;
		tWallEffect *wallEffectP;
		tAnimationInfo *animInfoP;
		while ((effectInfoP = FindEffect (effectInfoP, nIndex))) {
			//e->vc.nFrameCount = nFrames;
			effectInfoP->flags |= EF_ALTFMT;
			//effectInfoP->animationInfo.flags |= WCF_ALTFMT;
			}
		if (!effectInfoP) {
			if ((wallEffectP = FindWallEffect (nIndex))) {
			//w->nFrameCount = nFrames;
				wallEffectP->flags |= WCF_ALTFMT;
				}
			else if ((animInfoP = FindAnimation (nIndex))) {
				//v->nFrameCount = nFrames;
				animInfoP->flags |= WCF_ALTFMT;
				}
			else {
				PrintLog (0, "couldn't find animation for '%s'\n", bmName);
				}
			}
		}
	}

#if TEXTURE_COMPRESSION
if (bmP->Compressed ())
	UseBitmapCache (bmP, bmP->CompressedSize ());
else
#endif
	{
	UseBitmapCache (bmP, nSize);
	bmP->SetType (BM_TYPE_ALT);
	bmP->SetTranspType (-1);
	if (IsOpaqueDoor (nIndex)) {
		bmP->DelFlags (BM_FLAG_TRANSPARENT);
		bmP->TransparentFrames () [0] &= ~1;
		}
#if TEXTURE_COMPRESSION
	if (CompressTGA (bmP))
		bmP->SaveS3TC (gameFolders.var.szTextures [(nFile < 2) ? 4 : (nFile < 4) ? 3 : bD1 + 1], bmName);
	else {
#endif
		int32_t nBestShrinkFactor = BestShrinkFactor (bmP, ShrinkFactor (bmName));
		CTGA tga (bmP);
		//PrintLog (0, "shrinking '%s' by factor %d (%d)\n", bmName, nBestShrinkFactor, bmP->Width () >> (nBestShrinkFactor - 1));
		if ((nBestShrinkFactor > 1) && tga.Shrink (nBestShrinkFactor, nBestShrinkFactor, 1)) {
			nSize /= (nBestShrinkFactor * nBestShrinkFactor);
			if (gameStates.app.bCacheTextures) {
				tTGAHeader&	h = tga.Header ();

				memset (&h, 0, sizeof (h));
				h.bits = bmP->BPP () * 8;
				h.width = bmP->Width ();
				h.height = bmP->Height ();
				h.imageType = 2;
				// nFile < 2: mod level texture folder
				// nFile < 4: mod texture folder
				// otherwise standard D1 or D2 texture folder
				tga.Save (bmName, gameFolders.var.szTextures [(nFile < 2) ? 4 : (nFile < 4) ? 3 : bD1 + 1]);
				}
			}
		}
#if TEXTURE_COMPRESSION
	}
#endif
return 1;
}

//------------------------------------------------------------------------------

#if DBG
int32_t nPrevIndex = -1;
char szPrevBm [FILENAME_LEN] = "";
#endif

int32_t PageInBitmap (CBitmap* bmP, const char* bmName, int32_t nIndex, int32_t bD1, bool bHires)
{
	int32_t		bHaveTGA;

#if DBG
if ((nDbgTexture > 0) && (nIndex == nDbgTexture))
	BRP;
if (!(bmName && *bmName))
	return 0;
if ((nDbgTexture > 0) && (nIndex == nDbgTexture))
	BRP;
#endif
if (bmP->Buffer ())
	return 1;

StopTime ();
if (nIndex >= 0)
	GetFlagData (bmName, nIndex);
#if DBG
if (nIndex == 262)
	nIndex = nIndex;
#endif
if (gameStates.app.bNostalgia)
	gameOpts->render.textures.bUseHires [0] = 0;

if (bmP->Texture ())
	bmP->Texture ()->Release ();
bmP->SetBPP (1);
bmP->SetName (bmName);
bmP->SetKey (nIndex);

#if DBG
if ((nIndex >= 0) && (nIndex == nDbgTexture))
	BRP;
#endif

if ((nIndex >= 0) && !(IsCockpit (bmName) || bHires || gameOpts->render.textures.bUseHires [0]))
	bHaveTGA = 0;
else {
	bHaveTGA = ReadHiresBitmap (bmP, bmName, nIndex, bD1);
	if (bHaveTGA < 0) {
		StartTime (0);
		return 0;	// addon textures are only available as hires texture
		}
	}

if (/*gameStates.render.bBriefing ||*/ !(bHaveTGA || HaveHiresBitmap (bmName, bD1) || HaveHiresModel (bmName)))	// hires addon texture not loaded
	ReadLoresBitmap (bmP, nIndex, bD1);
#if DBG
nPrevIndex = nIndex;
strcpy (szPrevBm, bmName);
#endif
CFloatVector3 color;
if (0 <= (bmP->AvgColor (&color)))
	bmP->SetAvgColorIndex (uint8_t (bmP->Palette ()->ClosestColor (&color)));
StartTime (0);
return 1;
}

//------------------------------------------------------------------------------

int32_t PiggyBitmapPageIn (int32_t bmi, int32_t bD1, bool bHires)
{
	CBitmap*	bmP, * bmoP;
	int32_t		i;

if (bmi < 1)
	return 0;
if (bmi >= MAX_BITMAP_FILES)
	return 0;
if (bmi >= gameData.pig.tex.nBitmaps [bD1])
	return 0;
if (bitmapOffsets [bD1][bmi] == 0)
	return 0;		// A read-from-disk bmi!!!

#if DBG
if (bmi == nDbgTexture)
	BRP;
#endif
bmP = &gameData.pig.tex.bitmaps [bD1][bmi];
if ((bmoP = bmP->Override ()))
	bmP = bmoP;
while (0 > (i = PageInBitmap (bmP, gameData.pig.tex.bitmapFiles [bD1][bmi].name, bmi, bD1, bHires)))
	G3_SLEEP (0);
if (!i)
	return 0;
gameData.pig.tex.bitmaps [bD1][bmi].DelFlags (BM_FLAG_PAGED_OUT);
return 1;
}

//------------------------------------------------------------------------------

int32_t PiggyBitmapExistsSlow (char * name)
{
for (int32_t i = 0, j = gameData.pig.tex.nBitmaps [gameStates.app.bD1Data]; i < j; i++)
	if (!strcmp (gameData.pig.tex.bitmapFileP[i].name, name))
		return 1;
return 0;
}

//------------------------------------------------------------------------------

void LoadReplacementBitmaps (const char *pszLevelName)
{
	char		szFilename [FILENAME_LEN];
	CFile		cf;
	int32_t		i, j;
	CBitmap	bm;

//first, free up data allocated for old bitmaps
PrintLog (1, "loading replacement textures\n");
CFile::ChangeFilenameExtension (szFilename, pszLevelName, ".pog");
if (cf.Open (szFilename, gameFolders.game.szData [0], "rb", 0)) {
	int32_t					id, version, nBitmapNum, bHaveTGA;
	int32_t					bmDataOffset, bmOffset;
	uint16_t				*indices;
	tPIGBitmapHeader	*bmh;

	id = cf.ReadInt ();
	version = cf.ReadInt ();
	if (id != MAKE_SIG ('G','O','P','D') || version != 1) {
		cf.Close ();
		PrintLog (-1);
		return;
		}
	nBitmapNum = cf.ReadInt ();
	indices = new uint16_t [nBitmapNum];
	bmh = new tPIGBitmapHeader [nBitmapNum];
#if 0
	cf.Read (indices, nBitmapNum * sizeof (uint16_t), 1);
	cf.Read (bmh, nBitmapNum * sizeof (tPIGBitmapHeader), 1);
#else
	for (i = 0; i < nBitmapNum; i++)
		indices [i] = cf.ReadShort ();
	for (i = 0; i < nBitmapNum; i++)
		PIGBitmapHeaderRead (bmh + i, cf);
#endif
	bmDataOffset = (int32_t) cf.Tell ();

	for (i = 0; i < nBitmapNum; i++) {
		bmOffset = bmh [i].offset;
		memset (&bm, 0, sizeof (CBitmap));
		//bm.Init ();
		bm.AddFlags (bmh [i].flags & (BM_FLAGS_TO_COPY | BM_FLAG_TGA));
		bm.SetWidth (bmh [i].width + ((int16_t) (bmh [i].wh_extra & 0x0f) << 8));
		bm.SetRowSize (bm.Width ());
		if ((bHaveTGA = (bm.Flags () & BM_FLAG_TGA)) && (bm.Width () > 256))
			bm.SetHeight (bm.Width () * bmh [i].height);
		else
			bm.SetHeight (bmh [i].height + ((int16_t) (bmh [i].wh_extra & 0xf0) << 4));
		bm.SetBPP (bHaveTGA ? 4 : 1);
		if (!(bm.Width () * bm.Width ()))
			continue;
		bm.SetAvgColorIndex (bmh [i].avgColor);
		bm.SetType (BM_TYPE_ALT);
		if (!bm.CreateBuffer ())
			break;
		cf.Seek (bmDataOffset + bmOffset, SEEK_SET);
#if DBG
		if (indices [i] == nDbgTexture)
			BRP;
#endif
		if (bHaveTGA) {
			CTGA			tga (&bm);
			tTGAHeader&	h = tga.Header ();
			int32_t			nFrames = bm.Height () / bm.Width ();

			h.width = 
			h.height = bm.Width ();
			h.bits = 32;
			if (!tga.ReadData (cf, -1, 1.0, 0, 1)) {
				bm.DestroyBuffer ();
				break;
				}
			bm.SetFrameCount ((uint8_t) nFrames);
			if (nFrames > 1) {
				tEffectInfo	*effectInfoP = NULL;
				tWallEffect *wallEffectP;
				tAnimationInfo *animInfoP;
				while ((effectInfoP = FindEffect (effectInfoP, indices [i]))) {
					//e->vc.nFrameCount = nFrames;
					effectInfoP->flags |= EF_ALTFMT | EF_FROMPOG;
					}
				if (!effectInfoP) {
					if ((wallEffectP = FindWallEffect (indices [i]))) {
						//w->nFrameCount = nFrames;
						wallEffectP->flags |= WCF_ALTFMT | WCF_FROMPOG;
						}
					else if ((animInfoP = FindAnimation (i))) {
						//v->nFrameCount = nFrames;
						animInfoP->flags |= WCF_ALTFMT | WCF_FROMPOG;
						}
					}
				}
			j = indices [i];
			bm.SetKey (j);
			}
		else {
			ReadBitmap (&cf, &bm, int32_t (bm.Width ()) * int32_t (bm.Height ()), false);
			j = indices [i];
#if DBG
			if (j == nDbgTexture)
				BRP;
#endif
			bm.SetKey (j);
			bm.RLEExpand (NULL, 0);
			*bm.Props () = *gameData.pig.tex.bitmapP [j].Props ();
			bm.SetPalette (paletteManager.Game (), TRANSPARENCY_COLOR, SUPER_TRANSP_COLOR);
			}
#if DBG
		if (j == nDbgTexture)
			BRP;
#endif
		gameData.pig.tex.bitmapP [j].Unload (j, 0);
		bm.SetFromPog (1);
		char szName [20];
		if (*gameData.pig.tex.bitmapP [j].Name ())
			sprintf (szName, "[%s]", gameData.pig.tex.bitmapP [j].Name ());
		else
			sprintf (szName, "POG#%04d", j);
		bm.SetName (szName);
		gameData.pig.tex.altBitmapP [j] = bm;
		gameData.pig.tex.altBitmapP [j].SetBuffer (bm.Buffer (), 0, bm.Length ());
		bm.SetBuffer (NULL);
		gameData.pig.tex.bitmapP [j].SetOverride (gameData.pig.tex.altBitmapP + j);
		CBitmap* bmP = gameData.pig.tex.altBitmapP + j;
		CFloatVector3 color;
		if (0 <= bmP->AvgColor (&color))
			bmP->SetAvgColorIndex (bmP->Palette ()->ClosestColor (&color));
		UseBitmapCache (gameData.pig.tex.altBitmapP + j, (int32_t) bm.Width () * (int32_t) bm.RowSize ());
		}
	delete[] indices;
	delete[] bmh;
	cf.Close ();
	paletteManager.SetLastPig ("");
	TexMergeFlush ();       //for re-merging with new textures
	}
PrintLog (-1);
}

//------------------------------------------------------------------------------

void LoadTextureColors (const char *pszLevelName, CFaceColor *colorP)
{
	char			szFilename [FILENAME_LEN];
	CFile			cf;
	int32_t			i;

//first, free up data allocated for old bitmaps
PrintLog (1, "loading texture colors\n");
CFile::ChangeFilenameExtension (szFilename, pszLevelName, ".clr");
if (cf.Open (szFilename, gameFolders.game.szData [0], "rb", 0)) {
	if (!colorP)
		colorP = gameData.render.color.textures.Buffer ();
	for (i = MAX_WALL_TEXTURES; i; i--, colorP++) {
		ReadColor (cf, colorP, 0, 0);
		colorP->index = 0;
		}
	cf.Close ();
	}
PrintLog (-1);
}

//------------------------------------------------------------------------------

bool BitmapLoaded (int32_t bmi, int32_t nFrame, int32_t bD1)
{
	CBitmap*		bmoP, * bmP = gameData.pig.tex.bitmaps [bD1] + bmi;
	CTexture*	texP;

if (!bmP)
	return false;
#if 1
if (nFrame && gameData.pig.tex.bitmaps [bD1][bmi - nFrame].Override ()) {
	bmP->DelFlags (BM_FLAG_PAGED_OUT);
	bmP->SetOverride (gameData.pig.tex.bitmaps [bD1][bmi - nFrame].Override ());
	return true;
	}
#endif
if (bmP->Flags () & BM_FLAG_PAGED_OUT)
	return false;
if ((bmoP = bmP->Override (-1)))
	bmP = bmoP;
return ((texP = bmP->Texture ()) && (texP->Handle ())) || (bmP->Buffer () != 0);
}

//------------------------------------------------------------------------------

void LoadTexture (int32_t bmi, int32_t nFrame, int32_t bD1, bool bHires)
{
#if DBG
if ((nDbgTexture >= 0) && (bmi == nDbgTexture))
	BRP;
#endif
if (!BitmapLoaded (bmi, nFrame, bD1))
	PiggyBitmapPageIn (bmi, bD1, bHires);
}

//------------------------------------------------------------------------------

#define BACKGROUND_NAME "statback.pcx"

extern CBitmap bmBackground;

void LoadGameBackground (void)
{
	int32_t pcxError;
	
	static char szBackground [2][13] = {"statback.pcx", "johnhead.pcx"};

try {
	bmBackground.DestroyBuffer ();
	pcxError = PCXReadBitmap (szBackground [gameStates.app.cheats.bJohnHeadOn], &bmBackground, BM_LINEAR, 0);
	if (pcxError != PCX_ERROR_NONE)
		Error ("PCX error %s when reading background image '%s'", PcxErrorMsg (pcxError), BACKGROUND_NAME);
	bmBackground.SetPalette (NULL, -1, -1);
	}
catch(...) {
	PrintLog (0, "Error reading background image '%s'\n", BACKGROUND_NAME);
	try {
		bmBackground.Destroy ();
		}
	catch(...) {
		}
	}
}

//------------------------------------------------------------------------------
//eof
