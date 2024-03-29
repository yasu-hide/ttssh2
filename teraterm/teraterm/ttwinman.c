/*
 * Copyright (C) 1994-1998 T. Teranishi
 * (C) 2005-2019 TeraTerm Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* TERATERM.EXE, variables, flags related to VT win and TEK win */

#include "teraterm.h"
#include "tttypes.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "ttlib.h"
#include "helpid.h"
#include "i18n.h"
#include "commlib.h"

HWND HVTWin = NULL;
HWND HTEKWin = NULL;

int ActiveWin = IdVT; /* IdVT, IdTEK */
int TalkStatus = IdTalkKeyb; /* IdTalkKeyb, IdTalkCB, IdTalkTextFile */
BOOL KeybEnabled = TRUE; /* keyboard switch */
BOOL Connecting = FALSE;

/* 'help' button on dialog box */
WORD MsgDlgHelp;

TTTSet ts;
TComVar cv;

/* pointers to window objects */
void* pTEKWin = NULL;
/* instance handle */
HINSTANCE hInst;

int SerialNo;

void VTActivate()
{
  ActiveWin = IdVT;
  ShowWindow(HVTWin, SW_SHOWNORMAL);
  SetFocus(HVTWin);
}


// タイトルバーのCP932への変換を行う
// 現在、SJIS、EUCのみに対応。
// (2005.3.13 yutaka)
void ConvertToCP932(char *str, int destlen)
{
#define IS_SJIS(n) (ts.KanjiCode == IdSJIS && IsDBCSLeadByte(n))
#define IS_EUC(n) (ts.KanjiCode == IdEUC && (n & 0x80))
	extern WORD PASCAL JIS2SJIS(WORD KCode);
	int len = strlen(str);
	char *cc = _alloca(len + 1);
	char *c = cc;
	int i;
	unsigned char b;
	WORD word;

	if (strcmp(ts.Locale, DEFAULT_LOCALE) == 0) {
		for (i = 0 ; i < len ; i++) {
			b = str[i];
			if (IS_SJIS(b) || IS_EUC(b)) {
				word = b<<8;

				if (i == len - 1) {
					*c++ = b;
					continue;
				}

				b = str[i + 1];
				word |= b;
				i++;

				if (ts.KanjiCode == IdSJIS) {
					// SJISはそのままCP932として出力する

				} else if (ts.KanjiCode == IdEUC) {
					// EUC -> SJIS
					word &= ~0x8080;
					word = JIS2SJIS(word);

				} else if (ts.KanjiCode == IdJIS) {

				} else if (ts.KanjiCode == IdUTF8) {

				} else if (ts.KanjiCode == IdUTF8m) {

				} else {

				}

				*c++ = word >> 8;
				*c++ = word & 0xff;

			} else {
				*c++ = b;
			}
		}

		*c = '\0';
		strncpy_s(str, destlen, cc, _TRUNCATE);
	}
}

// キャプションの変更
//
// (2005.2.19 yutaka) format ID=13の新規追加、COM5以上の表示に対応
// (2005.3.13 yutaka) タイトルのSJISへの変換（日本語）を追加
// (2006.6.15 maya)   ts.KanjiCodeがEUCだと、SJISでもEUCとして
//                    変換してしまうので、ここでは変換しない
// (2007.7.19 maya)   TCP ポート番号 と シリアルポートのボーレートの表示に対応
/*
 *  TitleFormat
 *    0 0 0 0 0 0 (2)
 *    | | | | | +----- displays TCP host/serial port
 *    | | | | +------- displays session no
 *    | | | +--------- displays VT/TEK
 *    | | +----------- displays TCP host/serial port first
 *    | +------------- displays TCP port number
 *    +--------------- displays speed of serial port
 */
