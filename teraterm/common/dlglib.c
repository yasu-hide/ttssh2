/*
 * Copyright (C) 1994-1998 T. Teranishi
 * (C) 2008-2019 TeraTerm Project
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

/* Routines for dialog boxes */
#include "dlglib.h"

#include "i18n.h"
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <commctrl.h>
#include <tchar.h>
#include <crtdbg.h>
#include "ttlib.h"	// for get_lang_font()

#if defined(_DEBUG) && !defined(_CRTDBG_MAP_ALLOC)
#define malloc(l) _malloc_dbg((l), _NORMAL_BLOCK, __FILE__, __LINE__)
#define free(p)   _free_dbg((p), _NORMAL_BLOCK)
#endif

void EnableDlgItem(HWND HDlg, int FirstId, int LastId)
{
	int i;
	HWND HControl;

	for (i = FirstId ; i <= LastId ; i++) {
		HControl = GetDlgItem(HDlg, i);
		EnableWindow(HControl,TRUE);
	}
}

void DisableDlgItem(HWND HDlg, int FirstId, int LastId)
{
	int i;
	HWND HControl;

	for (i = FirstId ; i <= LastId ; i++) {
		HControl = GetDlgItem(HDlg, i);
		EnableWindow(HControl,FALSE);
	}
}

void ShowDlgItem(HWND HDlg, int FirstId, int LastId)
{
	int i;
	HWND HControl;

	for (i = FirstId ; i <= LastId ; i++) {
		HControl = GetDlgItem(HDlg, i);
		ShowWindow(HControl,SW_SHOW);
	}
}

void SetRB(HWND HDlg, int R, int FirstId, int LastId)
{
	HWND HControl;
	DWORD Style;

	if ( R<1 ) {
		return;
	}
	if ( FirstId+R-1 > LastId ) {
		return;
	}
	HControl = GetDlgItem(HDlg, FirstId + R - 1);
	SendMessage(HControl, BM_SETCHECK, 1, 0);
	Style = GetClassLongPtr(HControl, GCL_STYLE);
	SetClassLongPtr(HControl, GCL_STYLE, Style | WS_TABSTOP);
}

void GetRB(HWND HDlg, LPWORD R, int FirstId, int LastId)
{
	int i;

	*R = 0;
	for (i = FirstId ; i <= LastId ; i++) {
		if (SendDlgItemMessage(HDlg, i, BM_GETCHECK, 0, 0) != 0) {
			*R = (WORD)(i - FirstId + 1);
			return;
		}
	}
}

void SetDlgNum(HWND HDlg, int id_Item, LONG Num)
{
	TCHAR Temp[16];

	/* In Win16, SetDlgItemInt can not be used to display long integer. */
	_sntprintf_s(Temp, _countof(Temp), _TRUNCATE, _T("%d"), Num);
	SetDlgItemText(HDlg,id_Item,Temp);
}

void InitDlgProgress(HWND HDlg, int id_Progress, int *CurProgStat) {
	HWND HProg;
	HProg = GetDlgItem(HDlg, id_Progress);

	*CurProgStat = 0;

	SendMessage(HProg, PBM_SETRANGE, (WPARAM)0, MAKELPARAM(0, 100));
	SendMessage(HProg, PBM_SETSTEP, (WPARAM)1, 0);
	SendMessage(HProg, PBM_SETPOS, (WPARAM)0, 0);

	ShowWindow(HProg, SW_SHOW);
	return;
}

void SetDlgPercent(HWND HDlg, int id_Item, int id_Progress, LONG a, LONG b, int *p)
{
	// 20MB以上のファイルをアップロードしようとすると、buffer overflowで
	// 落ちる問題への対処。(2005.3.18 yutaka)
	// cf. http://sourceforge.jp/tracker/index.php?func=detail&aid=5713&group_id=1412&atid=5333
	double Num;
	TCHAR NumStr[10];

	if (b==0) {
		Num = 100.0;
	}
	else {
		Num = 100.0 * (double)a / (double)b;
	}
	_sntprintf_s(NumStr, _countof(NumStr),_TRUNCATE,_T("%3.1f%%"),Num);
	SetDlgItemText(HDlg, id_Item, NumStr);

	if (id_Progress != 0 && p != NULL && *p >= 0 && (double)*p < Num) {
		*p = (int)Num;
		SendMessage(GetDlgItem(HDlg, id_Progress), PBM_SETPOS, (WPARAM)*p, 0);
	}
}

