// cjvs.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "key_config.h"
#include "cjvs.h"
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <dinput.h>

/*#define dbgraise(exp) if(exp) { __asm int 3 }*/
/*FILE *f = NULL;*/
COMMTIMEOUTS timeouts = { 0, //interval timeout. 0 = not used
							  0, // read multiplier
							 10, // read constant (milliseconds)
							  0, // Write multiplier
							  0  // Write Constant
};

void LogEvent(HANDLE hFile, wchar_t text[], int size)
{
	DWORD lwrote;
	char *conv = new CHAR[size];
	WideCharToMultiByte(CP_UTF8, 0, text, size, conv, size, NULL, NULL);
	WriteFile(hFile, conv, size, &lwrote, NULL);
	FlushFileBuffers(hFile);
}

void SendByte(HANDLE hCom, uint8_t byte)
{
	DWORD bsent;
	WriteFile(hCom, &byte, 1, &bsent, NULL);
}

/*DWORD SendByte(HANDLE hCom, uint8_t byte) {
	DWORD bsent;
	WriteFile(hCom, &byte, 1, &bsent, NULL);
	return bsent;
}*/

uint8_t ReadByte(HANDLE hCom, LPDWORD lpRead, HANDLE hFile) {
	//wchar_t ltext[256];
	uint8_t read = NULL;
	SetCommTimeouts(hCom, &timeouts);
	(void)ReadFile(hCom, &read, 1, lpRead, NULL);
	return read;
}

int SetupJVS(JVSKEY *jkey, HANDLE hCom, HANDLE hFile)
{
	DCB dcb;
	COMMTIMEOUTS ct = { 0 };
	COMSTAT comStat = { 0 };
	DWORD errors = NULL;
	BOOL fSuccess;
	wchar_t ltext[256];

	jkey->coin[0] = 0x00;
	jkey->coin[1] = 0x00;

	SecureZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);

	ClearCommError(hCom, &errors, &comStat);
	fSuccess = GetCommState(hCom, &dcb);

	if (!fSuccess)
	{
		LogEvent(hFile, ltext, swprintf_s(ltext, sizeof(ltext) / sizeof(wchar_t), L"SetupJVS - GetCommState failed with error %d.\r\n", GetLastError()));
		return (0);
	}

	SetupComm(hCom, 516, 516);

	GetCommState(hCom, &dcb);

	LogEvent(hFile, ltext, swprintf_s(ltext, sizeof(ltext) / sizeof(wchar_t), L"BaudRate = %d, ByteSize = %d, Parity = %d, StopBits = %d\r\n", dcb.BaudRate, dcb.ByteSize, dcb.Parity, dcb.StopBits));
	dcb.BaudRate = CBR_115200;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	
	SetCommState(hCom, &dcb);
	GetCommState(hCom, &dcb);
	LogEvent(hFile, ltext, swprintf_s(ltext, sizeof(ltext) / sizeof(wchar_t), L"BaudRate = %d, ByteSize = %d, Parity = %d, StopBits = %d\r\n", dcb.BaudRate, dcb.ByteSize, dcb.Parity, dcb.StopBits));

	EscapeCommFunction(hCom, CLRRTS);
	EscapeCommFunction(hCom, SETDTR);
	SetCommMask(hCom, EV_RXCHAR);
	SetCommTimeouts(hCom, &ct);

	return(1);
}
void SetupJVSKEY(JVSKEY *jkey, INPUT *ip, LPDIRECTINPUT8 *di)
{
	ip->type = INPUT_KEYBOARD;
	ip->ki.dwFlags = KEYEVENTF_SCANCODE;
	ip->ki.wScan = 0;
	ip->ki.time = 0;
	ip->ki.dwExtraInfo = 0;
}

