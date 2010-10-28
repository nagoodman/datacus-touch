/*
 Touch2Tuio - Windows 7 Touch to TUIO Bridge
 http://dm.tzi.de/research/hci/touch2tuio
 
 Copyright (c) 2010 Marc Herrlich and Benjamin Walther-Franks, Research Group Digital Media, TZI, University of Bremen
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// TouchHook.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "TouchHook.h"

// fwd declaration
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

// from dllmain.cpp
extern HMODULE	g_this;
extern DWORD	g_consoleId;
extern int		g_serverPort;
extern char		g_host[MAX_NAME_LENGTH];
extern HWND		g_targetWnd;
extern HHOOK	g_hook;
extern HHOOK	g_mouseHook;

// process private data
const unsigned int TIMER_IDENTIFIER(123);
const unsigned int TIMER_CALL_TIME(32);
const unsigned int MAX_CURSOR_IDLE_TIME(50);
const ULONG TOUCH_FLAGS(/*TWF_FINETOUCH|*/TWF_WANTPALM);

bool				s_initialized(false);
HANDLE				s_out(0);
UINT_PTR			s_timer(0);

TUIO::TuioServer*	s_server(0);
std::map<DWORD, TUIO::TuioCursor*> *s_cursors(0);
std::map<DWORD, TUIO::TuioCursor*> *s_cursorBuf(0);

unsigned int		s_screen_width(0);
unsigned int		s_screen_height(0);

char				s_buf[256] = "";
DWORD				s_ccount(0);


// This is an example of an exported variable
//TOUCHHOOK_API int nTouchHook=0;

// This is an example of an exported function.
//TOUCHHOOK_API int fnTouchHook(void)
//{
//	return 42;
//}

// This is the constructor of a class that has been exported.
// see TouchHook.h for the class definition
//CTouchHook::CTouchHook()
//{
//	return;
//}

static unsigned long GetTargetProcessIdFromWindow(const char *className, const char *windowName)
{
	unsigned long procID(0);
	HWND targetWnd(FindWindow(className, windowName));

	if (targetWnd)
	{
		GetWindowThreadProcessId(targetWnd, &procID);
	}

	return procID;
}

static unsigned long GetTargetThreadIdFromWindow(const char *className, const char *windowName)
{
	HWND targetWnd(FindWindow(className, windowName));

	if (targetWnd)
	{
		return GetWindowThreadProcessId(targetWnd, 0);
	}

	return 0;
}

TOUCHHOOK_API void SetConsoleId(DWORD consoleId)
{
	g_consoleId = consoleId;
}

TOUCHHOOK_API void SetTuioServerAddress(const char* host, int port)
{
	strcpy_s(g_host, host);
	g_serverPort = port;
}

TOUCHHOOK_API bool InstallHookFromWindowTitle(const char *windowTitle)
{
	g_targetWnd = FindWindow(0, windowTitle);
	unsigned long threadId(GetTargetThreadIdFromWindow(0, windowTitle));
	if (g_targetWnd && threadId)
	{
		//g_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, g_this, 0); // mouse_ll global only
		g_hook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, g_this, threadId);
		return (g_hook /*&& g_mouseHook*/) ? true : false;
	}
	return false;
}

