/*
 * (C) 2019 TeraTerm Project
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

#include <windows.h>
#include <tchar.h>
#include <assert.h>
#if !defined(_CRTDBG_MAP_ALLOC)
#define _CRTDBG_MAP_ALLOC
#endif
#include <crtdbg.h>

#include "dllutil.h"

typedef struct {
	const TCHAR *dllName;
	DLLLoadFlag LoadFlag;
	HMODULE handle;
	int refCount;
} HandleList_t;

static HandleList_t *HandleList;
static int HandleListCount;

static HMODULE GetHandle(const TCHAR *dllName, DLLLoadFlag LoadFlag)
{
	TCHAR dllPath[MAX_PATH];
	HMODULE module;
	int i;
	HandleList_t *p;
	int r;

	if (LoadFlag == DLL_GET_MODULE_HANDLE) {
		module = GetModuleHandle(dllName);
		assert(module != NULL);
		return module;
	}

	// �ȑO�Ƀ��[�h����?
	p = HandleList;
	for (i = 0; i < HandleListCount; i++) {
		if (_tcscmp(p->dllName, dllName) == 0) {
			p->refCount++;
			return p->handle;
		}
		p++;
	}

	// �V���Ƀ��[�h����
	dllPath[0] = 0;
	switch (LoadFlag) {
	case DLL_LOAD_LIBRARY_SYSTEM:
		r = GetSystemDirectory(dllPath, _countof(dllPath));
		assert(r != 0);
		if (r == 0) return NULL;
		break;
	case DLL_LOAD_LIBRARY_CURRENT:
		r = GetModuleFileName(NULL, dllPath, _countof(dllPath));
		assert(r != 0);
		if (r == 0) return NULL;
		*_tcsrchr(dllPath, _T('\\')) = 0;
		break;
	default:
		return NULL;
	}
	_tcscat_s(dllPath, _countof(dllPath), _T("\\"));
	_tcscat_s(dllPath, _countof(dllPath), dllName);
	module = LoadLibrary(dllPath);
	if (module == NULL) {
		// ���݂��Ȃ�,dll����Ȃ�?
		return NULL;
	}

	// ���X�g�ɒǉ�
	HandleListCount++;
	HandleList = (HandleList_t *)realloc(HandleList, sizeof(HandleList_t)*HandleListCount);
	p = &HandleList[i];
	p->dllName = _tcsdup(dllName);
	p->handle = module;
	p->LoadFlag = LoadFlag;
	p->refCount = 1;
	return module;
}

static void FreeHandle(const TCHAR *dllName, DLLLoadFlag LoadFlag)
{
	int i;
	HandleList_t *p;

	if (LoadFlag == DLL_GET_MODULE_HANDLE) {
		// ���̂܂ܒu���Ă���
		return;
	}

	// ���X�g����폜����
	p = HandleList;
	for (i = 0; i < HandleListCount; i++) {
		if (_tcscmp(p->dllName, dllName) != 0) {
			continue;
		}

		// ��������
		p->refCount--;
		if (p->refCount > 0) {
			continue;
		}

		// free
		FreeLibrary(p->handle);
		free((void *)p->dllName);
		memcpy(p, p+1, sizeof(*p) + (HandleListCount - i - 1));
		HandleListCount--;
		HandleList = (HandleList_t *)realloc(HandleList, sizeof(HandleList_t)*HandleListCount);
		return;
	}
	// ���X�g�Ɍ�����Ȃ�����
}

/**
 * DLL���̊֐��ւ̃A�h���X���擾����
 * @param[in,out]	pFunc		�֐��ւ̃A�h���X
 *								������Ȃ�����NULL����������
 * @param[in] 		FuncFlag	�֐���������Ȃ��Ƃ��̓���
 *					DLL_ACCEPT_NOT_EXIST	������Ȃ��Ă�ok
 *					DLL_ERROR_NOT_EXIST		������Ȃ��ꍇ�G���[
 * @retval	NO_ERROR				�G���[�Ȃ�
 * @retval	ERROR_FILE_NOT_FOUND	DLL��������Ȃ�(�s���ȃt�@�C��)
 * @retval	ERROR_PROC_NOT_FOUND	�֐��G���g����������Ȃ�
 */
DWORD DLLGetApiAddress(const TCHAR *dllPath, DLLLoadFlag LoadFlag,
					   const char *ApiName, void **pFunc)
{
	HMODULE hDll = GetHandle(dllPath, LoadFlag);
	if (hDll == NULL) {
		*pFunc = NULL;
		return ERROR_FILE_NOT_FOUND;
	} else {
		*pFunc = (void *)GetProcAddress(hDll, ApiName);
		if (*pFunc == NULL) {
			return ERROR_PROC_NOT_FOUND;
		}
		return NO_ERROR; // = 0
	}
}

