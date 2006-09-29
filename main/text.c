/* $Id: text.c,v 1.11 2003/11/26 12:26:33 btb Exp $ */
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

/*
 *
 * Code for localizable text
 *
 * Old Log:
 * Revision 1.1  1995/05/16  15:31:44  allender
 * Initial revision
 *
 * Revision 2.0  1995/02/27  11:33:09  john
 * New version 2.0, which has no anonymous unions, builds with
 * Watcom 10.0, and doesn't require parsing BITMAPS.TBL.
 *
 * Revision 1.11  1994/12/14  12:53:23  matt
 * Improved error handling
 *
 * Revision 1.10  1994/12/09  18:36:44  john
 * Added code to make text read from hogfile.
 *
 * Revision 1.9  1994/12/08  20:56:34  john
 * More cfile stuff.
 *
 * Revision 1.8  1994/12/08  17:20:06  yuan
 * Cfiling stuff.
 *
 * Revision 1.7  1994/12/05  15:10:36  allender
 * support encoded descent.tex file (descent.txb)
 *
 * Revision 1.6  1994/12/01  14:18:34  matt
 * Now support backslash chars in descent.tex file
 *
 * Revision 1.5  1994/10/27  00:13:10  john
 * Took out cfile.
 *
 * Revision 1.4  1994/07/11  15:33:49  matt
 * Put in command-line switch to load different text files
 *
 * Revision 1.3  1994/07/10  09:56:25  yuan
 * #include <stdio.h> added for FILE type.
 *
 * Revision 1.2  1994/07/09  22:48:14  matt
 * Added localizable text
 *
 * Revision 1.1  1994/07/09  21:30:46  matt
 * Initial revision
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#ifdef RCS
static char rcsid[] = "$Id: text.c,v 1.11 2003/11/26 12:26:33 btb Exp $";
#endif

#include <stdlib.h>
#include <string.h>

#include "pstypes.h"
#include "cfile.h"
#include "u_mem.h"
#include "error.h"

#include "inferno.h"
#include "text.h"
#include "args.h"
#include "compbit.h"

#ifdef RELEASE
#	define DUMP_TEXTS 0
#else
#	define DUMP_TEXTS 0
#endif

#define SHAREWARE_TEXTSIZE  14677

char *text;

char *baseGameTexts [N_BASE_TEXTS];

void InitGameTexts (void);

//------------------------------------------------------------------------------

void _CDECL_ free_text(void)
{
	char	*p = pszGameTexts [0] - 1;

LogErr ("unloading game texts\n");
if (pszGameTexts && pszGameTexts [0]) {
	p = pszGameTexts [0] - 1;
	d_free (text);
	d_free (p);
	d_free (pszGameTexts);
	pszGameTexts = NULL;
	}
if (pszHelpTexts && pszHelpTexts [0]) {
	p = pszHelpTexts [0] - 1;
	d_free (text);
	d_free (p);
	d_free (pszHelpTexts);
	pszHelpTexts = NULL;
	}
}

//------------------------------------------------------------------------------

char **pszGameTexts = NULL;

char *d2GameTexts [] = {
	"New game",
	"High scores",
	"Quit",
	"Cannot set screen mode for game",
	"No joystick detected",
	"This game requires a VGA adapter.",
	"Type '%s -help' for a list of command-line options.",
	"Thank-you for playing DESCENT 2!",
	"Sound disabled.",
	"Can't initialize graphics, error=%d",
	"EXTRA LIFE!",
	"Copyright (C) 1994-1996 Parallax Software Corporation",
	"BLUE",
	"RED",
	"YELLOW",
	"Access denied",
	"Access granted",
	"boosted to",
	"Energy",
	"Shield",
	"Laser",
	"Your %s is maxed out!",
	"Quad Lasers",
	"You already have",
	"Vulcan Ammo",
	"Vulcan Rounds",
	"You already are",
	"Cloaked",
	"Cloaking Device",
	"Invulnerable",
	"Invulnerability",
	"<Create New>",
	"Yes",
	"No",
	"Ok",
	"No Demo Files found.",
	"Use F5",
	"during game to create one.",
	"No files matching",
	"were found!",
	"Delete Pilot:",
	"Delete Demo:",
	"Couldn't",
	"Exiting the mine!",
	"Warning!",
	"Unable to open",
	"Enter your cool saying.\nPress ESC when done.",
	"HIGH SCORE!",
	"You placed",
	"You placed 1st!",
	"HIGH SCORES",
	"Name",
	"Score",
	"Skill",
	"Levels",
	"Time",
	"D2 Strategy Guide available at 1-800-531-2343!",
	"1st",
	"2nd",
	"3rd",
	"4th",
	"5th",
	"6th",
	"7th",
	"8th",
	"9th",
	"10th",
	"-empty-",
	"killed",
	"committed suicide!",
	"You",
	"yourself",
	"That macro is not defined.",
	"Sending",
	"Send Message:",
	"says",
	"tells you",
	"has destroyed the main reactor!",
	"The main reactor has been destroyed!",
	"has escaped the mine!",
	"has found the secret level!",
	"has left the game!",
	"You are the only person\nremaining in this netgame.",
	"Your opponent has left.\nReturning to menu.",
	"You destroyed the main reactor!",
	"Define Macro #",
	"Message sent to",
	"Nobody.",
	"Pause",
	"You can't pause in a multiplayer game!",
	"ESC\t  Abort Game",
	"F2\t  Options menu",
	"F3\t  Toggle cockpit",
	"F4\t  Drop marker",
	"F5\t  Toggle recording",
	"Pause\t  Pause",
	"-/+\t  Change screen size",
	"PrintScrn\t  Save screen shot",
	"1-5\t  Select primary weapon",
	"6-0\t  Select secondary weapon",
	" To view control keys, select\nconfiguration in options menu",
	"KEYS",
	"Abort Autodemo?",
	"Abort Game?",
	"Laser Cannon",
	"Vulcan Cannon",
	"Spreadfire Cannon",
	"Plasma Cannon",
	"Fusion Cannon",
	"Super Laser Cannon",
	"Gauss Cannon",
	"Helix Cannon",
	"Phoenix Cannon",
	"Omega Cannon",
	"Concussion Missile",
	"Homing Missile",
	"Proximity Bomb",
	"Smart Missile",
	"Mega Missile",
	"Flash Missile",
	"Guided Missile",
	"Smart Mine",
	"Mercury Missile",
	"Earthshaker Missile",
	"Laser",
	"Vulcan",
	"Spread",
	"Plasma",
	"Fusion",
	"Super\nLaser",
	"Gauss",
	"Helix",
	"Phoenx",
	"Omega",
	"Concsn\nMissile",
	"Homing\nMissile",
	"Proxim.\nBomb",
	"Smart\nMissile",
	"Mega\nMissile",
	"Flash\nMissile",
	"Guided\nMissile",
	"Smart\nMine",
	"Merc\nMissile",
	"Shaker\nMissile",
	"selected!",
	"You don't have the",
	"You don't have ammo for the",
	"You have no",
	"s",
	"s!",
	"No primary weapons available",
	"You already have the",
	"You cannot open this door",
	"Move joystick",
	"to\nthe upper-left corner",
	"and press any button.",
	"joystick",
	"upper-left",
	"to\nthe lower-right corner",
	"lower-right",
	"to\nthe center",
	"center",
	"Keyboard only",
	"Joystick",
	"CH Flightstick Pro",
	"Thrustmaster FCS &\nWingman Extreme",
	"Gravis Gamepad",
	"Mouse",
	"Cyberman",
	"Windows 95 Joystick",
	"Customize Above",
	"Customize Keyboard",
	"Controls",
	"Keyboard",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"This copy of Descent 2 is for use by:",
	"Error initializing selector for segment A000.",
	"Error trying to initialize unsupported graphics mode.",
	"Calibrate",
	"Skip",
	"It looks like your joystick\nisn't centered.  Do you want\nto calibrate it?",
	"Choose input device",
	"Error",
	"Cannot open player file\nVersion mismatch.",
	"Demo Playback",
	"Demo Recording",
	"Cruise:",
	"Dumping screen to",
	"Cheats Enabled!!",
	"That game is closed to new players.",
	"The game is already full.",
	"The game is between levels.\nTry again later.",
	"You were not selected for the game.",
	"The netgame was not started.",
	"You are already connected.\nTry again in a minute.",
	"Wrong level selected.\nPlease re-join.",
	"kills",
	"Waiting for other players..\n",
	"Are you sure you want\nto leave the game?",
	"is joining the game.",
	"is rejoining the game",
	"Sorry, but a you can only select\nup to",
	"netplayers for this mine.",
	"'s game",
	"Description:",
	"Level:",
	"Mode:",
	"Anarchy",
	"Team Anarchy",
	"Robo-Anarchy",
	"Cooperative",
	"Options:",
	"Closed Game",
	"Game Setup",
	"That start level is out of range",
	"Sorry",
	"That game mode is\nonly available in\nthe registered/commercial\nversion",
	"Wait...",
	"Found",
	"active games.",
	"Starting netgame on level",
	"Your level file does not\nmatch the other player's.\nCannot join game.",
	"Team",
	"Team selection\nSelect names to switch teams",
	"You must place at least\none player on each team.",
	"Select up to",
	"players\nPress ENTER to begin.",
	"You must select at least two\nplayers to start a network game",
	"An active IPX driver was\nnot found.  Check your\nnetwork software",
	"   FORMING, level:",
	"players:",
	"   OPEN,    level:",
	"   CLOSED",
	"   BETWEEN LEVELS",
	"\nESC to leave netgame",
	"Waiting for signal from",
	"to enter the mine",
	"Searching for Netgames...",
	"That choice is invalid.\n",
	"That game is between levels.\nWait for status to change\nand try joining again.",
	"Your version of Descent 2\ndoes not match the version\nin use for that game.",
	"That game is currently full.",
	"You cannot join that\ngame in progress.",
	"has disconnected!",
	"Error opening serial driver.\nCheck your serial parameters\nAnd free conventional memory.",
	"Error!\nCarrier Lost.\n  Leaving Multiplayer game.",
	"Error writing to the file\nserial.cfg.  Can't save settings.",
	"Error reading serial settings.\nUsing defaults.",
	"No message received from\n%s for 10 seconds.\nConnection may be lost.",
	"Your opponent has selected\nstart game.  Are you\nready for descent?",
	"Your opponent has disconnected.",
	"Dial modem...",
	"Answer modem",
	"Establish null-modem link",
	"COM settings...",
	"Start game...",
	"Send message...",
	"Hang up modem",
	"Close link",
	"Serial",
	"link active to",
	"Modem",
	"Not currently connected",
	"Serial Game",
	"Exiting this menu\nwill close the link\nContinue?",
	"Baud Rate:",
	"Modem Init String:",
	"Accept and Save",
	"Serial Settings",
	"Warning!\nYou must have a\n16550 UART\nto use 38400",
	"Difficulty:",
	"Serial Game Setup",
	"Only Anarchy mode is\navailable in the\nDemo version.",
	"Save!",
	"Accept",
	"Select a number to edit",
	"Edit phonebook entry",
	"Manual Entry",
	"Edit Phonebook",
	"Select a number to dial",
	"Enter number to dial",
	"NO DIAL TONE",
	"BUSY",
	"NO ANSWER",
	"NO CARRIER",
	"VOICE",
	"Error!\nModem returned:",
	"CONNECT",
	"Error!\nYou must establish a\n9600 baud connection\nor higher to play.",
	"RING",
	"Descent 2 was started with\nthe serial option disabled.",
	"Resetting Modem",
	"No modem detected.\nCheck your com settings.",
	"That phone number\nis not defined.\n",
	"Dialing:",
	"ESC to abort",
	"Waiting for answer...",
	"Waiting for call...",
	"Waiting for carrier...",
	"Failed to negotiate!",
	"Negotiation with remote player\nfailed.  Cannot continue.",
	"Fatal error.\nMy level =",
	"Other level =",
	"Your level ",
	"file does\nnot match",
	"'s.\nCheck your versions.",
	"Your version of Descent 2\ndoes not match that of\nyour opponent.",
	"Your opponent is not\nready to start the game.",
	"Waiting for remote player...",
	"LOCK",
	"DEATHS",
	"LIVES",
	"LVL",
	"QUAD",
	"REAR VIEW",
	"Trainee",
	"Rookie",
	"Hotshot",
	"Ace",
	"Insane",
	"Lowest",
	"Low",
	"Medium",
	"High",
	"Highest",
	"Custom...",
	"Load Game...",
	"Multiplayer...",
	"Options...",
	"Change Pilots...",
	"View Demo...",
	"Credits",
	"Ordering Info",
	"Select Demo\n<Ctrl-D> deletes",
	"Difficulty Level",
	"set to",
	"Detail Level",
	"Object Complexity",
	"Object Detail",
	"Wall Detail",
	"Wall Render Depth",
	"Amount of Debris",
	"Sound Channels",
	"                       LO  HI",
	"Detail Level Customization",
	"You may start on\nany level up to",
	"New Game\n\nSelect starting level",
	"Press ENTER to Continue",
	"Invalid level number",
	"Error Loading Game",
	"Save Game\n\nSelect slot & enter save name\nPress ESC if you don't want to save",
	"Save Error!",
	"FX Volume",
	"Music Volume",
	"Reverse Stereo",
	"Brightness",
	"C~Ontrols...",
	"~Detail levels...",
	"Calibrate Joystick",
	"Joystick/Mouse\nSensitivity",
	"Start an IPX network game...",
	"Join an IPX network game...\n",
	"Modem/serial game...",
	"Multiplayer",
	"Continue",
	"Can't playback demo",
	"because\ndemo file contains corrupt\ndata.",
	"because\ndemo version is too old.",
	"recorded",
	"with the Commercial version",
	"with the Demo version",
	"of Descent 2.",
	"because\nlevel cannot be loaded.",
	"Demo is probably too old\nor contains corrupt data.",
	"Error reading demo data.",
	"Save Demo as:",
	"Please use only letters,\nnumbers and the underscore\ncharacter in filename.",
	"Automap",
	"Flight controls move - Accel/Reverse zooms in/out",
	"+/- Changes viewing distance",
	"Level",
	"Pitch forward",
	"Pitch backward",
	"Turn left",
	"Turn right",
	"Slide on",
	"Slide left",
	"Slide right",
	"Slide up",
	"Slide down",
	"Bank on",
	"Bank left",
	"Bank right",
	"Fire primary",
	"Fire secondary",
	"Fire flare",
	"Accelerate",
	"reverse",
	"Drop Bomb",
	"Cruise Faster",
	"Cruise Slower",
	"Cruise Off",
	"Pitch U/D",
	"Turn L/R",
	"Slide L/R",
	"Slide U/D",
	"Bank L/R",
	"throttle",
	"You must select at least three\nplayers to start a team game",
	"Disconnected",
	"Playing",
	"Escaped",
	"Died in mine",
	"Found secret level",
	"In Escape tunnel",
	"Viewing Level Scores",
	"Wowie Zowie Weapons!!",
	"All Keys!",
	"Cloak",
	"Shields Recharged!",
	"On",
	"Off",
	"Not available in Demo version",
	"Game Over",
	"Select pilot\n<Ctrl-D> deletes",
	"Enter your pilot name:",
	"Player",
	"already exists!",
	"Prepare for Descent...",
	"Full Rescue bonus:    \t",
	"Shield bonus:\t",
	"Energy bonus:\t",
	"Hostage bonus:    \t",
	"Skill Bonus:\t",
	"Total Bonus:\t",
	"Total Score:\t",
	"Secret Level",
	"Complete",
	"Destroyed!",
	"Save Game?",
	"Press <Ctrl-R> to reset",
	"Reset the high scores?",
	"You were",
	"was",
	"killed by the reactor",
	"IMPORTANT NOTE",
	"Use this option for the FCS\nor Wingman Extreme when\nused alone.  If you\nalso use a WCS or FLCS, you\nmust select joystick.\nSee manual/readme for details.\n",
	"Press any key or button to continue...",
	"Hostage rescued!",
	"Initializing VictorMaxx tracking on COM port",
	"N",
	"Y",
	"Start at any level.",
	"Demo Levels Completed",
	"Press new key",
	"Press new joystick button",
	"Press new mouse button",
	"Move new joystick axis",
	"Move new mouse axis",
	"Using VFX1 Head Tracking...Press Shift+Z during game to set zero.",
	"Error: Can't use VFX1 head tracking because no head tracking device\nwas found.",
	"Error: Can't use VFX1 head tracking because VFX1.COM does not\nappear to be loaded!",
	"Enter changes, ^D deletes, ^R resets defaults, Esc exits",
	" Buttons ",
	" Axes ",
	"Axis",
	"Invert?",
	"BTN 1",
	"BTN 2",
	"BTN 3",
	"BTN 4",
	"TRIG",
	"HAT �",
	"HAT �",
	"HAT ",
	"HAT �",
	"LEFT",
	"RIGHT",
	"MID",
	"UP",
	"DOWN",
	"X1",
	"Y1",
	"X2",
	"Y2",
	"L/R",
	"F/B",
	"forward",
	"Move throttle all\nthe way forward\nand press any button",
	"Move throttle all\nthe way back\nand press any button",
	"Move throttle to\nits center and\npress any button",
	"Reactor has exploded.",
	"Time Remaining",
	"seconds.",
	"Error writing demo file.  Current",
	"demo size is",
	"You are nearly out of space on\nthe current device.  Enter demo\nname now or press ESC to delete\ndemo.",
	"bytes.",
	"You died in the mine.\n\nYour ship and its contents\nwere incinerated.",
	"Ship bonus:  \t",
	"Phone Number",
	"ANARCHY SUMMARY",
	"Waiting for OK to\nstart game\n",
	"Aborting will quit the game\nare you sure?",
	"Error writing player file.\nUnable to save current player.\n",
	"Ship destroyed!",
	"Ship destroyed, 1 hostage lost!",
	"Ship destroyed, %i hostages lost!",
	"This socket is already full.\nPlease choose a different\nsocket to start on.\n\n",
	"Not enough space on current\ndevice to start demo recording.",
	"HAT2�",
	"HAT2",
	"HAT2�",
	"HAT2�",
	"Warp to which level?",
	"DESCENT is a trademark of Interplay Productions, Inc.",
	"Failed to join the netgame.\nYou are missing packets.  Check\nyour network card and\ntry again.",
	"done",
	"I am a",
	"CHEATER!",
	"Loading Data",
	"ALT-F2\t  Save Game",
	"ALT-F3\t  Load Game",
	"Only in Registered version!",
	"Concussion",
	"Homing",
	"ProxBomb",
	"SmrtMisl",
	"Mega",
	"Mission '%s' not found.\nYou must have this mission\nfile in order to playback\nthis demo.",
	"All player callsigns on screen",
	"There is already a game\nin progress with that name",
	"This mission cannot be played\nin Coop or Robo-anarchy games",
	"Force level start",
	"Quitting now means ending the\nentire netgame\n\nAre you sure?",
	"The mission for that netgame\nis not installed on your\nsystem.  Cannot join.",
	"Start Multiplayer Game\n\nSelect mission",
	"Error loading mission file",
	"Custom (return to set)",
	"Base address (in Hex)",
	"IRQ Number",
	"Reset to Default",
	"Valid IRQ values are 2-7",
	"No UART was detected\nat those settings",
	"You will pay dearly for that!",
	"Revenge is mine!!",
	"Man I'm good!",
	"Its almost too easy!",
	"   Mission:",
	"1-9 selects marker to view   Ctrl-D deletes",
	"Secret Teleporter found!\n\nProceed to Secret Level!",
	"Show all players on automap",
	"Killed by a robot",
	"Baud",
	"A consistency error has been\ndetected in your network connection.\nCheck you hardware and re-join",
	"Press any key to continue (Print Screen to save screenshot)",
	"An error occured while writing\ndemo.  Demo is likely corrupted.\nEnter demo name now or\npress ESC to delete demo.",
	"The main reactor is invulnerable for",
	"The level being loaded is not\navailable in Destination Saturn.\nUnable to continue demo playback.\n\nPress any key to continue.",
	"Reactor life",
	"min",
	"Current IPX Socket is default",
	"This program requires MS-DOS 5.0 or higher.\nYou are using MS-DOS",
	"You can use the -nodoscheck command line\nswitch to override this check, but it\nmay have unpredictable results, namely\nwith DOS file error handling.\n",
	"Not enough file handles!",
	"of the necessary file handles\nthat Descent 2 requires to execute properly.  You will\nneed to increase the FILES=n line in your config.sys.",
	"If you are running with a clean boot, then you will need\nto create a CONFIG.SYS file in your root directory, with\nthe line FILES=15 in it.  If you need help with this,\ncontact Interplay technical support.",
	"You may also run with the -nofilecheck command line option\nthat will disable this check, but you might get errors\nwhen loading saved games or playing demos.",
	"Available memory",
	"more bytes of DOS memory needed!",
	"more bytes of virtual memory needed.  Reconfigure VMM.",
	"more bytes of extended/expanded memory needed!",
	"Or else you you need to use virtual memory (See README.TXT)",
	"more bytes of physical memory needed!",
	"Check to see that your virtual memory settings allow\nyou to use all of your physical memory (See README.TXT)",
	"Initializing DPMI services",
	"Initializing critical error handler",
	"Enables Virtual I/O Iglasses! stereo display",
	"Enables Iglasses! head tracking via COM port",
	"Enables Kasan's 3dMax stereo display in low res.",
	"3DBios must be installed for 3dMax operation.",
	"Enables Kasan's 3dMax stereo display in high res",
	"Press any key for more options...",
	"Enables dynamic socket changing",
	"Disables the file handles check",
	"Getting settings from DESCENT.CFG...",
	"Initializing timer system...",
	"Initializing keyboard handler...",
	"Initializing mouse handler...",
	"Mouse support disabled...",
	"Initializing joystick handler...",
	"Slow joystick reading enabled...",
	"Polled joystick reading enabled...",
	"BIOS joystick reading enabled...",
	"Joystick support disabled...",
	"Initializing divide by zero handler...",
	"Initializing network...",
	"Using IPX network support on channel",
	"No IPX compatible network found.",
	"Error opening socket",
	"Not enough low memory for IPX buffers.",
	"Error initializing IPX.  Error code:",
	"Network support disabled...",
	"Initializing graphics system...",
	"SOUND: Error opening",
	"SOUND: Error locking down instruments",
	"SOUND: (HMI)",
	"SOUND: Error locking down drums",
	"SOUND: Error locking midi track map!",
	"SOUND: Error locking midi callback function!",
	"Using external control:",
	"Invalid serial port parameter for -itrak!",
	"Initializing i-glasses! head tracking on serial port %d",
	"Make sure the glasses are turned on!",
	"Press ESC to abort",
	"Failed to open serial port.  Status =",
	"Message",
	"Macro",
	"Error locking serial interrupt routine!",
	"Error locking serial port data!",
	"Robots are normal",
	"Robots move fast, fire seldom",
	"Robot painting OFF",
	"Robot painting with texture %d",
	"Start a TCPIP network game...",
	"Join a TCPIP network game...\n",
	"Afterburner",
	"Z1",
	"UN",
	"P1",
	"R1",
	"Y1",
	"SIngle Player Game"
	};

char *defaultGameTexts [][2] = {
	// menu messages
	{"Ein~Zelspieler-Spiel", "SIngle Player Game"},
	{"~Filme abspielen", "PLAY MOVIES..."},
	{"~Musik abspielen", "PLAY SONGS..."},
	{"D~2 Missionen spielen", "Play D2 Missions"},
	{"D~1 Missionen spielen", "Play D1 Missions"},
	{"IP-Addresse des Spielleiters", "Enter game host's IP address:"},
	{"Teilnehmer-Portnummer", "\nEnter client port:"},
	{"\n (0 fuer bel. verfuegbaren Port)", "\n (0 for arbitrary, available port, "},
	{"+/- Wert fuer Serverport + Wert)", "+/- offset for server port + offset)"},
	{"Waehle Film", "Select Movie"},
	{"Waehle Lied", "Select Song"},
	{"Keine oder ungueltige IP-Adresse.\nBeitritt abgebrochen", "No or invalid IP address specified.\nJoin request cancelled."},
	{"Keine Tracker verfuegbar", "No trackers available."},
	{"Breitwand-Aufloesungen", "Widescreen resolutions"},
	{"Waehle Aufloesung", "Select screen mode"},
	{"Aufloesung auf dieser Hardware nicht verfuegbar", "Cannot set requested\nmode on this video card."},
	{"Neues Spiel\n\nWaehle Mission", "New Game\n\nSelect mission"},
	{"Neues Descent 1-Spiel\n\nWaehle Mission", "New Descent 1 Game\n\nSelect mission"},
	{"Neues Descent 2-Spiel\n\nWaehle Mission", "New Descent 2 Game\n\nSelect mission"},
	{"Missionsdatei fehlerhaft", "Error in Mission file"},
	{"Minifenster-Groesse: klein", "Cockpit window Size: small"},
	{"Minifenster-Groesse: mittel", "Cockpit window Size: medium"},
	{"Minifenster-Groesse: gross", "Cockpit window Size: large"},
	{"Minifenster-Groesse: riesig", "Cockpit window Size: huge"},
	{"Minifenster-Zoom: %dx", "Cockpit Window Zoom: %dx"},
	{"~HUD anzeigen", "Show HUD"},
	{"~Fadenkreuz anzeigen", "Show Reticle"},
	{"~Grafische Skalen anzeigen", "Show graPhical gauges"},
	{"~Skalen an Aufloesung anpassen", "SCale gauges to screen resolution"},
	{"Skalen blinken wenn fast ~Leer", "Flash gauges on low values"},
	{"~Raketensicht zeigen", "Show MIssile View"},
	{"~Vollbild-Lenkraketensicht", "Fullscreen Guided Missile View"},
	{"~Powerups auf Radar zeigen", "Show Powerups On Radar"},
	{"Ro~Bots auf Radar zeigen", "Show RoBots On Radar"},
	{"~Waffensymbole anzeigen", "show Weapon icons"},
	{"~Ausruestungssymbole anzeigen", "show eQuipment icons"},
	{"~Kleine Waffensymbole", "small Weapon Icons"},
	{"Waffensymbole nach Prio ~Ordnen", "SorT Weapon Icons by Weapon Order"},
	{"M~Unition in Waffensymbolen anzeigen", "Show Ammo Count in Weapon Icons"},
	{"Waffensymbole ~Oben", "Weapon Icons At Screen Top"},
	{"Waffensymbole ~Unten", "Weapon Icons At Screen Bottom"},
	{"Waffen l/r, Ausruestung ~Unten", "Weapons L/R, Inventory bottom"},
	{"Waffen l/r, Ausruestung ~Oben", "Weapons L/R, Inventory top"},
	{"Symbol-Ab~Dunkelung", "Icon dimming"},
	{"~Cockpit-Einstellungen", "cockpit options"},
	{"min", "low"},
	{"mittel", "medium"},
	{"hoch", "high"},
	{"max", "highest"},
	{"~Grafikqualitaet", "Render Quality: %s"},
	{"~Texturqualitaet", "Texture QUality: %s"},
	{"Bildaufbau: %d fps", "Framecap: %d fps"},
	{"Bildwiederholrate: max", "Framecap: none"},
	{"Wandtransparenz: %d %c", "Wall Transparency: %d %c"},
	{"Kamera-Bildaufbau: %d BPS", "CAmera Refresh: %d FPS"},
	{"Kamera-Geschw.: %d s/90 Grd", "CAmera SPeed: %d s/90 deg"},
	{"Reichweite Lightmaps: %d %c", "Lightmap RaNge: %d %c"},
	{"Lightmaps anwenden", "Use Light MaPs"},
	{"Gesamte Mine zeichnen", "Render Entire Mine"},
	{"Farbiges Licht einschalten", "Enable Colored Light"},
	{"Farbige Waffeneffekte einschalten", "Enable Colored Weapon LIght"},
	{"Licht und Waffeneffekte mischen", "MiX ambient and weapon light"},
	{"Transparente Waende faerben", "Colorize transparent walls"},
	{"transparente Explosionen", "enable Transparent Explosions"},
	{"Kameras aktivieren", "Use Cameras"},
	{"Kameraausgabe an Wand anpassen", "Fit Camera outpUt to wall"},
	{"Helligkeit per Gamma regeln", "Adjust Brightness Via Gamma"},
	{"Hohe Filmqualitaet", "High Movie Quality"},
	{"Vollbild-Filme", "Fullscreen MovieS"},
	{"Untertitel anzeigen", "Show MOvie subTitles"},
	{"Grafik-Einstellungen", "render options"},
	{"~Konfigurieren", "cOnfigure..."},
	{"Neues Spiel", "New Game"},
	{"IP-Adresse Server", "Server IP Address"},
	{"Unzulaessige IP-Adresse", "Invalid IP Address"},
	{"Material-~Wartezeit: %ld s", "Respawn Delay: %ld s "},
	{"~Koennen: %s", "Difficulty: %s"},
	{"~Beschleunigung: %d %c", "Speed Boost: %d %c"},
	{"~Staerke Fusionskanone %d %c", "Fusion PoWer: %d %c"},
	{"Feste Material-~Entstehungsorte", "Fixed Powerup Spawn Points"},
	{"Raketen-~Doppelabschuss", "DUal Missile Launch"},
	{"~gesamtes Material abwerfen", "Drop All Missiles On Death"},
	{"~Material immer wieder erzeugen", "Always Respawn Powerups"},
	{"Abwurf von ~Quad/Superlaser zul.", "Drop Quad and Super Lasers"},
	{"~Robots treffen einander", "RObots Hit Robots"},
	{"~Multiple Boss-Robots", "Multiple Bosses"},
	{"Fluessigkeiten-Ph~ysik aktivieren", "enable fluid phYsics"},
	{"~intelligente Waffenwahl", "smart Weapon Switch"},
	{"keine ~autom. Waffenwahl", "Never Autoselect Weapon"},
	{"~umschalten wenn Waffe leer", "Auto-select If EmptY"},
	{"~Immer beste Waffe aktivieren", "Always Auto-SelecT"},
	{"Kein ~Waffenzoom", "Zoom Disabled"},
	{"Feste ~Zoomstufen", "FiXed Zoom Stages"},
	{"St~ufenloser Zoom", "Smooth Zoom"},
	{"Spiel-Optionen", "Gameplay options"},
	{"~Klaenge und Musik...", "Sound ~Effects & Music..."},
	{"~Bildschirm-Aufloesung...", "Screen ~Resolution..."},
	{"~Primaerwaffen-Rang", "~PRIMARY AUTOSELECT ORDERING..."},
	{"~Sekundaerwaffen-Rang", "S~ECONDARY AUTOSELECT ORDERING..."},
	{"~Cockpit-Einstellungen", "~COCKPIT OPTIONS..."},
	{"~Grafik-Einstellungen", "~RENDER OPTIONS..."},
	{"~Spiel-Einstellungen", "~GAMEPLAY OPTIONS..."},
	{"~Diverse Einstellungen...", "TOGGLES..."},
	{"~Klaenge und Musik", "Sound effects & Music"},
	{"MIDI-~Lautstaerke", "MIDI music volume"},
	{"MIDI-~Musik", "MIDI MUSIC"},
	{"Redbook-Audio ist per Kommando-\nzeile deaktiviert worden", "Redbook audio has been disabled\non the command line"},
	{"Kann CD-Musik nicht starten.  Ein anderes\nProgramm verwendet das CD-Laufwerk.", "Cannot start CD Music.\nAnother application is\nusing the CD player.\n"},
	{"Kann CD-Musik nicht starten.  Lege die\nDescent II-CD ein und versuche es nochmal", "Cannot start CD Music.  Insert\nyour Descent II CD and try again"},
	{"~CD-Lautstaerke", "CD music Volume"},
	{"CD-Musik (Redbook) aktiviert", "CD Music (Redbook) Enabled"},
	{"Auto-DL ~Timeout: %d s", "Auto-DL Timeout: %d s"},
	{"~Automatische Ausrichtung", "Ship Auto-Leveling"},
	{"~Schiff auf und ab schwingen lassen", "Wiggle Ship"},
	{"Scheinwerfer ~an wenn aufgenommen", "Headlight On When Picked Up"},
	{"~Guidebot-Schnelltasten verwenden", "Use Guidebot Hot Keys"},
	{"Schnelle Aufe~Rstehung", "Fast Respawn"},
	{"~Multiplayer-Makros zulassen", "Use Multiplayer Macros"},
	{"~UDP/IP-Netzwerk-Qualitaet verbessern", "Improve UDP/IP networking Quality"},
	{"Intelligente Menuesuche verwenden", "Use Smart File Search In Menus"},
	{"~Level-Version in Menues anzeigen", "Show Level Version In Menus"},
	{"Auto-~Download aktivieren", "Enable Auto Download"},
	{"~UDP/IP-Netzspiel starten", "START UDP/IP NETGAME"},
	{"UDP/IP-Netzspiel ~Beitreten\n", "JOIN UDP/IP NETGAME\n"},
	{"UDP/IP-Netzspiel via ~Tracker starten", "StARt UDP/IP NEtGAME VIA TRACKER"},
	{"UDP/IP-Netzspiel via T~Racker beitreten\n", "JOIN UDP/IP NETGAME VIA TRACKER\n"},
	{"UDP/IP-~Multicast-Netzspiel starten", "START MULTICAST UDP/IP NETGAME"},
	{"UDP/IP-Multi~Cast-Netzspiel beitreten\n", "JOIN MULTICAST UDP/IP NETGAME\n"},
	{"~KALI-Netzspiel starten", "START KALI NETGAME"},
	{"KAL~I-Netzspiel beitreten", "JOIN KALI NETGAME\n"},
	{"M~Odem/serielles ~Spiel", "MODEM/SERIAL GAME..."},
	{"TCP/~IP-Spiel beitreten", "Join a TCP/IP game"},
	{"Unzulaessige Adresse", "That address is not valid!"},
	{"hochaufgeloeste ~Filme", "Show High Res movies"},
	{"Max. # Spieler: %d", "Maximum players: %d"},
	{"Game Host: IP-Adresse nicht gefunden", "Game Host: Failed to get IP address"},
	{"M~Ission", "MIssion:"},
	{"~Kooperativ", "COOPERATIVE"},
	{"~Flagge erobern", "CAPTURE THE FLAG"},
	{"Flagge erobern (~Verbessert)", "ENHANCED CTF"},
	{"~Horten", "HOARD"},
	{"Team-H~Orten", "TEAM HOARD"},
	{"~Offenes Spiel", "OPEN GAME"},
	{"~Weitere Einstellungen...", "MORE OPTIONS..."},
	{"Spiel mit ~Zugangsbeschr.", "Restricted Game"},
	{"En~Tropie-Einstellungen", "ENTROPY OPTIONS..."},
	{"~Entropie", "ENTROPY"},
	{"(Abbruch mit Esc)", "(Press Escape to cancel)"},
	{"suche Netzspiele", "Looking for netgames"},
	{"Hey, uns gibt's zweimal!\n", "Hey, we've found ourselves twice!\n"},
	{"Fuer ein Teamspiel solltest Du\nwenigstens 2 Spieler auswaehlen", "You should select at least two\nplayers to start a team game"},
	{"%d Tracker gefunden", "%d tracker%s found"},
	{"Aktueller %s-Socket ist Standard %+d (aendern mit PgUp/PgDn)", "\tCurrent %s Socket is default %+d (PgUp/PgDn to change)"},
	{"Wechsle zu Socket %d\n", "Changing to socket %d\n"},
	{"\tSpiel\tModus\t#Teiln\tMission\tLev\tStatus", "\tGAME\tMODE\t#PLYRS\tMISSION\tLEV\tSTATUS"},
	{"Netzspiele", "NETGAMES"},
	{"Deine Version von Descent 2\nist inkompatibel zur\nDemo-Version", "Your version of Descent 2\nis incompatible with the\nDemo version"},
	{"Diese Demo-Version von\nDescent 2 ist inkompatibel\nzur Verkaufsversion", "This Demo version of\nDescent 2 is incompatible\nwith the full commercial version"},
	{"Lade Mission: %s.", "Loading mission:%s.\n"},
	{"Diese OEM-Version erlaubt nur\ndie ersten 8 Level!", "This OEM version only supports\nthe first 8 levels!"},
	{"Diese Shareware-Version erlaubt\nnur die ersten 4 Level!", "This SHAREWARE version only supports\nthe first 4 levels!"},
	{"Fehler beim Beitritt!", "There was a join error!"},
	{"Verbinde...", "Connecting..."},
	{"Zusatz-Laser", "Laser upgrade"},
	{"Superlaser", "Super lasers"},
	{"Quad-Laser", "Quad Lasers"},
	{"Vulkankanone", "Vulcan cannon"},
	{"Streufeuerkanone", "Spreadfire cannon"},
	{"Plasmakanone", "Plasma cannon"},
	{"Fusionskanone", "Fusion cannon"},
	{"Gausskanone", "Gauss cannon"},
	{"Helixkanone", "Helix cannon"},
	{"Phoenixkanone", "Phoenix cannon"},
	{"Omegakanone", "Omega cannon"},
	{"Zielsuchrakete", "Homing Missiles"},
	{"Kontaktmine", "Proximity Bombs"},
	{"Intell. Rakete", "Smart Missiles"},
	{"Mega-Rakete", "Mega Missiles"},
	{"Blendrakete", "Flash Missiles"},
	{"Lenkrakete", "Guided Missiles"},
	{"Intell. Mine", "Smart Mines"},
	{"Quecksilber-Rakete", "Mercury Missiles"},
	{"Erdbeben-Rakete", "EarthShaker Missiles"},
	{"Unverwundbarkeit", "Invulnerability"},
	{"Tarnvorrichtung", "Cloaking"},
	{"Nachbrenner", "Afterburners"},
	{"Waffentraeger", "Ammo rack"},
	{"Energiewandler", "Energy Converter"},
	{"Scheinwerfer", "Headlight"},
	{"zugelassene Objekte", "objects to allow"},
	{"Spieler-Handicap aktivieren", "Apply Player Handicap"},
	{"Eroberungsalarm abspielen", "Play Conquer Warning Sound"},
	{"Labore zurueck verwandeln", "Revert rooms converted to labs"},
	{"Virus-Stabilitaet - zerstoeren:", "Virus stability - destroy:"},
	{"wenn abgeworfen", "when Dropped"},
	{"wenn von Gegner beruehrt", "when Touched by Enemy player"},
	{"wenn ausserhalb Labor beruehrt", "when touched while not in Lab"},
	{"Nie", "Never"},
	{"~Entropy-Einstellungen", "Entropy Toggles"},
	{"Raumtexturen er~Halten", "Keep room textures"},
	{"Raumtexturen er~Setzen", "Override room textures"},
	{"Teamraeume ~Faerben", "Colorize team rooms"},
	{"Teamraeume ~Erhellen", "Brighten team rooms"},
	{"~Raumtexturen", "Room textures"},
	{"# ~Viren fuer Eroberung", "Viruses required for capture:"},
	{"Er~Oberungsdauer [Sek]", "Time required for capture [sec]:"},
	{"Max. Virus~Kapazitaet (0: unbegr.):", "Max. virus capacity (0=unlimited):"},
	{"Viruskap. ~Waechst bei Abschuss um:", "Increase virus cap. on kill by:"},
	{"Viruskap. ~Sinkt bei Eroberung um:", "Decrease virus cap. on capture by:"},
	{"Virus-~Produktionszeit [Sek]:", "Virus production time [sec]:"},
	{"~Energiefuellrate [Energie/Sek]:", "Energy fill rate [energy/sec]:"},
	{"~Schildfuellrate [Schilde/Sek]:", "Shield fill rate [shields/sec]:"},
	{"~Beschaedigungsrate [Schilde/Sek]:", "Shield damage rate [shields/sec]:"},
	{"En~Tropie-Schalter...", "Entropy Toggles..."},
	{"~Handhabung Raumtexturen...", "Room Texture Handling..."},
	{"~Max. Zeit: %d %s", "MAX TIME: %d %s"},
	{"~Abschussziel: %d Abschuesse", "KILL GOAL: %d kills"},
	{"~Unverwundbar beim Erscheinen", "INVULNERABLE WHEN REAPPEARING"},
	{"~Kamerasicht von Bojen zulassen", "ALLOW CAMERA VIEWS FROM MARKERS"},
	{"Unzerstoerbare ~Lichter", "INDESTRUCTIBlE LIGHTS"},
	{"~Helle Spielerschiffe", "BRIGHT PLAYER SHIPS"},
	{"~Gegnernamen anzeigen", "SHOW ENEMY NAMES ON HUD"},
	{"~Alle Spieler auf Karte anzeigen", "Show all players on Automap"},
	{"~Freundfeuer", "Friendly Fire"},
	{"~Selbstmord verhindern", "Inhibit SUicide"},
	{"Schnelle ~Maus", "MOuselook"},
	{"Schnelle ~Vertikalbew.", "fast Pitch"},
	{"~Teams ausgleichen", "Auto-Balance Teams"},
	{"M~Issions-Rundlauf", "CYcle Through Missions"},
	{"Reaktor ~Deaktivieren", "Disable Reactor"},
	{"Kurze Daten~Pakete", "SHort Packets"},
	{"Zugelassene ~Objekte...", "SET objects ALLOWED..."},
	{"Netzwerk-Socket", "NETWORK SOCKET"},
	{"Datenpakete pro Sekunde (2-20)", "PACKETS PER SECOND (2 - 20)"},
	{"Weitere Netzwerk-Einstellungen", "Additional netgame options"},
	{"Unzulaessige Paketrate\nstelle 20 ein", "Packet value out of range\nSetting value to 20"},
	{"Unzulaessige Paketrate\nstelle 2 ein", "Packet value out of range\nSetting value to 2"},
	{"Unzulaessiger Socket\nstelle %d ein", "Socket value out of range\nSetting value to %d"},
	{"Bei Koop-Spielen nicht moeglich", "You can't change those for coop!"},
	{"(keine ausgewaehlt)", "(none selected)"},
	{"Joy ~Nullbereich: %d%%", "Joy Deadzone: %d%%"},
	{"Joy %c ~Nullbereich: %d%%", "Joy %c Deadzone: %d%%"},
	{"Tastatur~Geschw. (%d %%)", "Keyboard ramping (%d %%)"},
	{"~Maus aktivieren", "Use Mouse"},
	{"Maus ~Einstellen...", "Customize MOuse..."},
	{"Mausachsen ~Koppeln", "SYnc Mouse Axes"},
	{"~Empfindlichkeit Maus", "MOuse Sensitivity"},
	{"~Empfindlichkeit Maus %c", "Mouse %c Sensitivity"},
	{"Standardmaus", "Standard mouse"},
	{"Cyberman", "Cyberman"},
	{"~Joystick aktivieren", "Use Joystick"},
	{"Joystick ei~Nstellen...", "Customize Joystick..."},
	{"Joystick ~Linear skalieren", "LInear joystick sensitivity scaling"},
	{"Joystick-Achsen k~Oppeln", "SYnc Joystick Axes"},
	{"E~Mpfindlichkeit Joystick", "Joystick Sensitivity"},
	{"E~Mpfindlichkeit Joystick %c", "Joy %c Sensitivity"},
	{"St~Andard-Joystick", "Standard joystick"},
	{"Flightstick Pro", "Flightstick Pro"},
	{"Thrustmaster FCS", "Thrustmaster FCS"},
	{"Gravis Gamepad", "Gravis Gamepad"},
	{"Waffen-Schnelltasten akti~Vieren", "Use Weapon Hotkeys"},
	{"Waffen-Schnelltasten ~Belegen...", "Customize Weapon Hotkeys"},
	{"~Tastaturbelegung...", "Customize Keyboard"},
	{"Beschleunigung steigern", "Ramp Acceleration Keys"},
	{"Drehungsgeschw. steigern", "Ramp ROtation Keys"},
	{"Gleitgeschw. steigern", "Ramp sliDe Keys"},
	{"Tastatur", "KEYBOARD"},
	{"Joystick", "JOYSTICK"},
	{"Maus", "MOUSE"},
	{"Waffen-Kurztasten", "WEAPON HOTKEYS"},
	{"Nase runter", "Pitch forward"},
	{"Nase hoch", "Pitch backward"},
	{"Linksdrehung", "Turn left"},
	{"Rechtsdrehung", "Turn right"},
	{"Gleiten an", "Slide on"},
	{"Gleite links", "Slide left"},
	{"Gleite rechts", "Slide right"},
	{"Gleite hoch", "Slide up"},
	{"Gleite runter", "Slide down"},
	{"Rollen an", "Bank on"},
	{"Rolle links", "Bank left"},
	{"Rolle rechts", "Bank right"},
	{"Primaerfeuer", "Fire primary"},
	{"Sekundaerfeuer", "Fire secondary"},
	{"Leuchtkugel", "Fire flare"},
	{"Schneller", "Accelerate"},
	{"Langsamer", "reverse"},
	{"Bombenabwurf", "Drop Bomb"},
	{"Rueckspiegel", "Rear View"},
	{"Kreuze schneller", "Cruise Faster"},
	{"Kreuze langsamer", "Cruise Slower"},
	{"Kreuzen aus", "Cruise Off"},
	{"Uebersicht", "Automap"},
	{"Nachbrenner", "Afterburner"},
	{"Wahl prim.Waffe", "Cycle Primary"},
	{"Wahl sek.Waffe", "Cycle Second"},
	{"Vergroessern", "Zoom In"},
	{"Scheinwerfer", "Headlight"},
	{"Energie->Schild", "Energy->Shield"},
	{"Bombe waehlen", "Toggle Bomb"},
	{"Nase h/r", "Pitch U/D"},
	{"Drehe l/r", "Turn L/R"},
	{"Gleite l/r", "Slide L/R"},
	{"Gleite h/r", "Slide U/D"},
	{"Rolle l/r", "Bank L/R"},
	{"Schub", "throttle"},
	{"Waffe 1", "WEAPON 1"},
	{"Waffe 2", "WEAPON 2"},
	{"Waffe 3", "WEAPON 3"},
	{"Waffe 4", "WEAPON 4"},
	{"Waffe 5", "WEAPON 5"},
	{"Waffe 6", "WEAPON 6"},
	{"Waffe 7", "WEAPON 7"},
	{"Waffe 8", "WEAPON 8"},
	{"Waffe 9", "WEAPON 9"},
	{"Waffe 10", "WEAPON 10"},
	{"%s ist noch nicht frei", "%s has not been released"},
	{"Blaue Codekarte", "blue key"},
	{"Gelbe Codekarte", "yellow key"},
	{"Rote Codekarte", "red key"},
	{"Reaktor", "reactor"},
	{"Boss", "boss"},
	{"Ausgang", "Exit"},
	{"Boje %i", "marker %i"},
	{"Unterdruecke", "Suppress"},
	{"Aktiviere", "Enable"},
	{"Nachrichten aktiviert", "Messages enabled."},
	{"Nachrichten unterdrueckt", "Messages suppressed."},
	{"OPT-F2  (%c-s)\t Spiel speichern", "OPT-F2  (%c-s)\t Save Game"},
	{"OPT-F3  (%c-o)\t Spiel laden", "OPT-F3  (%c-o)\t Load Game"},
	{"Pause  (F15)\t  Pause", "Pause  (F15)\t  Pause"},
	{"Alt+F9\t  Bildschirmabzug", "Alt+F9\t  save screen shot"},
	{"Umsch-F1\t  Li. Fenster durchschalten", "Shift-F1\t  Cycle left window"},
	{"Umsch-F2\t  Re. Fenster durchschalten", "Shift-F2\t  Cycle right window"},
	{"Umsch-F3\t  Fenstergroesse aendern", "Shift-F3\t  Change cockpit window size"},
	{"Strg-F3\t  Fensterposition aendern", "Ctrl-F3\t  Change cockpit window pos"},
	{"Umsch-Strg-F3\t  Fensterzoom aendern", "Shift-Ctrl-F3\t  Change cockpit window zoom"},
	{"Umsch-F4\t  GuideBot-Menue", "Shift-F4\t  GuideBot menu"},
#ifdef MACINTOSH
	{"Opt-Shift-F4\t  GuideBot umbenennen", "Opt-Shift-F4\t  Rename GuideBot"},
#else
	{"Alt-Shift-F4\t  GuideBot umbenennen", "Alt-Shift-F4\t  Rename GuideBot"},
#endif
	{"Umsch-F5\t  prim. Waffe abwerfen", "Shift-F5\t  Drop primary"},
	{"Umsch-F6\t  sek. Waffe abwerfen", "Shift-F6\t  Drop secondary"},
	{"Umsch-F7\t  Joystick kalibrieren", "Shift-F7\t  Calibrate joystick"},
	{"Umsch-Ziffer\t  GuideBot-Befehle", "Shift-number\t  GuideBot commands"},
	{"0. Naechstes Ziel: %s", "0.  Next Goal: %s"},
	{"1. Finde Energie", "1.  Find Energy Powerup"},
	{"2. Finde Energie-Zentrum", "2.  Find Energy Center"},
	{"3. Finde Schilde", "3.  Find Shield Powerup"},
	{"4. Finde Ausruestung", "4.  Find Any Powerup"},
	{"5. Finde einen Robot", "5.  Find a Robot"},
	{"6. Finde eine Geisel", "6.  Find a Hostage"},
	{"7. Geh mir vom Acker!", "7.  Stay Away From Me"},
	{"8. Finde meine Ausruestung", "8.  Find My Powerups"},
	{"9. Finde den Ausgang", "9.  Find the exit"},
	{"\nT.  %s Nachrichten", "\nT.  %s Messages"},
	{"Spiel speichern", "Save Game"},
	{"Waehle zu ladendes Spiel", "Select Game to Restore"},
	{"Spiel laden?", "Restore Game?"},
	{"Fehler!\nKann Mission nicht laden:\n'%s'\n", "Error!\nUnable to load mission\n'%s'\n"},
	{"Kann in Geheimleveln nicht laden!", "Can't restore in secret level!"},
	{"Kann in Geheimleveln nicht speichern!", "Can't save in secret level!"},
	{"Keine gespeicherten Spiele gefunden!", "No saved games were found!"},
	{"Kann nicht aus Datei <%s> lesen: %s", "Cannot read from file <%s>: %s"},
	{"Kann nicht in Datei <%s> schreiben: %s", "Cannot write to file <%s>: %s"},
	{"Fehler beim Speichern des Spiels.\nEvtl. zuwenig Speicherplatz vorhanden.", "Error writing savegame.\nPossibly out of disk\nspace."},
	// HUD messages
	{"Kein Platz mehr fuer Bojen", "No free marker slots"},
	{"Boje %s: %s", "MARKER %s: %s"},
	{"Boje %d: %s", "MARKER %d: %s"},
	{"Boje %d", "MARKER %d"},
	{"Kein Guidebot in der Mine.", "No Guidebot present in mine."},
	{"Kein Guidebot in Netzspielen!", "No Guidebot in Multiplayer!"},
	{"Flagge zurueckgekehrt!", "Flag was returned!"},
	{"Brauche ueber %i Energie fuer den Transfer", "Need more than %i energy to enable transfer"},
	{"Kein Transfer: Schild am Anschlag", "No transfer: Shields already at max"},
	{"Cockpit in 3dfx-Version nicht verfuegbar.", "Cockpit not available in 3dfx version."},
	{"Pause in Modem/seriellem Spiel nicht moeglich!", "You cannot pause in a modem/serial game!"},
	{"-Stereoskopie-Parameter ruecksetzen-", "-Stereoscopic Parameters Reset-"},
	{"Achsentrennung = %.2f", "Interaxial Separation = %.2f"},
	{"Stereobalance = %.2f", "Stereo balance = %.2f"},
	{"Normale Aufloesung", "Normal Resolution"},
	{"niedrige vert. Aufloesung", "Low Vertical Resolution"},
	{"niedrige horiz. Aufloesung", "Low Horizontal Resolution"},
	{"niedrige Aufloesung", "Low Resolution"},
	{"-Augen getauscht-", "-Eyes toggled-"},
	{"rechtes Auge -- linkes Auge", "Right Eye -- Left Eye"},
	{"linkes Auge -- rechtes Auge", "Left Eye -- Right Eye"},
	{"Empfindlichkeit Kopfmessung = %d", "Head tracking sensitivy = %d"},
	{"(Standardwert ist %.2f)", "(The default value is %.2f)"},
	{"Spieler akzeptiert!", "Player accepted!"},
	{"Kumpel geplaettet! *schnueff*", "Toasted the Buddy! *sniff*"},
	{"%i Robots plattgemacht!", "%i robots toasted!"},
	{"Dieb plattgemacht!", "Thief toasted!"},
	{"Zerstoeren, Punkte, usw.", "Killing, awarding, etc.!"},
	{"Nimm das...Schummler!", "Take that...cheater!"},
	{"Ausruestung!!", "Accessories!!"},
	{"Abwaerts geht's...", "Coming down..."},
	{"Und hoch!", "Going up!"},
	{"Robotfeuer AN", "Robot firing ON!"},
	{"Robotfeuer AUS", "Robot firing OFF!"},
	{"%s wird sauer!", "%s gets angry!"},
	{"%s beruhigt sich", "%s calms down"},
	{"Was ist das? Noch ein Kumpel-Bot!", "What's this? Another buddy bot!"},
	{"Abpraller-Waffen!", "Bouncing weapons!"},
	{"Gib neue Segmentnr. ein:", "Enter new segment number:"},
	{"Komplettkarte!!", "Full Map!!"},
	{"Schluuuuuuuuueeeeeeeerf!", "Sluuuuuuuuuuuuuuurp!"},
	{"Zielsuchende Waffen!", "Homing weapons!"},
	{"Hi John!", "Hi John!!"},
	{"Ciao John!", "Bye John!!"},
	{"Nur die Gerechten werden leben!", "Only the righteous shall survive!"},
	{"Armageddon!", "Armageddon!!"},
	{"Oh je, das war's mit Tokio!", "Oh no, there goes Tokyo!"},
	{"Was tust Du - ich schrumpfe!", "What have you done, I'm shrinking!!"},
	{"Geisterstunde", "Ghosty mode"},
	{"Schnellfeuer AN", "Rapid fire ON!"},
	{"Schnellfeuer AUS", "Rapid fire OFF!"},
	{"Toetet den Spieler", "Kill the player!"},
	{"Rasende Robots!", "Rabid robots!"},
	{"Ich fuehl mich langgezogen...", "I feel dilated..."},
	{"Schwuuuuuuuupps", "Swoooooooosh!"},
	{"Panzerknacker-Zeit!", "Robbing the Bank of England!"},
	{"Lass mich rueber!", "Let me over!"},
	{"Willste schummeln, was?", "Wanna cheat, huh?"},
	{"Ich bin der Gute!", "I am the good guy!"},
	{"Das hilft jetzt auch nicht mehr...", "This won't help you anymore..."},
	{"Zurueck zum Cockpit mit F3", "Press F3 to return to Cockpit mode"},
	{"Keine Bomben verfuegbar!", "No bombs available!"},
	{"Keine %s verfuegbar!", "No %s available!"},
	{"Intell. Minen", "Smart mines"},
	{"Kontaktminen", "Proximity bombs"},
	{"Du wurdest von einer Mine zerstoert!", "You were killed by a mine!"},
	{"%s wurde von einer Mine zerstoert!", "%s was killed by a mine!"},
	{"Du hast das Abschussziel erreicht!", "You reached the kill goal!"},
	{"%s hat das Abschussziel erreicht!", "%s has reached the kill goal!"},
	{"Das Kontrollzentrum wurde zerstoert", "The control center has been destroyed!"},
	{"Nur %s kann andere %s", "Only %s can %s others!"},
	{"Du musst einen Namen zum %s angegeben", "You must specify a name to %s"},
	{"Unzulaessige Spielernr. zum %s", "Invalid player number for %s."},
	{"Du kannst in der Teamansicht nicht nach # %s", "You cannot %s by # within team display."},
	{"Entsorge %s...", "Dumping %s..."},
	{"Fuer Ping musst Due einen Namen angeben", "You must specify a name to ping"},
	{"Pinge %s...", "Pinging %s..."},
	{"Pinge Gegenspieler", "Pinging opponent..."},
	{"Nur %s kann Spieler verschieben!", "Only %s can move players!"},
	{"Du musst einen Namen fuers verschieben angeben", "You must specify a name to move"},
	{"Kann Spieler %s nicht verschieben: Ist Flaggentraeger!", "Can't move player because s/he has a flag!"},
	{"Du hast die Seiten gewechselt!", "You have changed teams!"},
	{"%s hat die Seiten gewechselt!", "%s has changed teams!"},
	{"Namensnennung ist jetzt %s", "Name returning is now %s."},
	{"aktiv", "active"},
	{"inaktiv", "disabled"},
	{"Handicap:", "Handicap:"},
	{"NoBombs", "NoBombs"},
	{"Ping:", "Ping:"},
	{"Move:", "Move:"},
	{"Kick:", "Kick:"},
	{"Ban:", "Ban:"},
	{"Kann nicht speichern...\nalle Spieler muessen am Leben sein!", "Can't save...all players must be alive!"},
	{"Kann nicht laden...\nalle Spieler muessen am Leben sein!", "Can't restore...all players must be alive!"},
	{"Speichere Spiel #%d, '%s'", "Saving game #%d, '%s'"},
	{"Das %se Team gewinnt die Partie!", "The %s team wins this match!"},
	{"Du hast die meisten Punkte mit %d Abschuessen!", "You have the best score at %d kills!"},
	{"%s hat die meisten Punkte mit %d Abschuessen!", "%s has the best score with %d kills!"},
	{"Du hast gepunktet!", "You have Scored!"},
	{"%s hat gepunktet!", "%s has Scored!"},
	{"Du hast %d Punkte gemacht!", "You have scored %d points!"},
	{"%s hat mit %d Orbs gepunktet!", "%s has scored with %d orbs!"},
	{"Du hast den Rekord mit %d Punkten!", "You have the record with %d points!"},
	{"%s hat den Rekord mit %d Punkten!", "%s has the record with %d points!"},
	{"%s hat eine Flagge aufgenommen!", "%s picked up a flag!"},
	{"%s hat einen Virus aufgenommen!", "%s picked up a virus!"},
	{"%s hat eine Orb aufgenommen!", "%s picked up an orb!"},
	{"Keine Orbs abzuwerfen!", "No orbs to drop!"},
	{"Kein Virus abzuwerfen!", "No virus to drop!"},
	{"Keine Flagge abzuwerfen!", "No flag to drop!"},
	{"Orb abgeworfen!", "Orb dropped!"},
	{"Virus abgeworfen!", "Virus dropped!"},
	{"Flagge abgeworfen!", "Flag dropped!"},
	{"Du bist zu %s befoerdert worden!", "You have been promoted to %s!"},
	{"Du bist zu %s degradiert worden!", "You have been demoted to %s!"},
	{"%s ist zu %s %s worden!", "%s has been %s to %s!"},
	{"befoerdert", "promoted"	},
	{"degradiert", "demoted"},
	{"Pingzeit fuer Gegenspier ist %d ms!", "Ping time for opponent is %d ms!"},
	{"Autsch! %s hat eine Kopfnuss von %s bekommen!", "Ouch! %s has been smacked by %s!"},
	{"Haha! %s hat Hiebe von %s bekommen!", "Haha! %s has been spanked by %s!"},
	{"%s hat versucht, Dich zu kicken", "%s attempted to kick you out."},
	{"%s hat Dich gekickt!", "%s has kicked you out!"},
	{"%s will teilnehmen...zulassen mit F6", "%s wants to join...press F6 to connect"},
	{"die Komplettkarte", "the FULL MAP"},
	{"KOMPLETTKARTE!", "FULL MAP!"},
	{"der Konverter", "the Converter"},
	{"Energie->Schild-Konverter! (benutzen mit %c)", "Energy->Shield converter! (Press %c to use)"},
	{"Superlaser am Anschlag!", "SUPER LASER MAXED OUT!"},
	{"Laser wurde auf Superausfuehrung %d gehoben", "Super Boost to Laser level %d"},
	{"der Waffentraeger", "the Ammo rack"},
	{"WAFFENTRAEGER!", "AMMO RACK!"},
	{"der Nachbrenner", "the Afterburner"},
	{"Nachbrenner!", "AFTERBURNER!"},
	{"der Scheinwerfer", "the Headlight boost"},
	{"SCHEINWERFER! (Scheinwerfer ist %s)", "HEADLIGHT BOOST! (Headlight is %s)"},
	{"Geheimlevel-Teleporter in Netzspielen inaktiv!", "Secret Level Teleporter disabled in multiplayer!"},
	{"Geheimlevel zerstoert.  Ausgang deaktiviert.", "Secret Level destroyed.  Exit disabled."},
	{"Geheimlevel-Teleporter in der Descent 2-Demo inaktiv!", "Secret Level Teleporter disabled in Descent 2 Demo"},
	{"%s Level %d %s", "%s Level %d %s"},
	{"Keine Sekundaerwaffen gewaehlt!", "No secondary weapons selected!"},
	{"Keine Sekundaerwaffen verfuegbar!", "No secondary weapons available!"},
	{"Du kannst die Basiswaffe nicht abwerfen!", "You cannot drop your base weapon!"},
	{"Quadlaser abgeworfen!", "Quad Lasers dropped!"},
	{"Superlaser abgeworfen!", "Super Laser Cannon dropped!"},
	{"Laserkanone abgeworden!", "Laser Cannon dropped!"},
	{"%s abgeworfen", "%s dropped!"},
	{"Keine Sekundaerwaffe abzuwerfen!", "No secondary weapon to drop!"},
	{"Du brauchst mindestens 4 fuer den Abwurf!", "You need at least 4 to drop!"},
	{"% hat die Seiten gewechselt!", "%s has changed teams!"},
	{"Du bist dem %sen Team beigetreten!", "You have joined the %s team!"},
	{"Spieler %s ist dem %sen Team beigetreten", "Player %s has joined the %s team."},
	{"Handicap von %s ist jetzt %d", "%s handicap is now %d"},
	{"%s hat versucht zu schummeln!", "%s has tried to cheat!"},
	{"Sage den anderen Dein Handicap von %d", "Telling others of your handicap of %d!"},
	{"!Names", "!Names"},
	{"Fuer diesen Level wurde kein Rekord erzielt.", "There was no record set for this level."},
	{"%s hat den hoechsten Rekord mit %d Punkten.", "%s had the best record at %d points."},
	{"(zu viele Missionen)", "(too many missions)"},
	{"0 Tracker gefunden", "0 trackers found"},
	{"Warum koennen wir nicht miteinander auskommen?", "Why can't we all just get along?"},
	{"Hey, ich habe ein Geschenk fuer Dich", "Hey, I got a present for ya"},
	{"Hey, mir ist nach Pruegeln", "I got a hankerin' for a spankerin'"},
	{"Der ist unterwegs zum Uranus", "This one's headed for Uranus"},
	{"Fehler: ", "Error: "},
	{"{Strg}", "{Ctrl}"},
	{"{Alt}", "{Alt}"},
	{"{Umsch}", "{Shift}"},
	{"%d%c fertig", "%d%c done"},
	{"Boje %d: %s", "Marker %d: %s"},
	{"Verbleibende Zeit: %d Sek.", "Time left: %d secs"},
	{"PAUSE\n\nKoennen:  %s\nGeiseln an Bord:  %d\nZeit im Level: %s\nGesamte Spielzeit: %s", 
	 "PAUSE\n\nSkill level:  %s\nHostages on board:  %d\nTime on level: %s\nTotal time in game: %s"},
	{"PAUSE\n\nKoennen:  %s\nGeisen an Bord:  %d\n", 
	 "PAUSE\n\nSkill level:  %s\nHostages on board:  %d\n"},
	{"Anarchie", "Anarchy"},
	{"Team-Anarchie", "Team Anarchy"},
	{"Robo-Anarchie", "Robo Anarchy"},
	{"Kooperativ", "Cooperative"},
	{"Flagge erobern", "Capture the Flag"},
	{"Horten", "Hoard"},
	{"Team-Horten", "Team Hoard"},
	{"Entr~Opie", "Entrop~Y"},
	{"Unbekannt", "Unknown"},
	{"ruiniert", "trashing"},
	{"verschlechert wirklich", "really hurting"},
	{"beeintraechtigt wirklich", "seriously affecting"},
	{"verschlechert", "hurting"},
	{"beintraechtigt", "affecting"},
	{"verringert", "tarnishing"},
	{"Fuer diesen Level gibt es keinen Rekord.", "There is no record yet for this level."},
	{"%s hat den Rekord mit %d Punkten.", "%s has the record at %d points."},
	{"Deine lebenslange Effizienz von %d%s%s", "Your lifetime efficiency of %d%%"},
	{"%s Deinen Rang.", "is %s your ranking."},
	{"gibt ein gutes Bild ab.", "is serving you well."},
	{"Spiel:\t%s", "Game:\t%s"},
	{"Mission:\t%s", "Mission:\t%s"},
	{"Aktueller Level:\t%d", "Current Level:\t%d"},
	{"Koennen:\t%s", "Difficulty:\t%s"},
	{"Spielmodus:\t%s", "Game Mode:\t%s"},
	{"Spielleiter:\t%s", "Game Master:\t%s"},
	{"Anzahl Spieler:\t%d/%d", "Number of players:\t%d/%d"},
	{"Datenpakete/Sekunde:\t%d", "Packets per second:\t%d"},
	{"Kurze Datenpakete:\t%s", "Short Packets:\t%s"},
	{"Verlorene Daten:\t%d (%d%%)", "Packets lost:\t%d (%d%%)"},
	{"Abschussziel:\t%d", "Kill goal:\t%d"},
	{"Verbinde Spieler:", "Connected players:"},
	{"\nGeschwindigkeitstest beendet: %i Bilder, %7.3f Sekunden, %7.3f Bilder/sek.\n", 
	 "\nSpeedtest done:  %i frames, %7.3f seconds, %7.3f frames/second.\n"},
	{"Boje: %s_", "Marker: %s_"},
	{"Warnung: %i Fehler in %s!\n", "Warning: %i errors in %s!\n"},
	{"Warnung: %i Fehler in dieser Mine!\n", "Warning: %i errors in this mine!\n"},
	{"Speichern abbrechen", "Cancel Save"},
	{"Speichern", "Save"},
	{"Geheimlevel zerstoert.\nWeiter mit Level %i.", "Secret level already destroyed.\nAdvancing to level %i."},
	{"Kehre zu Level %i zurueck", "Returning to level %i"},
	{"SCHREIBE...", "TYPING..."},
	{"SlagelSlagel!!", "SlagelSlagel!!"},
	{"Das Spiel bei dem Du Namen angefragt hast\nist nicht mehr da.\n", "The game you have requested\nnames from is gone.\n"},
	{"Spieler von Spiel '%s':", "players of game '%s':"},
	{"Kurze Datenpakete: %s", "Short packets: %s"},
	{"Datenpakete/Sek.: %d", "Packets Per Second: %d"},
	{"Netzspiel-Information", "netGame Information"},
	{"Spiel: %s", "Game: %s"},
	{"Mission: %s", "Mission: %s"},
	{"Aktueller Level: %d", "Current Level: %d"},
	{"Koennen: %s", "Difficulty: %s"},
	{"Kein Kommentar", "No Comment"},
	{"kicken", "kick"},
	{"bannen", "ban"},
	{"verschieben", "move"},
	{"%s ist angeschlagen...schnappt ihn!", "%s is crippled...get him!"},
	{"\nAudio-Geraet konnte nicht geoeffnet werden\n(%s)", "\nError: Couldn't open audio\n(%s)"},
	{"Konnte SDL-Bibliothek nicht initialisieren\n(%s)", "SDL library initialisation failed\n(%s)"},
	{"Keine CD-Laufwerke gefunden!\n", "No cdrom drives found!\n"},
	{"Konnte CD nicht fuer Redbook-Audio oeffnen!\n", "Could not open cdrom for redbook audio!\n"},
	{"Kann Aussengelaende nicht aus\nDatei %s laden:\nIFF-Fehler: %s", "Can't load exit terrain from\nfile %s:\nIFF error: %s"},
	{"Kann Satellit nicht aus\nDatei %s laden:\nIFF-Fehler: %s", "Can't load exit satellite from\nfile %s: IFF error: %s"},
	{"Kann unbekannte Descent 1\nTextur #%s nicht konvertieren", "can't convert unknown descent 1\ntexture #%d.\n"},
	{"D1-Hogdatei hat unbekannte Groesse", "Unknown D1 hogsize %d\n"},
	{"Hogdatei hat unbekannte Groesse %d,\nversuche es mit %s", "Unknown hogsize %d,\ntrying %s\n"},
	{"descent.hog nicht verfuegbar.\nDieser Mission fehlen evt. einige\nDateien, die fuer Briefings\nund Endsequenzen benoetigt werden.\n", 
	 "descent.hog not available.\nThis mission may be missing\nsome files required for\nbriefings and exit sequence\n"},
	{"Kann Robotfilm <%s> nicht oeffnen", "Cannot open robot movie file <%s>"},
	{"Kann Filmdatei <%s> nicht oeffnen", "Cannot open movie file <%s>"},
	{"Kann keinen Film namens <%s> finden", "Cannot open any movie file <%s>"},
	{"%s Joystickachsen gefunden, aber nur %s unterstuetzt.\n", "Found joystick %d axes, only %d supported.\n"},
	{"%s Joysticktasten gefunden, aber nur %s unterstuetzt.\n", "Found joystick %d buttons, only %d supported.\n"},
	{"%s Joystickhats gefunden, aber nur %s unterstuetzt.\n", "Found joystick %d hats, only %d supported.\n"},
	{"Kennung der HXM-Datei unzulaessig", "ID of HXM! file incorrect"},
	{"Version der HXM-Datei zu alt (%d)", "HXM! version too old (%d)"},
	{"Robotnr. %d aus <%s> unzulaessig.\n(Bereich = 0 - %d)", "Robot number %d from <%s>\nout of range in.\n(Range = 0 - %d)"},
	{"Konnte Leveldatei <%s> nicht laden", "Couldn't load level file\n<%s>"},
	{"Dieser Level ist nicht fuer Entropie geeignet!\nWechsle zu Team-Anarchie.", "This level is not Entropy enabled!\nChanging game mode to Team Anarchy."},
	{"Kann Hilfetexte nicht laden", "Cannot load help text file."},
	{"Keine gueltige Hog-Datei gefunden (descent2.hog)", "Could not find a valid hog file (descent2.hog)"},
	{"Konnte keine %d Bytes der Objekte fuer\ninterpoliertes Abspielen finden.\n", "Couldn't get %d bytes for objects\nin interpolate playback\n"},
	{"Ladefehler:", "Failed loading"},
	{"descent.pig v1.0 und PC-Shareware-\nVersionen nicht unterstuetzt", "descent.pig of v1.0 and all\nPC shareware versions not supported."},
	{"Unbekannte Groesse von ", "Unknown size for "},
	{"Gespeicherter Spielstand beschaedigt!", "Save game data corrupted!"},
	{"Kann weder DESCENT.TEX noch DESCENT.TXB oeffnen", "Cannot open file DESCENT.TEX or DESCENT.TXB"},
	{"Schalter verweist auf einseitige Wand\n(Segment:%d, Seite: %d, Schalter:%d)!",
	 "Trigger targets single sided wall\n(segment:%d, side:%d, trigger:%d)!"},
	{"Zerstoere einseitige Wand\n(Segment:%d, Seite: %d, Wand:%d)!", 
	 "Blasting single sided wall\n(segment:%d, side:%d, wall:%d)!"},
	{"Beschaedige einseitige Wand\n(Segment:%d, Seite: %d, Wand:%d)!", 
	 "Damaging single sided wall\n(segment:%d, side:%d, wall:%d)!"},
	{"Oeffne einseitige Tuer\n(Segment:%d, Seite: %d, Wand:%d)!", 
	 "Opening single sided door\n(segment:%d, side:%d, wall:%d)!"},
	{"Einseitige Illusion\n(Segment:%d, Seite: %d)!", 
	 "Single sided illusion\n(segment:%d, side:%d)!"},
	{"Unzulaessiger Seitentyp in render_side\n(Typ:%i, Segment:%i, Seite:%i)\n", 
	 "Illegal side type in render_side\n(type; %i, segment: %i, side:%i)\n"},
	{"Schriftdatei evtl. beschaedigt", "Font file probably damaged"},
	{"Schriftart passt nicht wirklich (%i/%i)?\n", "font doesn't really fit (%i/%i)?\n"},
	{"Kann Schriftdatei <%s> nicht oeffnen", "Can't open font file %s"},
	{"Datei <%s> ist keine Schriftdatei", "File %s is not a font file"},
	{"Kann weder Palettendatei <%s>\nnoch Standardpalette <%s> oeffnen.\n", 
	 "Can open neither palette file <%s>\nnor default palette file <%s>.\n"},
	{"Konnte SDL-Audio nicht initialisieren:\n%s.\n", "SDL audio initialisation failed:\n%s."},
	{"Konnte SDL-Vidio nicht initialisieren:\n%s.\n", "SDL video initialisation failed:\n%s."},
	{"Brnr: %d%%", "burn: %d%%"},
	{"Blend", "Flash"},
	{"Lenk", "Guided"},
	{"IntlMine", "SmrtMine"},
	{"Quecks", "Mercury"},
	{"Beben", "Shaker"},
	{"N/V", "N/A"},
	{"~IPX", "IPX"},
	{"~UDP/IP", "UDP/IP"},
	{"UDP/IP via ~Tracker", "UDP/IP via Tracker"},
	{"UDP/IP ~Multicast", "UDP/IP Multicast"},
	{"~KALI", "KALI"},
	{"Netzwerkspiel ~Starten", "Start network game"},
	{"Netzwerkspiel ~Beitreten", "Join network game"},
	{"Verbindungsart:", "Connection type:"},
	{"%s gestohlen!", "%s stolen!"},
	{"Staerke verringert!", "%s level decreased!"},
	{"Virus-Lebensdauer [sek] (0=ewig):", "Virus life span [sec] (0=unlimited):"},
	{"Iconpos.", "Toggle Icons"},
	{"Rauchdichte: %s", "Smoke Density: %s"},
	{"Partikelgroesse: %s", "Particle size: %s"},
	{"Rauch ~Verwenden", "use smoke"},
	{"rauchende Sch~Iffe", "smoking player ships"},
	{"rauchende R~Obots", "smoking robots"},
	{"Raketen-~Rauchspuren", "Missile smoke trails"},
	{"Schiess auf seinen Ruecken!", "Hit him in the back!"},
	{"Da ist er verwundbar!", "He's vulnerable there!"},
	{"Flieg hinter ihn und schiess!", "Get behind him and fire!"},
	{"Triff den pulsierenden Fleck!", "Hit the glowing spot!"},
	{"klein", "small"},
	{"mittel", "medium"},
	{"gross", "large"},
	{"sehr gross", "very large"},
	{"sehr hoch", "very high"},
	{"extrem", "extreme"},
	{"E~Xplosionen wenn beschaedigt", "Show eXplosions if damaged"},
	{"zeige ~Abgasstrahl", "show thruster Flames"},
	{"~Kollisionserkennung Rauch", "smoke collIsion detection"},
	{"Warnton wenn Schild schwach", "Warning sound on low shields"},
	{"E~Xperten-Optionen anzeigen", "show eXpert options"},
	{"Sauber, %s!", "Nice job, %s!"},
	{"Suche blaue Codekarte", "Finding BLUE KEY"},
	{"Suche gelbe Codekarte", "Finding YELLOW KEY"},
	{"Suche rote Codekarte", "Finding RED KEY"},
	{"Suche Reaktor", "Finding REACTOR"},
	{"Suche Ausgane", "Finding EXIT"},
	{"Suche Energie", "Finding ENERGY"},
	{"Suche Energiezentrum", "Finding ENERGY CENTER"},
	{"Suche Schild", "Finding a SHIELD"},
	{"Suche Ausruestung", "Finding a POWERUP"},
	{"Suche Roboter", "Finding a ROBOT"},
	{"Suche Geiseln", "Finding a HOSTAGE"},
	{"Suche Boss Roboter", "Finding BOSS robot"},
	{"Suche Deine Ausruestung", "Finding your powerups"},
	{"Suche Boje %i: '%s'", "Finding marker %i: '%s'"},
	{"Bin schon weg... ", "Staying away..."},
	{"Kein(e) %s in der Mine", "No %s in mine."},
	{"Kann %s nicht erreichen", "Can't reach %s."},
	{"Hey, Dein Scheinwerfer ist an!", "Hey, your headlight's on!"},
	{"Ups, 'chulligung ...", "Oops, sorry 'bout that..."},
	{"Ich komm und hol Dich.", "Coming back to get you."},
	{"KLICK!", "CLICK!"},
	{"AAAHUGA!", "GAHOOGA!"},
	{"KRAWUMM!", "WHAMMO!"},
	{"Autsch!", "ouch!"},
	{"Schatten ~Zeichnen", "render shado~Ws"},
	{"Licht~Quellen: %d", "Light ~Sources: %d"},
	{"Blaue Codekarte", "BLUE KEY"},
	{"Gelbe Codekarte", "YELLOW KEY"},
	{"Rote Codekarte", "RED KEY"},
	{"Reaktor", "REACTOR"},
	{"Ausgang", "EXIT"},
	{"Energie", "ENERGY"},
	{"Energiezentrum", "ENERGYCEN"},
	{"Schilde", "SHIELD"},
	{"Ausr�stung", "POWERUP"},
	{"Roboter", "ROBOT"},
	{"Geiseln", "HOSTAGES"},
	{"Ladung", "SPEW"},
	{"Versteck", "SCRAM"},
	{"Ausgang", "EXIT"},
	{"Boss", "BOSS"},
	{"Boje 1", "MARKER 1"},
	{"Boje 2", "MARKER 2"},
	{"Boje 3", "MARKER 3"},
	{"Boje 4", "MARKER 4"},
	{"Boje 5", "MARKER 5"},
	{"Boje 6", "MARKER 6"},
	{"Boje 7", "MARKER 7"},
	{"Boje 8", "MARKER 8"},
	{"Boje 9", "MARKER 9"},
	{"Unverwundbarkeit gestohlen!", "Invulnerability stolen!"},
	{"Tarnkappe gestohlen!", "Cloak stolen!"},
	{"Komplettkarte gestohlen!", "Full map stolen!"},
	{"Quadlaser gestohlen!", "Quad lasers stolen!"},
	{"Nachbrenner gestohlen!", "Afterburner stolen!"},
	{"Konverter gestohlen!", "Converter stolen!"},
	{"Scheinwerfer gestohlen!", "Headlight stolen!"},
	{"~Kontrast: %s", "~Contrast: %s"},
	{"niedrig", "low"},
	{"standard", "standard"},
	{"hoch", "high"},
	{"Schilds~Phaeren zeichnen", "render shield s~Pheres"},
	{"Erweiterte Render-Optionen...", "~Advanced Render Options..."},
	{"Erweiterte Render-Optionen", "Advanced Render Options"},
	{"~Verschiedenes...", "M~Iscellaneous..."},
	{"Verschiedenes", "Miscellaneous"},
	{"sehr langsam", "very slow"},
	{"langsam", "slow"},
	{"schnell", "fast"},
	{"sehr schnell", "very fast"},
	{"~Computergeschw.: %s", "~Computer Speed: %s"},
	{"~Standardeinstellungen verwenden", "~Use default values"},
	{"Kehre aus Geheimlevel zurueck", "Returning from secret level"},
	{"Ladekapazitaet ueberschritten", "Inventory full"},
	{"Inventarsystem ~Verwenden", "use in~Ventory"},
	{"Tarnen", "Use cloak"},
	{"Haerten", "Use invul"},
	{"\n(erwarte %d statt %d)", "\n(expected %d, got %d)"},
	{"Maus wie einen Joystick behandeln", "Handle mouse like a joystick"},
	{"Schliessen", "Close"},
	{"Tasten und Hats", "Buttons and Hats"},
	{"Erzeuge Guidebot!", "Creating Guidebot!"},
	{"auf Abstieg vorbereiten", "Prepare for Descent"},
	{"Berechne Beleuchtung", "Computing Illumination"},
	{"Eigene Aufloesung", "Custom Resolution"},
	{"Mausp~Osition anzeigen", "Display Mouse Indicat~Or"},
	{"Teleporter zeigen Zielgebiet", "~Teleports show destination"},
	{"Spielername mehrfach vorhanden", "Duplicate player names"},
	{"Teilnahme vom Server abgelehnt", "Participation rejected by Server"},
	{"Level-Download fehlgeschlagen\n(Synchronisationsfehler)", "Level download failed\n(out of sync)"},
	{"Level-Download fehlgeschlagen\n(Datenpakete fehlen)", "Level download failed\n(missing data packets)"},
	{"Level-Download fehlgeschlagen\n(Dateifehler)", "Level download failed\n(file i/o error)"},
	{"Level-Download fehlgeschlagen", "Level download failed"},
	{"Missionsdatei fehlerhaft", "invalid mission file"},
	{"Spielerbotschaften se~Parat anzeigen", "Show se~Parate player messages"},
	{"Alternativer Ausgang gefunden!", "Alternate Exit Found!"},
	{"Monster~Ball", "Monster~Ball"},
	{"Konnte keine Zielgebiete finden.\nSchalte zu Team-Anarchie um.", 
	 "Couldn't find team goals.\nSwitching to Team Anarchy."},
	{"Konnte keinen Monsterball erzeugen.\nSchalte zu Team-Anarchie um.", 
	 "Couldn't create Monsterball object.\nSwitching to Team Anarchy."},
	{"Monster~Ball-Optionen...", "Monster~Ball Options..."},
	{"Tor-~Bonus: %d", "Goal ~Bonus: %d"},
	{"Monsterball-~Groesse: %d.%d", "Monsterball ~Size: %d.%d"},
	{"~Pyro-Kraft: x %d", "~Pyro Force: x %d"},
	{"Echtzeit-OpenGL-~Beleuchtung", "real-time OpenGL ~Lighting"},
	{"openGL ~Objekt-Beleuchtung", "openGL ~Object lighting"},
	{"L~Ichter/Segment: %d", "l~Ights/segment: %d"},
#if 0
	{"", ""},
	{"", ""},
#endif
	{"", ""}
	};

//------------------------------------------------------------------------------

char **pszHelpTexts = NULL;

char *defaultHelpTexts [][2] = {
	//main menu
	{"Neues Spiel starten.", "Start a new game."},
	{"Neues Einzelspieler-Spiel mit einer Mission aus dem\nMissions-Unterordner 'single' starten.", 
	 "Start a new single player game using a mission from the\nmissions subfolder 'single'."},
	{"Gespeicherten Spielstand laden.", "Load a saved game."},
	{"Mehrspieler-Netzwerkspiel starten.", "Start a multiplayer network game."},
	{"Programmeinstellungen �ndern.", "Change program settings."},
	{"Anderen Piloten laden.", "Load a different pilot."},
	{"Demoaufzeichnung abspielen.", "Playback a recorded demo."},
	{"Highscore-Liste anzeigen.", "Display Highscore list."},
	{"Einen Film aus Descent 2 abspielen.", "Play a Descent 2 movie."},
	{"Ein Lied aus Descent 2 spielen.", "Play a Descent 2 song."},
	{"Abspann abspielen.", "Show credits."},
	{"Programm verlassen.", "Quit program."},
	{"Ankreuzen um Descent 2-Missionen in der Missionsliste anzuzeigen", 
	 "Check to display Descent 2 missions in the mission list."},
	{"Ankreuzen um Descent 2-Missionen in der Missionsliste anzuzeigen", 
	 "Check to display Descent 1 missions in the mission list."},
	//controls configuration menu
	{"Ankreuzen, um die Steuerung des Schiffs per Maus zu ermoeglichen.", 
	 "Check to enable control of the ship using a mouse."},
	{"Mausbewegungen und -knoepfen Schiffsaktionen zuordnen.", 
	 "Assign ship actions to mouse movements and buttons."},
	{"Ankreuzen, um Reaktion des Schiffs auf Mausbewegungen zu verst�rken.", 
	 "Check to increase ship reaction on mouse movements."},
	{"Ankreuzen, damit das Schiff mit der Maus wie mit einem\nJoystick gesteuert werden kann: Das Schiff bewegt sich\nin die Richtung, in die der (unsichtbare) Mauszeiger von der Bildschirmmitte\nwegbewegt wird. Je groesser der Abstand von der Bildschirmmitte,\ndesto schneller die Bewegung.",
	 "Check to control the ship movement with the mouse like when\nusing a joystick: The ship moves to the direction\nthe (invisible) mousepointer has been moved away from the\nscreen center. The greater the distance from the screen center,\nthe faster the movement."},
	{"Wenn angekreuzt, erhalten alle Bewegungsachsen der Maus\ndieselbe Empfindlichkeit. Andernfalls kann die Empfind-\nlichkeit der Mausachen\ngetrennt eingestellt werden.", 
	 "If checked, all mouse axes have the same sensitivity.\nOtherwise, sensitivity can be adjusted per axis."},
	{"Staerke der Richtungsaenderungen des Schiffes bei Mausbewegungen einstellen.", 
	 "Adjust strength of changes of movement direction during mouse movements."},
	{"Ankreuzen, wenn Sie eine normale Maus verwenden.", "Check if you are using a normal mouse."},
	{"Ankreuzen, wenn Sie einen Cyberman verwenden.", "Check if you are using a Cyberman."},
	{"Ankreuzen, um die Steuerung des Schiffs per Joystick zu ermoeglichen.", 
	 "Check to enable control of the ship using a joystick."},
	{"Joystickbewegungen und -knoepfen Schiffsaktionen zuordnen.", 
	 "Assign ship actions to joystick movements and buttons."},
	{"Wenn angekreuzt, reagiert das Schiff gleichmaessig zunehmend\nauf Joystickbewegungen. Wenn nicht, ist die Reaktion anfangs\ngering und steigert sich mit zunehmenden Joystick-Ausschlag ueberproportional.", 
	 "If checked, ship reaction to joystick movements increases evenly,\nIf not, the ship reacts slowly to small joystick movements,\nand reaction increases disproportionately to stronger movements."},
	{"Wenn angekreuzt, erhalten alle Joystickachsen dieselbe Empfindlichkeit und Totzone.\nWenn nicht, k�nnen Empfindlichkeit und Totzone individuell fuer jede Joystickachse eingestellt werden.", 
	 "If checked, all joystick axes have the same sensitivity and deadzone.\nIf not, you can adjust sensitivity and deadzone individually for each joystick axis."},
	{"Staerke der Richtungsaenderungen\ndes Schiffes bei Joystick-\nbewegungen einstellen.", 
	 "Adjust strength of changes of movement direction during joystick\nmovements."},
	{"Groesse der Totzone des Joysticks einstellen.\nAchsenausschlaege innerhalb der Totzone haben keine Wirkung auf das Schiff.", 
	 "Adjust size of joystick deadzone. Axis movements inside the\ndeadzone do not affect ship movement."},
	{"Ankreuzen, wenn Sie einen normalen Joystick verwenden.", "Check if you are using a normal joystick."},
	{"Ankreuzen, wenn Sie einen Flightstick Pro verwenden.", "Check if you are using a Flightstick Pro."},
	{"Ankreuzen, wenn Sie ein FCS Flight Control System verwenden.", "Check if you are using an FCS Flight Control System."},
	{"Ankreuzen, wenn Sie eine Gravis Flugsteuerung verwenden.", "Check if you are using a Gravis flight controller."},
	{"Ankreuzen, um die Kurztasten zur Waffenwahl zu aktivieren.",
	 "Check to active the weapon selection hotkeys."},
	{"Ordnen Sie den einzelnen Waffen Kurzwahltasten zu, mit denen Sie\nsie direkt aktivieren koennen.", 
	 "Assign hotkeys to the weapons to directly select them."},
	{"Ordnen Sie den diversen Spielfunktionen Tasten Ihrer Tastatur zu.", 
	 "Assign keyboard keys to the various game functions."},
	{"Ankreuzen um die Tastaturbeschleunigung zu aktivieren.",
	 "Check to activate keyboard ramping."},
	{"Staerke der Tastaturbeschleunigung einstellen. Je staerker die Beschleunigung,\ndesto staerker die anfaengliche Reaktion des Schiffs\nauf Bewegungstasten. Anders ausgedrueckt: Hiermit kann die Feinsteuerung des Schiffs per Tastatur angepasst werden.", 
	 "Adjust the strength of keyboard acceleration. The stronger the\nacceleration, the stronger is the initial reaction of the ship to\nmovement keys. Put differently: Keyboard ramping affects fine\ncontrol of ship movements by the keyboard."},
	{"Feinkontrolle der Beschleunigungstasten anpassen.", "Adjust fine control of acceleration keys."},
	{"Feinkontrolle der Rotationstasten anpassen", "Adjust fine control of rotation keys."},
	{"Feinkontrolle der\nGleittasten anpassen.", "Adjust fine control of slide keys."},
	{"Ankreuzen, um die maximale vertikale Drehgeschwindigkeit (Nase hoch/runter)\nzu verdoppeln.", 
	 "Check to double pitch speed (nose up/down)."},
	//cockpit options
	{"Groesse der Cockpit-Informationsfenster einstellen.", 
	 "Chose a size of the cockpit window views."},
	{"Vergroesserungsfaktor fuer das 'Rueckspiegel'-Fenster auswaehlen.", 
	 "Chose an enlargement factor for the cockpit window views."},
	{"Ankreuzen, damit alle Cockpit-Informationen,\nwie z.B.\nFadenkreuz, gewaehlte Waffen, Schild- und Energiestatus\nangezeigt werden.", 
	 "Check to have all cockpit information like reticle, weapon\nselection, shield and energy status displayed."},
	{"Ankreuzen, wenn das Fadenkreuz eingeblendet werden soll.", "Check to have the reticle displayed."},
	{"Ankreuzen, wenn Schild-, Energie- und Nachbrennerstatus als Balken und\nnicht in Zahlen angezeigt werden sollen.", 
	 "Check to have shield, energy and afterburner status displayed\nas bars instead of numbers."},
	{"Ankreuzen, damit Schild-, Energie- und Nachbrenner-Skalen\nentsprechend\nder Bildaufloesung skaliert werden. Sie erscheinen dann\nunabhaengig von der Bildaufloesung immer gleich gross.\nAndernfalls erscheinen sie\nbei hoeheren Aufloesungen\nkleiner.", 
	 "Check to have the shield, energy and afterburner gauges scaled\nproportionally to screen resolution. That will make them appear\nthe same size regardless of screen resolution. Uncheck to make\nthem appear smaller at higher screen resolutions."},
	{"Ankreuzen, wenn Schild- und Energiebalken bei niedrigen Werten\nblinken sollen (langsam wenn unter 20, schnell, wenn unter 10).", 
	 "Check to have the shield and energy gauges flash at low values\n(slow flash if below 20, fast flash if below 10)."},
	{"Ankreuzen, wenn bei schwachen Schild ein Warnton ertoenen soll.", 
	 "Check to have a warning sound played to acustically indicate\nlow shields."},
	{"Ankreuzen, um Sicht abgefeuerter Raketen in einem Cockpit-Infofenster anzeigen", 
	 "Check to show view of missiles fired by you in a cockpit window."},
	{"Ankreuzen, um die Sicht von Lenkraketen auf den ganzen Bildschirm\nzu bringen.", 
	 "Check to have the sight of guided missiles displayed on the entire\nscreen."},
	{"Ankreuzen, um Powerups auf dem Radar sehen zu koennen.", 
	 "Check to have powerups displayed on the radar."},
	{"Ankreuzen, um Roboter auf dem Radar sehen zu koennen. Roboter sind pink.\nBoss-Roboter sind besonders grosse Punkte. Der Guidebot erscheint als blauer Punkt.", 
	 "Check to have robots displayed on the radar. Robots are shown pink.\nBoss robots are bigger blips. The Guidebot is shown as blue blip."},
	{"Wenn angekreuzt, werden Symbole fuer die vorhandenen Waffen auf dem Bildschirm angezeigt.\nVerfuegbare Waffen und Gegenstaende\nwerden gruen markiert, nicht verfuegbare grau. Die selektierte\nWaffe wird gelb markiert. Leergeschossene Waffen werden rot angezeigt.", 
	 "If checked, icons for the available weapons are displayed on the\nscreen. Available weapons and equipment are marked green,\nunavailable ones gray. The selected weapons are marked yellow.\nEmpty guns are marked red."},
	{"Wenn angekreuzt, werden Symbole fuer die vorhandenen Ausruestungs-\ngegenstaende auf dem Bildschirm angezeigt.", 
	 "If checked, icons for the available equipment are displayed on\nthe screen."},
	{"Wenn angekreuzt, werden kleinere Waffen- und Ausruestungssymbole verwendet.", 
	 "If checked, smaller weapon and equipment icons are used."},
	{"Ankreuzen, wenn die Waffensymbole entsprechend der gewaehlten Rangfolge der Waffen\ngeordnet werden sollen.", 
	 "Check to have the weapon icons sorted according to the chosen\npriorities of the weapons."},
	{"Wenn angekreuzt, wird die verbleibende Energie/Munition in die\nWaffensymbole eingeblendet. Bei Gauss- und Vulkankanone ist\ndie Anzeige in 1000.", 
	 "If checked, the remaining energy/ammunition is blended over the\nweapon icons. For Gauss and Vulcan gun the value displayed\ndenotes 1000s."},
	{"Position der Waffen- und Ausruestungssymbole auswaehlen.\n\nHinweis: Dieser Funktion kann eine Taste zugeordnet und\nmit dieser Taste waehrend des Spiels durch die moeglichen\nPositionen geschaltet werden, ohne dieses Menue aufzurufen.", 
	 "Chose the position of weapon and equipment icons.\n\nHint: You can assign a key to this function and toggle through\nthe possible positions using that key in-game without invoking\nthis menu."},
	{"Hier kann eingestellt werden, wie stark die farbige Unterlegung\nder Waffen- und Ausruestungssymbole wirkt.", 
	 "Adjust the colorization/graying of weapon and equipment icons."},
	//miscellaneous options
	{"Wenn angekreuzt, wird die Unterseite des Schiffs immer zur\nnaechstgelegenen Flaeche ausgerichtet, um die Orientierung einfacher zu machen.", 
	 "If checked, the ship's bottom will be aligned to the nearest\nwall to make orientation easier."},
	{"Wenn angekreuzt, pendeln die Schiffe sachte auf und ab, wenn\nkein Schub gegeben wird.", 
	 "If checked, the player ships will gently move up and down ('bob')\nwhen they're idling."},
	{"Ankreuzen, um den Scheinwerfer einschalten, wenn er aufgenommen wird.\n\nHinweis: Es ist besser, ihn ausgeschaltet zu lassen,\ndenn er verbraucht sehr viel Energie.", 
	 "Check to have the headlight turned on when you pick it up.\n\nHint: This should be turned off, as the headlight consumes a lot\nof energy."},
	{"Ankreuzen, um den Guidebot aus dem Spiel heraus mit\nSchnelltasten instruieren zu koennen.", 
	 "Check to allow for giving instructions to the guidebot in-game\nusing hotkeys."},
	{"Ankreuzen, wenn das Schiff nach seiner Zerstoerung sofort\nwieder entstehen soll, ohne dass nach der Explosionssequenz\nauf einen Tastendruck gewartet wird.",
	 "Check to make the ship reappear immediately after its\ndestruction without waiting for a keypress after the explosion sequence."},
	{"Ankreuzen, um vordefinierte Nachrichtentexte fuer\nMultiplayerspiele editieren und versenden zu koennen.", 
	 "Check if you want to be able to edit and send multiplayer\nmessage macros."},
	{"Wenn angekreuzt, bemerkt D2X-XL Datenverluste und fordert nicht\nerhaltene Datenpakete erneut an. Datenpakete, die aelter als 3\nSekunden sind, werden verworfen; genauso Daten, die der Absender\nnicht mehr gespeichert hat.\n\nDiese Option ist im Multiplayer-Zusatzmenue noch einmal vorhanden.", 
	 "If checked, D2X-XL will determine packet loss in multiplayer and\nrequest lost packets to be re-sent. Lost packets older than 3\nseconds are dropped as well as packets that are not stored by\ntheir sender any more.\n\nFor convenience, this option is duplicated in the 'more multiplayer\noptions' menu."},
	{"Wenn angekreuzt, werden Zeichen, die man in Dateiauswahllisten\neingibt, aneinandergehaengt, und es wird der erste Dateiname\nausgewaehlt, der mit der Zeichenkette beginnt. Andernfalls wird immer der\nDateiname ausgewaehlt, der mit dem zuletzt getippten Zeichen\nbeginnt. Durch Eingabe von Backspace kann man den Suchtext sukzessive\nwieder verkuerzen.", 
	 "If checked, characters you type in file menus will be stringed\ntogether and matched with each filename's beginning. Otherwise,\nthe first item in the list beginning with the last key pressed\nwill be selected. By entering backspace, the search pattern can\nbe shortened character by character."},
	{"Wenn angekreuzt, zeigt D2X-XL die Levelversion (Descent 1/2)\nin eckigen Klammern vor Levelnamen in Dateiauswahllisten und der Liste der Netzwerkspiele an.", 
	 "If checked, D2X-XL will display the level version (Descent 1/2) in\nsquare brackets in front of level names in file selector dialogs\nand the multiplayer game browser."},
	{"Wenn angekreuzt, zeigt D2X-XL in vielen Menues zusaetzliche\nOptionen an, mit denen man fast jeden Aspekt des Spiels feintunen\nkann. Diese Optionen setzen ein gruendliches Verstaendnis der\nArbeitsweise des Programms voraus und sollten nur geandert werden,\nwenn mit den Standardeinstellungen keine zufriedenstellende\nSpielerfahrung zustande kommt.", 
	 "If checked, D2X-XL will display additional options in many menus,\nallowing to tweak almost every aspect of the program's behaviour.\nThese options require a thorough understanding of the game's\nworkings and should only be changed if the default values do not\nyield a satisfactory game experience."},
	{"Ankreuzen, um den automatischen Download veralteter oder\nnicht vorhandener Multiplayer-Level zu erlauben. Der Game Host\n(Server) sendet dann seine Version der Leveldateien.", 
	 "Check to allow automatic download of outdated or unknown\nmultiplayer levels. The game host (server) will then send its\nversion of the level files."},
	{"Zeitspanne setzen, nach der ein Level-Up/Download abgebrochen\nwird, wenn der Sender bzw. Empfaenger nicht antwortet.", 
	 "Set timespan after which a level up/download is aborted if the\nsender/receiver do not respond."},
	{"Wenn angekreuzt, wird ein Schieber eingeblendet, mit dem\ndie Geschwindigkeit des Rechners angegeben werden kann. D2X-XL passt\ndann diverse Einstellungen entsprechend an, die die Leistung\ndes Programms beeinflussen. Diese Einstellungen werden sofort wirksam.", 
	 "If checked, D2X-XL will display a slider allowing to select the\ncomputer's speed and set various options that affect performance\naccording to the speed setting the user choses. These settings\nwill be applied immediately and override any settings made before."},
	{"Hier kann die Geschwindigkeit des Computers angegeben werden,\num D2X-XL diverse die Leistung des Programms beeinflussende Einstellungen\npassend waehlen zu lassen.", 
	 "Select your computer's speed here to have D2X-XL set various\noptions that affect performance accordingly."},
	//start/join multiplayer game
	{"IPX ist ein Netzwerkprotokoll fuer LANs und ist inzwischen\nueberholt. Server und Teilnehmer versenden ihre Nachrichten\nim gesamten LAN, an das sie angeschlossen sind, sozusagen in\nder Hoffnung, einander zu finden. Das ist eine einfache und direkte\nMethode, Server und Teilnehmer in einem LAN zu verbinden.\nDie Verwendung des IPX-Protokolls im Internet ist nur mithilfe\nspezieller Software wie KALI moeglich.", 
	 "IPX is a LAN-only network protocol and is rather obsolete\nnowadays. Game host and clients will 'broadcast' their messages\nthrough the entire LAN they're connected toin the hope to\nfind each other. This is a simple and straightforward way to\nconnect host and clients in a LAN. Playing IPX games over the\ninternet nis only possible with the help of some specialized\nsoftware like KALI."},
	{"UDP/IP ist ein Netzwerkprotokoll aehnlich TCP/IP. Es ist jedoch\n'verbindungslos', d.h. der Absender wartet nicht darauf, dass der\nEmpfaenger den Erhalt der Daten bestaetigt und sendet sie\nbei Uebertragungsfehlern nicht noch einmal. Dies ist das ideale\nProtokoll fuer Internetspiele, wo die meisten Daten Spielzustaende sind,\ndie sich sowieso staending aendern und staendig erneuert werden.\n\nBei diesem Protokoll muessen die Teilnehmer die Portnummer\ndes Servers angeben. Sie werden in einem gesonderten\nDialog danach gefragt. Der Standardport des Servers is 28342. Der\nServer erhaelt die Portnummern der Teilnehmer automatisch.", 
	 "UDP/IP is a network protocol similar to TCP/IP. It is however\n'connectionless', i.e. a sender does not wait for the addressee to\nacknowledge receipt of its data and does not retransmitdata in\ncase of an error. This is the ideal protocol for internet games,\nwhere most of the data consists of game states which are rather\nvolatile by their very nature and are constantly updated.\n\nThis protocol requires the clients to specify the port the server\nis using. They will be asked for it in a separate dialog. The\ndefault server port is 28342. The server will be automatically\ntold the clients' ports by the clients."},
	{"UDP/IP per Tracker ist die bequemere Alternativ zu normalen\nUDP/IP-Verbindungen. Wird diese Option ausgewaehlt,\nregistriert sich der Server bei allen Trackern, die ihm\nbekannt sind.  Ein Tracker ist einfach ein Computer, der\nirgendwo steht auf solche Registrations- Anforderungen wartet\nund IP-Adresse und Port des Servers in einer Tabelle vermerkt.\nSpielteilnehmer kontaktieren alle ihnen bekannten Tracker,\nerfragen die IP-Adressen und Ports der dort registrierten\nServer, und erhalten sie so automatisch. Die von den Servern\nangebotenen Spiele werden dann in der Spieleliste angezeigt.\nDas bedeutet, dass man sich nicht mehr vorab auf IP-Adressen\nund Ports einigen muss und macht es sehr einfach, Multiplayer-\nSpiele anzubieten und daran teilzunehmen.  D2X-XL hat schon\nein paar Tracker-Adressen eingebaut. Man kann weitere Tracker\nin d2x.ini angeben. Siehe das Online-Handbuch auf www.descent2.de.", 
	 "UDP/IP via tracker is the more comfortable alternative to\nstandard UDP/IP connections. If this option is chosen, the\ngame host will automatically register with all trackers made\nknown to it.  A tracker is a computer sitting somewhere,\nwaiting for such registration requests, and store the servers'\nIP address and port number in a table. Clients will contact\nall trackers known to them, asking them for any servers that\nhave registered with them and thus automatically find out\nabout available servers and their IP addresses and ports, and\ndisplay the games they offer in the game browser. This means\nyou don't need to communicate IP addresses and port numbers\nahead of playing and makes setting up and joining multiplayer\ngames much more comfortable.  D2X-XL already contains a few\nbuilt-in tracker addresses. You can specify more trackers in\nd2x.ini. Please see the online manual on www.descent2.de\nfor details."},
	{"Multicast UDP/IP funktioniert wie ein IPX-Spiel, bei dem statt\ndes IPX-Protokolls UDP/IP verwendet wird. Alle Daten werden im\ngesamten LAN verschickt. Es ist keine explizite Kenntnis der\nIP-Adressen und Ports der Teilnehmer erforderlich. Dieses\nProtokoll ist deshalb gut fuer Multiplayerspiele in einem LAN\ngeeignet.", 
	 "Multicast UDP/IP works basically like an IPX game, replacing\nthe IPX protocol with UDP/IP. All data is broadcast through\nthe entire LAN. No explicit knowledge of the participants'\nIP addresses and ports is required. This protocol is therefore\nwell suited for multiplayer games in a LAN."},
	{"KALI-Spiele rufen das externe Programm KALI auf, um Verbindungen\nim Internet mit dem IPX-Protokoll herzustellen. KALI ist ein Protokoll-\nKonverter, der alle IPX-Aufrufe in ein fuer das Internet\ngeeignetes Protokoll wie TCP/IP oder UDP/IP uebersetzt, bevor\ner sie verschickt. Die KALI-Instanz auf der Empfaengerseite\nuebersetzt die Daten zurueck nach IPX.", 
	 "KALI games call the external program KALI to connect over the\ninternet using the IPX protocol. KALI is a protocol converter,\ntranslating all IPX calls to a protocol more suited for the\ninternet, like TCP/IP or UDP/IP, before transmitting them.\nThe KALI instance on the receiver's side will retranslate the\ndata to IPX."},
	{"Modem-Spiele koennen nur 1 gegen 1 durchgefuehrt werden. Hier\nwird die Netzwerkverbindung ueber ein Modem aufgebaut.",
	 "Modem games can only be played 1 on 1. The network connection\nis established using a modem here."},
	{"Waehlen Sie diese Option, wenn Sie der Server in einem\nMultiplayerspiel sein wollen. Dies ermoeglicht Ihnen, den\nLevel auszuwaehlen, der gespielt werden soll, die Teilnehmer\nzu bestimmen, und alle Multiplayer-Einstellungen nach Ihren\nWuenschen vorzunehmen.", 
	 "Select this option if you want to host a multiplayer game as\nthe server. This will allow you to chose the level to be played\nas well as the participating players and adjust all multiplayer\ngame settings to your likings."},
	{"Waehlen Sie diese Option, wenn Sie an Multiplayerspielen\nteilnehmen wollen, die von anderen, als Server fungierenden\nSpielern angeboten werden.", 
	 "Select this option if you want to join a multiplayer game\noffered by other players, who took the role of the game server."},
	//render options
	{"Anpassen der Helligkeit.", "Adjust the brightness here."},
	{"Bildrate begrenzen (d.h. wie viele Bilder pro Sekunde D2X-XL\ndarstellt). Wird der Schieber nach ganz links gebracht, wird\ndie Limitierung ausser Kraft gesetzt. Auf schnellen Rechnern\nkann das allerdings dazu fuehren, dass sich das Schiff nicht\nmehr steuern laesst.", 
	 "Limit the framerate (i.e. how many images per second D2X-XL\nrenders). Moving the slider to the far left will disable\nframerate limitation. On fast computers this can however lead\nto the ship becoming impossible to maneuver and freezing."},
	{"Ankreuzen, um Lightmaps zu aktivieren. Lightmaps sind vor-\nberechnete Farbinformation fuer die Lichtquellen in einem\nLevel. Mit Lightmaps koennen also auch aeltere Level in\nfarbigem Licht erstrahlen. Dynamische Lichteffekte, wie sie\nvon Leuchtraketen und zerschossenen Lichtern erzeugt werden,\nfunktionieren damit allerdings u.U. nicht mehr.\n\nWird diese Option aktiviert, kann es zu einer laengeren Pause\nkommen, waehrend derer die Lightmaps berechnet werden.",
	 "Check to enable lightmaps. Lightmaps are precomputed color\ninformation for a level's light sources. Hence you can add\ncolored lighting to legacy levels. Dynamic light effects like\nthose created by flares oder by destroying lights may however\nnot work properly with lightmaps.\n\nIf this feature is turned on, it may come to a longer pause\nwhile D2X-XL is computing the lightmaps."},
	{"Ankreuzen, um farbige Beleuchtung in Leveln einzuschalten, die\ndiese enthalten (nur spezielle D2X-XL-Level). Nicht ankreuzen,\nwenn solche Level nicht farbig beleuchtet werden sollen.", 
	 "Check to enable colored light in levels prepared for it (only\nspecial D2X-XL levels). Uncheck if you don't want colored\nlighting in such levels."},
	{"Ankreuzen, wenn Schuesse und Raketen ihre Umgebung farbig\nausleuchten sollen", 
	 "Check to have weapon fire and missiles cast colored light\non the environment."},
	{"Ankreuzen, wenn bei stark beschaedigten Schiffen und Robotern\nExplosionseffekte erscheinen sollen. Der Effekt beginnt, wenn\ndie Schilde eines Robots oder Schiffes unter 50% des Ausgangs-\nwertes sinken, und nimmt bei weiterer Beschaedigung zu.\n\nDiese Option kann die Leistung des Programms verringern.", 
	 "Check if you want to have explosions appear on badly damaged\nships and robots. The effect will start if an robot's or\nship's shields are down to 50% of the initial value, and will\nincrease the more additional damage the object takes.\n\nThis option may cause a performance hit."},
	{"Ankreuzen, wenn aus den Antriebsduesen der Schiffe Auspuff-\nflammen kommen sollen.", 
	 "Check to have flames come out of a ship's thrusters."},
	{"Ankreuzen, wenn die Schiffe von einer transparenten Schildkugel\numgeben sein sollen (blau: normal, weiss: unverwundbar,\norange: getroffen)", 
	 "Check if a transparent shield sphere is to be rendered around\nthe ships (blue: normal, white: invulnerable, orange: hit)."},
	{"Ankreuzen, wenn den Filmen zum Spiel Untertitel unterlegt\nwerden sollen", 
	 "Check to have subtitles displayed when playing the game movies."},
	{"Ankreuzen, wenn die Filme zum Spiel zur Qualitaetsverbesserung\ngefiltert werden sollen.\nDies kann zu ruckelnder oder ganz fehlender Wiedergabe fuehren.", 
	 "Check to have additional filtering applied to the game movies,\nincreasing their quality.\n\nThis may cause choppy movie playback or inhibit movie playback\naltogether."},
	{"Ankreuzen wenn die Filme zum Spiel im Vollbildmodus wieder-\ngegeben werden sollen.", 
	 "Check if movies are to be played back in fullscreen mode."},
	{"Menue mit zusaetzlichen Darstellungsoptionen aufrufen.", 
	 "Invoke a menu offering additional render options."},
	{"Kontrast anpassen. Hoeherer Kontrast laesst helle Stellen\nheller und dunkle Stellen dunkler erscheinen. Niedrigerer\nKontrast hellt dunkle Stellen auf und dunkelt helle Stellen\nab und fuehrt generell zu einer Nivellierung der Beleuchtung.", 
	 "Adjust the contrast here. Higher contrast will make bright\nareas brighter and dark areas darker. Lower contrast will\nbrighten dark and darken bright areas, and generally level\nlighting differences."},
	{"Hier kann die Darstellungsqualitaet ausgewaehlt werden. Sie\nreicht vom pixeligen Retro-Look bis zu voll gefiltertem Anti-\naliasing mit Mipmaps.", 
	 "Chose a render quality here. That reaches from pixelated retro\nlook to fully filtered, anti-aliased and mipmapped."},
	{"Texturqualitaet auswaehlen. Diese Option beeinflusst die Dar-\nstellungsqualitaet und den Speicherbedarf hochaufloesender\nTexturen. Um Speicher zu sparen, kann Qualitaet geopfert\nwerden. D2X-XL skaliert hochaufloesende Texturen je nach\nEinstellung um den Faktor 1/2, 1/4, 1/8 oder gar nicht herunter.\n\nDiese Einstellung kann nicht aus dem Spiel\nheraus vorgenommen werden, da die Texturen dann schon\ngeladen sind.", 
	 "Chose a texture quality here. This option affects the render\nquality and memory requirements of hires textures. In order\nto save memory, you can sacrifice texture quality. Depending\non your choice, D2X-XL will scale down hires textures by a\nfactor of 1/2, 1/4 or 1/8, or not all.\n\nThis setting cannot be adjusted from in-game, as the textures\n are already loaded into memory then."},
	{"Transparenz durchsichtiger Waende (nur D2X-XL-Level)\neinstellen.", 
	 "Adjust the transparency of walls marked as transparent\n(only D2X-XL levels)."},
	{"Reichweite von Lightmaps einstellen. Erhoehung der Reichweite\nfuehrt zur Aufhellung des Levels.\n\nJede hier gemachte\nAenderung veranlasst D2X-XL, die Lightmaps neu zu berechnen,\nwas eine Weile dauern kann.\n\nDiese Option ist nur verfuegbar, wenn Lightmaps eingeschaltet sind.", 
	 "Adjust the range of lightmaps here. Increasing the range will\nbrighten the level.\n\nAny change made here will lead to\nD2X-XL recompute the lightmaps, which can take a while.\n\nThis options is only available if lightmaps are enabled."},
	{"Ankreuzen, damit farbiges Licht von Schuessen mit dem Umgebungs-\nlicht gemischt wird. Das erzeugt natuerlich aussehende Licht-\neffekte, macht das Licht von Waffen aber weniger auffaellig.\nWenn nicht angekreuzt, ersetzt Licht von Waffen das Umgebungs-\nlicht voellig, was sehr starke Lichteffekte ergibt.",
	 "Check to have colored weapon light mixed with the ambient light.\nThis will lead to a very natural looking light effect, but may\nmake weapon light stand out less. If unchecked, weapon light\nwill completely replace ambient light, which will lead to\nstrong weapon light effects."},
	{"Wenn angekreuzt, werden transparente Waende, denen eine Farbe\ngegeben wurde (nur D2X-XL-Level) in dieser Farbe gezeichnet.\nAndernfalls werden sie je nach Transparenz in einem hellen\noder dunkleren Grauton dargestellt.", 
	 "If checked, transparent walls that have a color assigned to them\n(only D2X-XL levels) are rendered with this color. Otherwise,\nthey are rendered in some shade of gray, depending on\ntheir transparency."},
	{"Wenn angekreuzt, werden Explosionen sowie einige Projektile\nund Powerups transparent gezeichnet.", 
	 "If checked, explosions and some weapon projectiles and\npowerups are rendered transparent."},
	{"Wenn angekreuzt, sorgt D2X-XL dafuer, dass mehrere Ebenen von\nTransparenz korrekt gezeichnet werden. Wenn nicht\nangekreuzt, kann es vorkommen, dass transparente Waende, die\nhinter anderen transparenten Waenden liegen, schwarz dar-\ngestellt werden.", 
	 "If checked, D2X-XL will make sure that several layers of trans-\nparency will still be rendered correctly. If unchecked,\ntransparent walls that lie behind other transparent walls\nmay be rendered black."},
	{"Wenn angekreuzt, wird die Helligkeit mittels des globalen\nGammawerts des Videotreibers eingestellt.", 
	 "If checked, brightness is adjusted using the graphics driver's\nglobal gamma setting."},
	{"Wenn angekreuzt, zeichnet D2X-XL die Schatten von Robotern und Schiffen.", 
	 "If checked, D2X-XL will render robot and ship shadows."},
	{"Anzahl der naechstgelegenen Lichtquellen einstellen, die D2X-XL\nbeim Zeichnen der Schatten eines Objects beruecksichtigt.", 
	 "Adjust the number of light sources close by D2X-XL will take into\naccount if rendering an object's shadow."},
	{"Ankreuzen, wenn Kameras in Leveln aktiviert werden sollen, die\nwelche enthalten (nur D2X-XL-Level)", 
	 "Check if you want to enable cameras in levels containing some\n(only D2X-XL levels)"},
	{"Ankreuzen, wenn die Ausgabe einer Kamera ueber die ganze\nFlaeche gedehnt werden soll, auf die sie projeziert wird.", 
	 "Check if you want to have a camera output extended over the\nentire plane it is projected to."},
	{"Intervall einstellen, innerhalb dessen die Kameraausgabe\naktualisiert wird. Wird der Schieber ganz nach links gebracht, wird\ndas Kamerabild jeden Frame erneuert (falls es fuer den Spieler\nsichtbar ist).", 
	 "Adjust the interval in which a camera image is updated. Moving\nthe slider to the far left means the camera image is rendered\n every frame (provided its output is visible by the player)."},
	{"Geschwindigkeit rotierender Kameras einstellen. Rotierende\nKameras ueberstreichen einen Winkel von 90 Grad in der Anzahl\neingestellter Sekunden.", 
	 "Adjust the movement speed of rotating cameras here. Rotating\ncameras cover an angle of 90 degrees in the amount of seconds\nspecified."},
	{"Ankreuzen um die Erzeugung von Raucheffekten einzuschalten.", 
	 "Check to enable creation of smoke effects."},
	{"Ankreuzen um die Erzeugung von Raucheffekten fuer Schiffsduesen\nund beschaedigte Schiffe einzuschalten.", 
	 "Check to enable creation of smoke effects for ship thrusters and\ndamaged ships."},
	{"Ankreuzen um die Erzeugung von Raucheffekten fuer beschaedigte\nRoboter einzuschalten.", 
	 "Check to neable creation of smoke effects for damaged robots."},
	{"Ankreuzen um die Erzeugung von Rauchschleppen von Raketen\neinzuschalten.", 
	 "Check to enable creation of smoke trails for missiles."},
	{"Ankreuzen, um die Erkennung von Kollisionen von Rauchpartikeln\nmit Waenden einzuschalten. Dies kann Raucheffekte realistischer\nerscheinen lassen, macht sich in der Praxis jedoch kaum\nbemerkbar und verursacht eine spuerbare Leistungseinbusse.", 
	 "Check to enable the detection of collisions of smoke particles\nwith walls. This may make smoke effects appear more realis-\ntically, but usually is hardly noticed and causes a tangible\nperformance hit."},
	{"Dichte von Raucheffekten einstellen. Je dichter der Rauch,\ndesto mehr Partikel enthaelt eine Rauchwolke. Hohe Dichte kann\nzu massiven Leistungseinbussen fuehren, besonders wenn viele\nRauchspuren sichtbar sind.", 
	 "Adjust the smoke density here. The higher the density, the\nmore particles a smoke trail can contain. High smoke density\ncan cause a massive performance hit, particularly if lots of\nsmoke trails are visible."},
	{"Rauchpartikelgroesse einstellen. Groessere Partikel lassen\nRaucheffekte i.a. besser aussehen. Diese Option hat *keinen*\nwesentlichen Einfluss auf die Leistung.", 
	 "Adjust smoke particle size. Bigger particle size usually leads\nto better looking smoke effects. This option does *not*\ninfluence performance significantly."},
	{"Schwierigkeitsgrad einstellen. Je hoeher der Schwierigkeits-\ngrad, desto aggressiver and widerstandfaehiger sind Roboter\nund desto schwacher sind Schild-Powerups.", 
	 "Adjust the difficulty here. The higher the difficulty, the more\naggressive and tougher robots and the weaker shield powerups are."},
	{"Entstehungszeit einstellen. Die Entstehungszeit ist die Zeit-\nspanne, die zwischen der Zerstoerung eines Schiffes oder\neiner Rakete und seines/ihres Wiedereintritts ins Spiel verstreicht.", 
	 "Adjust the respawn delay here. The respawn delay determines\nthe time that has to elapse until a killed player or destroyed\nor missile will respawn."},
	{"Staerke der Beschleunigung in Beschleunigungsbereichen ('Wind-\ntunneln') einstellen.", 
	 "Adjust the acceleration in speed boost areas ('wind tunnels')."},
	{"Staerke der Fusionskanone einstellen.", 
	 "Adjust the strength of the fusion cannon here."},
	{"Ankreuzen, wenn Powerups und Waffen immer dort wieder entstehen\nsollen, wo sie urspruenglich platziert waren.", 
	 "Check to have powerups and weapons always respawn where they\nhad been initially placed in the level."},
	{"Ankreuzen, um immer zwei Raketen auf einmal abzufeuern, wenn\nder Raketentyp das erlaubt. Um die erhoehte Schadenswirkung\nauszugleichen, wird hierbei die Feuergeschwindigkeit halbiert.", 
	 "Check to launch two missiles at once, if the missile type\nallows it. To compensate for the increased damage, dual\nmissile firing speed is halved."},
	{"Ankreuzen, damit ein zerstoertes Schiff alle Waffen und Muni-\ntion hinterlaesst, die es dabei hatte. Wenn nicht angekreuzt,\nbleibt nur ein zufaellig bestimmter Teil seiner Waffen und\nMunition zurueck.", 
	 "Check to have a destroyed ship drop all the weapons and ammo it\ncarries. If unchecked, only a randomly determined, reduced\namount of weapons and ammo is dropped."},
	{"Ankreuzen, damit verbrauchte Waffen und Powerups in Einzelspie-\nlermissionen wieder entstehen.", 
	 "Check to have used up powerups and weapons respawn in single\nplayer missions."},
	{"Ankreuzen, damit Quad- und Superlaser abgeworfen werden koennen\n(mit Shift+F5). Wenn nicht angekreuzt, geht das nicht.", 
	 "Check to allow dropping of quad and super lasers (by pressing\nShift+F5). If unchecked, you cannot drop these."},
	{"Ankreuzen, damit Tarn- und Unverwundbarkeitsvorrichtungen nicht\nsofort aktiviert, sondern gesammelt werden und per Tastendruck\naktiviert werden koennen.",
	 "Check to avoid activating cloak and invulnerability powerups\nimmediately, but instead be able to collect and activate them\nwith a keypress."},
	{"Ankreuzen, damit von Robotern abgefeuerte Schuesse, die\nzufaellig andere Roboter treffen, diese beschaedigen.", 
	 "Check to allow fire from robots that accidentally hits other\nrobots to damage them."},
	{"Ankreuzen, damit ein Level erst zerstoert wird, wenn alle darin\nenthaltenen Boss-Roboter und Reaktoren zerstoert worden sind.\nDiese Einstellung wird fuer D2X-XL-Level benoetigt, die ent-\nsprechend gemacht worden sind und mehrere Bosse enthalten.", 
	 "Check to have a level only destroyed after all boss robots and\nreactors contained in it have been destroyed. This option\nsupports D2X-XL levels built using this feature and containing\nseveral boss robots."},
	{"Ankreuzen, damit (Pseudo-) Fluessigkeitenphysik auf das Schiff\nangewandt wird, wenn es Wasser- oder Lavabereiche durchquert\n(nur D2X-XL-Level): Das Schiff vibriert dann und wird von\nLava beschaedigt.", 
	 "Check to have (fake) fluid physics applied to the ship when it\ntraverses water or lava areas (only D2X-XL levels): The ship\nwill shake in such places and be damaged by lava."},
	{"Ankreuzen, wenn Vulkan- und Streufeuerkanone ignoriert werden\nsollen, wenn Gauss- und Helixkanone verfuegbar sind.", 
	 "Check if Vulcan and Spreadfire cannons shall be ignored when\nGauss and Helix cannon are available."},
	{"Niemals selbsttaetig die Waffe wechseln.", 
	 "Never automatically switch weapons."},
	{"Waffe wechseln, wenn leergeschossen.", 
	 "Switch weapon if current weapon is empty."},
	{"Immer die Waffe mit der hoechsten Prioritaet aktivieren.", 
	 "Always switch to the weapon that has the highest priority."},
	{"Zielfernrohr abschalten.", 
	 "Disable weapon zoom."},
	{"Zielfernrohr hat mehrere Vergroesserungsstufen (nur Vulkan-\nund Gausskanone).", 
	 "Enable weapon zoom in several distinct stages (only for Vulcan\nand Gauss cannon)."},
	{"Stufenlose Vergroesserung durch Zielfernrohr einschalten (nur\nVulkan- und Gausskanone).", 
	 "Enable smooth zooming up to the maximum value (only for Vulcan\nand Gauss cannon)."},
	//options menu
	{"Konfigurationsmenue fuer Klangeffekte und Musik aufrufen.", 
	 "Invoke the sound and music configuration menu."},
	{"Konfigurationsmenue fuer Eingabegeraete aufrufen.", 
	 "Invoke the game controls configuration menu."},
	{"Joystick-Kalibrierung starten.", 
	 "Invoke the joystick calibration."},
	{"Konfiguration der Spieldetaillierung aufrufen.", 
	 "Invoke the detail levels menu."},
	{"Auswahlmenue der Bildaufloesung aufrufen.", 
	 "Invoke the screen resolution selection menu."},
	{"Menue aufrufen, in dem die Reihenfolge eingestellt werden kann,\nin der Waffen aktiviert werden, wenn sie neu eingesammelt\nwerden oder die Munition fuer die aktive Waffe ausgeht.", 
	 "Invoke the menu where you can set the sequence in which weapons\nare activated when you pick up a new one or run out of ammo\nfor your current weapon."},
	{"Menue aufrufen, in dem die Reihenfolge eingestellt werden kann,\nin der Raketen aktiviert werden, wenn sie neu eingesammelt\nwerden oder der momentan aktive Raketentyp ausgeht.", 
	 "Invoke the menu where you can set the sequence in which missiles\nare activated when you pick up a new one or run out of the\ncurrently selected one."},
	{"Menue aufrufen, in dem diverse Einstellungen vorgenommen werden\nkoennen.", 
	 "Invoke a menu where you can set miscellaneous options."},
	{"Konfigurationsmenue fuer die Pilotenkanzel aufrufen.", 
	 "Invoke the cockpit configuration menu."},
	{"Konfigurationsmenue fuer die grafische Darstellung aufrufen.", 
	 "Invoke the render options menu."},
	{"Konfigurationsmenue fuer die Spieleinstellungen aufrufen.", 
	 "Invoke the gameplay options menu."},
	//get ip address
	{"Hier IP-Adresse und optional ort des Servers eingeben.\n\nFormat: <IP>[:<Port>], z.B. 123.123.123.123:12345.", 
	 "Enter IP address and optionally the port of the game server here.\n\nFormat: <IP>[:<Port>], e.g. 123.123.123.123:12345."},
	{"Hier den UDP-Port eingeben, den Ihr Rechner verwendet, um\nDescent-Mehrspielerdaten zu senden und empfangen.\n\nBei Eingabe von 0 waehlt das Betriebssystem einen verfuegbaren\nPort aus (das ist also eine gute Idee ;).", 
	 "Enter the UDP port your machine is using to send and receive\nDescent multiplayer data.\n\nIf you enter 0, the operation system will chose an available\nport for you (so this would be a good idea ;)."},
	//multiplayer options
	{"Spielbeschreibung eingeben. Sie wird in der Spieleliste aller\nverbundenen Teilnehmer angezeigt.", 
	 "Add a descriptive text for your game here. It will be displayed\nin all connected client's game browser."},
	{"Missionsliste oeffnen, um die Mission auszuwaehlen, der gespielt\nwerden soll.", 
	 "Open the mission list to chose the mission to be played."},
	{"Wenn die gewaehlte Mission mehrere Level enthaelt, kann hier\nder erste zu spielende Level ausgewaehlt werden.", 
	 "If the chosen mission contains multiple levels, specify the\nlevel to start playing in here."},
	{"Spieltyp 'Anarchie' (Deathmatch) auswaehlen: Jeder Spieler auf\nsich gestellt, und der Spieler, der die meisten\nAbschuesse erzielt, gewinnt die Runde.", 
	 "Chose to play an 'anarchy' (deathmatch) game: Every player\nplays for himself and the player racking up the most kills\nwins the round."},
	{"Spieltyp Team-'Anarchy' (Deathmatch) auswaehlen: Die Spieler\ngehoeren einem von zwei Teams an, und das Team, das gemeinsam\ndie meisten Abschuesse erzielt, gewinnt die Runde.", 
	 "Chose to play a team 'anarchy' (deathmatch) game: Players join\none of two teams, and the team that collectively racks up\nthe most kills wins the round."},
	{"Spieltyp Roboter-'Anarchie' (Deathmatch) auswaehlen: Wie\nAnarchie, nur mit Robotern, falls der Level welche enthaelt.", 
	 "Chose to play a robot 'anarchy' (deathmatch) game: It works\nlike anarchy, just with robots present if the level contains some."},
	{"Spieltyp 'kooperativ' auswaehlen: Bis zu vier Spieler (je nach\nLevel) spielen gemeinsam gegen die Roboter.", 
	 "Chose to play a cooperative game: Up to four players (depending\non the level) play together against the robots."},
	{"Spieltyp 'Flagge erobern' auswaehlen: Die Spieler bilden zwei\nTeams. Jedes Team besitzt eine Flagge, die es verteidigen\nmuss, und muss die gegnerische Flagge erobern (aufnehmen) und\nsie in einen Zielbereich bringen, der mit einer pulsieren-\nden blauen oder roten Textur markiert ist. Das Team, das die\nmeisten Flaggen in sein Ziel bringt, gewinnt die Runde.", 
	 "Chose a capture the flag ('CTF') game: The players form two\nteams. Each team owns a flag it has to defend, and has to\ncapture (pick up) the enemy flag and fly it to some goal area\nmarked with a pulsating blue or red texture. The team that\nmanaged to bring the most flags to their goal wins the round."},
	{"Spieltyp CTF 'plus' auswaehlen: CTF plus funktioniert wie CTF, behebt aber dessen Maengel:\n\n- Jedes Team hat seine eigenen Entstehungspunkte nahe seines Heimatzone\n\nEine eroberte Flagge taucht in ihrer Heimatzone und nicht irgendwo im Level wieder auf\n- Ein Team kann seine Flagge zurueckbringen, indem es darueber\n  weg fliegt, wenn sie irgendwo im Level liegt\n- Um eine Flagge erobern zu koennen, muss die eigene Flagge\n  in ihrer Heimatzone sein\nFuer CTF plus sind speziell vorbereitete Level erforderlich (nur D2X-XL).", 
	 "Chose a CTF 'plus' game: CTF plus works like CTF, but fixes its\nflaws:\n\n- Each team has its own spawn locations close to its home area\n- A captured flag will reappear in its home area and not spawn\n  randomly in the level\n- You can return your flag by flying over it when it has been\n  dropped somewhere in the level\n- A team can only score if its flag is in its home area\n\nYou need to use a specially CTF plus enabled level (D2X-XL only)\nto play it."},
	{"Spieltyp 'Horten' auswaehlen: Wird ein Schiff zerstoert,\nverliert es eine gruene Kugel, sowie alle bereits gesammelten\nKugeln. Diese Kugeln koennen aufgesammelt und in ein Ziel-\ngebiet gebracht werden, das mit einer pulsierenden gruenen\nTextur markiert ist. Je mehr Kugeln ein Spieler einsammelt,\nbevor er sie ins Ziel bringt, desto mehr Punkte erhaelt er.\nDer Spass an der Sache liegt in dem Risiko, laenger auf\nKugeljagd zu gehen, bevor man die Punkte dafuer absahnt,\ndafuer aber auch die Gefahr zu erhoehen, selber abgeschossen\nzu werden und einem Gegner alle gesammelten Kugeln zu\nueberlassen.", 
	 "Chose a 'hoard' game: If a ship is destroyed, it drops a green\norb plus any orbs the player has collected. These orbs can\nbe picked up and brought to some goal area marked with a\npulsating green texture. The more orbs a player picks up\nbefore entering such a goal, the higher the score he receives.\nThe fun of this is the risk of staying around longer before\nscoring, thus gathering more orbs, but also increasing the\ndanger of being shot down and losing all orbs to an opponent."},
	{"Spieltyp Team-'Horten' auswaehlen: Team-Horten funktioniert\nwie Horten. Die Spieler bilden aber zwei Teams, und das Team\n das gemeinsam die meisten Punkte erzielt, gewinnt die Runde.", 
	 "Chose a team 'hoard' game: Team hoard works like hoard, but\nthe players form two teams, and the team that collectively\nachieves the highest score wins the round."},
	{"Spieltyp 'Entropie' auswaehlen: Entropie stammt eigentlich aus\nDescent 3. Die Regeln sind etwas ausgefeilter, aber wer sie\nerlernt, wird mit einer extrem spannenden und teamorientierten\nSpielerfahrung belohnt.\n\nDie Spieler bilden zwei Teams. Jedes Team besitzt einige\nbesondere Zonen ('Raeume') mit speziellen Funktionen im Level\n(Energiezentren, Reparaturzentren, Viruslabore). Das Team,\ndas alle gegnerischen Raeume erobert, gewinnt die Runde. Um\neinen Raum zu erobern, muss ein Spieler zuerst eine gewisse\nMenge an Viren aufnehmen. Um das zu koennen, muss er seine\nLadekapazitaet fuer Viren erhoehen, indem er gegnerische\nSpieler abschiesst. Dabei darf er nicht selber abgeschossen\nwerden, sonst faellt seine Ladekapazitaet auf Null zurueck.\nSobald der Spieler genuegend Viren geladen hat, kann er einen\ngegnerischen Raum betreten. Dort muss er fuer eine gewisse\nZeit bewegungslos ausharren, um ihn zu erobern. Das wird\ndadurch erschwert, dass ein Schiff bei Aufenthalt in einem\ngegnerischen Raum beschaedigt wird. Ueberlebt der Spieler\nlange genug, wechselt der Raum den Besitzer.", 
	 "Chose an 'entropy' game: This game mode was actually introduced\nwith Descent 3. The rules are a little more sophisticated, but\nlearning them will be rewarded with an extremely thrilling\nand team-oriented gaming experience.\n\nThe players form two teams. Each team owns a few distinct\nlevel areas ('rooms') with special functions (energy center,\nrepair center, virus lab). The team conquering all enemy rooms\nwins the round. To conquer a room, a player must first pick\nup a certain number of viruses. Before he can do that, he needs\nto increase his virus transport capacity, which he does by\nkilling enemy players. He must be careful though not to get\nkilled himself, or his transport capacity will drop to zero\nagain. Once the player has enough viruses on board, he can\nenter a hostile room, where he must not move for a certain\namount of time to conquer it. To make things harder, staying\nin enemy rooms afflicts damage to your ship. If he survives\nlong enough, the room changes ownership."},
	{"Diese Option auswaehlen um ein Spiel zu beginnen, an dem jeder\nteilnehmen kann, selbst wenn es schon laeuft.", 
	 "Chose this option to create a game everybody can join even if\nthe game is already in progress."},
	{"Diese Option auswaehlen um ein Spiel zu beginnen, bei dem keine\nSpieler einer laufenden Partie mehr beitreten koennen.", 
	 "Chose this option to create a game where no new players can\njoin a game in progress."},
	{"Diese Option auswaehlen um ein Spiel zu beginnen, bei dem\nSpieler, die einer laufenden Partie beitreten wollen, erst\nvom Server die Erlaubnis dazu erhalten muessen.",
	 "Chose this option to create a game where players trying to join\na game in progress must first receive permission by the game\nhost (server)."},
	{"Die maximale Anzahl Spielteilnehmer festlegen.", 
	 "Adjust the maximum number of game participants here."},
	{"Menue mit weiteren Mehrspieler-Einstellungen aufrufen.", 
	 "Invoke a menu offering more multiplayer game settings."},
	{"Menue der Entropie-Einstellungen aufrufen.", 
	 "Invoke the Entropy settings menu."},
	//more multiplayer options
	{"Zeitspanne setzen, innerhalb derer der Reaktor nicht zerstoert\nwerden kann, falls vorhanden.", 
	 "Set the time span during which the reactor is indestructible,\nif the level contains one."},
	{"Zeitspanne setzen, nach der der Reaktor sich selbst zerstoert\nund damit die Runde beendet, falls vorhanden.", 
	 "Set the time after which the reactor auto-destructs and ends\nthe round, if the level contains one."},
	{"Anzahl Abschuesse setzen, die ein Spieler erzielen muss, um\ndie Runde siegreich zu beenden.", 
	 "Set the number of kills a player has to achieve to win and\nend the round."},
	{"Zeitspanne setzen, innerhalb derer ein Spieler nach seinem\nErscheinen unverwundbar ist.", 
	 "Set the time span during which a player is invulnerable after\nrespawning."},
	{"Ankreuzen, damit die Spieler eine Kamerasicht von Ihnen abgewor-\nfener Bojen in einem Cockpitfenster sehen koennen.", 
	 "Check to allow players to see a camera view from markers they\ndrop in a HUD window."},
	{"Ankreuzen, damit die Lampen im Level nicht zerstoert werden\nkoennen.", 
	 "Check to prevent lights in the level being destroyed by gun\nfire or missiles."},
	{"Ankreuzen, damit die Schiffe hell beleuchtet und damit\nbesser sichtbar werden (empfohlen).\n\nHinweis: In anderen Descent 2-Versionen erscheinen Schiffe\nhell, wenn diese Option *nicht* angekreuzt wird. In D2X-XL\nist das korrigiert.", 
	 "Check to have player ships always appear brightly lit to\nincrease their visibility (recommended).\n\nHint: In other Descent 2 versions, you need to uncheck this\noption to make player ships appear bright. In D2X-XL this has\nbeen fixed."},
	{"Ankreuzen, damit die Namen der Spieler ueber ihren Schiffen\neingeblendet werden.", 
	 "Check to have the players' names displayed over their ships."},
	{"Ankreuzen, damit die Schiffe der Spieler als Punkte in der\nUebersichtskarte angezeigt werden.", 
	 "Check to have player ships appear as blips in the automap."},
	{"Ankreuzen, damit Schuesse und Raketen von Spielern die\nSchiffe anderer Spieler aus demselben Team beschaedigen koennen.\nNicht ankreuezn, wenn Schiffe nicht von Schuessen von\nTeamkollegen beschaedigt werden sollen.\n\nDiese Option ist in kooperativen Spielen recht praktisch. ;)", 
	 "Check to allow for gun fire or missiles from players to damage\nteam mates' ships. Uncheck, if player ships should not be\ndamaged by fire coming from team mates.\n\nThis option comes handy in cooperative games. ;)"},
	{"Ankreuzen, um versehentlichen oder beabsichtigten Selbstmord\nzu verhindern.\n\nDiese Option kann die Spieltaktik besonders in Teamspielen\nstark beeinflussen, wo ein stark beschaedigter Spieler viel-\nleicht bei geeigneter Gelegenheit Selbstmord begeht, um\nseine Ladung (Waffen, Flaggen, Hortkugeln) einem Teamkollegen\nund nicht dem gegnerischen Team zu ueberlassen", 
	 "Check to inhibit accidental or intentional suicides.\n\nThis option can greatly affect tactics particularly in team\ngames, where a badly damaged player might commit suicide in\nan appropriate location to allow a team mate to collect his\ngoodies (weapons, flags, hoard orbs) to avoid leaving them\nto the opposing team."},
	{"Ankreuzen, um 'Mouselook' zu aktivieren. Mouselook erhoeht\ndie Empfindlichkeit der Maus stark und macht das Schiff da-\ndurch manoevrierfaehiger. Der Nachteil ist, dass auch das\nZielen schwieriger wird.\n\nDeaktiviert der Server Mouselook, ist es fuer alle Teilnehmer\ndeaktiviert. Laesst er es zu, kann jeder Teilnehmer es indi-\nviduell abschalten. Wenn der Server nicht D2X-XL verwendet,\nwird Mouselook fuer alle Teilnehmer deaktiviert.", 
	 "Check to enable 'mouselook'. Mouselook greatly increases mouse\nsensitivity, making the shop more maneuverable. The backdraw\nof this is that aiming gets harder.\n\nIf the server disables mouselook, it is disabled for all par-\nticipants in the game. If he enables it, participants can\nstill individually turn it off. If the server does not use\nD2X-XL, mouselook is turned off for all participants."},
	{"Ankreuzen, um die vertikale Drehgeschwindigkeit (Nase auf/ab)\nzu verdoppeln. Standardmaessig ist sie nur halb so gross wie\ndie horizontale Drehgeschwindigkeit (Nase nach links/rechts).\n\nWie bei Mouselook kontrolliert der Server auch diese Einstel-\nlung fuer alle Teilnehmer, und vertikale Drehgeschwindigkeit\nbleibt beim Standard, wenn der Server nicht D2X-XL verwendet.", 
	 "Check to double vertical turn speed. By default, vertical turn\nspeed (nose up/down) is only half of the horizontal turn speed\n(nose left/right).\n\nLike with mouselook, the server controls the setting for the\ngame participants too, and vertical turn speed stays at the\ndefault value for all participants if the server doesn't use\nD2X-XL."},
	{"Ankreuzen, um automatischen Teamausgleich zu aktivieren. Wenn\nautomatischer Teamausgleich aktiv ist, weist D2X-XL neu bei-\ntretende Spieler automatisch dem Team mit weniger Punkten zu.\nHaben die Teams gleichviele Punkte, tritt der Spieler dem\nkleineren Team bei.\n\nDieses Ausgleichsschema soll dafuer sorgen, dass ein Spieler\neinem deutlich ueberlegenen Team zugeteilt wird, nur weil es\nkleiner ist, und der Verliererseite eine bessere Chance geben.", 
	 "Check to enable team auto balance. If team auto balance is\nenabled, D2X-XL will automatically assign newly joining\nplayers to the team with the lower score. If team scores are\nequal, the new player will join the smaller team.\n\nThis balancing scheme should avoid putting a player on a\nvastly superior team only because that team is smaller, giving\nthe losing team a better chance."},
	{"Ankreuzen, damit D2X-XL nach Beenden des letzten Levels wieder\nmit dem ersten Level der aktuellen Mission beginnt. Damit\nwird im Grunde eine Levelrotation hergestellt.\n\nUm eine persoenliche Levelrotation zu erstellen, kann man die\ngewuenschten Level mit DLE-XP (siehe www.descent2.de) in einer\nHog-Datei ablegen und diese im Missions-Ordner speichern.", 
	 "Check to have D2X-XL restart with the first level of the\ncurrent mission if the last one gets finished. Basically this\nis constitutes 'map rotation'.\n\nTo create a custom map rotation, you can put the desired\nlevels in a hog file using DLE-XP (see www.descent2.de) and\nplace it in the missions folder."},
	{"Ankreuzen, damit der Reaktor aus dem Level entfernt wird,\nfalls vorhanden. So kann man verhindern, dass irgendein Troll\nden Reaktor vorzeitig hochjagt. :P", 
	 "Check to have the reactor removed from the level if it contains\none. That way you can prevent some jerk from blowing it up\nprematurely. :P"},
	{"Ankreuzen, damit D2X-XL kleinere Datenpakete versendet. Der Nachteil ist verringerte Genauigkeit der uebertragenen Spielerpositionen.\n\nBei Verbindungen mit hoher Bandbreite oder LAN-Spielen sollte\ndiese Option deshalb nicht angekreuzt werden.", 
	 "Check to make D2X-XL use smaller network data packets. The\nbackdraw of this is reduced precision in the player position\nthat is reported to the other participants.\n\nFor high bandwidth connections and LAN games, this option\nshould therefore remain unchecked."},
	{"Einen alternativen Netzwerk-Port angeben. Wird ein Vorzeichen\nvorangestellt, wird der Wert zum Standardport addiert bzw.\ndavon subtrahiert.\n\nWird dieser Wert fuer UDP/IP-Verbindungen geaendert, sollten\ndie anderen Teilnehmer davon erfahren, oder sie erhalten\nkeine Verbindung mit dem Server. ;)", 
	 "Specify an alternative networking port here. If you prefix\nthe number with a sign, it is added to or subtracted from\nthe standard port.\n\nIf you change the socket for UDP/IP\ngames, make sure the other participants know about it, or they\nwill not be able to connect to your game. ;)"},
	{"Anzahl der Datenpakete vorgeben, die D2X-XL pro Sekunde versen-\nden soll. Je hoeher der Wert (max. 20), desto praeziser werden\ndie Statusaktualisierungen, die zwischen den Spielern ausge-\ntauscht werden, aber desto hoeher wird auch die Verbindungs-\nbelastung. 10 ist ein guter Wert fuer die meisten Verbindungen.", 
	 "Set the number of network data packets D2X-XL should transmit\nper second. The higher the number (max. 20), the more precise\nare status updates exchanged between the players, but the\nhigher the network load gets. 10 is a good value suiting most\nconnections."},
	{"Menue aufrufen, in dem angegeben werden kann, welche Waffen\nund Powerups zugelassen sind und im Level erscheinen falls\nvorhanden, und welche nicht.", 
	 "Invoke a menu where you can specify which weapons and powerups\nare allowed and will appear in the level if present, and\nwhich don't."},
	{"Bitte konsultieren Sie das Online-Handbuch auf www.descent2.de.", 
	 "Please consult the online manual on www.descent2.de."},
	{"Wenn angetickt, werden zwei Eingabefelder eingeblendet, in denen\neine beliebige Bildschirmaufloesung angegeben werden kann. Wenn\n die Grafikkarte die angegebene Aufloesung darstellen kann,\nwird sie angenommen,\nandernfalls erscheint eine Fehlermeldung.", 
	 "If checked, two additional input fields are displayed, where you\ncan enter an arbitrary screen resolution. If your graphics card\ncan display the specified resolution, it is accepted; otherwise\nan error message is displayed."},
	{"Wenn angekreuzt, wird ein gelber Ring angezeigt, der die Position\nder Maus relativ zur Bildschirmmitte anzeigt, um die Ver-\nwendung der Maus als Joystick-Simulation zu erleichtern.",
	 "If checked, a yellow circle showing the position of the mouse\nrelative to the screen center will be displayed to aid in using\nthe mouse in joystick simulation mode."},
	{"Wenn angekreuzt, zeigen Teleporter ein Live-Bild ihres Sprungziels.", 
	 "If checked, teleports show a live image of their jump destination."},
	{"Wenn angekreuzt, werden Spielerbotschaften in einem eigenen Bild-\nschirmbereich unterhalb der regularen Spielnachrichten angezeigt.",
	 "If checked, player messages will be shown in a separate screen\narea below the regular game messages."},
	{"Spieltyp 'Monsterball' auswaehlen: Monsterball stammt eigentlich aus\nDescent 3. Es geht darum, einen riesigen Ball mit Sch�ssen und\ndurch Rammen in das Ziel zu bugsieren.", 
	 "Chose a 'Monsterball' game: This game mode was actually introduced\nwith Descent 3. The rules are simple: Move a giant ball into the\ngoal by shooting and ramming it."},
	{"Menue der Monsterball-Einstellungen aufrufen.", 
	 "Invoke the Monsterball settings menu."},
	{"Hier kann man einstellen, wieviele Punkte ein Spieler und sein Team\nerhalten, wenn sie ein Tor erzielen.", 
	 "Here you can adjust how many points a player and his team receive\nfor scoring a goal."},
	{"Hier kann die Groesse des Monsterballs im Vergleich zur Schildkugel\neingestellt werden. Achtung: Ab einer Groesse von 4 passt\nder Ball nicht mehr in ein Standardsegment!", 
	 "Here you can set the size of the Monsterball compared to a shield\norb. Beware: A ball of size 4 or bigger does not fit in a\nstandard segment any more!"},
	{"Wenn angekreuzt, verwendet D2X-XL die OpenGL-Beleuchtungsverfahren.\n\nD2X-XL emuliert die OpenGL-Beleuchtung wegen der grossen Anzahl\nbenoetigter Lichter per Software. Dadurch kann es dadurch zu\nstarken Leistungseinbussen kommen.\n\nDie Leistung dieser Beleuchtungsmethode haengt nicht von der\nGrafikkarte, sondern ausschliesslich von der Geschwindigkeit\nvon CPU und Speicher ab.", 
	 "If checked, D2X-XL uses the OpenGL lighting methods.\n\nDue to the large number of lights in Descent, D2X-XL emulates\nOpenGL lighting by software. This can cause a strong\nperformance hit.\n\nThe speed of this lighting method does not depend on the graphics hardware, but exclusively on the CPU and memory speed."},
	{"Wenn angekreuzt, verwendet D2X-XL die OpenGL-Beleuchtungs-\nmethode auch fuer Hires-Objekte. Nur fuer sehr schnelle\nRechner empfohlen.", 
	 "If checked, D2X-XL uses the OpenGL lighting method for\nhires objects too. Only recommended for really fast computers."},
	{"Hier kann die Anzahl nahegelegener Lichter eingestellt werden,\ndie die Beleuchtung eines gegebenen Punktes in der Mine be-\neinflussen. D2X-XL verwendet die hier eingestellte Anzahl der\njedem Segment am naechsten gelegenen Lichter, um das Segment\nzu beleuchten.",
	 "Adjust the number of nearby lights that influence lighting\nfor a given point in the mine. D2X-XL will use the number of\nnearest lights set here to each segment of the mine to\nilluminate that segment."},
#if 0
	{"", ""},
	{"", ""},
	{"", ""},
	{"", ""},
#endif
	};

//------------------------------------------------------------------------------

inline int GameTextCount (void)
{
return sizeof (defaultGameTexts) / (2 * sizeof (char *)) - 1;	//last entry always empty!
}

//------------------------------------------------------------------------------

int GameTextSize (void)
{
	int	h, i, j = GameTextCount ();

for (h = i = 0; i < j; i++)
	h += (int) strlen (defaultGameTexts [i][gameStates.app.bEnglish]) + 2;
for (i = 0; i < N_BASE_TEXTS; i++)
	if (baseGameTexts [i])
		h += (int) strlen (baseGameTexts [i]) + 2;
	else
		h += 2;
return h;
}

//------------------------------------------------------------------------------

char **InitTexts (char *szTextFile, int bInitHotKeys)
{
	int	h, i, j, l, bHotKeys = 0;
	char	k;
	char	*pk, *pDest, *pSrc, **pszTexts;
#if DUMP_TEXTS == 1
	FILE	*fTxt = fopen (szTextFile);
#endif

j = N_BASE_TEXTS + GameTextCount ();
if (!(pszTexts = (char **) d_malloc ((j + 1) * sizeof (char *))))
	return NULL;
h = GameTextSize ();
if (!(*pszTexts = (char *) d_malloc (h))) {
	d_free (pszTexts);
	return NULL;
	}
for (i = 0; i < j; i++) {
	pSrc = (i < N_BASE_TEXTS) ? baseGameTexts [i] : defaultGameTexts [i - N_BASE_TEXTS][gameStates.app.bEnglish];
#if DUMP_TEXTS == 1
	{
		char *pi, *pj, s [200];

	Assert (sizeof (s) > strlen (ps));
	strcpy (s, ps);
	for (pi = pj = s; *pi; pi++) {
		if (*pi == '\n') {
			*pi = 0;
			fprintf (fTxt, "%s\\n", pj);
			pj = pi + 1;
			}
		else if (*pi == '\t') {
			*pi = 0;
			fprintf (fTxt, "%s\\t", pj);
			pj = pi + 1;
			}
/*
		else if (*pi == '%') {
			*pi = 0;
			fprintf (fTxt, "%s%c", pj, '%');
			pj = pi + 1;
			}
*/
		else if (*pi == '\\') {
			*pi = 0;
			fprintf (fTxt, "%s\\\\", pj);
			pj = pi + 1;
			}
		}
	fprintf (fTxt, "%s\n", pj);
	fflush (fTxt);
	}