void WriteJVS(HANDLE hCom, uint8_t dest, uint8_t* data, int size)
{
	int i;

	SendByte(hCom, JVS_SYNC_CODE);
	SendByte(hCom, dest);
	SendByte(hCom, (uint8_t)(size));

	/*dbgraise(!SendByte(hCom, JVS_SYNC_CODE));
	dbgraise(!SendByte(hCom, dest));
	dbgraise(!SendByte(hCom, (uint8_t)(size)));*/

	/*fprintf_s(f, "W: E0 %02x %02x ", dest, size);
	for (int i = 0; i < size; i++)
	{
		fprintf_s(f, "%02x ", data[i]);
	}
	fprintf_s(f, "\r\n");*/

	for (i = 0; i < size; i++) 
	{
		if (data[i] == JVS_SYNC_CODE || data[i] == JVS_ESCAPE_CODE) 
		{
			/*dbgraise(SendByte(hCom, JVS_ESCAPE_CODE));
			dbgraise(SendByte(hCom, data[i] - 1));*/
			SendByte(hCom, JVS_ESCAPE_CODE);
			SendByte(hCom, data[i] - 1);
		}
		else
		{
			/*dbgraise(SendByte(hCom, data[i]));*/
			SendByte(hCom, data[i]);
		}
	}
}

void ReadJVS(HANDLE hCom, HANDLE hFile, uint8_t *buffer, int size)
{
	uint8_t buf = NULL;
	uint8_t dest;
	uint8_t data_size;
	int i;
	DWORD read;

	memset(buffer, 0, size);

	while (true) 
	{
		buf = ReadByte(hCom, &read, hFile);

		if (read == NULL) 
		{
			//LogEvent(hFile, ltext, swprintf_s(ltext, sizeof(ltext) / sizeof(wchar_t), L"ReadJVS - Read Error!\r\n"));
			break;
		}

		if (buf == JVS_SYNC_CODE) 
		{

			dest = ReadByte(hCom, &read, hFile);
			data_size = ReadByte(hCom, &read, hFile) - 1;

			for (i = 0; i < data_size; i++)
			{
				buffer[i] = ReadByte(hCom, &read, hFile);

				if (buffer[i] == JVS_ESCAPE_CODE)
				{
					buffer[i] = ReadByte(hCom, &read, hFile) + 1;
				}

			}

			/*fprintf_s(f, "R: ");
			for (int i = 0; i < data_size; i++)
			{
				fprintf_s(f, "%02x ", buffer[i]);
			}
			fprintf_s(f, "\r\n");*/

			break;
		}

	}
}

int init_master(HANDLE hCom, HANDLE hFile)
{
	uint8_t reset[] = { JVS_OP_RESET, 0xD9, 0xCB };
	uint8_t set[] = { JVS_OP_ADDRESS, 0x01, 0xF4 };
	uint8_t jdata[256];

	WriteJVS(hCom, JVS_ADDR_BROADCAST, reset, 3);
	Sleep(10);

	WriteJVS(hCom, JVS_ADDR_BROADCAST, set, 3);
	Sleep(10);

	if (!ClearCommError(hCom, NULL, 0))
	{
		return(0);
	}

	if (!EscapeCommFunction(hCom, CLRRTS))
	{
		return(0);
	}

	Sleep(10);

	ReadJVS(hCom, hFile, jdata, 256);

	if (jdata[0] != JVS_OK)
	{
		//LogEvent(hFile, ltext, swprintf_s(ltext, sizeof(ltext) / sizeof(wchar_t), L"Initialize Master - jdata[0] JVS NOT OK!\r\n"));
		return(0);
	}
	if (jdata[1] != JVS_OK)
	{
		//LogEvent(hFile, ltext, swprintf_s(ltext, sizeof(ltext) / sizeof(wchar_t), L"Initialize Master - jdata[1] JVS NOT OK!\r\n"));
		return(0);
	}

	return(1);
}

