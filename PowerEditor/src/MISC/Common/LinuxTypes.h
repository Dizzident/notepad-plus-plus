// This file is part of Notepad++ project
// Copyright (C)2021 Don HO <don.h@free.fr>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option) any later version.

#pragma once

// Linux platform types
#include <cstdint>
#include <cwchar>
#include <cstdarg>
#include <cstring>
#include <cerrno>
#include <strings.h>  // For strcasecmp
#include <string>

using HWND = void*;
using UINT = unsigned int;
using WPARAM = uintptr_t;
using LPARAM = intptr_t;
using LRESULT = intptr_t;
using UINT_PTR = uintptr_t;
using INT_PTR = intptr_t;
using LONG_PTR = intptr_t;
using BYTE = uint8_t;
using UCHAR = unsigned char;
using WORD = uint16_t;
using DWORD = uint32_t;
using BOOL = int;
using INT = int;
using LONG = long;
using WCHAR = wchar_t;
using TCHAR = wchar_t;
using LPWSTR = wchar_t*;
using LPCWSTR = const wchar_t*;
using PWSTR = wchar_t*;
using LPTSTR = wchar_t*;
using _locale_t = void*;
using HFONT = void*;
using HBRUSH = void*;
using HBITMAP = void*;
using HICON = void*;
using HCURSOR = void*;
using HPEN = void*;
using HDC = void*;
using HANDLE = void*;
using COLORREF = uint32_t;
using HINSTANCE = void*;
using HKEY = void*;
using HRESULT = long;
using DWORD_PTR = uintptr_t;
using ULONG_PTR = uintptr_t;
using LPDWORD = DWORD*;
using LPBYTE = BYTE*;
using HDROP = void*;
using LPVOID = void*;
using CHAR = char;
using HIMAGELIST = void*;
using HMENU = void*;
using HACCEL = void*;
using HHOOK = void*;
using DPI_AWARENESS_CONTEXT = void*;

// Boolean constants
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// Basic structures
#ifndef RECT_DEFINED
#define RECT_DEFINED
struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};
#endif

#ifndef POINT_DEFINED
#define POINT_DEFINED
struct POINT {
    long x;
    long y;
};
#endif

#ifndef SIZE_DEFINED
#define SIZE_DEFINED
struct SIZE {
    long cx;
    long cy;
};
#endif

// Pointer types
using LPRECT = RECT*;
using LPCRECT = const RECT*;
using LPPOINT = POINT*;
using LPSIZE = SIZE*;
using LPNMHDR = struct NMHDR*;
using LPDRAWITEMSTRUCT = struct DRAWITEMSTRUCT*;
using LPNMTOOLBARW = struct NMTOOLBARW*;

// NMHDR structure
struct NMHDR {
    HWND hwndFrom;
    UINT_PTR idFrom;
    UINT code;
};

// DRAWITEMSTRUCT
struct DRAWITEMSTRUCT {
    UINT CtlType;
    UINT CtlID;
    UINT itemID;
    UINT itemAction;
    UINT itemState;
    HWND hwndItem;
    HDC hDC;
    RECT rcItem;
    ULONG_PTR itemData;
};

// NMTOOLBARW
struct NMTOOLBARW {
    NMHDR hdr;
    int iItem;
    struct TBBUTTON* tbButton;
    int cchText;
    LPWSTR pszText;
};

// TCITEM
struct TCITEM {
    UINT mask;
    DWORD dwState;
    DWORD dwStateMask;
    LPWSTR pszText;
    int cchTextMax;
    int iImage;
    LPARAM lParam;
};

// LVCOLUMN
struct LVCOLUMN {
    UINT mask;
    int fmt;
    int cx;
    LPWSTR pszText;
    int cchTextMax;
    int iSubItem;
    int iImage;
    int iOrder;
};

// TBBUTTON
struct TBBUTTON {
    int iBitmap;
    int idCommand;
    BYTE fsState;
    BYTE fsStyle;
    BYTE bReserved[2];
    DWORD_PTR dwData;
    INT_PTR iString;
};

// REBARBANDINFO
struct REBARBANDINFO {
    UINT cbSize;
    UINT fMask;
    UINT fStyle;
    COLORREF clrFore;
    COLORREF clrBack;
    LPTSTR lpText;
    UINT cch;
    int iImage;
    HWND hwndChild;
    UINT cxMinChild;
    UINT cyMinChild;
    UINT cx;
    HBITMAP hbmBack;
    UINT wID;
    UINT cyChild;
    UINT cyMaxChild;
    UINT cyIntegral;
    UINT cxIdeal;
    LPARAM lParam;
    UINT cxHeader;
};

// TCHITTESTINFO
struct TCHITTESTINFO {
    POINT pt;
    UINT flags;
};

// MINMAXINFO
struct MINMAXINFO {
    POINT ptReserved;
    POINT ptMaxSize;
    POINT ptMaxPosition;
    POINT ptMinTrackSize;
    POINT ptMaxTrackSize;
};

// WINDOWPLACEMENT
struct WINDOWPLACEMENT {
    UINT length;
    UINT flags;
    UINT showCmd;
    POINT ptMinPosition;
    POINT ptMaxPosition;
    RECT rcNormalPosition;
    RECT rcDevice;
};

// MSG
struct MSG {
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
};

// SYSTEMTIME
struct SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
};

// FILETIME
struct FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
};