/**
 * DLL���̕����̊֐��ւ̃A�h���X���擾����
 * @param[in] FuncFlag	�֐���������Ȃ��Ƃ��̓���
 *				DLL_ACCEPT_NOT_EXIST	������Ȃ��Ă�ok
 *				DLL_ERROR_NOT_EXIST		������Ȃ��ꍇ�G���[
 * @retval	NO_ERROR				�G���[�Ȃ�
 * @retval	ERROR_FILE_NOT_FOUND	DLL��������Ȃ�(�s���ȃt�@�C��)
 * @retval	ERROR_PROC_NOT_FOUND	�֐��G���g����������Ȃ�
 */
DWORD DLLGetApiAddressFromList(const TCHAR *dllPath, DLLLoadFlag LoadFlag,
							   DLLFuncFlag FuncFlag, const APIInfo *ApiInfo)
{
	HMODULE hDll = GetHandle(dllPath, LoadFlag);
	if (hDll == NULL) {
		while(ApiInfo->ApiName != NULL) {
			void **func = ApiInfo->func;
			*func = NULL;
			ApiInfo++;
		}
		return ERROR_FILE_NOT_FOUND;
	} else {
		BOOL exist_all = TRUE;
		const APIInfo *p = ApiInfo;

		// �A�h���X�擾
		while(p->ApiName != NULL) {
			void **func = p->func;
			*func = (void *)GetProcAddress(hDll, p->ApiName);
			if (*func == NULL) {
				exist_all = FALSE;
			}
			p++;
		}

		// ���ׂČ������� or ������Ȃ�API�������Ă�ok
		if (exist_all || FuncFlag == DLL_ACCEPT_NOT_EXIST) {
			return NO_ERROR;
		}

		// ������Ȃ�API���������̂ŃG���[
		p = ApiInfo;
		while(p->ApiName != NULL) {
			void **func = p->func;
			*func = NULL;
			p++;
		}
		FreeHandle(dllPath, LoadFlag);
		return ERROR_PROC_NOT_FOUND;
	}
}

void DLLGetApiAddressFromLists(const DllInfo *dllInfos)
{
	while (dllInfos->DllName != NULL) {
		DLLGetApiAddressFromList(dllInfos->DllName,
								 dllInfos->LoadFlag,
								 dllInfos->FuncFlag,
								 dllInfos->APIInfoPtr);
		dllInfos++;
	}
}

static void SetupLoadLibraryPath(void)
{
	BOOL (WINAPI *pSetDefaultDllDirectories)(DWORD);
	BOOL (WINAPI *pSetDllDirectoryA)(LPCSTR);
	const TCHAR *kernel32 = _T("kernel32.dll");

#if !defined(LOAD_LIBRARY_SEARCH_SYSTEM32)
#define LOAD_LIBRARY_SEARCH_SYSTEM32        0x00000800
#endif

	// SetDefaultDllDirectories() ���g����ꍇ�́A
	// �����p�X�� %WINDOWS%\system32 �݂̂ɐݒ肷��
	DLLGetApiAddress(kernel32, DLL_GET_MODULE_HANDLE,
					 "SetDefaultDllDirectories", (void **)&pSetDefaultDllDirectories);
	if (pSetDefaultDllDirectories != NULL) {
		pSetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32);
		return;
	}

	// SetDefaultDllDirectories() ���g���Ȃ��Ă�
	// SetDllDirectory() ���g����ꍇ��
	// �J�����g�f�B���N�g�������ł������p�X����͂����Ă����B
	DLLGetApiAddress(kernel32, DLL_GET_MODULE_HANDLE,
					 "SetDllDirectoryA", (void **)&pSetDllDirectoryA);
	if (pSetDllDirectoryA != NULL) {
		pSetDllDirectoryA("");
	}
}

void DLLInit()
{
	HandleList = NULL;
	HandleListCount = 0;
	SetupLoadLibraryPath();
}

void DLLExit()
{
	int i;
	for (i = 0; i < HandleListCount; i++) {
		HandleList_t *p = &HandleList[i];
		if (p->LoadFlag != DLL_GET_MODULE_HANDLE) {
			FreeLibrary(p->handle);
		}
		free((void *)p->dllName);
	}
	free(HandleList);
	HandleList = NULL;
	HandleListCount = 0;
}
