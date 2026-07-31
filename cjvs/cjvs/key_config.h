/*
===============================================================================
  ARTAX 4.1 / TAITO TYPE X - COMMUNITY JVS KEY CONFIGURATION
===============================================================================
  This header defines the default DirectInput / Hardware Scancodes (Set 1)
  specifically tailored and mapped for Artax 4.1 arcade cabinet layouts.
===============================================================================
*/

#pragma once
#include <windows.h>
#include <tchar.h>

// Global structure holding all configuration settings (loaded from fastio.ini)
struct FastioConfig {
	TCHAR comPort[16];
	int debugLog;
	int coinMultiplier;
	int coinPress;
	int coinDelay;
	WORD sCoin1;
	WORD sCoin2;
	WORD sTest;
	WORD sService1;
	WORD sService2;
	WORD sEsc;
};

// --- PLAYER 1 CONTROLS (Artax 4.1 Default Layout) ---
#define P1_UP		0xC8    // UP ARROW
#define P1_DOWN		0xD0    // DOWN ARROW
#define P1_LEFT		0xCB    // LEFT ARROW
#define P1_RIGHT	0xCD    // RIGHT ARROW
#define P1_START	0x02    // Key '1'
#define P1_BTN1		0x10    // Key 'Q'
#define P1_BTN2		0x11    // Key 'W'
#define P1_BTN3		0x12    // Key 'E'
#define P1_BTN4		0x1E    // Key 'A'
#define P1_BTN5		0x1F    // Key 'S'
#define P1_BTN6		0x20    // Key 'D'

// --- PLAYER 2 CONTROLS (Artax 4.1 Default Layout) ---
#define P2_UP		0x16    // Key 'U'
#define P2_DOWN		0x24    // Key 'J'
#define P2_LEFT		0x23    // Key 'H'
#define P2_RIGHT	0x25    // Key 'K'
#define P2_START	0x03    // Key '2'
#define P2_BTN1		0x47    // NUMPAD 7
#define P2_BTN2		0x48    // NUMPAD 8
#define P2_BTN3		0x49    // NUMPAD 9
#define P2_BTN4		0x4B    // NUMPAD 4
#define P2_BTN5		0x4C    // NUMPAD 5
#define P2_BTN6		0x4D    // NUMPAD 6

// --- SYSTEM & GENERAL CONTROLS (Required by PollJVS / Artax 4.1 Layout) ---
#define S_COIN1		0x06    // Key '5' (Coins / Coin Chute 1)
#define S_COIN2		0x07    // Key '6' (Coin Chute 2 for Multi-Coin games)
#define S_TEST		0x08    // Key '7' (Test Menu)
#define S_SERVICE1	0x09    // Key '8' (Service Menu / Service Credit)
#define S_SERVICE2	0x0A    // Key '9' (Service Credit 2)
#define S_ESC		0x01    // ESC Key (Exit Game / Return to Frontend)