// WIN32_FILE_ATTRIBUTE_DATA
struct WIN32_FILE_ATTRIBUTE_DATA {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
};

// TOOLINFO
struct TOOLINFO {
    UINT cbSize;
    UINT uFlags;
    HWND hwnd;
    UINT_PTR uId;
    RECT rect;
    HINSTANCE hinst;
    LPWSTR lpszText;
    LPARAM lParam;
};

// GUID
struct GUID {
    DWORD Data1;
    WORD Data2;
    WORD Data3;
    BYTE Data4[8];
};

// NOTIFYICONDATA
struct NOTIFYICONDATA {
    UINT cbSize;
    HWND hWnd;
    UINT uID;
    UINT uFlags;
    UINT uCallbackMessage;
    HICON hIcon;
    WCHAR szTip[128];
    DWORD dwState;
    DWORD dwStateMask;
    WCHAR szInfo[256];
    union {
        UINT uTimeout;
        UINT uVersion;
    };
    WCHAR szInfoTitle[64];
    DWORD dwInfoFlags;
    GUID guidItem;
    HICON hBalloonIcon;
};

// LOGFONT
struct LOGFONT {
    LONG lfHeight;
    LONG lfWidth;
    LONG lfEscapement;
    LONG lfOrientation;
    LONG lfWeight;
    BYTE lfItalic;
    BYTE lfUnderline;
    BYTE lfStrikeOut;
    BYTE lfCharSet;
    BYTE lfOutPrecision;
    BYTE lfClipPrecision;
    BYTE lfQuality;
    BYTE lfPitchAndFamily;
    WCHAR lfFaceName[32];
};

// ACCEL
struct ACCEL {
    BYTE fVirt;
    WORD key;
    WORD cmd;
};


// Constants
#define WM_USER 0x0400
#define WM_APP 0x8000

// Window messages
#define WM_ERASEBKGND 0x0014
#define WM_NOTIFY 0x004E
#define WM_SETTEXT 0x000C
#define WM_GETTEXT 0x000D
#define WM_GETTEXTLENGTH 0x000E
#define WM_COMMAND 0x0111
#define WM_PAINT 0x000F
#define WM_CLOSE 0x0010
#define WM_CREATE 0x0001
#define WM_DESTROY 0x0002
#define WM_MOVE 0x0003
#define WM_SIZE 0x0005
#define WM_ACTIVATE 0x0006
#define WM_SETFOCUS 0x0007
#define WM_KILLFOCUS 0x0008
#define WM_ENABLE 0x000A
#define WM_SETREDRAW 0x000B
#define WM_KEYDOWN 0x0100
#define WM_KEYUP 0x0101
#define WM_CHAR 0x0102
#define WM_SYSKEYDOWN 0x0104
#define WM_SYSKEYUP 0x0105
#define WM_INITDIALOG 0x0110
#define WM_SYSCOMMAND 0x0112
#define WM_TIMER 0x0113
#define WM_HSCROLL 0x0114
#define WM_VSCROLL 0x0115
#define WM_MENUSELECT 0x011F
#define WM_MOUSEMOVE 0x0200
#define WM_LBUTTONDOWN 0x0201
#define WM_LBUTTONUP 0x0202
#define WM_LBUTTONDBLCLK 0x0203
#define WM_RBUTTONDOWN 0x0204
#define WM_RBUTTONUP 0x0205
#define WM_RBUTTONDBLCLK 0x0206
#define WM_MBUTTONDOWN 0x0207
#define WM_MBUTTONUP 0x0208
#define WM_MBUTTONDBLCLK 0x0209
#define WM_MOUSEWHEEL 0x020A
#define WM_DROPFILES 0x0233
#define WM_COPYDATA 0x004A
#define WM_CONTEXTMENU 0x007B
#define WM_DPICHANGED 0x02E0
#define WM_DPICHANGED_BEFOREPARENT 0x02E2
#define WM_DPICHANGED_AFTERPARENT 0x02E3
#define WM_GETDPISCALEDSIZE 0x02E4

// Dialog messages
#define DM_SETDEFID (WM_USER + 1)
#define DM_REPOSITION 0x0402

// Button messages
#define BM_GETCHECK 0x00F0
#define BM_SETCHECK 0x00F1
#define BM_GETSTATE 0x00F2
#define BM_SETSTATE 0x00F3

// Button states
#define BST_CHECKED 0x0001
#define BST_UNCHECKED 0x0000

// List box messages
#define LB_GETCURSEL 0x0188

// Combo box messages
#define CB_GETCOUNT 0x0146
#define CB_GETCURSEL 0x0147
#define CB_RESETCONTENT 0x014B
#define CB_ADDSTRING 0x0143
#define CB_SETCURSEL 0x014E

// ListView constants
#define LVCF_TEXT 0x0004
#define LVIS_SELECTED 0x0002
#define LVIS_FOCUSED 0x0001

// Tab control constants
#define TCN_FIRST (-550)
#define TCM_FIRST 0x1300
#define TCM_GETCURSEL (TCM_FIRST + 11)
#define TCM_GETITEMCOUNT (TCM_FIRST + 4)
#define TCM_DELETEALLITEMS (TCM_FIRST + 9)
#define TCM_GETROWCOUNT (TCM_FIRST + 44)
#define TCM_HITTEST (TCM_FIRST + 13)
#define TCM_SETIMAGELIST (TCM_FIRST + 46)
#define TCM_GETIMAGELIST (TCM_FIRST + 2)
#define TCM_GETITEM (TCM_FIRST + 5)
#define TCM_SETITEM (TCM_FIRST + 6)
#define TCM_INSERTITEM (TCM_FIRST + 7)
#define TCM_DELETEITEM (TCM_FIRST + 8)
#define TCM_SETCURSEL (TCM_FIRST + 12)
#define TCIF_TEXT 0x0001
#define TCIF_IMAGE 0x0002
#define TCIF_PARAM 0x0008
#define TCHT_NOWHERE 0x0001

