#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <linux/input.h>

// I know this is terrible coding, but some of the doom key definitions overshadow the linux ones,
// And I need them both so... sorry not sorry

#ifndef __DOOMKEYS__
#define __DOOMKEYS__

//
// DOOM keyboard definition.
// This is the stuff configured by Setup.Exe.
// Most key data are simple ascii (uppercased).
//
#define DOOM_KEY_RIGHTARROW	0xae
#define DOOM_KEY_LEFTARROW	0xac
#define DOOM_KEY_UPARROW		0xad
#define DOOM_KEY_DOWNARROW	0xaf
#define DOOM_KEY_STRAFE_L	0xa0
#define DOOM_KEY_STRAFE_R	0xa1
#define DOOM_KEY_USE			0xa2
#define DOOM_KEY_FIRE		0xa3
#define DOOM_KEY_ESCAPE		27
#define DOOM_KEY_ENTER		13
#define DOOM_KEY_TAB			9
#define DOOM_KEY_F1			(0x80+0x3b)
#define DOOM_KEY_F2			(0x80+0x3c)
#define DOOM_KEY_F3			(0x80+0x3d)
#define DOOM_KEY_F4			(0x80+0x3e)
#define DOOM_KEY_F5			(0x80+0x3f)
#define DOOM_KEY_F6			(0x80+0x40)
#define DOOM_KEY_F7			(0x80+0x41)
#define DOOM_KEY_F8			(0x80+0x42)
#define DOOM_KEY_F9			(0x80+0x43)
#define DOOM_KEY_F10			(0x80+0x44)
#define DOOM_KEY_F11			(0x80+0x57)
#define DOOM_KEY_F12			(0x80+0x58)

#define DOOM_KEY_BACKSPACE	0x7f
#define DOOM_KEY_PAUSE	0xff

#define DOOM_KEY_EQUALS	0x3d
#define DOOM_KEY_MINUS	0x2d

#define DOOM_KEY_RSHIFT	(0x80+0x36)
#define DOOM_KEY_RCTRL	(0x80+0x1d)
#define DOOM_KEY_RALT	(0x80+0x38)

#define DOOM_KEY_LALT	DOOM_KEY_RALT

// new keys:

#define DOOM_KEY_CAPSLOCK    (0x80+0x3a)
#define DOOM_KEY_NUMLOCK     (0x80+0x45)
#define DOOM_KEY_SCRLCK      (0x80+0x46)
#define DOOM_KEY_PRTSCR      (0x80+0x59)

#define DOOM_KEY_HOME        (0x80+0x47)
#define DOOM_KEY_END         (0x80+0x4f)
#define DOOM_KEY_PGUP        (0x80+0x49)
#define DOOM_KEY_PGDN        (0x80+0x51)
#define DOOM_KEY_INS         (0x80+0x52)
#define DOOM_KEY_DEL         (0x80+0x53)

#define KEYP_0          0
#define KEYP_1          DOOM_KEY_END
#define KEYP_2          DOOM_KEY_DOWNARROW
#define KEYP_3          DOOM_KEY_PGDN
#define KEYP_4          DOOM_KEY_LEFTARROW
#define KEYP_5          '5'
#define KEYP_6          DOOM_KEY_RIGHTARROW
#define KEYP_7          DOOM_KEY_HOME
#define KEYP_8          DOOM_KEY_UPARROW
#define KEYP_9          DOOM_KEY_PGUP

#define KEYP_DIVIDE     '/'
#define KEYP_PLUS       '+'
#define KEYP_MINUS      '-'
#define KEYP_MULTIPLY   '*'
#define KEYP_PERIOD     0
#define KEYP_EQUALS     DOOM_KEY_EQUALS
#define KEYP_ENTER      DOOM_KEY_ENTER

#endif          // __DOOMKEYS__

#include "doomgeneric.h"



#define KEYQUEUE_SIZE 16

unsigned short s_KeyQueue[KEYQUEUE_SIZE];
unsigned int s_KeyQueueWriteIndex = 0;
unsigned int s_KeyQueueReadIndex = 0;

unsigned char convertToDoomKey(unsigned int key)
{
	switch (key)
	{
    case KEY_ENTER:
		key = DOOM_KEY_ENTER;
		break;
    case KEY_ESC:
		key = DOOM_KEY_ESCAPE;
		break;
    case KEY_LEFT:
		key = DOOM_KEY_LEFTARROW;
		break;
    case KEY_RIGHT:
		key = DOOM_KEY_RIGHTARROW;
		break;
    case KEY_UP:
		key = DOOM_KEY_UPARROW;
		break;
    case KEY_DOWN:
		key = DOOM_KEY_DOWNARROW;
		break;
    case KEY_LEFTCTRL:
    case KEY_RIGHTCTRL:
		key = DOOM_KEY_FIRE;
		break;
    case KEY_SPACE:
		key = DOOM_KEY_USE;
		break;
    case KEY_LEFTSHIFT:
    case KEY_RIGHTSHIFT:
		key = DOOM_KEY_RSHIFT;
		break;
	default:
		key = tolower(key);
		break;
	}
	return key;
}

void addKeyToQueue(int pressed, unsigned int keyCode)
{
	unsigned char key = convertToDoomKey(keyCode);

	unsigned short keyData = (pressed << 8) | key;

	s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
	s_KeyQueueWriteIndex++;
	s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}

int DG_GetKey(int* pressed, unsigned char* doomKey)
{
	if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex)
	{
		//key queue is empty

		return 0;
	}
	else
	{
		unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
		s_KeyQueueReadIndex++;
		s_KeyQueueReadIndex %= KEYQUEUE_SIZE;

		*pressed = keyData >> 8;
		*doomKey = keyData & 0xFF;

		return 1;
	}
}
