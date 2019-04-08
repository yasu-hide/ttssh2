/*
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

/* Routines for dialog boxes */

#include <windows.h>
#include <crtdbg.h>

#include "dlglib.h"
#include "ttlib.h"

#if defined(_DEBUG) && !defined(_CRTDBG_MAP_ALLOC)
#define malloc(l) _malloc_dbg((l), _NORMAL_BLOCK, __FILE__, __LINE__)
#define free(p)   _free_dbg((p), _NORMAL_BLOCK)
#endif

// �_�C�A���O���[�_����Ԃ̎��AOnIdle()�����s����
//#define ENABLE_CALL_IDLE_MODAL	1

extern BOOL CallOnIdle(LONG lCount);

typedef struct {
	DLGPROC OrigProc;	// Dialog proc
	LONG_PTR OrigUser;	// DWLP_USER
	LPARAM ParamInit;
	int DlgResult;
	bool EndDialogFlag;
} TTDialogData;

static TTDialogData *TTDialogTmpData;

#if ENABLE_CALL_IDLE_MODAL
static int TTDoModal(HWND hDlgWnd)
{
	LONG lIdleCount = 0;
	MSG Msg;
	TTDialogData *data = (TTDialogData *)GetWindowLongPtr(hDlgWnd, DWLP_USER);

	for (;;)
	{
		if (!IsWindow(hDlgWnd)) {
			// �E�C���h�E������ꂽ
			return IDCANCEL;
		}
#if defined(_DEBUG)
		if (!IsWindowVisible(hDlgWnd)) {
			// �����EndDialog()���g��ꂽ? -> TTEndDialog()���g������
			::ShowWindow(hDlgWnd, SW_SHOWNORMAL);
		}
#endif
		if (data->EndDialogFlag) {
			// TTEndDialog()���Ă΂ꂽ
			return data->DlgResult;
		}

		if(!::PeekMessage(&Msg, NULL, NULL, NULL, PM_NOREMOVE))
		{
			// ���b�Z�[�W���Ȃ�
			// OnIdel() ����������
			if (!CallOnIdle(lIdleCount++)) {
				// Idle�������Ȃ��Ȃ���
				lIdleCount = 0;
				Sleep(10);
			}
			continue;
		}
		else
		{
			// ���b�Z�[�W������

			// pump message
			BOOL quit = !::GetMessage(&Msg, NULL, NULL, NULL);
			if (quit) {
				// QM_QUIT
				PostQuitMessage(0);
				return IDCANCEL;
			}

			if (!::IsDialogMessage(hDlgWnd, &Msg)) {
				// �_�C�A���O�ȊO�̏���
				::TranslateMessage(&Msg);
				::DispatchMessage(&Msg);
			}
		}
	}

	// �����ɂ͗��Ȃ�
	return IDOK;
}
#endif

static INT_PTR CALLBACK TTDialogProc(
	HWND hDlgWnd, UINT msg,
	WPARAM wParam, LPARAM lParam)
{
	TTDialogData *data = (TTDialogData *)GetWindowLongPtr(hDlgWnd, DWLP_USER);
	if (msg == WM_INITDIALOG) {
		data = (TTDialogData *)lParam;
		SetWindowLongPtr(hDlgWnd, DWLP_USER, (LONG_PTR)lParam);
		lParam = data->ParamInit;
	}

	if (data == NULL) {
		// WM_INITDIALOG�����O�͐ݒ肳��Ă��Ȃ�
		data = TTDialogTmpData;
	} else {
		// TTEndDialog()���Ă΂ꂽ�Ƃ��ADWLP_USER ���Q�Ƃł��Ȃ�
		TTDialogTmpData = data;
	}

	SetWindowLongPtr(hDlgWnd, DWLP_DLGPROC, (LONG_PTR)data->OrigProc);
	SetWindowLongPtr(hDlgWnd, DWLP_USER, (LONG_PTR)data->OrigUser);
	LRESULT Result = data->OrigProc(hDlgWnd, msg, wParam, lParam);
	data->OrigProc = (DLGPROC)GetWindowLongPtr(hDlgWnd, DWLP_DLGPROC);
	data->OrigUser = GetWindowLongPtr(hDlgWnd, DWLP_USER);
	SetWindowLongPtr(hDlgWnd, DWLP_DLGPROC, (LONG_PTR)TTDialogProc);
	SetWindowLongPtr(hDlgWnd, DWLP_USER, (LONG_PTR)data);

	if (msg == WM_NCDESTROY) {
		SetWindowLongPtr(hDlgWnd, DWLP_USER, 0);
		free(data);
	}

	return Result;
}

/**
 *	EndDialog() �݊��֐�
 */
BOOL TTEndDialog(HWND hDlgWnd, INT_PTR nResult)
{
#if ENABLE_CALL_IDLE_MODAL
	TTDialogData *data = TTDialogTmpData;
	data->DlgResult = nResult;
	data->EndDialogFlag = true;
	return TRUE;
#else
	return EndDialog(hDlgWnd, nResult);
#endif
}

/**
 *	CreateDialogIndirectParam() �݊��֐�
 */