#endif

	pDest = pszTexts [i] + 1;
	if (!pSrc) {
		l = 1;
		*pDest = 0;
		k = -1;
		}
	else {
		l = (int) strlen (pSrc) + 1;
		if (pk = strchr (pSrc, '~')) {
			bHotKeys = 1;
			--l;
			k = (char) (pk - pSrc);
			memcpy (pDest, pSrc, k);
			memcpy (pDest + k, ++pk, l - k);
			}
		else {
			memcpy (pDest, pSrc, l);
			k = -1;
			}
		}
	*(pDest - 1) = (k < 0) ? 0 : toupper (pDest [k]);
	pszTexts [i] = pDest;
	pszTexts [i + 1] = pDest + l;
	}
#if DUMP_TEXTS == 1
fclose (fTxt);
#endif
if (bInitHotKeys && !bHotKeys && !gameStates.app.bEnglish && (gameOpts->menus.nHotKeys > 0))
	gameOpts->menus.nHotKeys = -1;
return pszTexts;
}

//------------------------------------------------------------------------------

void InitGameTexts (void)
{
pszGameTexts = InitTexts (gameStates.app.bEnglish ? "descent.tex.eng" : "descent.tex.ger", 1);
}

//------------------------------------------------------------------------------

void InitHelpTexts (void)
{
pszHelpTexts = InitTexts (gameStates.app.bEnglish ? "descent.hlp.eng" : "descent.hlp.ger", 0);
}