int PollJVS(JVSKEY *jkey, INPUT *ip, HANDLE hCom, HANDLE hFile, int node)
{
	uint8_t request[4] = { 0x20, 0x02, 0x02, JVS_NODE1 };
	uint8_t jdata[256];

	if (node != 1)
	{
		return(0);
	}

	WriteJVS(hCom, node, request, 4);

	if (!ClearCommError(hCom, 0, 0))
	{
		return(0);
	}

	if (!EscapeCommFunction(hCom, SETRTS))
	{
		return(0);
	}

	Sleep(10);

	ReadJVS(hCom, hFile, jdata, 256);

	if (jdata[0] != JVS_OK)
	{
		//LogEvent(hFile, ltext, swprintf_s(ltext, sizeof(ltext), L"PollJVS - jdata[0] JVS NOT OK!\r\n"));
		return(1);
	}

	if (jdata[1] != JVS_OK)
	{
		//LogEvent(hFile, ltext, swprintf_s(ltext, sizeof(ltext), L"PollJVS - jdata[1] JVS NOT OK!\r\n"));
		return(1);
	}

	if (jdata[3] & 0x20)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY;
		ip->ki.wScan = P1_UP;
		SendInput(1, ip, sizeof(INPUT));
		jkey->kup_flag = 1;
		
	}
	else if (jkey->kup_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP;
		ip->ki.wScan = P1_UP;

		SendInput(1, ip, sizeof(INPUT));
		jkey->kup_flag = 0;
		
	}

	if (jdata[3] & 0x10)
	{
		//printf("DOWN\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY;
		ip->ki.wScan = P1_DOWN;
		SendInput(1, ip, sizeof(INPUT));
		jkey->kdown_flag = 1;
		
	}
	else if (jkey->kdown_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP;
		ip->ki.wScan = P1_DOWN;

		SendInput(1, ip, sizeof(INPUT));
		jkey->kdown_flag = 0;
		
	}

	if (jdata[3] & 0x08)
	{
		//printf("LEFT\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY;
		ip->ki.wScan = P1_LEFT;
		SendInput(1, ip, sizeof(INPUT));
		jkey->kleft_flag = 1;
		
	}
	else if (jkey->kleft_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP;
		ip->ki.wScan = P1_LEFT;

		SendInput(1, ip, sizeof(INPUT));
		jkey->kleft_flag = 0;
		
	}

	if (jdata[3] & 0x04)
	{
		//printf("RIGHT\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY;
		ip->ki.wScan = P1_RIGHT;
		SendInput(1, ip, sizeof(INPUT));
		jkey->kright_flag = 1;
		

	}
	else if (jkey->kright_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP;
		ip->ki.wScan = P1_RIGHT;

		SendInput(1, ip, sizeof(INPUT));
		jkey->kright_flag = 0;
		
	}

	if (jdata[3] & 0x80)
	{
		//printf("START\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE;
		ip->ki.wScan = P1_START;
		SendInput(1, ip, sizeof(INPUT));
		jkey->kstart_flag = 1;
	}
	else if (jkey->kstart_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = P1_START;

		SendInput(1, ip, sizeof(INPUT));
		jkey->kstart_flag = 0;
	}
	else if (jkey->kstart_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = P1_START;

		SendInput(1, ip, sizeof(INPUT));
		jkey->kstart_flag = 0;
		
	}

	if (jdata[3] & 0x40)
	{
		//printf("SERVICE\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE;
		ip->ki.wScan = S_SERVICE1;
		SendInput(1, ip, sizeof(INPUT));
		jkey->kservice_flag = 1;
		

	}
	else if (jkey->kservice_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = S_SERVICE1;

		SendInput(1, ip, sizeof(INPUT));
		jkey->kservice_flag = 0;
		
	}

	if (jdata[2] & 0x80)
	{
		//printf("TEST\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE;
		ip->ki.wScan = S_TEST;
		SendInput(1, ip, sizeof(INPUT));
		jkey->ktest_flag = 1;
		

	}
	else if (jkey->ktest_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = S_TEST;

		SendInput(1, ip, sizeof(INPUT));
		jkey->ktest_flag = 0;
		
	}

	if (jdata[3] & 0x02)
	{
		//printf("BUTTON 1\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE;
		ip->ki.wScan = P1_BTN1;
		SendInput(1, ip, sizeof(INPUT));
		jkey->kb1_flag = 1;
		
	}
	else if (jkey->kb1_flag)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = P1_BTN1;
		SendInput(1, ip, sizeof(INPUT));
		jkey->kb1_flag = 0;
		
	}

	if (jdata[3] & 0x01)
	{
		//printf("BUTTON 2\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE;
		ip->ki.wScan = P1_BTN2;
		SendInput(1, ip, sizeof(INPUT));
		jkey->kb2_flag = 1;
		

	}
	else if (jkey->kb2_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = P1_BTN2;

		SendInput(1, ip, sizeof(INPUT));
		jkey->kb2_flag = 0;
		
	}

	if (jdata[4] & 0x80)
	{
		//printf("BUTTON 3\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE;
		ip->ki.wScan = P1_BTN3;
		SendInput(1, ip, sizeof(INPUT));
		jkey->kb3_flag = 1;
		

	}
	else if (jkey->kb3_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = P1_BTN3;

		SendInput(1, ip, sizeof(INPUT));
		jkey->kb3_flag = 0;
		
	}

	if (jdata[4] & 0x40)
	{
		//printf("BUTTON 4\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE;
		ip->ki.wScan = P1_BTN4;
		SendInput(1, ip, sizeof(INPUT));
		jkey->kb4_flag = 1;
		

	}
	else if (jkey->kb4_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = P1_BTN4;

		SendInput(1, ip, sizeof(INPUT));
		jkey->kb4_flag = 0;
		
	}

	if (jdata[4] & 0x20)
	{
		//printf("BUTTON 5\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE;
		ip->ki.wScan = P1_BTN5;
		SendInput(1, ip, sizeof(INPUT));
		jkey->kb5_flag = 1;
		

	}
	else if (jkey->kb5_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = P1_BTN5;

		SendInput(1, ip, sizeof(INPUT));
		jkey->kb5_flag = 0;
		
	}

	if (jdata[4] & 0x10)
	{
		//printf("BUTTON 6\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE;
		ip->ki.wScan = P1_BTN6;
		SendInput(1, ip, sizeof(INPUT));
		jkey->kb6_flag = 1;
		

	}
	else if (jkey->kb6_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = P1_BTN6;

		SendInput(1, ip, sizeof(INPUT));
		jkey->kb6_flag = 0;
		
	}

	if (jdata[5] & 0x20)
	{
		//printf("P2 UP\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE;
		ip->ki.wScan = P2_UP;
		SendInput(1, ip, sizeof(INPUT));
		jkey->k2up_flag = 1;
	}
	else if (jkey->k2up_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = P2_UP;

		SendInput(1, ip, sizeof(INPUT));
		jkey->k2up_flag = 0;
	}

	if (jdata[5] & 0x10)
	{
		//printf("P2 DOWN\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE;
		ip->ki.wScan = P2_DOWN;
		SendInput(1, ip, sizeof(INPUT));
		jkey->k2down_flag = 1;
	}
	else if (jkey->k2down_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = P2_DOWN;

		SendInput(1, ip, sizeof(INPUT));
		jkey->k2down_flag = 0;
	}

	if (jdata[5] & 0x08)
	{
		//printf("P2 LEFT\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE;
		ip->ki.wScan = P2_LEFT;
		SendInput(1, ip, sizeof(INPUT));
		jkey->k2left_flag = 1;
	}
	else if (jkey->k2left_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = P2_LEFT;

		SendInput(1, ip, sizeof(INPUT));
		jkey->k2left_flag = 0;
	}

	if (jdata[5] & 0x04)
	{
		//printf("P2 RIGHT\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE;
		ip->ki.wScan = P2_RIGHT;
		SendInput(1, ip, sizeof(INPUT));
		jkey->k2right_flag = 1;
	}
	else if (jkey->k2right_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = P2_RIGHT;

		SendInput(1, ip, sizeof(INPUT));
		jkey->k2right_flag = 0;
	}

	if (jdata[5] & 0x80)
	{
		//printf("P2 START\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE;
		ip->ki.wScan = P2_START;
		SendInput(1, ip, sizeof(INPUT));
		jkey->k2start_flag = 1;
	}
	else if (jkey->k2start_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = P2_START;

		SendInput(1, ip, sizeof(INPUT));
		jkey->k2start_flag = 0;
	}

	if (jdata[5] & 0x40)
	{
		//printf("P2 SERVICE\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE;
		ip->ki.wScan = S_SERVICE2;
		SendInput(1, ip, sizeof(INPUT));
		jkey->k2service_flag = 1;
	}
	else if (jkey->k2service_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = S_SERVICE2;

		SendInput(1, ip, sizeof(INPUT));
		jkey->k2service_flag = 0;
	}

	if (jdata[5] & 0x02)
	{
		//printf("P2 BUTTON 1\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE;
		ip->ki.wScan = P2_BTN1;
		SendInput(1, ip, sizeof(INPUT));
		jkey->k2b1_flag = 1;
	}
	else if (jkey->k2b1_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = P2_BTN1;

		SendInput(1, ip, sizeof(INPUT));
		jkey->k2b1_flag = 0;
	}

	if (jdata[5] & 0x01)
	{
		//printf("P2 BUTTON 2\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE;
		ip->ki.wScan = P2_BTN2;
		SendInput(1, ip, sizeof(INPUT));
		jkey->k2b2_flag = 1;
	}
	else if (jkey->k2b2_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = P2_BTN2;

		SendInput(1, ip, sizeof(INPUT));
		jkey->k2b2_flag = 0;
	}

	if (jdata[6] & 0x80)
	{
		//printf("P2 BUTTON 3\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE;
		ip->ki.wScan = P2_BTN3;
		SendInput(1, ip, sizeof(INPUT));
		jkey->k2b3_flag = 1;
	}
	else if (jkey->k2b3_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = P2_BTN3;

		SendInput(1, ip, sizeof(INPUT));
		jkey->k2b3_flag = 0;
	}

	if (jdata[6] & 0x40)
	{
		//printf("P2 BUTTON 4\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE;
		ip->ki.wScan = P2_BTN4;
		SendInput(1, ip, sizeof(INPUT));
		jkey->k2b4_flag = 1;
	}
	else if (jkey->k2b4_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = P2_BTN4;

		SendInput(1, ip, sizeof(INPUT));
		jkey->k2b4_flag = 0;
	}

	if (jdata[6] & 0x20)
	{
		//printf("P2 BUTTON 5\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE;
		ip->ki.wScan = P2_BTN5;
		SendInput(1, ip, sizeof(INPUT));
		jkey->k2b5_flag = 1;
	}
	else if (jkey->k2b5_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = P2_BTN5;

		SendInput(1, ip, sizeof(INPUT));
		jkey->k2b5_flag = 0;
	}

	if (jdata[6] & 0x10)
	{
		//printf("P2 BUTTON 6\n");
		ip->ki.dwFlags = KEYEVENTF_SCANCODE;
		ip->ki.wScan = P2_BTN6;
		SendInput(1, ip, sizeof(INPUT));
		jkey->k2b6_flag = 1;
	}
	else if (jkey->k2b6_flag == 1)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = P2_BTN6;

		SendInput(1, ip, sizeof(INPUT));
		jkey->k2b6_flag = 0;
	}



	//printf("first eight bytes from reply buffer: %02x %02x %02x %02x %02x %02x %02x %02x\n", jdata[0], jdata[1], jdata[2], jdata[3], jdata[4], jdata[5], jdata[6], jdata[7]);
	return(1);
}
// ============================================================================
// TOMY EDITION BLOCK (CONFIG, COINS & MAIN) - ORIGINAL TIMING RESTORED
// ============================================================================