// Toolbar constants
#define TB_ENABLEBUTTON (WM_USER + 1)
#define TB_CHECKBUTTON (WM_USER + 2)
#define TB_PRESSBUTTON (WM_USER + 3)
#define TB_HIDEBUTTON (WM_USER + 4)
#define TB_INDETERMINATE (WM_USER + 5)
#define TB_MARKBUTTON (WM_USER + 6)
#define TB_ISBUTTONENABLED (WM_USER + 9)
#define TB_ISBUTTONCHECKED (WM_USER + 10)
#define TB_ISBUTTONPRESSED (WM_USER + 11)
#define TB_ISBUTTONHIDDEN (WM_USER + 12)
#define TB_ISBUTTONINDETERMINATE (WM_USER + 13)
#define TB_ISBUTTONHIGHLIGHTED (WM_USER + 14)
#define TB_SETSTATE (WM_USER + 17)
#define TB_GETSTATE (WM_USER + 18)
#define TB_ADDBITMAP (WM_USER + 19)
#define TB_ADDBUTTONS (WM_USER + 20)
#define TB_INSERTBUTTON (WM_USER + 21)
#define TB_DELETEBUTTON (WM_USER + 22)
#define TB_GETBUTTON (WM_USER + 23)
#define TB_BUTTONCOUNT (WM_USER + 24)
#define TB_COMMANDTOINDEX (WM_USER + 25)
#define TB_SAVERESTORE (WM_USER + 26)
#define TB_CUSTOMIZE (WM_USER + 27)
#define TB_ADDSTRING (WM_USER + 28)
#define TB_GETITEMRECT (WM_USER + 29)
#define TB_BUTTONSTRUCTSIZE (WM_USER + 30)
#define TB_SETBUTTONSIZE (WM_USER + 31)
#define TB_SETBITMAPSIZE (WM_USER + 32)
#define TB_AUTOSIZE (WM_USER + 33)
#define TB_GETTOOLTIPS (WM_USER + 35)
#define TB_SETTOOLTIPS (WM_USER + 36)
#define TB_SETPARENT (WM_USER + 37)
#define TB_SETROWS (WM_USER + 39)
#define TB_GETROWS (WM_USER + 40)
#define TB_SETCMDID (WM_USER + 42)
#define TB_CHANGEBITMAP (WM_USER + 43)
#define TB_GETBITMAP (WM_USER + 44)
#define TB_GETBUTTONTEXT (WM_USER + 45)
#define TB_REPLACEBITMAP (WM_USER + 46)
#define TB_SETINDENT (WM_USER + 47)
#define TB_SETIMAGELIST (WM_USER + 48)
#define TB_GETIMAGELIST (WM_USER + 49)
#define TB_LOADIMAGES (WM_USER + 50)
#define TB_GETRECT (WM_USER + 51)
#define TB_SETHOTIMAGELIST (WM_USER + 52)
#define TB_GETHOTIMAGELIST (WM_USER + 53)
#define TB_SETDISABLEDIMAGELIST (WM_USER + 54)
#define TB_GETDISABLEDIMAGELIST (WM_USER + 55)
#define TB_SETSTYLE (WM_USER + 56)
#define TB_GETSTYLE (WM_USER + 57)
#define TB_GETBUTTONSIZE (WM_USER + 58)
#define TB_SETBUTTONWIDTH (WM_USER + 59)
#define TB_SETMAXTEXTROWS (WM_USER + 60)
#define TB_GETTEXTROWS (WM_USER + 61)

#define TBSTATE_CHECKED 0x01
#define TBSTATE_PRESSED 0x02
#define TBSTATE_ENABLED 0x04
#define TBSTATE_HIDDEN 0x08
#define TBSTATE_INDETERMINATE 0x10
#define TBSTATE_WRAP 0x20
#define TBSTATE_ELLIPSES 0x40
#define TBSTATE_MARKED 0x80

#define TBSTYLE_BUTTON 0x0000
#define TBSTYLE_SEP 0x0001
#define TBSTYLE_CHECK 0x0002
#define TBSTYLE_GROUP 0x0004
#define TBSTYLE_CHECKGROUP (TBSTYLE_GROUP | TBSTYLE_CHECK)
#define TBSTYLE_DROPDOWN 0x0008
#define TBSTYLE_AUTOSIZE 0x0010
#define TBSTYLE_NOPREFIX 0x0020

#define ILC_COLOR 0x0000
#define ILC_MASK 0x0001
#define ILC_COLOR32 0x0020


// SetWindowPos flags
#define SWP_NOSIZE 0x0001
#define SWP_NOMOVE 0x0002
#define SWP_NOZORDER 0x0004
#define SWP_NOREDRAW 0x0008
#define SWP_NOACTIVATE 0x0010
#define SWP_SHOWWINDOW 0x0040
#define SWP_HIDEWINDOW 0x0080
#define SWP_NOCOPYBITS 0x0100
#define SWP_NOOWNERZORDER 0x0200
#define SWP_NOSENDCHANGING 0x0400