//------------------------------------------------------------------------------
// rotates a byte left one bit, preserving the bit falling off the right
char EncodeRotateLeft (char c)
{
int b = (c & 0x80);
c <<= 1;
if (b)
	c |= 0x01;
return c;
}

//------------------------------------------------------------------------------
//decode and encoded line of text
void DecodeTextLine (char *p)
{
for (; *p; p++)
	*p = EncodeRotateLeft ((char) (EncodeRotateLeft (*p) ^ BITMAP_TBL_XOR));
}

//------------------------------------------------------------------------------

#ifndef _MSC_VER
#include <unistd.h>
#endif

//load all the text strings for Descent

void LoadGameTexts (void)
{
#if DUMP_TEXTS == 2
	FILE *fTxt = fopen (gameStates.app.bEnglish ? "descent.tex.e" : "descent.tex.g", "wt");
#elif DUMP_TEXTS == 3
	FILE *fTxt = fopen ("d:\\temp\\basetex.h", "wt");
#endif
	CFILE  *tfile;
	CFILE *ifile;
	int len, h, i, j, bBinary = 0;
	char *psz;
	char *filename = "descent.tex";

if ((i = FindArg ("-text")))
	filename = Args [i+1];
if (!(tfile = CFOpen (filename, gameFolders.szDataDir, "rt", 0))) {
	filename= "descent.txb";
	if (!(ifile = CFOpen (filename, gameFolders.szDataDir, "rb", 0))) {
		Warning (TXT_NO_TEXTFILES);
		return;
	}
	bBinary = 1;
	len = CFLength (ifile, 0);
	MALLOC (text, char, len);
	atexit (free_text);
	CFRead (text, 1, len, ifile);
	CFClose (ifile);
	}
else {
	int i;
	char *pi, *pj;

	len = CFLength (tfile, 0);
	MALLOC (text, char, len);
	atexit (free_text);
#if 1
	CFRead (text, 1, len, tfile);
	for (i = len, pi = pj = text; i; i--, pi++)
		if (*pi != 13)
			*pj++ = *pi;
	len = (int) (pj - text);
#else
	p = text;
	do {
		i = CFGetC (tfile);
		if (i != 13)
			*p++ = c;
	} while (c != EOF);
#endif
	CFClose (tfile);
	}

j = N_BASE_TEXTS + GameTextCount ();
for (h = i = 0, psz = text; (i < j) && (psz - text < len); i++) {
	char *ph, *pi, *pj;

	ph = psz;
	if (!(psz = strchr (psz, '\n'))) {
		if (i == 644) 
			break;    /* older datafiles */
#if DUMP_TEXTS 
		fclose (fTxt);
#endif
		Error ("Not enough strings in text file - expecting %d, found %d\n", N_BASE_TEXTS, i);
		}
	*psz++ = 0;
	if (!bBinary && ((*ph == ';') || ((*ph == '/') && ph [1] == '/')))
		continue;
	if (i < N_BASE_TEXTS) {
		baseGameTexts [h] = ph;
#if DUMP_TEXTS == 3
		{
		char s [200], *pi, *pj;
		if (h == 364)
			h = h;
		strcpy (s, d2GameTexts [h]);
		fprintf (fTxt, "\t{\"%s\", ", ph);
		if (strlen (ph) > 50)
			fprintf (fTxt, "\n\t ");
		fprintf (fTxt, "\"");
		for (pi = pj = s; *pj; pj++) {
			switch (*pj) {
				case '\t':
					*pj = '\0';
					fprintf (fTxt, "%s\\t", pi);
					*pj = '\t';
					pi = pj + 1;
					break;
				case '\n':
					*pj = '\0';
					fprintf (fTxt, "%s\\n", pi);
					*pj = '\n';
					pi = pj + 1;
					break;
				}
			}
		fprintf (fTxt, "%s\"},\n", pi);
		}
#endif
		}
	else if (gameStates.app.bEnglish)
		break;
	else {
		bBinary = 0;
		defaultGameTexts [h - N_BASE_TEXTS][0] = ph;
		}
	h++;
	if (bBinary)
		DecodeTextLine (ph);
#if DUMP_TEXTS == 2
	fprintf (fTxt, "%s\n", ph);
#endif
	for (pi = pj = ph; *pi; pi++, pj++) {
		if (*pi != '\\') 
			*pj = *pi;
		else {
			if (*++pi == 'n') 
				*pj = '\n';
			else if (*pi == 't') 
				*pj = '\t';
			else if (*pi == '\\') 
				*pj = '\\';
			else
				Error ("Unsupported key sequence <\\%c> on line %d of file <%s>", *pi, i + 1, filename);
			}
		}
	*pj = 0;
	}

if (i == 644) {
	if (len == SHAREWARE_TEXTSIZE) {
		baseGameTexts[173] = baseGameTexts[172];
		baseGameTexts[172] = baseGameTexts[171];
		baseGameTexts[171] = baseGameTexts[170];
		baseGameTexts[170] = baseGameTexts[169];
		baseGameTexts[169] = "Windows Joystick";
		}
	baseGameTexts[644] = "Z1";
	baseGameTexts[645] = "UN";
	baseGameTexts[646] = "P1";
	baseGameTexts[647] = "R1";
	baseGameTexts[648] = "Y1";
	}
#if DUMP_TEXTS 
fclose (fTxt);
#endif
InitGameTexts ();
}

//------------------------------------------------------------------------------
//eof