FastioConfig g_config;

// Lecture compatible Hexadécimal ("0x06") ET Décimal ("6") depuis l'INI
WORD ReadIniScancode(const TCHAR* section, const TCHAR* key, WORD defaultValue, const TCHAR* iniPath)
{
	TCHAR buf[32] = { 0 };
	GetPrivateProfileString(section, key, _T(""), buf, 32, iniPath);
	if (buf[0] == _T('\0')) return defaultValue;
	return (WORD)_tcstoul(buf, NULL, 0);
}

void LoadConfig()
{
	const TCHAR* iniPath = _T(".\\fastio.ini");

	GetPrivateProfileString(_T("SYSTEM"), _T("COM_PORT"), _T("COM2"), g_config.comPort, 16, iniPath);
	g_config.debugLog = GetPrivateProfileInt(_T("SYSTEM"), _T("DEBUG_LOG"), 0, iniPath);

	// On récupère le multiplicateur et LE délai unique d'origine (par défaut 50ms)
	g_config.coinMultiplier = GetPrivateProfileInt(_T("COIN_SETTINGS"), _T("MULTIPLIER"), 4, iniPath);
	g_config.coinPress = GetPrivateProfileInt(_T("COIN_SETTINGS"), _T("PRESS_MS"), 50, iniPath);

	g_config.sCoin1 = ReadIniScancode(_T("SYSTEM_KEYS"), _T("COIN1"), 0x06, iniPath);
	g_config.sCoin2 = ReadIniScancode(_T("SYSTEM_KEYS"), _T("COIN2"), 0x07, iniPath);
	g_config.sTest = ReadIniScancode(_T("SYSTEM_KEYS"), _T("TEST"), 0x08, iniPath);
	g_config.sService1 = ReadIniScancode(_T("SYSTEM_KEYS"), _T("SERVICE1"), 0x09, iniPath);
	g_config.sService2 = ReadIniScancode(_T("SYSTEM_KEYS"), _T("SERVICE2"), 0x0A, iniPath);
	g_config.sEsc = ReadIniScancode(_T("SYSTEM_KEYS"), _T("ESC"), 0x01, iniPath);
}

