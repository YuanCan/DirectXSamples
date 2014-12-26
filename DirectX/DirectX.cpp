// DirectXSetup.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "DirectX.h"
#include "Direct3DDevice.h"
#include "dxerr.h"
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 )
#pragma comment(lib, "dxerr.lib")

LPDIRECT3DVERTEXBUFFER9 Vb;
LPDIRECT3DINDEXBUFFER9 Ib;

//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_DESTROY:
		Direct3DDevice::GetInstance()->DestroyInstance();
		PostQuitMessage( 0 );
		return 0;

	case WM_PAINT:
		//Direct3DDevice::GetInstance()->RenderTutorial();
		ValidateRect(hWnd,NULL);
		return 0;
	}

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
		WS_OVERLAPPEDWINDOW, 0, 0, 800, 600,
		NULL, NULL, wc.hInstance, NULL );
	HRESULT result = Direct3DDevice::GetInstance()->CreateDevice(hWnd,TRUE,800,600);
	// Initialize Direct3D
	if( SUCCEEDED(result))
	{
		// Show the window
		ShowWindow( hWnd, SW_SHOWDEFAULT );
		UpdateWindow( hWnd );
		Direct3DDevice::GetInstance()->InitRenderPara();
		Direct3DDevice::GetInstance()->InitGeometry(Vb,Ib);
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
				Direct3DDevice::GetInstance()->RenderTarget(Vb,Ib);
			}
		}
	}
	else
	{
		//MessageBoxA(NULL,DXGetErrorDescriptionA(result),"Error",MB_OK);
	}
	if(Vb)
	{
		Vb->Release();
		Vb = NULL;
	}
	if(Ib)
	{
		Ib->Release();
		Ib = NULL;
	}
	Direct3DDevice::GetInstance()->DestroyInstance();
	UnregisterClass( L"D3D Tutorial", wc.hInstance );
	return 0;
}
