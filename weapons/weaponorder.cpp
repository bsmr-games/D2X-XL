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

#include "descent.h"
#include "error.h"
#include "text.h"
#include "network.h"

//	-----------------------------------------------------------------------------

int32_t POrderList (int32_t nWeapon)
{
	int32_t i;

for (i = 0; i < MAX_PRIMARY_WEAPONS + 1; i++)
	if (primaryOrder [i] == nWeapon)
		return i;
Error ("Primary Weapon is not in order list!!!");
return 0;
}

//	-----------------------------------------------------------------------------

int32_t SOrderList (int32_t nWeapon)
{
	int32_t i;

for (i = 0; i < MAX_SECONDARY_WEAPONS + 1; i++)
	if (secondaryOrder [i] == nWeapon)
		return i;
Error ("Secondary Weapon is not in order list!!!");
return 0;
}


//	-----------------------------------------------------------------------------

void ValidatePrios (uint8_t *order, uint8_t *defaultOrder, int32_t n)
{
	uint8_t		f [MAX_PRIMSEC_WEAPONS + 1];
	int32_t		i, s = (n + 1) * sizeof (uint8_t);

//check for validity
memset (f, 0, s);
for (i = 0; i <= n; i++)
	if (order [i] == 255)
		f [10]++;
	else if (order [i] < n)
		f [order [i]]++;
for (i = 0; i <= n; i++)
	if (f [i] != 1) {
		memcpy (order, defaultOrder, s);
		break;
		}
}

//	-----------------------------------------------------------------------------

char szSeparator [] = "\x88\x88\x88\x88\x88\x88\x88 Never autoselect \x88\x88\x88\x88\x88\x88\x88";

void ReorderPrimary (void)
{
	CMenu	m (MAX_PRIMARY_WEAPONS + 2);
	int32_t	i;

ValidatePrios (primaryOrder, defaultPrimaryOrder, MAX_PRIMARY_WEAPONS);
for (i = 0; i < MAX_PRIMARY_WEAPONS + 1; i++) {
	if (primaryOrder [i] == 255)
		m.AddMenu ("", szSeparator);
	else
		m.AddMenu ("", const_cast<char*> (PRIMARY_WEAPON_NAMES (primaryOrder [i])));
	m [i].Value () = primaryOrder [i];
}
gameStates.menus.bReordering = 1;
i = m.Menu ("Reorder Primary", "Shift+Up/Down arrow to move item");
gameStates.menus.bReordering = 0;
for (i = 0; i < MAX_PRIMARY_WEAPONS + 1; i++)
	primaryOrder [i] = m [i].Value ();
}

//	-----------------------------------------------------------------------------

void ReorderSecondary (void)
{
	CMenu	m (MAX_SECONDARY_WEAPONS + 2);
	int32_t	i;

ValidatePrios (secondaryOrder, defaultSecondaryOrder, MAX_SECONDARY_WEAPONS);
for (i = 0; i < MAX_SECONDARY_WEAPONS + 1; i++) {
	if (secondaryOrder [i] == 255)
		m.AddMenu ("", szSeparator);
	else
		m.AddMenu ("", const_cast<char*> (SECONDARY_WEAPON_NAMES (secondaryOrder [i])));
	m [i].Value () = secondaryOrder [i];
}
gameStates.menus.bReordering = 1;
i = m.Menu ("Reorder Secondary", "Shift+Up/Down arrow to move item");
gameStates.menus.bReordering = 0;
for (i = 0; i < MAX_SECONDARY_WEAPONS + 1; i++)
	secondaryOrder [i] = m [i].Value ();
}

//	-----------------------------------------------------------------------------

int32_t CheckToUsePrimary (int32_t nWeaponIndex)
{
	uint16_t oldFlags = LOCALPLAYER.primaryWeaponFlags;
	uint16_t flag = 1 << nWeaponIndex;
	int32_t cutpoint;

cutpoint = POrderList (255);
if (!(oldFlags & flag) && 
	 (gameOpts->gameplay.nAutoSelectWeapon == 2) &&
	 (POrderList (nWeaponIndex) < cutpoint) && 
	 (POrderList (nWeaponIndex) < POrderList (gameData.weapons.nPrimary))) {
	if (nWeaponIndex==SUPER_LASER_INDEX)
		SelectWeapon (LASER_INDEX, 0, 0, 1);
	else
		SelectWeapon (nWeaponIndex, 0, 0, 1);
	}
paletteManager.BumpEffect (7, 14, 21);
return 1;
}

//	-----------------------------------------------------------------------------
//eof