// Injection stricte : calquée sur la boucle originale qui fonctionnait en jeu
void InjectCoinPulses(INPUT* ip, WORD scancode)
{
	for (int c = 0; c < g_config.coinMultiplier; c++)
	{
		ip->ki.dwFlags = KEYEVENTF_SCANCODE;
		ip->ki.wScan = scancode;
		SendInput(1, ip, sizeof(INPUT));

		// Maintien
		Sleep(g_config.coinPress);

		ip->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		ip->ki.wScan = scancode;
		SendInput(1, ip, sizeof(INPUT));

		// Pause avant le prochain crédit (utilise exactement le même délai)
		Sleep(g_config.coinPress);
	}
}

// Copie exacte de la fonction GetCoin de derole1 (jdata[3] et jdata[5])
bool GetCoin(JVSKEY* jkey, INPUT* ip, HANDLE hCom, HANDLE hFile, int node)
{
	uint8_t request[4] = { 0x21, 0x02, 0x01, JVS_NODE1 };
	uint8_t jdata[256];

	if (node != 1)
	{
		return false;
	}

	WriteJVS(hCom, node, request, 4);

	if (!ClearCommError(hCom, 0, 0))
	{
		return false;
	}

	if (!EscapeCommFunction(hCom, SETRTS))
	{
		return false;
	}

	Sleep(10);
	ReadJVS(hCom, hFile, jdata, 256);

	// Monnayeur 1 : index original derole1 = jdata[3]
	if (jdata[3] != jkey->coin[0])
	{
		InjectCoinPulses(ip, g_config.sCoin1);
		jkey->coin[0] = jdata[3];
	}

	// Monnayeur 2 : index original derole1 = jdata[5]
	if (jdata[5] != jkey->coin[1])
	{
		InjectCoinPulses(ip, g_config.sCoin2);
		jkey->coin[1] = jdata[5];
	}

	return true;
}