void SetDlgTime(HWND HDlg, int id_Item, DWORD stime, int bytes)
{
	static int prev_elapsed;
	int elapsed, rate;
	TCHAR buff[64];

	elapsed = (GetTickCount() - stime) / 1000;

	if (elapsed == 0) {
		prev_elapsed = 0;
		SetDlgItemText(HDlg, id_Item, _T("0:00"));
		return;
	}

	if (elapsed == prev_elapsed) {
		return;
	}
	prev_elapsed = elapsed;

	rate = bytes / elapsed;
	if (rate < 1200) {
		_sntprintf_s(buff, _countof(buff), _TRUNCATE, _T("%d:%02d (%dBytes/s)"), elapsed / 60, elapsed % 60, rate);
	}
	else if (rate < 1200000) {
		_sntprintf_s(buff, _countof(buff), _TRUNCATE, _T("%d:%02d (%d.%02dKB/s)"), elapsed / 60, elapsed % 60, rate / 1000, rate / 10 % 100);
	}
	else {
		_sntprintf_s(buff, _countof(buff), _TRUNCATE, _T("%d:%02d (%d.%02dMB/s)"), elapsed / 60, elapsed % 60, rate / (1000 * 1000), rate / 10000 % 100);
	}

	SetDlgItemText(HDlg, id_Item, buff);
}

void SetDropDownList(HWND HDlg, int Id_Item, const char *List[], int nsel)
{
	int i;

	i = 0;
	while (List[i] != NULL) {
		SendDlgItemMessageA(HDlg, Id_Item, CB_ADDSTRING,
							0, (LPARAM)List[i]);
		i++;
	}
	SendDlgItemMessage(HDlg, Id_Item, CB_SETCURSEL,nsel-1,0);
}

#if defined(UNICODE)
void SetDropDownListW(HWND HDlg, int Id_Item, const wchar_t *List[], int nsel)
{
	int i;

	i = 0;
	while (List[i] != NULL) {
		SendDlgItemMessageW(HDlg, Id_Item, CB_ADDSTRING,
							0, (LPARAM)List[i]);
		i++;
	}
	SendDlgItemMessage(HDlg, Id_Item, CB_SETCURSEL,nsel-1,0);
}
#endif

LONG GetCurSel(HWND HDlg, int Id_Item)
{
	LRESULT n;

	n = SendDlgItemMessage(HDlg, Id_Item, CB_GETCURSEL, 0, 0);
	if (n==CB_ERR) {
		n = 0;
	}
	else {
		n++;
	}

	return (LONG)n;
}

typedef struct {
	WNDPROC OrigProc;	// Original window procedure
	LONG_PTR OrigUser;	// DWLP_USER
	BOOL ComboBox;
} EditSubclassData;

// C-n/C-p のためにサブクラス化 (2007.9.4 maya)
// C-p/C-n/C-b/C-f/C-a/C-e をサポート (2007.9.5 maya)
// C-d/C-k をサポート (2007.10.3 yutaka)
// ドロップダウンの中のエディットコントロールを
// サブクラス化するためのウインドウプロシージャ
static LRESULT CALLBACK HostnameEditProc(HWND dlg, UINT msg,
                                         WPARAM wParam, LPARAM lParam)
{
	EditSubclassData *data = (EditSubclassData *)GetWindowLongPtr(dlg, GWLP_USERDATA);
	LRESULT Result;
	LRESULT select;
	LRESULT max;
	int len;
	char *str, *orgstr;

	switch (msg) {
		// キーが押されたのを検知する
		case WM_KEYDOWN:
			if (GetKeyState(VK_CONTROL) < 0) {
				switch (wParam) {
					case 0x50: // Ctrl+p ... up
						if (data->ComboBox) {
							HWND parent = GetParent(dlg);
							select = SendMessage(parent, CB_GETCURSEL, 0, 0);
							if (select > 0) {
								PostMessage(parent, CB_SETCURSEL, select - 1, 0);
							}
							return 0;
						}
						break;
					case 0x4e: // Ctrl+n ... down
						if (data->ComboBox) {
							HWND parent = GetParent(dlg);
							max = SendMessage(parent, CB_GETCOUNT, 0, 0);
							select = SendMessage(parent, CB_GETCURSEL, 0, 0);
							if (select < max - 1) {
								PostMessage(parent, CB_SETCURSEL, select + 1, 0);
							}
							return 0;
						}
						break;
					case 0x42: // Ctrl+b ... left
						SendMessage(dlg, EM_GETSEL, 0, (LPARAM)&select);
						PostMessage(dlg, EM_SETSEL, select-1, select-1);
						return 0;
					case 0x46: // Ctrl+f ... right
						SendMessage(dlg, EM_GETSEL, 0, (LPARAM)&select);
						max = GetWindowTextLength(dlg) ;
						PostMessage(dlg, EM_SETSEL, select+1, select+1);
						return 0;
					case 0x41: // Ctrl+a ... home
						PostMessage(dlg, EM_SETSEL, 0, 0);
						return 0;
					case 0x45: // Ctrl+e ... end
						max = GetWindowTextLength(dlg) ;
						PostMessage(dlg, EM_SETSEL, max, max);
						return 0;

					case 0x44: // Ctrl+d
					case 0x4b: // Ctrl+k
					case 0x55: // Ctrl+u
						SendMessage(dlg, EM_GETSEL, 0, (LPARAM)&select);
						max = GetWindowTextLength(dlg);
						max++; // '\0'
						orgstr = str = (char *)malloc(max);
						if (str != NULL) {
							len = GetWindowTextA(dlg, str, (int)max);
							if (select >= 0 && select < len) {
								if (wParam == 0x44) { // カーソル配下の文字のみを削除する
									memmove(&str[select], &str[select + 1], len - select - 1);
									str[len - 1] = '\0';

								} else if (wParam == 0x4b) { // カーソルから行末まで削除する
									str[select] = '\0';

								}
							}

							if (wParam == 0x55) { // カーソルより左側をすべて消す
								if (select >= len) {
									str[0] = '\0';
								} else {
									str = &str[select];
								}
								select = 0;
							}

							SetWindowTextA(dlg, str);
							SendMessage(dlg, EM_SETSEL, select, select);
							free(orgstr);
							return 0;
						}
						break;
				}
			}
			break;

		// 上のキーを押した結果送られる文字で音が鳴るので捨てる
		case WM_CHAR:
			switch (wParam) {
				case 0x01:
				case 0x02:
				case 0x04:
				case 0x05:
				case 0x06:
				case 0x0b:
				case 0x0e:
				case 0x10:
				case 0x15:
					return 0;
			}
			break;
	}

	SetWindowLongPtr(dlg, GWLP_WNDPROC, (LONG_PTR)data->OrigProc);
	SetWindowLongPtr(dlg, GWLP_USERDATA, (LONG_PTR)data->OrigUser);
	Result = CallWindowProc(data->OrigProc, dlg, msg, wParam, lParam);
	data->OrigProc = (WNDPROC)GetWindowLongPtr(dlg, GWLP_WNDPROC);
	data->OrigUser = GetWindowLongPtr(dlg, GWLP_USERDATA);
	SetWindowLongPtr(dlg, GWLP_WNDPROC, (LONG_PTR)HostnameEditProc);
	SetWindowLongPtr(dlg, GWLP_USERDATA, (LONG_PTR)data);

	switch (msg) {
		case WM_NCDESTROY:
			SetWindowLongPtr(dlg, GWLP_WNDPROC, (LONG_PTR)data->OrigProc);
			SetWindowLongPtr(dlg, GWLP_USERDATA, (LONG_PTR)data->OrigUser);
			free(data);
			break;
	}

	return Result;
}

