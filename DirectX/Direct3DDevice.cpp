
#include "stdafx.h"
#include "Direct3DDevice.h"
Direct3DDevice * Direct3DDevice::Instance = NULL;
const DWORD Vertex::FVF = D3DFVF_XYZ;
const DWORD MultiTexVertex::FVF = D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX3;

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
	frameIndex = 0;
}

Direct3DDevice::~Direct3DDevice()
{
	SAFE_RELEASE(texture0);
	SAFE_RELEASE(texture1);
	SAFE_RELEASE(texture2);
	SAFE_RELEASE(pEffect);
	SAFE_RELEASE(MultiTexCT);
	SAFE_RELEASE(MultiTexPS);
	SAFE_RELEASE(vertexBuffer);
	SAFE_RELEASE(surface);
	SAFE_RELEASE(Vb);
	SAFE_RELEASE(Ib);
	SAFE_RELEASE(device);
	SAFE_RELEASE(_d3d9);
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

void Direct3DDevice::ResetRenderPara(int windowWidth,int windowHeight)
{

}

void Direct3DDevice::RenderTarget()
{
	if(device == NULL || Vb == NULL || Ib == NULL)
	{
		return;
	}

	// clear render buffer
	device->Clear(0,0,D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(255,255,255),1.0f,0);
	if(SUCCEEDED(device->BeginScene()))
	{
		device->SetStreamSource(0,Vb,0,sizeof(Vertex));
		device->SetIndices(Ib);
		device->SetFVF(Vertex::FVF);

		// draw
		device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,3,0,1);
	}
	device->EndScene();
	device->Present(NULL,NULL,NULL,NULL);
}