// Window styles
#define WS_OVERLAPPED 0x00000000
#define WS_POPUP 0x80000000
#define WS_CHILD 0x40000000
#define WS_MINIMIZE 0x20000000
#define WS_VISIBLE 0x10000000
#define WS_DISABLED 0x08000000
#define WS_CLIPSIBLINGS 0x04000000
#define WS_CLIPCHILDREN 0x02000000
#define WS_MAXIMIZE 0x01000000
#define WS_CAPTION 0x00C00000
#define WS_BORDER 0x00800000
#define WS_DLGFRAME 0x00400000
#define WS_VSCROLL 0x00200000
#define WS_HSCROLL 0x00100000
#define WS_SYSMENU 0x00080000
#define WS_THICKFRAME 0x00040000
#define WS_GROUP 0x00020000
#define WS_TABSTOP 0x00010000
#define WS_MINIMIZEBOX 0x00020000
#define WS_MAXIMIZEBOX 0x00010000
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)

// Extended window styles
#define WS_EX_DLGMODALFRAME 0x00000001
#define WS_EX_NOPARENTNOTIFY 0x00000004
#define WS_EX_TOPMOST 0x00000008
#define WS_EX_ACCEPTFILES 0x00000010
#define WS_EX_TRANSPARENT 0x00000020
#define WS_EX_MDICHILD 0x00000040
#define WS_EX_TOOLWINDOW 0x00000080
#define WS_EX_WINDOWEDGE 0x00000100
#define WS_EX_CLIENTEDGE 0x00000200
#define WS_EX_CONTEXTHELP 0x00000400
#define WS_EX_RIGHT 0x00001000
#define WS_EX_LEFT 0x00000000
#define WS_EX_RTLREADING 0x00002000
#define WS_EX_LTRREADING 0x00000000
#define WS_EX_LEFTSCROLLBAR 0x00004000
#define WS_EX_RIGHTSCROLLBAR 0x00000000
#define WS_EX_CONTROLPARENT 0x00010000
#define WS_EX_STATICEDGE 0x00020000
#define WS_EX_APPWINDOW 0x00040000
#define WS_EX_LAYERED 0x00080000
#define WS_EX_NOINHERITLAYOUT 0x00100000
#define WS_EX_LAYOUTRTL 0x00400000
#define WS_EX_COMPOSITED 0x02000000
#define WS_EX_NOACTIVATE 0x08000000

// ShowWindow commands
#define SW_HIDE 0
#define SW_SHOWNORMAL 1
#define SW_SHOWMINIMIZED 2
#define SW_SHOWMAXIMIZED 3
#define SW_SHOWNOACTIVATE 4
#define SW_SHOW 5
#define SW_MINIMIZE 6
#define SW_SHOWMINNOACTIVE 7
#define SW_SHOWNA 8
#define SW_RESTORE 9
#define SW_SHOWDEFAULT 10
#define SW_FORCEMINIMIZE 11

// DPI awareness
#define DPI_AWARENESS_CONTEXT_UNAWARE ((DPI_AWARENESS_CONTEXT)-1)
#define DPI_AWARENESS_CONTEXT_SYSTEM_AWARE ((DPI_AWARENESS_CONTEXT)-2)
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE ((DPI_AWARENESS_CONTEXT)-3)
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT)-4)
#define DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED ((DPI_AWARENESS_CONTEXT)-5)
#define USER_DEFAULT_SCREEN_DPI 96

// Code pages
#ifndef CP_UTF8_DEFINED
#define CP_UTF8_DEFINED
const UINT CP_ACP = 0;
const UINT CP_UTF8 = 65001;
#endif

// Macros
#ifndef CALLBACK
#define CALLBACK
#endif
#ifndef WINAPI
#define WINAPI
#endif
#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef __cdecl
#define __cdecl
#endif

#define LOWORD(l) ((WORD)((DWORD_PTR)(l) & 0xffff))
#define HIWORD(l) ((WORD)((DWORD_PTR)(l) >> 16))
#define MAKELONG(a, b) ((LONG)(((WORD)((DWORD_PTR)(a) & 0xffff)) | ((DWORD)((WORD)((DWORD_PTR)(b) & 0xffff))) << 16))
#define MAKEINTRESOURCE(i) ((wchar_t*)(uintptr_t)(i))

// RGB color component extraction macros
#ifndef GetRValue
#define GetRValue(rgb) ((rgb) & 0xFF)
#endif
#ifndef GetGValue
#define GetGValue(rgb) (((rgb) >> 8) & 0xFF)
#endif
#ifndef GetBValue
#define GetBValue(rgb) (((rgb) >> 16) & 0xFF)
#endif

#define S_OK 0
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define ERROR_SUCCESS 0L
#define NO_ERROR 0
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#define FILE_ATTRIBUTE_READONLY 0x00000001
#define FILE_ATTRIBUTE_HIDDEN 0x00000002
#define FILE_ATTRIBUTE_SYSTEM 0x00000004
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#define FILE_ATTRIBUTE_ARCHIVE 0x00000020
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#define GENERIC_READ 0x80000000
#define GENERIC_WRITE 0x40000000
#define FILE_SHARE_READ 0x00000001
#define FILE_SHARE_WRITE 0x00000002
#define FILE_SHARE_DELETE 0x00000004
#define CREATE_NEW 1
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define OPEN_ALWAYS 4
#define TRUNCATE_EXISTING 5

