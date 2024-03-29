#ifndef _YEBISOCKS_TTX_H_
#define _YEBISOCKS_TTX_H_

#include <YCL/DynamicLinkLibrary.h>
using namespace yebisuya;

#include "ProxyWSockHook.h"

extern "C" __declspec(dllexport) BOOL WINAPI TTXBind(WORD Version, TTXExports* exports);

char UILanguageFile[MAX_PATH];

class TTProxy : public DynamicLinkLibrary<TTProxy> {
	enum {
		ID_ABOUTMENU       = 53910,
		ID_PROXYSETUPMENU  = 53310,
		ID_ASYNCMESSAGEBOX = 63001,
	};
	enum {
		OPTION_NONE    = 0,
		OPTION_CLEAR   = 1,
		OPTION_REPLACE = 2,
	};
public:
	TTProxy():initialized(false), showing_error_message(false) {
	}
	bool processAttach() {
		DisableThreadLibraryCalls(GetInstanceHandle());
		return true;
	}
private:
	class Shower : public ProxyWSockHook::MessageShower {
	public:
		virtual void showMessage(const char* message)const {
			add_error_message(message);
		}
	} shower;
	friend class Shower;
	bool initialized;
	bool showing_error_message;
	String error_message;
	String realhost;
	PTTSet ts;
	PComVar cv;
	PReadIniFile ORIG_ReadIniFile;
	PWriteIniFile ORIG_WriteIniFile;
	PParseParam ORIG_ParseParam;

	static void add_error_message(const char* message) {
		if (getInstance().error_message != NULL) {
			StringBuffer buffer = (const TCHAR *)getInstance().error_message;
			buffer.append("\n\n");
			buffer.append(message);
			getInstance().error_message = buffer.toString();
		}else{
			getInstance().error_message = message;
		}
		::PostMessage(getInstance().cv->HWin, WM_COMMAND, ID_ASYNCMESSAGEBOX, 0);
	}
	static void read_options(const char* filename) {
		IniFile inifile(filename, "TTProxy");
		ProxyWSockHook::load(inifile);
		getInstance().initialized = true;
	}
	static void write_options(const char* filename) {
		IniFile inifile(filename, "TTProxy");
		ProxyWSockHook::save(inifile);
	}
	static String get_teraterm_dir_relative_name(char* basename) {
		if (basename[0] == '\\' || basename[0] == '/' || basename[0] != '\0' && basename[1] == ':') {
			return basename;
		}
		char buf[1024];
		::GetModuleFileName(NULL, buf, sizeof buf);
		char* filename = NULL;
		for (char* dst = buf; *dst != '\0'; dst++) {
			if (*dst == '\\' || *dst == '/' || *dst == ':') {
				filename = dst + 1;
			}
		}
		if (filename == NULL)
			return basename;

		*filename = '\0';
		StringBuffer buffer(buf);
		buffer.append(basename);
		return buffer.toString();
	}

	static void PASCAL TTXReadINIFile(PCHAR fileName, PTTSet ts) {
		getInstance().ORIG_ReadIniFile(fileName, ts);
		read_options(fileName);
	}
	static void PASCAL TTXWriteINIFile(PCHAR fileName, PTTSet ts) {
		getInstance().ORIG_WriteIniFile(fileName, ts);
		write_options(fileName);
	}