HRESULT Direct3DDevice::InitGeometry()
{
	if(!device)
	{
		return S_FALSE;
	}

	device->CreateVertexBuffer(4*sizeof(Vertex),D3DUSAGE_WRITEONLY,Vertex::FVF,D3DPOOL_MANAGED,&Vb,NULL);
	device->CreateIndexBuffer(6*sizeof(WORD),D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_MANAGED,&Ib,NULL);

	// fill the vertex buffer
	Vertex * vertices;
	Vb->Lock(0,0,(void**)&vertices,0);
	vertices[0] = Vertex(-1.0f,-1.0f,0.0f);
	vertices[1] = Vertex(0.0f,1.0f,0.0f);
	vertices[2] = Vertex(1.0f,0.0f,0.0f);
	vertices[3] = Vertex(-1.0f,1.0f,0.0f);
	Vb->Unlock();

	WORD*indices;
	Ib->Lock(0,0,(void**)&indices,0);
	indices[0] = 0;indices[1] = 1;indices[2] = 2;
	indices[3] = 0;indices[4] = 2;indices[5] = 3;
	Ib->Unlock();

	return S_OK;
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

HRESULT Direct3DDevice::InitVideoCaptureModePara()
{
	//compile the pixel shader
	ID3DXBuffer* shader = 0;
	ID3DXBuffer* errorBuffer = 0;
	HRESULT hr = D3DXCompileShaderFromFile(
		L"D:\\shader.txt",
		0,
		0,
		"ps_YUY_RGB", // entry point function name
		"ps_2_0",
		D3DXSHADER_DEBUG | D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY, 
		&shader,
		&errorBuffer,
		&MultiTexCT);
	// output any error messages
	if( errorBuffer )
	{
		::MessageBoxA(0, (LPCSTR)errorBuffer->GetBufferPointer(), 0, 0);
		errorBuffer->Release();
		errorBuffer = NULL;
	}
	if(FAILED(hr))
	{
		::MessageBox(0, L"D3DXCompileShaderFromFile() - FAILED", 0, 0);
		return false;
	}
	//create the shader
	if(FAILED(device-> CreatePixelShader((DWORD*)shader->GetBufferPointer(),&MultiTexPS)))
		return 0;
	SAFE_RELEASE(shader);
	//set the transform matrix
	D3DXMATRIX Ortho2D;
	D3DXMATRIX Identity;
	D3DXMatrixOrthoLH(&Ortho2D, windowWidth, windowHeight, 0.0f, 1.0f);
	D3DXMatrixIdentity(&Identity);
	device->SetTransform(D3DTS_PROJECTION, &Ortho2D);
	device->SetTransform(D3DTS_WORLD, &Identity);
	device->SetTransform(D3DTS_VIEW, &Identity);
	//create texture
	if(FAILED(D3DXCreateTexture(device,(float)windowWidth,(float)windowHeight,0,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&texture0)))
		return 0;
	//create texture
	if(FAILED(D3DXCreateTexture(device,(float)windowWidth,(float)windowHeight,0,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&texture1)))
		return 0;
	//create texture
	if(FAILED(D3DXCreateTexture(device,(float)windowWidth,(float)windowHeight,0,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&texture2)))
		return 0;
	//Get the texture surface
	texture0->GetLevelDesc(0,&desc);
	//texture->GetSurfaceLevel(0,&surface);
	//create vertex buffer
	if(FAILED(device->CreateVertexBuffer(4*sizeof(MultiTexVertex),D3DUSAGE_WRITEONLY,
		MultiTexVertex::FVF,D3DPOOL_MANAGED,&vertexBuffer,NULL)))
		return 0;
	float PanelWidth = (float)windowWidth;
	float PanelHeight = (float)windowHeight;
	MultiTexVertex* pVertices = NULL;
	vertexBuffer->Lock(0, 4 * sizeof(MultiTexVertex), (void**)&pVertices, 0);
	//Set all the colors to white
	pVertices[0].color = pVertices[1].color = pVertices[2].color = pVertices[3].color = 0xffffffff;
	//Set positions and texture coordinates
	pVertices[0].x = pVertices[3].x = -PanelWidth / 2.0f;
	pVertices[1].x = pVertices[2].x = PanelWidth / 2.0f;
	pVertices[0].y = pVertices[1].y = PanelHeight / 2.0f;
	pVertices[2].y = pVertices[3].y = -PanelHeight / 2.0f;
	pVertices[0].z = pVertices[1].z = pVertices[2].z = pVertices[3].z = 1.0f;
	pVertices[1].u0 = pVertices[2].u0 = 1.0f;
	pVertices[0].u0 = pVertices[3].u0 = 0.0f;
	pVertices[0].v0 = pVertices[1].v0 = 0.0f;
	pVertices[2].v0 = pVertices[3].v0 = 1.0f;

	pVertices[1].u1 = pVertices[2].u1 = 1.0f;
	pVertices[0].u1 = pVertices[3].u1 = 0.0f;
	pVertices[0].v1 = pVertices[1].v1 = 0.0f;
	pVertices[2].v1 = pVertices[3].v1 = 1.0f;

	pVertices[1].u2 = pVertices[2].u2 = 1.0f;
	pVertices[0].u2 = pVertices[3].u2 = 0.0f;
	pVertices[0].v2 = pVertices[1].v2 = 0.0f;
	pVertices[2].v2 = pVertices[3].v2 = 1.0f;
	vertexBuffer->Unlock();

	// Turn off culling
	device->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	// Turn off D3D lighting
	device->SetRenderState( D3DRS_LIGHTING, FALSE );

	// Turn on the zbuffer
	device->SetRenderState( D3DRS_ZENABLE, TRUE );
	// Set texture states
	device->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	device->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	device->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	device->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );


	//get handles
	BaseTexHandle = MultiTexCT->GetConstantByName(0,"BaseTex");
	pyramidTex0Handle = MultiTexCT->GetConstantByName(0,"PyramidTex");
	pyramidTex1Handle = MultiTexCT->GetConstantByName(0,"PyramidTexBackUp");
	frameIndexHandle = MultiTexCT->GetConstantByName(0,"frameIndex");
	ddxHandle = MultiTexCT->GetConstantByName(0,"ddx");
	ddyHandle = MultiTexCT->GetConstantByName(0,"ddy");
	UINT count;
	MultiTexCT->GetConstantDesc(BaseTexHandle,&texture0Desc,&count);
	MultiTexCT->GetConstantDesc(pyramidTex0Handle,&texture1Desc,&count);
	MultiTexCT->GetConstantDesc(pyramidTex1Handle,&texture2Desc,&count);
	MultiTexCT->SetDefaults(device);
	return 1;
}

