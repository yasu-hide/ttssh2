/*
 * Copyright (C) 2018-2019 TeraTerm Project
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

#pragma once

#include <tchar.h>

#ifdef __cplusplus
extern "C" {
#endif

// MultiByteToWideChar() wrappers
//	CP932(shift-jis) to wchar(UTF-16)
int CP932ToWideChar(const char *cp932_ptr, int cp932_len, wchar_t *wstr_ptr, int wstr_len);

// API wrappers
char *_WideCharToMultiByte(const wchar_t *wstr_ptr, size_t wstr_len, int code_page, size_t *mb_len_);
wchar_t *_MultiByteToWideChar(const char *str_ptr, size_t str_len, int code_page, size_t *w_len_);

// convinience funcs  (for windows api params)
const char *ToCharA(const char *strA);
const char *ToCharW(const wchar_t *strW);
const char *ToCharU8(const char *strU8);
const wchar_t *ToWcharA(const char *strA);
const wchar_t *ToWcharW(const wchar_t *strW);
const wchar_t *ToWcharU8(const char *strU8);
const char *ToU8A(const char *strA);
const char *ToU8W(const wchar_t *strW);

#if defined(_UNICODE)
#define ToTcharA(s)		ToWcharA(s)
#define ToTcharW(s)		ToWcharW(s)
#define ToTcharU8(s)	ToWcharU8(s)
#define ToCharT(s)		ToCharW(s)
#define ToU8T(s)		ToU8W(s)
#else
#define ToTcharA(s)		ToCharA(s)
#define ToTcharW(s)		ToCharW(s)
#define ToTcharU8(s)	ToCharU8(s)
#define ToCharT(s)		ToCharA(s)
#define ToU8T(s)		ToU8A(s)
#endif

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#if defined(__GNUC__) || (defined(_MSC_VER) && (_MSC_VER > 1910))
#define	MOVE_CONSTRUCTOR_ENABLE
#endif

class u8
{
public:
	u8();
	u8(const char *strA);
	u8(const char *strA, int code_page);
	u8(const wchar_t *strW);
	u8(const u8 &obj);
#if defined(MOVE_CONSTRUCTOR_ENABLE)
	u8(u8 &&obj) noexcept;
#endif
	~u8();
	u8& operator=(const char *strA);
	u8& operator=(const wchar_t *strW);
	u8& operator=(const u8 &obj);
#if defined(MOVE_CONSTRUCTOR_ENABLE)
	u8& operator=(u8 &&obj) noexcept;
#endif
	operator const char *() const;
	const char *cstr() const;
private:
	char *u8str_;
	void assign(const char *strA, int code_page);
	void assign(const wchar_t *strW);
	void copy(const u8 &obj);
	void move(u8 &obj);
};

class tc
{
public:
	tc();
	tc(const char *strA);
	tc(const char *strA, int code_page);
	tc(const wchar_t *strW);
	tc(const tc &obj);
#if defined(MOVE_CONSTRUCTOR_ENABLE)
	tc(tc &&obj) noexcept;
#endif
	~tc();
	tc& operator=(const char *strA);
	tc& operator=(const wchar_t *strW);
	tc& operator=(const tc &obj);
#if defined(MOVE_CONSTRUCTOR_ENABLE)
	tc& operator=(tc &&obj) noexcept;
#endif
	static tc fromUtf8(const char *strU8);
	operator const TCHAR *() const;
	const TCHAR *cstr() const;
private:
	TCHAR *tstr_;
	void assign(const char *strA, int code_page);
	void assign(const wchar_t *strW);
	void copy(const tc &obj);
	void move(tc &obj);
};
#endif