	static void PASCAL TTXParseParam(PCHAR param, PTTSet ts, PCHAR DDETopic) {
		int param_len=strlen(param);
		char option[1024];
		int opt_len = sizeof(option);
		int action;
		PCHAR start, cur, next;

		memset(&option, '\0', opt_len);

		/* the first term shuld be executable filename of Tera Term */
		start = GetParam(option, opt_len, param);

		cur = start;
		while (next = GetParam(option, opt_len, cur)) {
			DequoteParam(option, opt_len, option);
			action = OPTION_NONE;

			if ((option[0] == '-' || option[0] == '/')) {
				if ((option[1] == 'F' || option[1] == 'f') && option[2] == '=') {
					read_options(get_teraterm_dir_relative_name(option + 3));
				}
			}

			switch (action) {
				case OPTION_CLEAR:
					memset(cur, ' ', next-cur);
					break;
				case OPTION_REPLACE:
					memset(cur, ' ', next-cur);
					memcpy(cur+1, option, strlen(option));
					break;
			}

			cur = next;
		}

		cur = start;
		while (next = GetParam(option, opt_len, cur)) {
			DequoteParam(option, opt_len, option);
			action = OPTION_NONE;

			if ((option[0] == '-' || option[0] == '/')) {
				if (strlen(option + 1) >= 6 && option[6] == '=') {
					option[6] = '\0';
					if (_stricmp(option + 1, "proxy") == 0) {
						ProxyWSockHook::parseURL(option + 7, TRUE);
						action = OPTION_CLEAR;
					}else{
						option[6] = '=';
					}
				}
				else if (_stricmp(option+1, "noproxy") == 0) {
					// -noproxy は -proxy=none:// の別名
					ProxyWSockHook::parseURL("none://", TRUE);
					action = OPTION_CLEAR;
				}
			}else{
				String realhost = ProxyWSockHook::parseURL(option, FALSE);
				if (realhost != NULL) {
					getInstance().realhost = realhost;
					if (realhost.indexOf("://") == -1) {
						action = OPTION_CLEAR;
					}
					else {
						// -proxy= なしで、proto://proxy:port/ 以降の実ホストが含まれていない
						// ttermpro で処理してもらうため、TTXParseParam で消さない
						action = OPTION_REPLACE;
					}
				}
			}

			switch (action) {
				case OPTION_CLEAR:
					memset(cur, ' ', next-cur);
					break;
				case OPTION_REPLACE:
					memset(cur, ' ', next-cur);
					memcpy(cur+1, option, strlen(option));
					break;
			}

			cur = next;
		}

		getInstance().ORIG_ParseParam(param, ts, DDETopic);
		if (getInstance().ts->HostName[0] == '\0' && getInstance().realhost != NULL) {
			strcpy_s(getInstance().ts->HostName, sizeof getInstance().ts->HostName, getInstance().realhost);
		}
	}

	static void copy_UILanguageFile() {
		strncpy_s(UILanguageFile, sizeof(UILanguageFile),
		          getInstance().ts->UILanguageFile, _TRUNCATE);
	}

	static void PASCAL TTXInit(PTTSet ts, PComVar cv) {
		getInstance().ts = ts;
		getInstance().cv = cv;

		ProxyWSockHook::setMessageShower(&getInstance().shower);
	}

	static void PASCAL TTXGetSetupHooks(TTXSetupHooks* hooks) {
		getInstance().ORIG_ReadIniFile = *hooks->ReadIniFile;
		getInstance().ORIG_WriteIniFile = *hooks->WriteIniFile;
		getInstance().ORIG_ParseParam = *hooks->ParseParam;
		*hooks->ReadIniFile = TTXReadINIFile;
		*hooks->WriteIniFile = TTXWriteINIFile;
		*hooks->ParseParam = TTXParseParam;
		copy_UILanguageFile();
	}

	static void PASCAL TTXOpenTCP(TTXSockHooks* hooks) {
		if (!getInstance().initialized) {
			read_options(getInstance().ts->SetupFName);
		}
		(FARPROC&) *hooks->Pconnect = ProxyWSockHook::hook_connect((FARPROC) *hooks->Pconnect);
		(FARPROC&) *hooks->PWSAAsyncGetHostByName = ProxyWSockHook::hook_WSAAsyncGetHostByName((FARPROC) *hooks->PWSAAsyncGetHostByName);
		(FARPROC&) *hooks->PWSAAsyncGetAddrInfo = ProxyWSockHook::hook_WSAAsyncGetAddrInfo((FARPROC) *hooks->PWSAAsyncGetAddrInfo);
		(FARPROC&) *hooks->Pfreeaddrinfo = ProxyWSockHook::hook_freeaddrinfo((FARPROC) *hooks->Pfreeaddrinfo);
		(FARPROC&) *hooks->PWSAAsyncSelect = ProxyWSockHook::hook_WSAAsyncSelect((FARPROC) *hooks->PWSAAsyncSelect);
		(FARPROC&) *hooks->PWSACancelAsyncRequest = ProxyWSockHook::hook_WSACancelAsyncRequest((FARPROC) *hooks->PWSACancelAsyncRequest);
	}

	static void PASCAL TTXCloseTCP(TTXSockHooks* hooks) {
		// unhook functions
		ProxyWSockHook::unhook_connect((FARPROC) *hooks->Pconnect);
		ProxyWSockHook::unhook_WSAAsyncGetHostByName((FARPROC) *hooks->PWSAAsyncGetHostByName);
		ProxyWSockHook::unhook_WSAAsyncGetAddrInfo((FARPROC) *hooks->PWSAAsyncGetAddrInfo);
		ProxyWSockHook::unhook_freeaddrinfo((FARPROC) *hooks->Pfreeaddrinfo);
		ProxyWSockHook::unhook_WSAAsyncSelect((FARPROC) *hooks->PWSAAsyncSelect);
		ProxyWSockHook::unhook_WSACancelAsyncRequest((FARPROC) *hooks->PWSACancelAsyncRequest);
	}