#define WAIT_OBJECT_0 0
#define INFINITE 0xFFFFFFFF

// Hook types
#define WH_GETMESSAGE 3
#define WH_CALLWNDPROC 4
#define WH_CBT 5
#define WH_SYSMSGFILTER 6
#define WH_MOUSE 7
#define WH_HARDWARE 8
#define WH_DEBUG 9
#define WH_SHELL 10
#define WH_FOREGROUNDIDLE 11
#define WH_CALLWNDPROCRET 12

// Virtual keys
#ifndef VK_NULL
#define VK_NULL 0x00
#endif
#ifndef VK_DOWN
#define VK_DOWN 0x28
#endif
#ifndef VK_UP
#define VK_UP 0x26
#endif
#ifndef VK_LEFT
#define VK_LEFT 0x25
#endif
#ifndef VK_RIGHT
#define VK_RIGHT 0x27
#endif
#ifndef VK_HOME
#define VK_HOME 0x24
#endif
#ifndef VK_END
#define VK_END 0x23
#endif
#ifndef VK_PRIOR
#define VK_PRIOR 0x21
#endif
#ifndef VK_NEXT
#define VK_NEXT 0x22
#endif
#define VK_DELETE 0x2E
#define VK_INSERT 0x2D
#define VK_ESCAPE 0x1B
#define VK_BACK 0x08
#define VK_TAB 0x09
#define VK_RETURN 0x0D
#define VK_SPACE 0x20
#define VK_F1 0x70
#define VK_F2 0x71
#define VK_F3 0x72
#define VK_F4 0x73
#define VK_F5 0x74
#define VK_F6 0x75
#define VK_F7 0x76
#define VK_F8 0x77
#define VK_F9 0x78
#define VK_F10 0x79
#define VK_F11 0x7A
#define VK_F12 0x7B
#ifndef VK_F13
#define VK_F13 0x7C
#endif
#ifndef VK_F14
#define VK_F14 0x7D
#endif
#ifndef VK_F15
#define VK_F15 0x7E
#endif
#ifndef VK_F16
#define VK_F16 0x7F
#endif
#ifndef VK_F17
#define VK_F17 0x80
#endif
#ifndef VK_F18
#define VK_F18 0x81
#endif
#ifndef VK_F19
#define VK_F19 0x82
#endif
#ifndef VK_F20
#define VK_F20 0x83
#endif
#ifndef VK_F21
#define VK_F21 0x84
#endif
#ifndef VK_F22
#define VK_F22 0x85
#endif
#ifndef VK_F23
#define VK_F23 0x86
#endif
#ifndef VK_F24
#define VK_F24 0x87
#endif
#define VK_ADD 0x6B
#define VK_SUBTRACT 0x6D
#define VK_DIVIDE 0x6F
#define VK_MULTIPLY 0x6A
#define VK_OEM_2 0xBF
#define VK_OEM_3 0xC0
#define VK_OEM_4 0xDB
#define VK_OEM_5 0xDC
#define VK_OEM_6 0xDD
#define VK_OEM_7 0xDE
#define VK_CAPITAL 0x14
#define VK_NUMLOCK 0x90
#define VK_SCROLL 0x91
#define VK_LSHIFT 0xA0
#define VK_RSHIFT 0xA1
#define VK_LCONTROL 0xA2
#define VK_RCONTROL 0xA3
#define VK_LMENU 0xA4
#define VK_RMENU 0xA5
#define VK_LWIN 0x5B
#define VK_RWIN 0x5C
#define VK_APPS 0x5D
#define VK_NUMPAD0 0x60
#define VK_NUMPAD1 0x61
#define VK_NUMPAD2 0x62
#define VK_NUMPAD3 0x63
#define VK_NUMPAD4 0x64
#define VK_NUMPAD5 0x65
#define VK_NUMPAD6 0x66
#define VK_NUMPAD7 0x67
#define VK_NUMPAD8 0x68
#define VK_NUMPAD9 0x69

// ASCII-based VK codes for 0-9 and A-Z
#define VK_0 0x30
#define VK_1 0x31
#define VK_2 0x32
#define VK_3 0x33
#define VK_4 0x34
#define VK_5 0x35
#define VK_6 0x36
#define VK_7 0x37
#define VK_8 0x38
#define VK_9 0x39
#define VK_A 0x41
#define VK_B 0x42
#define VK_C 0x43
#define VK_D 0x44
#define VK_E 0x45
#define VK_F 0x46
#define VK_G 0x47
#define VK_H 0x48
#define VK_I 0x49
#define VK_J 0x4A
#define VK_K 0x4B
#define VK_L 0x4C
#define VK_M 0x4D
#define VK_N 0x4E
#define VK_O 0x4F
#define VK_P 0x50
#define VK_Q 0x51
#define VK_R 0x52
#define VK_S 0x53
#define VK_T 0x54
#define VK_U 0x55
#define VK_V 0x56
#define VK_W 0x57
#define VK_X 0x58
#define VK_Y 0x59
#define VK_Z 0x5A