int main()
{
	INPUT ip = { 0 };
	LPDIRECTINPUT8 di;
	JVSKEY jkey = { 0 };
	HANDLE hCom;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	wchar_t ltext[256];

	LoadConfig();

	_tprintf(_T("==================================================\n"));
	_tprintf(_T("  FASTIO - Artax 4.1 JVS Tomy Edition\n"));
	_tprintf(_T("==================================================\n"));
	_tprintf(_T("  [+] COM Port       : %s\n"), g_config.comPort);
	_tprintf(_T("  [+] Coin Multiplier: x%d\n"), g_config.coinMultiplier);
	_tprintf(_T("  [+] Pulse Timing   : %dms\n"), g_config.coinPress);
	_tprintf(_T("  [+] Coin Scancodes : Coin1=0x%02X / Coin2=0x%02X\n"), g_config.sCoin1, g_config.sCoin2);
	_tprintf(_T("  [+] SSD Protection : Log file %s\n"), g_config.debugLog ? _T("ENABLED") : _T("DISABLED"));
	_tprintf(_T("==================================================\n"));
	_tprintf(_T("  Connecting to JVS Master...\n\n"));

	if (g_config.debugLog == 1)
	{
		hFile = CreateFile(_T(".\\cjvs.log"), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	hCom = CreateFile(g_config.comPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (hCom == INVALID_HANDLE_VALUE)
	{
		if (hFile != INVALID_HANDLE_VALUE)
		{
			LogEvent(hFile, ltext, swprintf_s(ltext, sizeof(ltext) / sizeof(wchar_t), L"Main - COM Port Error: %d.\r\n", GetLastError()));
		}
		return 1;
	}

	SetupJVS(&jkey, hCom, hFile);
	SetupJVSKEY(&jkey, &ip, &di);

	while (!init_master(hCom, hFile))
	{
		Sleep(500);
	}

	while (true)
	{
		GetCoin(&jkey, &ip, hCom, hFile, 1);

		if (!PollJVS(&jkey, &ip, hCom, hFile, 1))
		{
			Sleep(100);
			while (!init_master(hCom, hFile))
			{
				Sleep(500);
			}
		}

		Sleep(1);
	}

	return 0;
}
