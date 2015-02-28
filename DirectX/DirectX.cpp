// DirectXSetup.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "DirectX.h"
#include "Direct3DDevice.h"
#include "VideoCaptue.h"
#include "dxerr.h"
#include <CommCtrl.h>
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 )
#pragma comment(lib, "dxerr.lib")


#define  IDD_COMBOBOX 100

VideoCapture captureInstance;
// Main window handler
HWND hwnd;
// DirectX window
HWND directXWindow;
// Video capture window
HWND videoWindow;
HINSTANCE instance;

int userWindowWidth,userWindowHeight;

#define  DEFAULT_WINDOW_WIDTH 1400
#define  DEFAULT_WINDOW_HEIGHT 900
#define  DEFAULT_CONTROL_HEIGHT 100
void CreateComboBox(HWND parent,HINSTANCE instance);

//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	int cxChar , cyChar;
	int count;
	static HWND hwndList;
	int width,height;
	switch( msg )
	{
	case WM_GRAPHNOTIFY:
		captureInstance.HandleGraphEvent();
		return 0;
	case WM_SETFOCUS:
		SetFocus(hwndList);
		return 0;
	case  WM_COMMAND:

		if(HIWORD(wParam) == CBN_SELCHANGE)
			// If the user makes a selection from the list:
				//   Send CB_GETCURSEL message to get the index of the selected list item.
					//   Send CB_GETLBTEXT message to get the item.
						//   Display the item in a message box.
		{ 
			int ItemIndex = SendMessage((HWND) lParam, (UINT) CB_GETCURSEL, 
				(WPARAM) 0, (LPARAM) 0);
			TCHAR  ListItem[256];
			(TCHAR) SendMessage((HWND) lParam, (UINT) CB_GETLBTEXT, 
				(WPARAM) ItemIndex, (LPARAM) ListItem);
			captureInstance.BindDevice((LPCWSTR)ListItem);                    
		}
		return 0;
	case WM_SIZE:
		InvalidateRect(hwnd, NULL,TRUE);
		RECT rc;
		// Make the preview video fill our window
		GetClientRect(hWnd, &rc);
		width = rc.right - rc.left;
		height = rc.bottom - rc.top;
		SetWindowPos(directXWindow,NULL,rc.left,rc.top + DEFAULT_CONTROL_HEIGHT,width >> 1,height - DEFAULT_CONTROL_HEIGHT,TRUE);
		SetWindowPos(videoWindow,NULL,rc.left + (width >>1),rc.top + DEFAULT_CONTROL_HEIGHT,width >> 1,height - DEFAULT_CONTROL_HEIGHT,TRUE);
		captureInstance.ResizeVideoWindow();
		return 0;

	case WM_WINDOWPOSCHANGED:
		captureInstance.ChangePreviewState(! (IsIconic(hwnd)));
		return 0;

	case WM_CLOSE:            
		// Hide the main window while the graph is destroyed
		ShowWindow(hWnd, SW_HIDE);
		return 0;
	case WM_DESTROY:
		Direct3DDevice::GetInstance()->DestroyInstance();
		PostQuitMessage(0);
		return 0;
	}
	// Pass this message to the video window for notification of system changes
	if (captureInstance.pVideoWindow)
		captureInstance.pVideoWindow->NotifyOwnerMessage((LONG_PTR) hwnd, msg, wParam, lParam);
	return DefWindowProc( hWnd, msg, wParam, lParam );
}




//-----------------------------------------------------------------------------
// Name: wWinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE hInst, HINSTANCE, LPWSTR, INT )
{
	UNREFERENCED_PARAMETER( hInst );

	// Register the window class
	WNDCLASSEX wc =
	{
		sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle( NULL ), NULL, NULL, NULL, NULL,
		L"D3D Tutorial", NULL
	};
	RegisterClassEx( &wc );

	// Create the application's window
	HWND hWnd = CreateWindow( L"D3D Tutorial", L"D3D Tutorial 01: CreateDevice",
		WS_OVERLAPPEDWINDOW, 0, 0, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT,
		NULL, NULL, wc.hInstance, NULL );
	hwnd = hWnd;
	instance = wc.hInstance;
	directXWindow = CreateWindow( WC_STATIC, L"",
		WS_CHILD | WS_VISIBLE, 0, DEFAULT_CONTROL_HEIGHT, DEFAULT_WINDOW_WIDTH >> 1, DEFAULT_WINDOW_HEIGHT - DEFAULT_CONTROL_HEIGHT,
		hWnd, NULL, wc.hInstance, NULL );
	videoWindow= CreateWindow( WC_STATIC, L"",
		WS_CHILD | WS_VISIBLE, DEFAULT_WINDOW_WIDTH >> 1, DEFAULT_CONTROL_HEIGHT, DEFAULT_WINDOW_WIDTH >> 1, DEFAULT_WINDOW_HEIGHT - DEFAULT_CONTROL_HEIGHT,
		hWnd, NULL, wc.hInstance, NULL );
	if(FAILED(captureInstance.CreateInstance(videoWindow)))
	{
		MessageBoxW(hWnd,captureInstance.m_errorMessage,L"Video capture",S_OK);
		return 0;
	}
	HRESULT result = Direct3DDevice::GetInstance()->CreateDevice(directXWindow,TRUE,DEFAULT_WINDOW_WIDTH >> 1, DEFAULT_WINDOW_HEIGHT - DEFAULT_CONTROL_HEIGHT);
	// Initialize Direct3D
	if( SUCCEEDED(result))
	{
		// Show the window
		ShowWindow( hWnd, SW_SHOWDEFAULT );
		UpdateWindow( hWnd );
		CreateComboBox(hWnd,instance);
		Direct3DDevice::GetInstance()->InitRenderPara();
		Direct3DDevice::GetInstance()->InitGeometry();
		// Enter the message loop
		MSG msg;
		ZeroMemory( &msg, sizeof( msg ) );
		while( msg.message != WM_QUIT )
		{
			if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
			else
			{
				Direct3DDevice::GetInstance()->RenderTarget();
			}
		}
	}
	else
	{
		//MessageBoxA(NULL,DXGetErrorDescriptionA(result),"Error",MB_OK);
	}
	Direct3DDevice::GetInstance()->DestroyInstance();
	UnregisterClass( L"D3D Tutorial", wc.hInstance );
	return 0;
}

void CreateComboBox(HWND parent,HINSTANCE instance)
{
	// Create the Combobox
	//
	// Uses the CreateWindow function to create a child window of 
	// the application window. The WC_COMBOBOX window style specifies  
	// that it is a combobox.

	int xpos = 0;            // Horizontal position of the window.
	int ypos = 0;            // Vertical position of the window.
	int nwidth = 150;          // Width of the window
	int nheight = 500;         // Height of the window

	HWND hWndComboBox = CreateWindow(WC_COMBOBOX, TEXT(""), 
		CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP,
		xpos, ypos, nwidth, nheight, parent, (HMENU)IDD_COMBOBOX, instance,
		NULL);
	captureInstance.CreateDeviceList(hWndComboBox);
	SendMessage(hWndComboBox,CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
}