// Message box constants
#ifndef MB_OK
#define MB_OK 0x00000000
#endif
#ifndef MB_OKCANCEL
#define MB_OKCANCEL 0x00000001
#endif
#ifndef MB_ABORTRETRYIGNORE
#define MB_ABORTRETRYIGNORE 0x00000002
#endif
#ifndef MB_YESNOCANCEL
#define MB_YESNOCANCEL 0x00000003
#endif
#ifndef MB_YESNO
#define MB_YESNO 0x00000004
#endif
#ifndef MB_RETRYCANCEL
#define MB_RETRYCANCEL 0x00000005
#endif
#ifndef MB_CANCELTRYCONTINUE
#define MB_CANCELTRYCONTINUE 0x00000006
#endif
#ifndef MB_ICONERROR
#define MB_ICONERROR 0x00000010
#endif
#ifndef MB_ICONHAND
#define MB_ICONHAND MB_ICONERROR
#endif
#ifndef MB_ICONSTOP
#define MB_ICONSTOP MB_ICONERROR
#endif
#ifndef MB_ICONQUESTION
#define MB_ICONQUESTION 0x00000020
#endif
#ifndef MB_ICONWARNING
#define MB_ICONWARNING 0x00000030
#endif
#ifndef MB_ICONINFORMATION
#define MB_ICONINFORMATION 0x00000040
#endif
#ifndef MB_RTLREADING
#define MB_RTLREADING 0x00100000
#endif
#ifndef MB_RIGHT
#define MB_RIGHT 0x00080000
#endif
#ifndef IDOK
#define IDOK 1
#endif
#ifndef IDCANCEL
#define IDCANCEL 2
#endif
#ifndef IDABORT
#define IDABORT 3
#endif
#ifndef IDRETRY
#define IDRETRY 4
#endif
#ifndef IDIGNORE
#define IDIGNORE 5
#endif
#ifndef IDYES
#define IDYES 6
#endif
#ifndef IDNO
#define IDNO 7
#endif
#ifndef IDTRYAGAIN
#define IDTRYAGAIN 10
#endif
#ifndef IDCONTINUE
#define IDCONTINUE 11
#endif

// Menu flags
#define TPM_LEFTALIGN 0x0000
#define TPM_CENTERALIGN 0x0004
#define TPM_RIGHTALIGN 0x0008
#define MF_ENABLED 0x0000
#define MF_GRAYED 0x0001
#define MF_DISABLED 0x0002
#define MF_BITMAP 0x0004
#define MF_CHECKED 0x0008
#define MF_POPUP 0x0010
#define MF_MENUBARBREAK 0x0020
#define MF_MENUBREAK 0x0040
#define MF_HILITE 0x0080
#define MF_OWNERDRAW 0x0100
#define MF_BYPOSITION 0x0400
#define MF_SEPARATOR 0x0800
#define MF_DEFAULT 0x1000
#define MF_SYSMENU 0x2000
#define MF_HELP 0x4000
#define MF_RIGHTJUSTIFY 0x4000
#define MF_MOUSESELECT 0x8000
#define MF_BYCOMMAND 0x0000
#define MF_STRING 0x0000
#define MF_UNCHECKED 0x0000

// Color
#define COLOR_3DFACE 15

// System metrics constants
#define SM_CXSMICON 49
#define SM_CYSMICON 50

// Stub for GetSystemMetrics
inline int GetSystemMetrics(int nIndex) {
    (void)nIndex;
    // Return default values for common metrics
    return 16; // Default icon size
}

// Image loading
#define IMAGE_BITMAP 0
#define IMAGE_ICON 1
#define IMAGE_CURSOR 2
#define LR_DEFAULTCOLOR 0x0000
#define LR_MONOCHROME 0x0001
#define LR_COLOR 0x0002
#define LR_COPYRETURNORG 0x0004
#define LR_COPYDELETEORG 0x0008
#define LR_LOADFROMFILE 0x0010
#define LR_LOADTRANSPARENT 0x0020
#define LR_DEFAULTSIZE 0x0040
#define LR_VGACOLOR 0x0080
#define LR_LOADMAP3DCOLORS 0x1000
#define LR_CREATEDIBSECTION 0x2000
#define LR_COPYFROMRESOURCE 0x4000
#define LR_SHARED 0x8000

// Window class constants
#define WC_BUTTON L"Button"

// Window style constants
#define GWL_STYLE (-16)

// Button styles
#define BS_TYPEMASK 0x0000000F

// Accelerator flags
#define FVIRTKEY 0x01
#define FNOINVERT 0x02
#define FSHIFT 0x04
#define FCONTROL 0x08
#define FALT 0x10

// Architecture constants
#define IMAGE_FILE_MACHINE_I386 0x014c
#define IMAGE_FILE_MACHINE_AMD64 0x8664
#define IMAGE_FILE_MACHINE_ARM64 0xaa64

// ULARGE_INTEGER
union ULARGE_INTEGER {
    struct {
        DWORD LowPart;
        DWORD HighPart;
    };
    struct {
        DWORD LowPart;
        DWORD HighPart;
    } u;
    unsigned long long QuadPart;
};

// Locale
#define LOCALE_NAME_SYSTEM_DEFAULT L""
#define LOCALE_IDEFAULTANSICODEPAGE 0x00001004
#define LOCALE_RETURN_NUMBER 0x20000000

// SAL annotations
#ifndef __inout
#define __inout
#endif

// Character functions
#define _istspace iswspace

// Dialog proc type
typedef INT_PTR (CALLBACK* DLGPROC)(HWND, UINT, WPARAM, LPARAM);

// Additional functions
inline HWND GetFocus() { return nullptr; }