HRESULT Direct3DDevice::UpdateTexture(BYTE *pData,LONG width,LONG height)
{
	frameIndex++;
	RECT           client;  // used to hold client rectangle
	//declare a lock rectangle area
	D3DLOCKED_RECT d3dr;
	D3DLOCKED_RECT textureRect;
	D3DLOCKED_RECT textureSpecialRect;
	LPDIRECT3DTEXTURE9 texture;
	if((frameIndex & 0x01) == 1)
	{
		texture = texture1;
	}
	else
	{
		texture = texture2;
	}
	//lock the texture surface
	if(!texture0)
		return S_FALSE;
	texture0->LockRect(0,&d3dr,0,0);
	texture->LockRect(0,&textureRect,0,0);
	UINT *pTexture1Dest = (UINT*)textureRect.pBits;
	UINT *pDest = (UINT*)d3dr.pBits;
	UINT *pSpecial = NULL;
	if(frameIndex == 1)
	{
		texture2->LockRect(0,&textureSpecialRect,0,0);
		pSpecial = (UINT *)textureSpecialRect.pBits;
	}
 	int mempitch = d3dr.Pitch>>2;
	//convert the Y0U0Y1V0 to YUV
	int count = 0;
	UINT * data = (UINT *)pData;
	for(int index_height=0; index_height<height; index_height++)
	{
		for(int index_width=0; index_width<width; index_width++)
		{
// 			unsigned char y = pData[index_width*2+2*width*index_height];
// 			unsigned char u,v;
// 			if(index_height%2==0||index_width%2==0)
// 			{
// 				u = pData[(index_width*2+1)+2*width*index_height];
// 				v = pData[(index_width*2+3)+2*width*index_height];
// 			}
// 			else
// 			{
// 				u = pData[(index_width*2-1)+2*width*index_height];
// 				v = pData[(index_width*2+1)+2*width*index_height];
// 			}
			pDest[index_width + mempitch*(index_height)] = data[index_width + (height - index_height - 1) * width];
			UINT color =  data[index_width + (height - index_height - 1) * width];
			unsigned char r,g,b;
			r = (color & 0x00ff0000) >> 16; g = (color & 0x0000ff00) >> 8;b = color & 0x000000ff;
			unsigned char y = (r >> 2) + (r >> 4) + (g >> 1) + (g >> 4) + (b >> 3);
			pDest[index_width + mempitch*(index_height)] = data[index_width + (height - index_height - 1) * width] + (y << 24);
			pTexture1Dest[index_width + mempitch*(index_height)] = _RGB32BIT(y,0,0,0);
			if(frameIndex == 1)
			{
				pSpecial[index_width + mempitch*(index_height)] = _RGB32BIT(y,0,0,0);
			}
// 			int index = index_width + index_height *width;
// 			pDest[index_width + mempitch*(index_height)] =_RGB32BIT(pData[index],pData[index+1],pData[index + 2],pData[index + 3]);
// 			pDest[index_width + mempitch*(index_height)] = _RGB32BIT(pData[count << 1] & 0x0f,pData[count << 1] &0x0f,pData[count << 1] &0xf0,pData[count << 1 + 1] &0xf0);
// 			count++;
		}
	}
	CreatePyramid(textureRect,width,height);
	if(frameIndex == 1)
	{
		CreatePyramid(textureSpecialRect,width,height);
		texture2->UnlockRect(0);
	}
	texture0->UnlockRect(0);
	texture->UnlockRect(0);
	RenderVideoStream(width,height);
	return S_OK;
}

