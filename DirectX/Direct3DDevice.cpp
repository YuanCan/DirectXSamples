
#include "stdafx.h"
#include "Direct3DDevice.h"
Direct3DDevice * Direct3DDevice::Instance = NULL;
const DWORD Vertex::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;

Direct3DDevice * Direct3DDevice::GetInstance()
{
	if(NULL == Instance)
	{
		Instance = new Direct3DDevice();
	}
	return Instance;
}

void Direct3DDevice::DestroyInstance()
{
	delete Instance;
	Instance = NULL;
}

Direct3DDevice :: Direct3DDevice()
{
	device = NULL;
	_d3d9 = NULL;
}

Direct3DDevice::~Direct3DDevice()
{
	if(device)
	{
		device->Release();
		device = NULL;
	}

	if(_d3d9)
	{
		_d3d9->Release();
		_d3d9 = NULL;
	}
}

HRESULT Direct3DDevice ::CreateDevice(HWND p_window,bool p_windowed,int p_windowWidth,int p_windowHeight)
{
	_d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
	InitPresentParamater(p_window,p_windowed,p_windowWidth,p_windowHeight);
	int vp = CheckDeviceCap();
	this->windowWidth = p_windowWidth;
	this->windowHeight = p_windowHeight;
	return _d3d9->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,p_window,vp,&d3dpp,&device);
}

int Direct3DDevice :: CheckDeviceCap()
{
	int cap = 0;
	_d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,&this->caps);
	if(caps.DevCaps && D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		return D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}
	else
	{
		return D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
}

void Direct3DDevice :: InitPresentParamater(HWND window,bool windowed,int windowWidth,int windowHeight)
{
	ZeroMemory(&d3dpp,sizeof(d3dpp));

	d3dpp.BackBufferWidth = windowWidth;
	d3dpp.BackBufferHeight = windowHeight;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.Windowed = windowed;
	//d3dpp.hDeviceWindow = window;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.Flags = 0;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
}

void Direct3DDevice :: InitRenderPara()
{
	// setup view matrix
	D3DXVECTOR3 position(0.0f,0.0f,-5.0f);
	D3DXVECTOR3 target(0.0f,0.0f,0.0f);
	D3DXVECTOR3 up(0.0f,1.0f,0.0f);
	D3DXMATRIX v;
	D3DXMatrixLookAtLH(&v,&position,&target,&up);
	device->SetTransform(D3DTS_VIEW,&v);

	// setup projection matrix
	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(&proj,
		D3DX_PI * 0.5f, /* view range : 90 degree*/
		(float)windowWidth / (float)windowHeight,
		1.0f,1000.0f);
	device->SetTransform(D3DTS_PROJECTION,&proj);

	// setup render state
	device->SetRenderState(D3DRS_FILLMODE,D3DFILL_WIREFRAME);
}

void Direct3DDevice::RenderTarget(LPDIRECT3DVERTEXBUFFER9 vertexBuffer,LPDIRECT3DINDEXBUFFER9 indexBuffer)
{
	if(device == NULL)
	{
		return;
	}

	// clear render buffer
	device->Clear(0,0,D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(255,255,255),1.0f,0);
	device->BeginScene();
	device->SetFVF(Vertex::FVF);
	device->SetStreamSource(0,vertexBuffer,0,sizeof(Vertex));
	device->SetIndices(indexBuffer);
	device->SetFVF(Vertex::FVF);

	// draw
	device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,4,0,2);
	device->EndScene();
	device->Present(NULL,NULL,NULL,NULL);
}

void Direct3DDevice::InitGeometry(LPDIRECT3DVERTEXBUFFER9 vb,LPDIRECT3DINDEXBUFFER9 ib)
{
	if(!device)
	{
		return;
	}
	device->CreateVertexBuffer(4*sizeof(Vertex),D3DUSAGE_WRITEONLY,Vertex::FVF,D3DPOOL_DEFAULT,&vb,NULL);
	device->CreateIndexBuffer(6*sizeof(WORD),D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_DEFAULT,&ib,NULL);

	// fill the vertex buffer
	Vertex * vertices;
	vb->Lock(0,0,(void**)&vertices,0);
	vertices[0] = Vertex(-1.0f,-1.0f,0.0f,D3DCOLOR_XRGB(255,255,0));
	vertices[1] = Vertex(1.0f,1.0f,0.0f,D3DCOLOR_XRGB(255,255,0));
	vertices[2] = Vertex(1.0f,-1.0f,0.0f,D3DCOLOR_XRGB(255,255,0));
	vertices[3] = Vertex(-1.0f,1.0f,0.0f,D3DCOLOR_XRGB(255,255,0));
	vb->Unlock();

	DWORD*indices;
	ib->Lock(0,0,(void**)&indices,0);
	indices[0] = 0;indices[1] = 1;indices[2] = 2;
	indices[3] = 0;indices[4] = 2;indices[5] = 3;
	ib->Unlock();
}

void Direct3DDevice::RenderTutorial()
{
	if( NULL == device )
		return;

	// Clear the backbuffer to a blue color
	device->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB( 0, 0, 255 ), 1.0f, 0 );

	// Begin the scene
	if( SUCCEEDED( device->BeginScene() ) )
	{
		// Rendering of scene objects can happen here

		// End the scene
		device->EndScene();
	}

	// Present the backbuffer contents to the display
	device->Present( NULL, NULL, NULL, NULL );

}