// Stub functions
inline void DestroyIcon(HICON) {}
inline void DestroyWindow(HWND) {}
inline void DestroyMenu(HMENU) {}
inline BOOL FreeLibrary(HINSTANCE) { return TRUE; }
inline BOOL CloseHandle(HANDLE) { return TRUE; }

// String functions
inline int lstrcmp(const wchar_t* s1, const wchar_t* s2) {
    return wcscmp(s1, s2);
}
inline int lstrcmpi(const wchar_t* s1, const wchar_t* s2) {
    return wcscasecmp(s1, s2);
}
inline int _stricmp(const char* s1, const char* s2) {
    return strcasecmp(s1, s2);
}
inline int _wcsicmp(const wchar_t* s1, const wchar_t* s2) {
    return wcscasecmp(s1, s2);
}

// Window functions
inline void SetWindowText(HWND, const wchar_t*) {}
inline int GetWindowText(HWND, wchar_t*, int) { return 0; }
inline int GetWindowTextLength(HWND) { return 0; }
inline void ShowWindow(HWND, int) {}
inline void MoveWindow(HWND, int, int, int, int, BOOL) {}
inline int GetClientRect(HWND, RECT*) { return 0; }
inline int GetWindowRect(HWND, RECT*) { return 0; }
inline void InvalidateRect(HWND, const RECT*, BOOL) {}
inline void UpdateWindow(HWND) {}
inline BOOL IsWindowVisible(HWND) { return FALSE; }
inline void SetFocus(HWND) {}
inline void SetActiveWindow(HWND) {}
inline BOOL EnableWindow(HWND, BOOL) { return TRUE; }
inline BOOL IsWindowEnabled(HWND) { return TRUE; }
inline HWND GetParent(HWND) { return nullptr; }
inline BOOL SetWindowPos(HWND, HWND, int, int, int, int, UINT) { return TRUE; }
inline HWND GetDlgItem(HWND, int) { return nullptr; }
inline void SetDlgItemText(HWND, int, const wchar_t*) {}
inline UINT GetDlgItemText(HWND, int, wchar_t*, int) { return 0; }
inline UINT GetDlgItemTextA(HWND, int, char*, int) { return 0; }
inline void CheckDlgButton(HWND, int, UINT) {}
inline UINT IsDlgButtonChecked(HWND, int) { return 0; }
inline void CheckRadioButton(HWND, int, int, int) {}
inline LRESULT SendDlgItemMessage(HWND, int, UINT, WPARAM, LPARAM) { return 0; }
inline LRESULT SendMessage(HWND, UINT, WPARAM, LPARAM) { return 0; }
inline LRESULT SendMessageW(HWND, UINT, WPARAM, LPARAM) { return 0; }

// Window information functions
inline int GetClassNameW(HWND, wchar_t* className, int maxCount) {
    (void)className;
    (void)maxCount;
    return 0;
}
inline intptr_t GetWindowLongPtrW(HWND, int) { return 0; }

// Dialog functions
inline void ScreenToClient(HWND, POINT*) {}
inline void ClientToScreen(HWND, POINT*) {}
inline BOOL GetCursorPos(POINT*) { return FALSE; }
inline void SetCursorPos(int, int) {}
inline HDC GetDC(HWND) { return nullptr; }
inline HDC GetWindowDC(HWND) { return nullptr; }
inline int ReleaseDC(HWND, HDC) { return 0; }

// Module functions
inline DWORD GetModuleFileName(HINSTANCE, wchar_t*, DWORD) { return 0; }

// Event functions
inline HANDLE CreateEvent(void*, BOOL, BOOL, const wchar_t*) { return nullptr; }
inline BOOL SetEvent(HANDLE) { return TRUE; }
inline BOOL ResetEvent(HANDLE) { return TRUE; }

// Wait functions
inline DWORD WaitForSingleObject(HANDLE, DWORD) { return WAIT_OBJECT_0; }

// Hook functions
using HOOKPROC = LRESULT (CALLBACK*)(int, WPARAM, LPARAM);
inline HHOOK SetWindowsHookEx(int, HOOKPROC, HINSTANCE, DWORD) { return nullptr; }
inline BOOL UnhookWindowsHookEx(HHOOK) { return TRUE; }
inline LRESULT CallNextHookEx(HHOOK, int, WPARAM, LPARAM) { return 0; }
inline DWORD GetCurrentThreadId() { return 0; }

// Menu functions
inline void TrackPopupMenu(HMENU, UINT, int, int, int, HWND, const RECT*) {}
inline void EnableMenuItem(HMENU, UINT, UINT) {}
inline void CheckMenuItem(HMENU, UINT, UINT) {}
// ModifyMenu - 4th param is UINT_PTR (command ID or submenu handle)
inline void ModifyMenu(HMENU, UINT, UINT, UINT_PTR, const wchar_t*) {}
inline HMENU GetSubMenu(HMENU, int) { return nullptr; }
inline int GetMenuString(HMENU, UINT, wchar_t*, int, UINT) { return 0; }
inline UINT GetMenuItemID(HMENU, int) { return 0; }

// Message box
inline int MessageBox(HWND, const wchar_t*, const wchar_t*, UINT) { return IDOK; }

