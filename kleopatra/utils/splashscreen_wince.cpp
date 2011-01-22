/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andreas Holzammer <andreas.holzammer@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "splashscreen_wince.h"
#include <windows.h>
#include <aygshell.h>


// Global Bitmap variable
HBITMAP hbm;

const wchar_t *szTitle = L"Kontact Mobile";		// title bar text
const wchar_t *szWindowClass = L"SplashScreen";	// main window class name

//Prototype of the main function from the loader
int main (int argc, char *argv[]);

//This functions rotates the screen by 270 degrees
BOOL RotateTo270Degrees()
{
	DEVMODE DevMode;
	memset(&DevMode, 0, sizeof (DevMode));
	DevMode.dmSize               = sizeof (DevMode);
	DevMode.dmFields             = DM_DISPLAYORIENTATION;
	DevMode.dmDisplayOrientation = DMDO_270;
	if (DISP_CHANGE_SUCCESSFUL != ChangeDisplaySettingsEx(NULL, &DevMode, NULL, 0, NULL)){
	  //error cannot change to 270 degrees
	  printf("failed to rotate!\n");
	  return false;
	}
   return true;
}

// Load Splashscreen from resource dll
BOOL onCreate(
   HWND hwnd)
{
  // Load Splashscreen dll
  HINSTANCE hinst = LoadLibrary(L"splashscreen.dll");

	if (!hinst) {
		printf("failed to load splashscreen dll!\n");
		return false;
	}
  hbm = LoadBitmap(hinst,MAKEINTRESOURCE(101));
  
  return true;
}

// Clean up
void onDestroy(
  HWND hwnd)
{
  DeleteObject(hbm);
  
  PostQuitMessage(0);
}

void onPaint(
  HWND hwnd)
{
  PAINTSTRUCT ps;
  HDC hdc = BeginPaint(hwnd,&ps);
  
  HDC hdcMem = CreateCompatibleDC(NULL);
  SelectObject(hdcMem, hbm);

  BITMAP bm;
  GetObject(hbm,sizeof(bm),&bm);
  
  BitBlt(hdc,0,0,bm.bmWidth,bm.bmHeight,hdcMem,0,0,SRCCOPY);

  DeleteDC(hdcMem);
  
  EndPaint(hwnd,&ps);
}  


LRESULT CALLBACK windowProc(
  HWND hwnd,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam)
{
  switch(uMsg)
  {
  case WM_CREATE:
    onCreate(hwnd);
    break;
  case WM_DESTROY:
    onDestroy(hwnd);
    break;
  case WM_PAINT:
	  onPaint(hwnd);
	  break;
  case WM_SETTINGCHANGE:
    RotateTo270Degrees();
    break;
  }
  return DefWindowProc(hwnd,uMsg,wParam,lParam);
}  

/* Restore a Window of a process based on the filename
 * of this process. With some special Case handling for
 * Kontact-Mobile
 * Returns false if the window can not be found */
bool
restore_existing_window( const wchar_t * filename )
{
    HWND windowID = NULL;
    wchar_t * basename;
    wchar_t * p;
    wchar_t c;

    if (! filename ) {
        return false;
    }
    c = L'\\';
    basename = wcsrchr(filename, c) + 1;
    if (! basename) {
        return false;
    }

    c = L'-';

    p = wcsrchr(filename, c);
    if (! p ) {
        return false;
    }
    *p = L'\0';

    windowID = FindWindow( NULL, basename);
    if (windowID)
    {
        wchar_t classname[255];
        //Find the general top level Window and bring to front
        SetForegroundWindow((HWND)(((ULONG)windowID) | 0x01 ));
        // Check for subwindows that should be laid on top of that
        if ( ! GetClassName(windowID, classname, 255) ) {
            return true;
        }
        return true;
    }

    return false;
}

void registerClass(
  HINSTANCE hInstance)
{
  WNDCLASS wc = {
    CS_NOCLOSE,
    windowProc,
    0,0,
    hInstance,
    NULL,
    NULL,
    (HBRUSH) GetStockObject(WHITE_BRUSH),
    NULL,
    szWindowClass
  };
  RegisterClass(&wc);
}

namespace Kleo {

void showWinceSplashscreen()
{
    HWND hwnd;
    RotateTo270Degrees();
    wchar_t app_name[MAX_PATH];
    int res;
    
    SetCursor( LoadCursor( NULL, IDC_WAIT ) );

    res = GetModuleFileName (GetModuleHandle (NULL), app_name, MAX_PATH);
    if (! res) {
        puts ("can not determine module filename\n");
        exit (1);
    }

    if (restore_existing_window(app_name)) {
        exit(0);
    }

    // If the splashscreen window is already loaded just show it
	hwnd = FindWindow(szWindowClass, szTitle);	
  if (hwnd) { 
    ::ShowWindow( hwnd, SW_SHOW );
    SetForegroundWindow( hwnd );
    SHFullScreen(hwnd, SHFS_HIDETASKBAR | SHFS_HIDESTARTICON | SHFS_HIDESIPBUTTON);
	} else {
    HINSTANCE hInstance = GetModuleHandle(0);
	  registerClass(hInstance);
	  
	  hwnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
			0, 0, 0, 0, NULL, NULL, hInstance, NULL);
      
    SHFullScreen(hwnd, SHFS_HIDETASKBAR | SHFS_HIDESTARTICON | SHFS_HIDESIPBUTTON);

    RECT rc;
    // Next resize the main window to the size of the screen.
    SetRect(&rc, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    MoveWindow(hwnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, TRUE);

    SetForegroundWindow(hwnd);

	  ShowWindow(hwnd,SW_SHOW);
	  UpdateWindow(hwnd);
	}
}

}