	static void PASCAL TTXModifyMenu(HMENU menu) {
		char uimsg[MAX_UIMSG];
		copy_UILanguageFile();
		/* inserts before ID_HELP_ABOUT */
		UTIL_get_lang_msg("MENU_ABOUT", uimsg, sizeof(uimsg), "About TT&Proxy...");
		InsertMenu(menu, 50990, MF_BYCOMMAND | MF_ENABLED, ID_ABOUTMENU, uimsg);
		/* inserts before ID_SETUP_TCPIP */
		UTIL_get_lang_msg("MENU_SETUP", uimsg, sizeof(uimsg), "&Proxy...");
		InsertMenu(menu, 50360, MF_BYCOMMAND | MF_ENABLED, ID_PROXYSETUPMENU, uimsg);
	}

	static int PASCAL TTXProcessCommand(HWND hWin, WORD cmd) {
		switch (cmd) {
		case ID_ABOUTMENU:
			copy_UILanguageFile();
			SetDialogFont(getInstance().ts->DialogFontName, getInstance().ts->DialogFontPoint, getInstance().ts->DialogFontCharSet,
						  getInstance().ts->UILanguageFile, "TTProxy", "DLG_TAHOMA_FONT");
			ProxyWSockHook::aboutDialog(hWin);
			return 1;
		case ID_PROXYSETUPMENU:
			copy_UILanguageFile();
			SetDialogFont(getInstance().ts->DialogFontName, getInstance().ts->DialogFontPoint, getInstance().ts->DialogFontCharSet,
				getInstance().ts->UILanguageFile, "TTProxy", "DLG_TAHOMA_FONT");
			ProxyWSockHook::setupDialog(hWin);
			return 1;
		case ID_ASYNCMESSAGEBOX:
			if (getInstance().error_message != NULL) {
				String msg = getInstance().error_message;
				getInstance().showing_error_message = true;
				getInstance().error_message = NULL;
				::MessageBox(hWin, msg, "TTProxy", MB_OK | MB_ICONERROR);
				getInstance().showing_error_message = false;
				if (getInstance().error_message != NULL) {
					::PostMessage(hWin, WM_COMMAND, ID_ASYNCMESSAGEBOX, 0);
				}
			}
			return 1;
		default:
			return 0;
		}
	}

	static void PASCAL TTXSetCommandLine(PCHAR cmd, int cmdlen, PGetHNRec rec) {
		String url = ProxyWSockHook::generateURL();
		if (url != NULL) {
			if (strlen(cmd) + 8 + url.length() >= (unsigned) cmdlen)
				return;
			strcat_s(cmd, cmdlen, " -proxy=");
			strcat_s(cmd, cmdlen, url);
		}
	}

	static void PASCAL TTXEnd() {
		if (getInstance().error_message != NULL) {
			::MessageBox(NULL, getInstance().error_message, "TTProxy", MB_OK | MB_ICONERROR);
			getInstance().error_message = NULL;
		}
	}

	friend BOOL PASCAL TTXBind(WORD Version, TTXExports* exports) {
		static const TTXExports EXPORTS = {
			/* This must contain the size of the structure. See below for its usage. */
			sizeof EXPORTS,
			/* load first */
			10,

			/* Now we just list the functions that we've implemented. */
			TTProxy::TTXInit,
			NULL,
			TTProxy::TTXGetSetupHooks,
			TTProxy::TTXOpenTCP,
			TTProxy::TTXCloseTCP,
			NULL,
			TTProxy::TTXModifyMenu,
			NULL,
			TTProxy::TTXProcessCommand,
			TTProxy::TTXEnd,
			TTProxy::TTXSetCommandLine
		};

		int size = sizeof EXPORTS - sizeof exports->size;
		/* do version checking if necessary */
		/* if (Version!=TTVERSION) return FALSE; */

		if (size > exports->size) {
			size = exports->size;
		}
		memcpy((char*) exports + sizeof exports->size,
			(char*) &EXPORTS + sizeof exports->size,
			size);
		return TRUE;
	}
};

#endif//_YEBISOCKS_TTX_H_
