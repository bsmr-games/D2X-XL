/*
 *
 * SDL tKeyboard input support
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __macosx__
# include <SDL/SDL.h>
#else
# include <SDL.h>
#endif

#include "descent.h"
#include "event.h"
#include "error.h"
#include "key.h"
#include "timer.h"
#include "hudmsgs.h"
#include "maths.h"

#define UNICODE_KEYS		0

#define KEY_BUFFER_SIZE 16

static ubyte bInstalled = 0;

//-------- Variable accessed by outside functions ---------

typedef struct tKeyInfo {
	ubyte		state;			// state of key 1 == down, 0 == up
	ubyte		lastState;		// previous state of key
	int		counter;		// incremented each time key is down in handler
	fix		timeWentDown;	// simple counter incremented each time in interrupt and key is down
	fix		timeHeldDown;	// counter to tell how long key is down -- gets reset to 0 by key routines
	ubyte		downCount;		// number of key counts key was down
	ubyte		upCount;		// number of times key was released
	ubyte		flags;
} tKeyInfo;

typedef struct tKeyboard {
	ushort		keyBuffer [KEY_BUFFER_SIZE];
	tKeyInfo		keys [256];
	fix			xTimePressed [KEY_BUFFER_SIZE];
	uint 			nKeyHead, nKeyTail;
} tKeyboard;

static tKeyboard keyData;

typedef struct tKeyProps {
	const char*	pszKeyText;
	wchar_t		asciiValue;
	wchar_t		shiftedAsciiValue;
	SDLKey		sym;
} tKeyProps;

tKeyProps keyProperties [] = {
{ "",       255,    255,    (SDLKey) -1        },
{ "ESC",    255,    255,    SDLK_ESCAPE        },
{ "1",      '1',    '!',    SDLK_1             },
{ "2",      '2',    '@',    SDLK_2             },
{ "3",      '3',    '#',    SDLK_3             },
{ "4",      '4',    '$',    SDLK_4             },
{ "5",      '5',    '%',    SDLK_5             },
{ "6",      '6',    '^',    SDLK_6             },
{ "7",      '7',    '&',    SDLK_7             },
{ "8",      '8',    '*',    SDLK_8             },
{ "9",      '9',    '(',    SDLK_9             },
{ "0",      '0',    ')',    SDLK_0             },
{ "-",      '-',    '_',    SDLK_MINUS         },
{ "=",      '=',    '+',    SDLK_EQUALS        },
{ "BSPC",   255,    255,    SDLK_BACKSPACE     },
{ "TAB",    255,    255,    SDLK_TAB           },
{ "Q",      'q',    'Q',    SDLK_q             },
{ "W",      'w',    'W',    SDLK_w             },
{ "E",      'e',    'E',    SDLK_e             },
{ "R",      'r',    'R',    SDLK_r             },
{ "T",      't',    'T',    SDLK_t             },
{ "Y",      'y',    'Y',    SDLK_y             },
{ "U",      'u',    'U',    SDLK_u             },
{ "I",      'i',    'I',    SDLK_i             },
{ "O",      'o',    'O',    SDLK_o             },
{ "P",      'p',    'P',    SDLK_p             },
{ "[",      '[',    '{',    SDLK_LEFTBRACKET   },
{ "]",      ']',    '}',    SDLK_RIGHTBRACKET  },
{ "�",      255,    255,    SDLK_RETURN        },
{ "LCTRL",  255,    255,    SDLK_LCTRL         },
{ "A",      'a',    'A',    SDLK_a             },
{ "S",      's',    'S',    SDLK_s             },
{ "D",      'd',    'D',    SDLK_d             },
{ "F",      'f',    'F',    SDLK_f             },
{ "G",      'g',    'G',    SDLK_g             },
{ "H",      'h',    'H',    SDLK_h             },
{ "J",      'j',    'J',    SDLK_j             },
{ "K",      'k',    'K',    SDLK_k             },
{ "L",      'l',    'L',    SDLK_l             },
{ ";",      ';',    ':',    SDLK_SEMICOLON     },
{ "'",      '\'',   '"',    SDLK_QUOTE         },
{ "`",      '`',    '~',    SDLK_BACKQUOTE     },
{ "LSHFT",  255,    255,    SDLK_LSHIFT        },
{ "\\",     '\\',   '|',    SDLK_BACKSLASH     },
{ "Z",      'z',    'Z',    SDLK_z             },
{ "X",      'x',    'X',    SDLK_x             },
{ "C",      'c',    'C',    SDLK_c             },
{ "V",      'v',    'V',    SDLK_v             },
{ "B",      'b',    'B',    SDLK_b             },
{ "N",      'n',    'N',    SDLK_n             },
{ "M",      'm',    'M',    SDLK_m             },
{ ",",      ',',    '<',    SDLK_COMMA			  },
{ ".",      '.',    '>',    SDLK_PERIOD		  },
{ "/",      '/',    '?',    SDLK_SLASH			  },
{ "RSHFT",  255,    255,    SDLK_RSHIFT		  },
{ "PAD*",   '*',    255,    SDLK_KP_MULTIPLY   },
{ "LALT",   255,    255,    SDLK_LALT          },
{ "SPC",    ' ',    ' ',    SDLK_SPACE         },
{ "CPSLK",  255,    255,    SDLK_CAPSLOCK      },
{ "F1",     255,    255,    SDLK_F1            },
{ "F2",     255,    255,    SDLK_F2            },
{ "F3",     255,    255,    SDLK_F3            },
{ "F4",     255,    255,    SDLK_F4            },
{ "F5",     255,    255,    SDLK_F5            },
{ "F6",     255,    255,    SDLK_F6            },
{ "F7",     255,    255,    SDLK_F7            },
{ "F8",     255,    255,    SDLK_F8            },
{ "F9",     255,    255,    SDLK_F9            },
{ "F10",    255,    255,    SDLK_F10           },
{ "NMLCK",  255,    255,    SDLK_NUMLOCK       },
{ "SCLK",   255,    255,    SDLK_SCROLLOCK     },
{ "PAD7",   255,    255,    SDLK_KP7           },
{ "PAD8",   255,    255,    SDLK_KP8           },
{ "PAD9",   255,    255,    SDLK_KP9           },
{ "PAD-",   255,    255,    SDLK_KP_MINUS      },
{ "PAD4",   255,    255,    SDLK_KP4           },
{ "PAD5",   255,    255,    SDLK_KP5           },
{ "PAD6",   255,    255,    SDLK_KP6           },
{ "PAD+",   255,    255,    SDLK_KP_PLUS       },
{ "PAD1",   255,    255,    SDLK_KP1           },
{ "PAD2",   255,    255,    SDLK_KP2           },
{ "PAD3",   255,    255,    SDLK_KP3           },
{ "PAD0",   255,    255,    SDLK_KP0           },
{ "PAD.",   255,    255,    SDLK_KP_PERIOD     },
{ "ALTGR",  255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "F11",    255,    255,    SDLK_F11           },
{ "F12",    255,    255,    SDLK_F12           },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "PAUSE",  255,    255,    SDLK_PAUSE			  },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",			255,    255,    (SDLKey) -1        },
{ "PAD�",	255,    255,    SDLK_KP_ENTER      },
{ "RCTRL",  255,    255,    SDLK_RCTRL         },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "PAD/",   255,    255,    SDLK_KP_DIVIDE     },
{ "",       255,    255,    (SDLKey) -1        },
{ "PRSCR",  255,    255,    SDLK_PRINT         },
{ "RALT",   255,    255,    SDLK_RALT          },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "HOME",   255,    255,    SDLK_HOME          },
{ "UP",		255,    255,    SDLK_UP            },
{ "PGUP",   255,    255,    SDLK_PAGEUP        },
{ "",       255,    255,    (SDLKey) -1        },
{ "LEFT",	255,    255,    SDLK_LEFT          },
{ "",       255,    255,    (SDLKey) -1        },
{ "RIGHT",	255,    255,    SDLK_RIGHT         },
{ "",       255,    255,    (SDLKey) -1        },
{ "END",    255,    255,    SDLK_END           },
{ "DOWN",	255,    255,    SDLK_DOWN          },
{ "PGDN",	255,    255,    SDLK_PAGEDOWN      },
{ "INS",		255,    255,    SDLK_INSERT        },
{ "DEL",		255,    255,    SDLK_DELETE        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "",       255,    255,    (SDLKey) -1        },
{ "RWIN",   255,    255,    SDLK_RSUPER        },
{ "LWIN",   255,    255,    SDLK_LSUPER        },
{ "RCMD",   255,    255,    SDLK_RMETA         },
{ "LCMD",   255,    255,    SDLK_LMETA         }
};

const char *pszKeyText [256];

//------------------------------------------------------------------------------

ubyte KeyToASCII (int keyCode)
{
	int shifted;

shifted = keyCode & KEY_SHIFTED;
keyCode &= 0xFF;
return shifted ? static_cast<ubyte> (keyProperties [keyCode].shiftedAsciiValue) : static_cast<ubyte> (keyProperties [keyCode].asciiValue);
}

//------------------------------------------------------------------------------

#if UNICODE_KEYS == 0

static int KeyGerman (int keyCode)
{
if (keyCode == KEY_Z)
	return KEY_Y;
else if (keyCode == KEY_Z)
	return KEY_Z;
else if (keyCode == KEY_SEMICOLON)
	return -1;
else if (keyCode == KEY_EQUALS)
	return KEY_RAPOSTRO;
else if (keyCode == KEY_RAPOSTRO)
	return -1;
else if (keyCode == KEY_LBRACKET)
	return -1;
else if (keyCode == KEY_RBRACKET)
	return KEY_EQUALS;
else if (keyCode == KEY_SLASH)
	return KEY_MINUS;
else if (keyCode == KEY_BACKSLASH)
	return KEY_RAPOSTRO;
else if (keyCode & KEY_SHIFTED) {
	int h = keyCode & 0xFF;
	if (h == KEY_7)
		return KEY_BACKSLASH;
	else if (h == KEY_0)
		return KEY_EQUALS;
	else if (h == KEY_COMMA)
		return KEY_SEMICOLON;
	}
else if (gameStates.input.keys.pressed [KEY_LCTRL] && gameStates.input.keys.pressed [KEY_RALT]) {	//ALTGR
	if ((keyCode == KEY_7) || (keyCode == KEY_8))
		return KEY_LBRACKET;
	else if ((keyCode == KEY_9) || (keyCode == KEY_0))
		return KEY_RBRACKET;
	else if (keyCode == KEY_0)
		return KEY_EQUALS;
	else if (keyCode == KEY_MINUS)
		return KEY_BACKSLASH;
	}
return keyCode;
}

//------------------------------------------------------------------------------

static int KeyFrench (int keyCode)
{
if (keyCode == KEY_A)
	return KEY_Q;
else if (keyCode == KEY_Q)
	return KEY_A;
else if (keyCode == KEY_W)
	return KEY_Z;
else if (keyCode == KEY_Z)
	return KEY_W;
else if (keyCode == KEY_COMMA)
	return KEY_SEMICOLON;
else if (keyCode == KEY_PERIOD)
	return KEY_SEMICOLON;
else if (keyCode == KEY_SEMICOLON)
	return KEY_M;
else if (keyCode == KEY_M)
	return KEY_COMMA;
else if (keyCode == KEY_RBRACKET)
	return KEY_EQUALS;
else if (keyCode == KEY_BACKSLASH)
	return -1;
else if (keyCode == KEY_RAPOSTRO)
	return -1;
else if (keyCode == KEY_SLASH)
	return -1;
else if (keyCode == KEY_LBRACKET)
	return -1;
else if (keyCode == KEY_SLASH)
	return -1;
else if (keyCode & KEY_SHIFTED) {
	int h = keyCode & 0xFF;
	if (h == KEY_COMMA)
		return KEY_PERIOD;
	else if (h == KEY_PERIOD)
		return KEY_SLASH;
	else if (h == KEY_COMMA)
		return KEY_SEMICOLON;
	}
else if (gameStates.input.keys.pressed [KEY_LCTRL] && gameStates.input.keys.pressed [KEY_RALT]) {	//ALTGR
	if ((keyCode == KEY_4) || (keyCode == KEY_5))
		return KEY_LBRACKET;
	else if ((keyCode == KEY_MINUS) || (keyCode == KEY_EQUALS))
		return KEY_RBRACKET;
	else if (keyCode == KEY_0)
		return KEY_EQUALS;
	else if (keyCode == KEY_8)
		return KEY_BACKSLASH;
	}
return keyCode;
}

//------------------------------------------------------------------------------

typedef struct tKeyMap {
	SDLKey	inKey, outKey;
} tKeyMap;

static int KeyDvorak (int keyCode)
{
	int keyMap [] = {
		KEY_A,
		KEY_X,
		KEY_J,
		KEY_E,
		KEY_PERIOD,
		KEY_U,
		KEY_I,
		KEY_D,
		KEY_C,
		KEY_H,
		KEY_T,
		KEY_N,
		KEY_M,
		KEY_B,
		KEY_R,
		KEY_L,
		KEY_LAPOSTRO,
		KEY_P,
		KEY_O,
		KEY_Y,
		KEY_G,
		KEY_K,
		KEY_COMMA,
		KEY_Q,
		KEY_F,
		KEY_SEMICOLON
		};

	int flags = keyCode & 0xff00;

keyCode &= 0xff;
if ((keyCode >= KEY_A) && (keyCode <= KEY_Z))
	return keyMap [keyCode];
else if (keyCode == KEY_COMMA)
	return KEY_W;
else if (keyCode == KEY_PERIOD)
	return KEY_W;
else if (keyCode == KEY_SLASH)
	return KEY_Z;
else if (keyCode == KEY_SEMICOLON)
	return KEY_S;
else if (keyCode == KEY_LAPOSTRO)
	return KEY_MINUS;
else if (keyCode == KEY_LBRACKET)
	return KEY_SLASH;
else if (keyCode == KEY_RBRACKET)
	return KEY_EQUALS;
else if (keyCode == KEY_MINUS)
	return KEY_LBRACKET;
else if (keyCode == KEY_EQUALS)
	return KEY_RBRACKET;
return keyCode | flags;
}

#endif

//------------------------------------------------------------------------------

void KeyHandler (SDL_KeyboardEvent *event)
{
	ubyte			state;
	int			keyCode, keyState, nKeyboard = gameOpts->input.keyboard.nType;
	SDLKey		keySym;
	wchar_t		unicode;
	tKeyInfo*	keyP;
	ubyte			temp;

keySym = event->keysym.sym;
unicode = event->keysym.unicode;
keyState = (event->state == SDL_PRESSED); //  !(wInfo & KF_UP);

//=====================================================
//Here a translation from win keycodes to mac keycodes!
//=====================================================

for (int i = 0, j = sizeofa (keyProperties); i < j; i++) {
	keyCode = i;
	keyP = keyData.keys + keyCode;

#if UNICODE_KEYS
	if (unicode && (keyProperties [i].asciiValue != wchar_t (255))) {
		if ((keyProperties [i].asciiValue == unicode) || (keyProperties [i].shiftedAsciiValue == unicode))
#	if DBG
			if (keyP->lastState)
				state = keyState;
			else
#	endif
			state = keyState;
		else
			state = keyP->lastState;
		}
   else if (keyProperties [i].sym == keySym)
#else
   if (keyProperties [i].sym == keySym)
#endif
		state = keyState;
	else
		state = keyP->lastState;
	
	if (keyP->lastState == state) {
		if (state) {
			keyP->counter++;
			gameStates.input.keys.nLastPressed = keyCode;
			gameStates.input.keys.xLastPressTime = TimerGetFixedSeconds ();
			keyP->flags = 0;
			if (gameStates.input.keys.pressed [KEY_LSHIFT] || gameStates.input.keys.pressed [KEY_RSHIFT])
				keyP->flags |= ubyte (KEY_SHIFTED / 256);
			if (gameStates.input.keys.pressed [KEY_LALT] || gameStates.input.keys.pressed [KEY_RALT])
				keyP->flags |= ubyte (KEY_ALTED / 256);
			if ((gameStates.input.keys.pressed [KEY_LCTRL] && (nKeyboard != 1)) || gameStates.input.keys.pressed [KEY_RCTRL])
				keyP->flags |= ubyte (KEY_CTRLED / 256);
			}
		}
	else {
		if (state) {
			gameStates.input.keys.nLastPressed = keyCode;
			keyP->timeWentDown = gameStates.input.keys.xLastPressTime = TimerGetFixedSeconds();
			keyData.keys [keyCode].timeHeldDown = 0;
			gameStates.input.keys.pressed [keyCode] = 1;
			keyP->downCount += state;
			keyP->counter++;
			keyP->state = 1;
			keyP->flags = 0;
			if (gameStates.input.keys.pressed [KEY_LSHIFT] || gameStates.input.keys.pressed [KEY_RSHIFT])
				keyP->flags |= ubyte (KEY_SHIFTED / 256);
			if (gameStates.input.keys.pressed [KEY_LALT] || gameStates.input.keys.pressed [KEY_RALT])
				keyP->flags |= ubyte (KEY_ALTED / 256);
			if (((nKeyboard != 1) && gameStates.input.keys.pressed [KEY_LCTRL]) || gameStates.input.keys.pressed [KEY_RCTRL])
				keyP->flags |= ubyte (KEY_CTRLED / 256);
//				keyP->timeWentDown = gameStates.input.keys.xLastPressTime = TimerGetFixedSeconds();
			}
		else {
			gameStates.input.keys.pressed [keyCode] = 0;
			gameStates.input.keys.nLastReleased = keyCode;
			keyP->upCount += keyP->state;
			keyP->state = 0;
			keyP->counter = 0;
			keyP->timeHeldDown += TimerGetFixedSeconds () - keyP->timeWentDown;
			}
		}
	if (state && (!keyP->lastState || ((keyP->counter > 30) && (keyP->counter & 1)))) {
		if (gameStates.input.keys.pressed [KEY_LSHIFT] || gameStates.input.keys.pressed [KEY_RSHIFT])
			keyCode |= KEY_SHIFTED;
		if (gameStates.input.keys.pressed [KEY_LALT] || gameStates.input.keys.pressed [KEY_RALT])
			keyCode |= KEY_ALTED;
		if (((nKeyboard != 1) && gameStates.input.keys.pressed [KEY_LCTRL]) || gameStates.input.keys.pressed [KEY_RCTRL])
			keyCode |= KEY_CTRLED;
		if (gameStates.input.keys.pressed [KEY_LCMD] || gameStates.input.keys.pressed [KEY_RCMD])
			keyCode |= KEY_COMMAND;
#if DBG
	   if (gameStates.input.keys.pressed [KEY_DELETE])
			keyCode |= KEYDBGGED;
#endif			
		temp = keyData.nKeyTail + 1;
		if (temp >= KEY_BUFFER_SIZE) 
			temp = 0;
		if (temp != keyData.nKeyHead) {
#if UNICODE_KEYS == 0
			if (nKeyboard == 1) {
				if (-1 == (keyCode = KeyGerman (keyCode)))
					continue;
				}
			else if (nKeyboard == 2) {
				if (-1 == (keyCode = KeyFrench (keyCode)))
					continue;
				}
			else if (nKeyboard == 3) {
				if (-1 == (keyCode = KeyDvorak (keyCode)))
					continue;
				}
#endif
#if DBG
			if ((i == KEY_LSHIFT) || (i == KEY_RSHIFT) || 
				 (i == KEY_LALT) || (i == KEY_RALT) || 
				 (i == KEY_RCTRL) || (i == KEY_LCTRL))
				keyData.keyBuffer [keyData.nKeyTail] = keyCode;
			else
#endif
				keyData.keyBuffer [keyData.nKeyTail] = keyCode;
			keyData.xTimePressed [keyData.nKeyTail] = gameStates.input.keys.xLastPressTime;
			keyData.nKeyTail = temp;
			}
		}
	keyP->lastState = state;
	}
}

//------------------------------------------------------------------------------

void _CDECL_ KeyClose (void)
{
 bInstalled = 0;
}

//------------------------------------------------------------------------------

void KeyInit (void)
{
  int i;
  
if (bInstalled) 
	return;
bInstalled = 1;
#if UNICODE_KEYS
SDL_EnableUNICODE (1);
#endif
gameStates.input.keys.xLastPressTime = TimerGetFixedSeconds ();
gameStates.input.keys.nBufferType = 1;
gameStates.input.keys.bRepeat = 1;
for(i = 0; i < 256; i++)
	pszKeyText [i] = keyProperties [i].pszKeyText;
// Clear the tKeyboard array
KeyFlush ();
atexit (KeyClose);
}

//------------------------------------------------------------------------------

void KeyFlush (void)
{
 	int i;

if (!bInstalled)
	KeyInit();
keyData.nKeyHead = keyData.nKeyTail = 0;
// clear the tKeyboard buffer
for (i = 0; i < KEY_BUFFER_SIZE; i++) {
	keyData.keyBuffer [i] = 0;
	keyData.xTimePressed [i] = 0;
	}
int curtime = TimerGetFixedSeconds ();
for (i = 0; i < 256; i++) {
	gameStates.input.keys.pressed [i] = 0;
	keyData.keys [i].state = 1;
	keyData.keys [i].lastState = 0;
	keyData.keys [i].timeWentDown = curtime;
	keyData.keys [i].downCount = 0;
	keyData.keys [i].upCount = 0;
	keyData.keys [i].timeHeldDown = 0;
	keyData.keys [i].counter = 0;
	}
}

//------------------------------------------------------------------------------

int KeyAddKey (int n)
{
return (++n < KEY_BUFFER_SIZE) ? n : 0;
}

//------------------------------------------------------------------------------

int KeyCheckChar (void)
{
event_poll (SDL_KEYDOWNMASK | SDL_KEYUPMASK);
return (keyData.nKeyTail != keyData.nKeyHead);
}

//------------------------------------------------------------------------------

int KeyInKeyTime (fix * time)
{
	int key = 0;
	int bLegacy = gameOpts->legacy.bInput;

gameOpts->legacy.bInput = 1;

if (!bInstalled)
	KeyInit ();
event_poll (SDL_KEYDOWNMASK | SDL_KEYUPMASK);
if (keyData.nKeyTail != keyData.nKeyHead) {
	key = keyData.keyBuffer [keyData.nKeyHead];
	if (key == KEY_CTRLED + KEY_ALTED + KEY_ENTER)
		exit (0);
	keyData.nKeyHead = KeyAddKey(keyData.nKeyHead);
	if (time)
		*time = keyData.xTimePressed [keyData.nKeyHead];
	}
else if (!time)
	G3_SLEEP (0);
gameOpts->legacy.bInput = bLegacy;
return key;
}

//------------------------------------------------------------------------------

int KeyInKey (void)
{
return KeyInKeyTime (NULL);
}

//------------------------------------------------------------------------------

int KeyPeekKey (void)
{
	int key = 0;

event_poll (SDL_KEYDOWNMASK | SDL_KEYUPMASK);
if (keyData.nKeyTail != keyData.nKeyHead)
	key = keyData.keyBuffer [keyData.nKeyHead];
return key;
}

//------------------------------------------------------------------------------

int KeyGetChar (void)
{
if (!bInstalled)
	return 0;
while (!KeyCheckChar ())
	;
return KeyInKey ();
}

//------------------------------------------------------------------------------

uint KeyGetShiftStatus (void)
{
	uint shift_status = 0;

if (gameStates.input.keys.pressed [KEY_LSHIFT] || gameStates.input.keys.pressed [KEY_RSHIFT])
	shift_status |= KEY_SHIFTED;
if (gameStates.input.keys.pressed [KEY_LALT] || gameStates.input.keys.pressed [KEY_RALT])
	shift_status |= KEY_ALTED;
if (gameStates.input.keys.pressed [KEY_LCTRL] || gameStates.input.keys.pressed [KEY_RCTRL])
	shift_status |= KEY_CTRLED;
#if DBG
if (gameStates.input.keys.pressed [KEY_DELETE])
	shift_status |=KEYDBGGED;
#endif
return shift_status;
}

//------------------------------------------------------------------------------
// Returns the number of seconds this key has been down since last call.
fix KeyDownTime (int scancode)
{
	static fix lastTime = -1;
	fix timeDown, time, slack = 0;

#ifndef FAST_EVENTPOLL
if (!bFastPoll)
event_poll(SDL_KEYDOWNMASK | SDL_KEYUPMASK);
#endif
if ((scancode<0)|| (scancode>255)) return 0;

if (!gameStates.input.keys.pressed [scancode]) {
	timeDown = keyData.keys [scancode].timeHeldDown;
	keyData.keys [scancode].timeHeldDown = 0;
	} 
else {
	QLONG s, ms;

	time = TimerGetFixedSeconds();
	timeDown = time - keyData.keys [scancode].timeWentDown;
	s = timeDown / 65536;
	ms = (timeDown & 0xFFFF);
	ms *= 1000;
	ms >>= 16;
	keyData.keys [scancode].timeHeldDown += (int) (s * 1000 + ms);
	// the following code takes care of clamping in KConfig.c::control_read_all()
	if (gameStates.input.bKeepSlackTime && (timeDown > gameStates.input.kcPollTime)) {
		slack = (fix) (timeDown - gameStates.input.kcPollTime);
		time -= slack + slack / 10;	// there is still some slack, so add an extra 10%
		if (time < lastTime)
			time = lastTime;
		timeDown = (fix) gameStates.input.kcPollTime;
		}
	keyData.keys [scancode].timeWentDown = time;
	lastTime = time;
	if (timeDown && timeDown < gameStates.input.kcPollTime)
		timeDown = (fix) gameStates.input.kcPollTime;
	}
return timeDown;
}

//------------------------------------------------------------------------------

uint KeyDownCount (int scancode)
{
	int n;

#ifndef FAST_EVENTPOLL
if (!bFastPoll)
event_poll(SDL_KEYDOWNMASK | SDL_KEYUPMASK);
#endif
if ((scancode<0)|| (scancode>255)) return 0;

n = keyData.keys [scancode].downCount;
keyData.keys [scancode].downCount = 0;
keyData.keys [scancode].flags = 0;
return n;
}

//------------------------------------------------------------------------------

ubyte KeyFlags (int scancode)
{
#ifndef FAST_EVENTPOLL
if (!bFastPoll)
	event_poll(SDL_KEYDOWNMASK | SDL_KEYUPMASK);
#endif
if ((scancode < 0)|| (scancode > 255)) 
	return 0;
return keyData.keys [scancode].flags;
}

//------------------------------------------------------------------------------

uint KeyUpCount (int scancode)
{
	int n;
#ifndef FAST_EVENTPOLL
if (!bFastPoll)
event_poll(SDL_KEYDOWNMASK | SDL_KEYUPMASK);
#endif
if ((scancode < 0)|| (scancode > 255)) 
	return 0;
n = keyData.keys [scancode].upCount;
keyData.keys [scancode].upCount = 0;
return n;
}

//------------------------------------------------------------------------------

fix KeyRamp (int scancode)
{
if (!gameOpts->input.keyboard.nRamp)
	return 1;
else {
		int maxRampTime = gameOpts->input.keyboard.nRamp * 20; // / gameOpts->input.keyboard.nRamp;
		fix t = keyData.keys [scancode].timeHeldDown;

	if (!t)
		return maxRampTime;
	if (t >= maxRampTime)
		return 1;
	t = maxRampTime / t;
	return t ? t : 1;
	}
}

//------------------------------------------------------------------------------
//eof