void ChangeTitle()
{
	char TempTitle[HostNameMaxLength + TitleBuffSize * 2 + 1]; // バッファ拡張
	char TempTitleWithRemote[TitleBuffSize * 2];

	if (Connecting || !cv.Ready || strlen(cv.TitleRemote) == 0) {
		strncpy_s(TempTitleWithRemote, sizeof(TempTitleWithRemote), ts.Title, _TRUNCATE);
		strncpy_s(TempTitle, sizeof(TempTitle), ts.Title, _TRUNCATE);
	}
	else {
		// リモートからのタイトルを別に扱う (2008.11.1 maya)
		if (ts.AcceptTitleChangeRequest == IdTitleChangeRequestOff) {
			strncpy_s(TempTitleWithRemote, sizeof(TempTitleWithRemote), ts.Title, _TRUNCATE);
		}
		else if (ts.AcceptTitleChangeRequest == IdTitleChangeRequestAhead) {
			_snprintf_s(TempTitleWithRemote, sizeof(TempTitleWithRemote), _TRUNCATE,
			            "%s %s", cv.TitleRemote, ts.Title);
		}
		else if (ts.AcceptTitleChangeRequest == IdTitleChangeRequestLast) {
			_snprintf_s(TempTitleWithRemote, sizeof(TempTitleWithRemote), _TRUNCATE,
			            "%s %s", ts.Title, cv.TitleRemote);
		}
		else {
			strncpy_s(TempTitleWithRemote, sizeof(TempTitleWithRemote), cv.TitleRemote, _TRUNCATE);
		}
		strncpy_s(TempTitle, sizeof(TempTitle), TempTitleWithRemote, _TRUNCATE);
	}

	if ((ts.TitleFormat & 1)!=0)
	{ // host name
		strncat_s(TempTitle,sizeof(TempTitle), " - ",_TRUNCATE);
		if (Connecting) {
			get_lang_msg("DLG_MAIN_TITLE_CONNECTING", ts.UIMsg, sizeof(ts.UIMsg), "[connecting...]", ts.UILanguageFile);
			strncat_s(TempTitle,sizeof(TempTitle),ts.UIMsg,_TRUNCATE);
		}
		else if (! cv.Ready) {
			get_lang_msg("DLG_MAIN_TITLE_DISCONNECTED", ts.UIMsg, sizeof(ts.UIMsg), "[disconnected]", ts.UILanguageFile);
			strncat_s(TempTitle,sizeof(TempTitle),ts.UIMsg,_TRUNCATE);
		}
		else if (cv.PortType==IdSerial)
		{
			// COM5 overに対応
			char str[24]; // COMxxxx:xxxxxxxxxxbps
			if (ts.TitleFormat & 32) {
				_snprintf_s(str, sizeof(str), _TRUNCATE, "COM%d:%ubps", ts.ComPort, ts.Baud);
			}
			else {
				_snprintf_s(str, sizeof(str), _TRUNCATE, "COM%d", ts.ComPort);
			}

			if (ts.TitleFormat & 8) {
				_snprintf_s(TempTitle, sizeof(TempTitle), _TRUNCATE, "%s - %s", str, TempTitleWithRemote);
			} else {
				strncat_s(TempTitle, sizeof(TempTitle), str, _TRUNCATE);
			}
		}
		else if (cv.PortType == IdNamedPipe)
		{
			char str[sizeof(TempTitle)];
			strncpy_s(str, sizeof(str), ts.HostName, _TRUNCATE);

			if (ts.TitleFormat & 8) {
				// format ID = 13(8 + 5): <hots/port> - <title>
				_snprintf_s(TempTitle, sizeof(TempTitle), _TRUNCATE, "%s - %s", str, TempTitleWithRemote);
			}
			else {
				strncat_s(TempTitle, sizeof(TempTitle), str, _TRUNCATE);
			}
		}
		else {
			char str[sizeof(TempTitle)];
			if (ts.TitleFormat & 16) {
				_snprintf_s(str, sizeof(str), _TRUNCATE, "%s:%d", ts.HostName, ts.TCPPort);
			}
			else {
				strncpy_s(str, sizeof(str), ts.HostName, _TRUNCATE);
			}

			if (ts.TitleFormat & 8) {
				// format ID = 13(8 + 5): <hots/port> - <title>
				_snprintf_s(TempTitle, sizeof(TempTitle), _TRUNCATE, "%s - %s", str, TempTitleWithRemote);
			}
			else {
				strncat_s(TempTitle, sizeof(TempTitle), str, _TRUNCATE);
			}
		}
	}

	if ((ts.TitleFormat & 2)!=0)
	{ // serial no.
		char Num[11];
		strncat_s(TempTitle,sizeof(TempTitle)," (",_TRUNCATE);
		_snprintf_s(Num,sizeof(Num),_TRUNCATE,"%u",SerialNo);
		strncat_s(TempTitle,sizeof(TempTitle),Num,_TRUNCATE);
		strncat_s(TempTitle,sizeof(TempTitle),")",_TRUNCATE);
	}

	if ((ts.TitleFormat & 4)!=0) // VT
		strncat_s(TempTitle,sizeof(TempTitle)," VT",_TRUNCATE);

	SetWindowText(HVTWin,TempTitle);

	if (HTEKWin!=0)
	{
		if ((ts.TitleFormat & 4)!=0) // TEK
		{
			strncat_s(TempTitle,sizeof(TempTitle)," TEK",_TRUNCATE);
		}
		SetWindowText(HTEKWin,TempTitle);
	}
}

void SwitchMenu()
{
  HWND H1, H2;

  if (ActiveWin==IdVT)
  {
    H1 = HTEKWin;
    H2 = HVTWin;
  }
  else {
    H1 = HVTWin;
    H2 = HTEKWin;
  }

  if (H1!=0)
    PostMessage(H1,WM_USER_CHANGEMENU,0,0);
  if (H2!=0)
    PostMessage(H2,WM_USER_CHANGEMENU,0,0);
}

void SwitchTitleBar()
{
  HWND H1, H2;

  if (ActiveWin==IdVT)
  {
    H1 = HTEKWin;
    H2 = HVTWin;
  }
  else {
    H1 = HVTWin;
    H2 = HTEKWin;
  }

  if (H1!=0)
    PostMessage(H1,WM_USER_CHANGETBAR,0,0);
  if (H2!=0)
    PostMessage(H2,WM_USER_CHANGETBAR,0,0);
}

HMODULE LoadHomeDLL(const char *DLLname)
{
	char DLLpath[MAX_PATH];
	_snprintf_s(DLLpath, sizeof(DLLpath), _TRUNCATE, "%s\\%s", ts.HomeDir, DLLname);
	return LoadLibrary(DLLpath);
}