TOUCHHOOK_API bool RemoveHook(void)
{
	if (!g_hook || ! g_targetWnd)
		return false;

	BOOL ret1 = UnhookWindowsHookEx(g_hook);
	g_hook = 0;
	BOOL ret2 = /*(!g_mouseHook || */UnhookWindowsHookEx(g_mouseHook);/*);*/
	g_mouseHook = 0;

	// freeing resources is problematic because they are usually created in a different context
	// TODO: employ smart pointers resource allocation
	//if (s_server)
	//{
	//	delete s_server;
	//	s_server = 0;
	//}

	//if (s_cursors)
	//{
	//	delete s_cursors;
	//	s_cursors = 0;
	//}

	//if (s_cursorBuf)
	//{
	//	delete s_cursorBuf;
	//	s_cursorBuf = 0;
	//}

	return ret1 && ret2;
}

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (!s_initialized)
	{
		if (!AttachConsole(g_consoleId))
		{
			FreeConsole();
			AttachConsole(g_consoleId);
		}
		s_out = GetStdHandle(STD_OUTPUT_HANDLE);
		sprintf_s(s_buf, "Attached console\n");
		WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);

		if (!g_mouseHook)
		{
			g_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, g_this, 0); // mouse_ll global only (requires process with message loop)
			if (g_mouseHook)
			{
				sprintf_s(s_buf, "Installed mouse hook\n");
				WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);
			}
			else
			{
				sprintf_s(s_buf, "Could not install mouse hook\n");
				WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);
			}
		}

		HWND win = GetDesktopWindow();
		RECT rec;
		GetWindowRect(win, &rec);
		s_screen_height = rec.bottom;
		s_screen_width = rec.right;
		sprintf_s(s_buf, "Determined screen size: %i x %i\n", s_screen_width, s_screen_height);
		WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);

		s_cursors = new std::map<DWORD, TUIO::TuioCursor*>;
		s_cursorBuf = new std::map<DWORD, TUIO::TuioCursor*>;

		s_server = new TUIO::TuioServer(g_host, g_serverPort);
		sprintf_s(s_buf, "Created server (%s:%i)\n", g_host, g_serverPort);
		WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);

		s_timer = SetTimer(g_targetWnd, TIMER_IDENTIFIER, TIMER_CALL_TIME, 0);

		//if (RegisterTouchWindow(g_targetWnd, TOUCH_FLAGS))
		//{
		//	sprintf_s(s_buf, "Registered main window for touch\n");
		//	WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);
		//	HWND child = 0;
		//	while (child = FindWindowEx(g_targetWnd, child, 0, 0))
		//	{
		//		if (RegisterTouchWindow(child, TOUCH_FLAGS))
		//		{
		//			sprintf_s(s_buf, "Registered child window for touch\n");
		//			WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);
		//		}
		//		else
		//		{
		//			sprintf_s(s_buf, "Couldn't register child window for touch\n");
		//			WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);
		//		}
		//	}
		//}
		//else
		//{
		//	sprintf_s(s_buf, "Couldn't register main window for touch input!\n");
		//	WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);
		//}

		s_initialized = true;
	}

	if (nCode < 0) // do not process message 
	{
		return CallNextHookEx(0, nCode, wParam, lParam);
	}

	switch (nCode) 
	{ 
	case HC_ACTION: 
		{
			LPMSG msg = (LPMSG)lParam;

			// register everything for touch
			// Note: pretty ugly but seems to be the only possibility to register every window
			RegisterTouchWindow(msg->hwnd, TOUCH_FLAGS);

			switch (msg->message)
			{
			//case WM_MOUSEMOVE:
			//	{
			//		if (!g_mouseHook)
			//		{
			//			g_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, g_this, 0); // mouse_ll global only
			//			if (g_mouseHook)
			//			{
			//				sprintf_s(s_buf, "Installed mouse hook\n");
			//				WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);
			//			}
			//			else
			//			{
			//				sprintf_s(s_buf, "Could not install mouse hook\n");
			//				WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);
			//			}
			//		}
			//	}
			//	break;
			//case WM_MOUSELEAVE:
			//	{
			//		if (g_mouseHook)
			//		{
			//			if (UnhookWindowsHookEx(g_mouseHook))
			//			{
			//				g_mouseHook = 0;
			//				sprintf_s(s_buf, "Removed mouse hook\n");
			//				WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);
			//			}
			//			else
			//			{
			//				sprintf_s(s_buf, "Couldn't remove mouse hook\n");
			//				WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);
			//			}
			//		}
			//	}
			//	break;
			//case WM_LBUTTONDOWN:
			//case WM_LBUTTONUP:
			//case WM_RBUTTONDOWN:
			//case WM_RBUTTONUP:
			//case WM_MBUTTONDOWN:
			//case WM_MBUTTONUP:
			//case WM_LBUTTONDBLCLK:
			//case WM_RBUTTONDBLCLK:
			//case WM_MBUTTONDBLCLK:
			//case WM_MOUSEMOVE:
			//	{
			//		sprintf_s(s_buf, "Mouse event received\n");
			//		WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);
			//	}
			//	break;
			case WM_TOUCH:
				{
					sprintf_s(s_buf, "Touches received:\n");
					WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);

					UINT cInputs = LOWORD(msg->wParam);
					PTOUCHINPUT pInputs = new TOUCHINPUT[cInputs];
					if (NULL != pInputs)
					{
						if (GetTouchInputInfo((HTOUCHINPUT)msg->lParam,
							cInputs,
							pInputs,
							sizeof(TOUCHINPUT)))
						{
							s_server->initFrame(TUIO::TuioTime::getSessionTime());

							// process pInputs
							for (UINT i=0; i<cInputs; ++i)
							{
								sprintf_s(s_buf, "ID: %i, X: %i, Y: %i\n", pInputs[i].dwID, pInputs[i].x, pInputs[i].y);
								WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);

								std::map<DWORD, TUIO::TuioCursor*>::iterator cursorIt = s_cursors->find(pInputs[i].dwID);
								if (cursorIt != s_cursors->end())
								{
									// found
									s_server->updateTuioCursor(cursorIt->second,
															   pInputs[i].x*0.01f/(float)s_screen_width,
															   pInputs[i].y*0.01f/(float)s_screen_height);
									(*s_cursorBuf)[pInputs[i].dwID] = cursorIt->second;
									s_cursors->erase(cursorIt);
								}
								else
								{
									// not found
									(*s_cursorBuf)[pInputs[i].dwID] = s_server->addTuioCursor(pInputs[i].x*0.01f/(float)s_screen_width, 
																							  pInputs[i].y*0.01f/(float)s_screen_height);
								}
							}
							
							for (std::map<DWORD, TUIO::TuioCursor*>::iterator it = s_cursors->begin(); it != s_cursors->end(); ++it)
							{
								s_server->removeTuioCursor(it->second);
							}
							
							s_cursors->clear();
							std::map<DWORD, TUIO::TuioCursor*> *tmp = s_cursorBuf;
							s_cursorBuf = s_cursors;
							s_cursors = tmp;	
							s_server->commitFrame();
						}
					}
					delete[] pInputs;
				}
				break;
			case WM_TIMER:
				{
					if (msg->wParam == TIMER_IDENTIFIER)
					{
						BOOL frameOpen(false);
						for (std::map<DWORD, TUIO::TuioCursor*>::iterator it = s_cursors->begin(); it != s_cursors->end();)
						{
							long delta = TUIO::TuioTime::getSessionTime().getTotalMilliseconds() - it->second->getTuioTime().getTotalMilliseconds();
							if (TUIO::TuioTime::getSessionTime().getTotalMilliseconds() - 
								it->second->getTuioTime().getTotalMilliseconds() > MAX_CURSOR_IDLE_TIME)
							{
								if (!frameOpen)
								{
									s_server->initFrame(TUIO::TuioTime::getSessionTime());
									frameOpen = true;
								}
								s_server->removeTuioCursor(it->second);
								std::map<DWORD, TUIO::TuioCursor*>::iterator tmp = it++;
								s_cursors->erase(tmp);
								sprintf_s(s_buf, "Removed cursor due to time limit\n");
								WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);
							}
							else
							{
								++it;
							}
						}
						if (frameOpen)
						{
							s_server->commitFrame();
						}
					}
				}
				break;
			case WM_QUIT:
				{
					// TODO: does not print/not sure if it is even called!
					sprintf_s(s_buf, "Hooked app shutting down...\n");
					WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);

					//KillTimer(g_targetWnd, s_timer);

					//if (UnregisterTouchWindow(g_targetWnd))
					//{
					//	sprintf_s(s_buf, "Unregistered touch window\n");
					//	WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);

					//	HWND child = 0;
					//	while (child = FindWindowEx(g_targetWnd, child, 0, 0))
					//	{
					//		UnregisterTouchWindow(child);
					//	}
					//}
					//else
					//{
					//	sprintf_s(s_buf, "Couldn't unregister window!\n");
					//	WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);
					//}

					sprintf_s(s_buf, "Cleaning up cursors\n");
					WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);
					s_server->initFrame(TUIO::TuioTime::getSessionTime());
					for (std::map<DWORD, TUIO::TuioCursor*>::iterator it = s_cursors->begin(); it != s_cursors->end(); ++it)
					{
						s_server->removeTuioCursor(it->second);
					}
					s_server->commitFrame();

					delete s_server;
					s_server = 0;

					delete s_cursors;
					s_cursors = 0;

					delete s_cursorBuf;
					s_cursorBuf = 0;
				}
				break;
			default:
				{
					//sprintf_s(s_buf, "Unknown event (%i)\n", msg->message);
					//WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);
				}
				break;
			}
		}
		break; 

	default:
		break;
	}

	return CallNextHookEx(0, nCode, wParam, lParam); 
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0) // do not process message 
	{
		return CallNextHookEx(0, nCode, wParam, lParam);
	}

	LPMSLLHOOKSTRUCT msg = (LPMSLLHOOKSTRUCT)lParam;
	if (msg->flags & LLMHF_INJECTED) // block injected events (in most cases generated by touches)
	{
		sprintf_s(s_buf, "Blocked injected (touch) mouse event\n");
		WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);
		//return CallNextHookEx(0, nCode, wParam, lParam);
		return 1;
	}

	//RECT rec;
	//if (GetWindowRect(g_targetWnd, &rec))
	//{
	//	if (msg->pt.x >= rec.left && msg->pt.x < rec.right && msg->pt.y >= rec.top && msg->pt.y < rec.bottom)
	//	{
	//		//sprintf_s(s_buf, "Mouse inside window\n");
	//		//WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);
	//		return 1; // block mouse inside window
	//	}
	//	//else
	//	//{
	//	//	sprintf_s(s_buf, "Mouse outside window\n");
	//	//	WriteConsole(s_out, s_buf, strlen(s_buf), &s_ccount, 0);
	//	//}
	//}

	// forward event
	return CallNextHookEx(0, nCode, wParam, lParam);
}