HWND TTCreateDialogIndirectParam(
	HINSTANCE hInstance,
	LPCTSTR lpTemplateName,
	HWND hWndParent,			// �I�[�i�[�E�B���h�E�̃n���h��
	DLGPROC lpDialogFunc,		// �_�C�A���O�{�b�N�X�v���V�[�W���ւ̃|�C���^
	LPARAM lParamInit)			// �������l
{
	TTDialogData *data = (TTDialogData *)malloc(sizeof(TTDialogData));
	data->OrigProc = lpDialogFunc;
	data->OrigUser = 0;
	data->ParamInit = lParamInit;
	data->EndDialogFlag = false;
	data->DlgResult = 0;
	DLGTEMPLATE *lpTemplate = TTGetDlgTemplate(hInstance, lpTemplateName);
	TTDialogTmpData = data;
	HWND hDlgWnd = CreateDialogIndirectParam(
		hInstance, lpTemplate, hWndParent, TTDialogProc, (LPARAM)data);
	TTDialogTmpData = NULL;
	ShowWindow(hDlgWnd, SW_SHOW);
    UpdateWindow(hDlgWnd);
	free(lpTemplate);
	return hDlgWnd;
}

/**
 *	CreateDialog() �݊��֐�
 */
HWND TTCreateDialog(
	HINSTANCE hInstance,
	LPCTSTR lpTemplateName,
	HWND hWndParent,
	DLGPROC lpDialogFunc)
{
	return TTCreateDialogIndirectParam(hInstance, lpTemplateName,
									   hWndParent, lpDialogFunc, NULL);
}

/**
 *	DialogBoxParam() �݊��֐�
 *		EndDialog()�ł͂Ȃ��ATTEndDialog()���g�p���邱��
 */
INT_PTR TTDialogBoxParam(
	HINSTANCE hInstance,
	LPCTSTR lpTemplateName,
	HWND hWndParent,			// �I�[�i�[�E�B���h�E�̃n���h��
	DLGPROC lpDialogFunc,		// �_�C�A���O�{�b�N�X�v���V�[�W���ւ̃|�C���^
	LPARAM lParamInit)			// �������l
{
#if ENABLE_CALL_IDLE_MODAL
	HWND hDlgWnd = TTCreateDialogIndirectParam(
		hInstance, lpTemplateName,
		hWndParent, lpDialogFunc, lParamInit);
	EnableWindow(hWndParent, FALSE);
	int DlgResult = TTDoModal(hDlgWnd);
	::DestroyWindow(hDlgWnd);
	EnableWindow(hWndParent, TRUE);
	return DlgResult;
#else
	DLGTEMPLATE *lpTemplate = TTGetDlgTemplate(hInstance, lpTemplateName);
	INT_PTR DlgResult = DialogBoxIndirectParam(
		hInstance, lpTemplate, hWndParent,
		lpDialogFunc, lParamInit);
	free(lpTemplate);
	return DlgResult;
#endif
}

/**
 *	DialogBoxParam() �݊��֐�
 *		EndDialog()�ł͂Ȃ��ATTEndDialog()���g�p���邱��
 */
INT_PTR TTDialogBox(
	HINSTANCE hInstance,
	LPCTSTR lpTemplateName,
	HWND hWndParent,
	DLGPROC lpDialogFunc)
{
	return TTDialogBoxParam(
		hInstance, lpTemplateName,
		hWndParent, lpDialogFunc, (LPARAM)NULL);
}

/**
 *	�g�p����_�C�A���O�t�H���g�����肷��
 */
void SetDialogFont(const char *SetupFName,
				   const char *UILanguageFile, const char *Section)
{
	// teraterm.ini�̎w��
	if (SetupFName != NULL) {
		LOGFONTA logfont;
		BOOL result;
		result = GetI18nLogfont("Tera Term", "DlgFont", &logfont, 0, SetupFName);
		if (result == TRUE) {
			result = IsExistFontA(logfont.lfFaceName, logfont.lfCharSet, TRUE);
			if (result == TRUE) {
				TTSetDlgFontA(logfont.lfFaceName, logfont.lfHeight, logfont.lfCharSet);
				return;
			}
		}
	}

	// .lng�̎w��
	if (UILanguageFile != NULL) {
		static const char *dlg_font_keys[] = {
			"DLG_FONT",
			"DLG_TAHOMA_FONT",
			"DLG_SYSTEM_FONT",
		};
		BOOL result = FALSE;
		LOGFONTA logfont;
		size_t i;
		if (Section != NULL) {
			for (i = 0; i < _countof(dlg_font_keys); i++) {
				result = GetI18nLogfont(Section, dlg_font_keys[i], &logfont, 0, UILanguageFile);
				if (result == FALSE) {
					continue;
				}
				if (logfont.lfFaceName[0] == '\0') {
					break;
				}
				if (IsExistFontA(logfont.lfFaceName, logfont.lfCharSet, TRUE)) {
					break;
				}
			}
		}
		if (result == FALSE) {
			for (i = 0; i < _countof(dlg_font_keys); i++) {
				result = GetI18nLogfont("Tera Term", dlg_font_keys[i], &logfont, 0, UILanguageFile);
				if (result == FALSE) {
					continue;
				}
				if (logfont.lfFaceName[0] == '\0') {
					break;
				}
				if (IsExistFontA(logfont.lfFaceName, logfont.lfCharSet, TRUE)) {
					break;
				}
			}
		}
		if (result == TRUE) {
			TTSetDlgFontA(logfont.lfFaceName, logfont.lfHeight, logfont.lfCharSet);
			return;
		}
	}

	// ini,lng�Ŏw�肳�ꂽ�t�H���g��������Ȃ������Ƃ��A
	// ���������Ő������\������Ȃ����ԂƂȂ�
	// messagebox()�̃t�H���g���Ƃ肠�����I�����Ă���
	{
		LOGFONT logfont;
		GetMessageboxFont(&logfont);
		TTSetDlgFont(logfont.lfFaceName, logfont.lfHeight, logfont.lfCharSet);
	}
}