HRESULT Direct3DDevice::RenderVideoStream(LONG width,LONG height)
{
	device->Clear(0,0,D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,0xffffffff,1.0f,0);
	device->BeginScene();
	//effect shader processing
	device->SetPixelShader(MultiTexPS);
	device->SetStreamSource(0,vertexBuffer,0,sizeof(MultiTexVertex));
	device->SetFVF(MultiTexVertex::FVF);

	device->SetTexture(0,texture0);
	// Add filtering
	device->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	device->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	device->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	device->SetTexture(texture1Desc.RegisterIndex,texture1);
	// Add filtering
	device->SetSamplerState( texture1Desc.RegisterIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	device->SetSamplerState( texture1Desc.RegisterIndex, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	device->SetSamplerState( texture1Desc.RegisterIndex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

	device->SetTexture(texture2Desc.RegisterIndex,texture2);
	// Add filtering
	device->SetSamplerState( texture2Desc.RegisterIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	device->SetSamplerState( texture2Desc.RegisterIndex, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	device->SetSamplerState( texture2Desc.RegisterIndex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

	MultiTexCT->SetInt(device,frameIndexHandle,frameIndex & 0x01);
	MultiTexCT->SetFloat(device , ddxHandle , 1 / windowWidth);
	MultiTexCT->SetFloat(device , ddyHandle , 1 / windowHeight);
	device->DrawPrimitive(D3DPT_TRIANGLEFAN,0,2);
	device->EndScene();
	device->Present(NULL,NULL,NULL,NULL);
	return (LRESULT) TRUE ; 
}

HRESULT Direct3DDevice::CreatePyramid(D3DLOCKED_RECT textureRect,LONG width,LONG height)
{
	UINT *pTexture1Dest = (UINT*)textureRect.pBits;
	int mempitch = textureRect.Pitch>>2;
	// Create Pyramid Texture
	float para0 = 1.0f / 4.0f;
	float para1 = 1.0f / 8.0f;
	float para2 = 1.0f / 16.0f;
	for(int index_height=0; index_height<height >> 1; index_height++)
	{
		for(int index_width=0; index_width<width >> 1; index_width++)
		{
			int tempHeight = index_height >> 1;
			int tempWidth = index_width >> 1;
			// init level 1
			unsigned char u = para0 * (float)((pTexture1Dest[tempWidth + mempitch * tempHeight] & 0xff000000) >> 24)
				+ para1 * ((float)((pTexture1Dest[Clamp(tempWidth - 1,0,width) + mempitch * tempHeight] & 0xff000000) >> 24) + 
				(float)((pTexture1Dest[Clamp(tempWidth + 1,0,width) + mempitch * tempHeight] & 0xff000000) >> 24) +
				(float)((pTexture1Dest[tempWidth + Clamp(mempitch * tempHeight -1,0,height)] & 0xff000000) >> 24) + 
				(float)((pTexture1Dest[tempWidth + Clamp(mempitch * tempHeight + 1,0,height)] & 0xff000000) >> 24)) +
				para2 * ((float)((pTexture1Dest[Clamp(tempWidth - 1,0,width), + Clamp(mempitch * tempHeight -1,0,height)] & 0xff000000) >> 24) + 
				(float)((pTexture1Dest[Clamp(tempWidth +tempHeight + 1,0,width), + Clamp(mempitch * tempHeight + 1,0,height)] & 0xff000000) >> 24) +
				(float)((pTexture1Dest[Clamp(tempWidth + 1,0,width), + Clamp(mempitch  * tempHeight -1,0,height)] & 0xff000000) >> 24) + 
				(float)((pTexture1Dest[Clamp(tempWidth - 1,0,width), + Clamp(mempitch  * tempHeight + 1,0,height)] & 0xff000000) >> 24));
			pTexture1Dest[index_width + mempitch*(index_height)] += (u << 16);
			float u3 = para0 * (float)((pTexture1Dest[tempWidth + mempitch * tempHeight] & 0xff000000) >> 24);
			unsigned char u1 = para0 * (float)(pTexture1Dest[tempWidth + mempitch * tempHeight] & 0xff000000 >> 24);
 			unsigned char u2 = para1 * (float)(pTexture1Dest[tempWidth + mempitch  * tempHeight] & 0xff000000 >> 24);
		}
	}
	for(int index_height=0; index_height<height >> 2; index_height++)
	{
		for(int index_width=0; index_width<width >> 2; index_width++)
		{
			int tempHeight = index_height >> 1;
			int tempWidth = index_width >> 1;
			// init level 2
			unsigned char u = para0 * (float)((pTexture1Dest[tempWidth  + mempitch  * tempHeight] & 0x00ff0000) >> 16)
				+ para1 * ((float)((pTexture1Dest[Clamp(tempWidth  - 1,0,width >> 1) + mempitch  * tempHeight] & 0x00ff0000) >> 16) + 
				(float)((pTexture1Dest[Clamp(tempWidth  + 1,0,width>> 1) + mempitch  * tempHeight] & 0x00ff0000) >> 16) +
				(float)((pTexture1Dest[index_width << 1, + Clamp(mempitch  * tempHeight -1,0,height>> 1)] & 0x00ff0000) >> 16) + 
				(float)((pTexture1Dest[index_width << 1, + Clamp(mempitch  * tempHeight + 1,0,height>> 1)] & 0x00ff0000) >> 16)) +
				para2 * ((float)((pTexture1Dest[Clamp(tempWidth  - 1,0,width>> 1), + Clamp(mempitch  * tempHeight -1,0,height>> 1)] & 0x00ff0000) >> 16) + 
				(float)((pTexture1Dest[Clamp(tempWidth  +tempHeight + 1,0,width>> 1), + Clamp(mempitch  * tempHeight + 1,0,height>> 1)] & 0x00ff0000) >> 16) +
				(float)((pTexture1Dest[Clamp(tempWidth  + 1,0,width>> 1), + Clamp(mempitch  * tempHeight -1,0,height>> 1)] & 0x00ff0000) >> 16) + 
				(float)((pTexture1Dest[Clamp(tempWidth - 1,0,width>> 1), + Clamp(mempitch  * tempHeight + 1,0,height>> 1)] & 0x00ff0000) >> 16));
			pTexture1Dest[index_width + mempitch*(index_height)] += (u << 8);
		}
	}

	return S_OK;
}