// C-n/C-p のためにサブクラス化
void SetEditboxSubclass(HWND hDlg, int nID, BOOL ComboBox)
{
	EditSubclassData *data;
	HWND hWndEdit = GetDlgItem(hDlg, nID);
	if (ComboBox) {
		hWndEdit = GetWindow(hWndEdit, GW_CHILD);
	}
	data = (EditSubclassData *)malloc(sizeof(EditSubclassData));
	data->OrigProc = (WNDPROC)GetWindowLongPtr(hWndEdit, GWLP_WNDPROC);
	data->OrigUser = (LONG_PTR)GetWindowLongPtr(hWndEdit, GWLP_USERDATA);
	data->ComboBox = ComboBox;
	SetWindowLongPtr(hWndEdit, GWLP_WNDPROC, (LONG_PTR)HostnameEditProc);
	SetWindowLongPtr(hWndEdit, GWLP_USERDATA, (LONG_PTR)data);
}

typedef struct {
	BOOL found;
	const char *face;
	BYTE charset;
} IsExistFontInfoA;

static int CALLBACK IsExistFontSubA(
	const ENUMLOGFONTA* lpelf, const NEWTEXTMETRICA* lpntm,
	int nFontType, LPARAM lParam)
{
	IsExistFontInfoA *info = (IsExistFontInfoA *)lParam;
	(void)lpntm;
	if (nFontType != DEVICE_FONTTYPE &&
		_stricmp(lpelf->elfLogFont.lfFaceName, info->face) == 0 &&
		lpelf->elfLogFont.lfCharSet == info->charset)
	{
		info->found = TRUE;
		return 0;
	}
	return 1;
}

/**
 *	IsExistFont
 *	フォントが存在しているかチェックする
 *
 *	@param[in]	face		フォント名(ファイル名ではない)
 *	@param[in]	charset		SHIFTJIS_CHARSETなど
 *	@param[in]	strict		TRUE	フォントリンクは検索に含めない
 *							FALSE	フォントリンクも検索に含める
 *	@retval		FALSE		フォントはしない
 *	@retval		TRUE		フォントは存在する
 *
 *	strict = FALSE時、存在しないフォントでも表示できるならTRUEが返る
 */
BOOL IsExistFontA(const char *face, BYTE charset, BOOL strict)
{
	HDC hDC = GetDC(NULL);
	LOGFONTA lf;
	IsExistFontInfoA info;
	memset(&lf, 0, sizeof(lf));
	lf.lfCharSet = !strict ? DEFAULT_CHARSET : charset;
	// ↑DEFAULT_CHARSETとするとフォントリンクも有効になるようだ
	lf.lfPitchAndFamily = 0;
	info.found = FALSE;
	info.face = face;
	info.charset = charset;
	EnumFontFamiliesExA(hDC, &lf, (FONTENUMPROCA)IsExistFontSubA, (LPARAM)&info, 0);
	ReleaseDC(NULL, hDC);
	return info.found;
}