// ImageList functions
inline HIMAGELIST ImageList_Create(int, int, UINT, int, int) { return nullptr; }
inline BOOL ImageList_Destroy(HIMAGELIST) { return TRUE; }
inline BOOL ImageList_RemoveAll(HIMAGELIST) { return TRUE; }
inline BOOL ImageList_Remove(HIMAGELIST, int) { return TRUE; }
inline int ImageList_Add(HIMAGELIST, HBITMAP, HBITMAP) { return 0; }
inline int ImageList_AddIcon(HIMAGELIST, HICON) { return 0; }
inline int ImageList_AddMasked(HIMAGELIST, HBITMAP, COLORREF) { return 0; }
inline int ImageList_GetImageCount(HIMAGELIST) { return 0; }
inline BOOL ImageList_SetImageCount(HIMAGELIST, UINT) { return FALSE; }
inline COLORREF ImageList_SetBkColor(HIMAGELIST, COLORREF) { return 0; }
inline COLORREF ImageList_GetBkColor(HIMAGELIST) { return 0; }
inline BOOL ImageList_Draw(HIMAGELIST, int, HDC, int, int, UINT) { return FALSE; }
inline BOOL ImageList_SetIconSize(HIMAGELIST, int, int) { return TRUE; }

// Rectangle functions
inline BOOL PtInRect(const RECT*, POINT) { return FALSE; }
inline BOOL IntersectRect(RECT*, const RECT*, const RECT*) { return FALSE; }
inline BOOL UnionRect(RECT*, const RECT*, const RECT*) { return FALSE; }
inline BOOL EqualRect(const RECT*, const RECT*) { return FALSE; }
inline void CopyRect(RECT*, const RECT*) {}
inline BOOL IsRectEmpty(const RECT*) { return TRUE; }
inline void SetRect(RECT*, int, int, int, int) {}
inline void SetRectEmpty(RECT*) {}
inline void OffsetRect(RECT*, int, int) {}
inline void InflateRect(RECT*, int, int) {}

// GDI functions
inline void FillRect(HDC, const RECT*, HBRUSH) {}

// COM functions
inline HRESULT CoInitializeEx(void*, DWORD) { return S_OK; }
inline void CoUninitialize() {}

// System functions
inline void GetSystemTimeAsFileTime(FILETIME*) {}
inline int GetLocaleInfoEx(const wchar_t*, uint32_t, wchar_t*, int) { return 0; }

// Math
inline int MulDiv(int nNumber, int nMult, int nDiv) {
    if (nDiv == 0) return 0;
    return (int)((((long long)nNumber) * nMult) / nDiv);
}

// Color conversion
struct HLSCOLOR {
    WORD hue;
    WORD lightness;
    WORD saturation;
};
inline void ColorRGBToHLS(COLORREF, WORD* h, WORD* l, WORD* s) {
    *h = 0; *l = 120; *s = 120;
}
inline COLORREF ColorHLSToRGB(WORD, WORD, WORD) {
    return 0x808080;
}

// ToolTip creation
inline HWND CreateToolTip(int, HWND, HINSTANCE, const wchar_t*) { return nullptr; }
inline HWND CreateToolTipRect(int, HWND, HINSTANCE, const wchar_t*, const RECT*) { return nullptr; }

// ControlInfoTip stub for Linux
class ControlInfoTip {
public:
    ControlInfoTip() = default;
    ~ControlInfoTip() = default;
    bool init(HINSTANCE, HWND, HWND, const std::wstring&, bool, unsigned int = 0, int = 200) { return false; }
    bool isValid() const { return false; }
    enum showPosition { beginning, middle, end };
    void show(showPosition = middle) const {}
    void hide() {}
    HWND getTipHandle() const { return nullptr; }
private:
    ControlInfoTip(const ControlInfoTip&) = delete;
    ControlInfoTip& operator=(const ControlInfoTip&) = delete;
};

// Font
inline HFONT createFont(const wchar_t*, int, bool, HWND) { return nullptr; }

// Misc
inline COLORREF GetSysColor(int) { return 0; }
inline void InitCommonControls() {}
inline BOOL DestroyAcceleratorTable(HACCEL) { return TRUE; }

// Safe string
using errno_t = int;
inline errno_t wcscpy_s(wchar_t* dest, size_t destSize, const wchar_t* src) {
    if (!dest || !src) return EINVAL;
    if (wcslen(src) >= destSize) return ERANGE;
    wcscpy(dest, src);
    return 0;
}

// Printf
inline int wsprintfW(wchar_t* buffer, const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vswprintf(buffer, 1024, format, args);
    va_end(args);
    return result;
}

// Scintilla search flags
#ifndef SCFIND_REGEXP_DOTMATCHESNL
#define SCFIND_REGEXP_DOTMATCHESNL 0x10000000
#endif

// ListView macros
inline BOOL ListView_SetColumn(HWND, int, const LVCOLUMN*) { return TRUE; }
inline int ListView_GetItemCount(HWND) { return 0; }
inline int ListView_GetSelectionMark(HWND) { return -1; }
inline BOOL ListView_SetItemState(HWND, int, UINT, UINT) { return TRUE; }
inline BOOL ListView_EnsureVisible(HWND, int, BOOL) { return TRUE; }
inline int ListView_SetSelectionMark(HWND, int) { return -1; }
inline BOOL ListView_DeleteItem(HWND, int) { return TRUE; }

// TabCtrl macros
inline BOOL TabCtrl_SetItem(HWND, int, const TCITEM*) { return TRUE; }

// Registry
inline LONG RegQueryInfoKey(HKEY, LPWSTR, DWORD*, DWORD*, DWORD*, DWORD*, DWORD*, DWORD*, DWORD*, DWORD*, DWORD*, FILETIME*) {
    return ERROR_SUCCESS;
}

