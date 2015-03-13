#pragma  once
#include <d3d9.h>
#include <D3DX10Math.h>
#include <D3DX9Effect.h>
#include <D3DX9Shader.h>

#define _RGB32BIT(a,r,g,b)            (UINT)((b) + ((g) << 8) + ((r) << 16) + ((a) << 24))

class  Direct3DDevice
{
public:
	// device parameter
	D3DPRESENT_PARAMETERS d3dpp;
	int windowWidth;
	int windowHeight;
	LPDIRECT3DVERTEXBUFFER9 Vb;
	LPDIRECT3DINDEXBUFFER9  Ib;
	//Direct3D texture
	LPDIRECT3DTEXTURE9      texture0;
	LPDIRECT3DTEXTURE9      texture1;
	// Texture to build change chain
	LPDIRECT3DTEXTURE9      texture2;
	D3DXCONSTANT_DESC		texture0Desc;
	D3DXCONSTANT_DESC		texture1Desc;
	D3DXCONSTANT_DESC		texture2Desc;
	//Direct3D surface
	LPDIRECT3DSURFACE9      surface;
	//vertex buffer
	LPDIRECT3DVERTEXBUFFER9 vertexBuffer;
	//the texture surface
	D3DSURFACE_DESC desc;
	ID3DXEffect*            pEffect;  //effect pointer
	//const variable handle
	D3DXHANDLE              hTechScreen;  //Effect handle  
	D3DXHANDLE              hTexScreen;  //texture handle 

	IDirect3DPixelShader9*  MultiTexPS ;
	ID3DXConstantTable*     MultiTexCT ;
	D3DXHANDLE			    BaseTexHandle;
	D3DXHANDLE              pyramidTex0Handle;
	D3DXHANDLE              pyramidTex1Handle;
	D3DXHANDLE              frameIndexHandle;
	D3DXHANDLE              ddxHandle;
	D3DXHANDLE              ddyHandle;

	~Direct3DDevice(); 

	static Direct3DDevice * GetInstance();
	static void DestroyInstance();
	HRESULT CreateDevice(HWND window,bool windowed,int windowWidth,int windowHeight);
	int CheckDeviceCap();
	void InitPresentParamater(HWND window,bool windowed,int windowWidth,int windowHeight);
	// Init the render in normal test mode
	void InitRenderPara();
	// Init the render para in video capture and processing mode
	HRESULT InitVideoCaptureModePara();
	// Update the texture data
	HRESULT UpdateTexture(BYTE *pData,LONG width,LONG height);
	HRESULT CreatePyramid(D3DLOCKED_RECT textureRect,LONG width,LONG height);
	// Render video stream
	HRESULT RenderVideoStream(LONG width,LONG height);

	void Direct3DDevice::ResetRenderPara(int windowWidth,int windowHeight);
	void RenderTarget();
	HRESULT InitGeometry();
	void RenderTutorial();

	// Static helper function
	static int Clamp(int data,int min,int max)
	{
		if(data <= min)
		{
			return min;
		}
		else if(data >= max)
		{
			return max;
		}
		else
		{
			return data;
		}
	}
private:
	IDirect3D9 *_d3d9;
	D3DCAPS9 caps;
	IDirect3DDevice9 *device;
	int frameIndex;
	static Direct3DDevice *Instance;

	// Singleton set the constructor private
	Direct3DDevice();
};

typedef struct VERTEX
{
	~VERTEX(){}
	VERTEX(float x,float y,float z)
	{
		_x = x;
		_y = y;
		_z = z;

	}
	FLOAT _x, _y, _z; // The transformed position for the vertex.

	const static DWORD FVF;
}Vertex,*PVertex;

typedef struct MULTITEXVERTEX{
	FLOAT x,  y,  z;
	UINT color;
	FLOAT u0 ,v0; //texture coordinates
	FLOAT u1,v1;
	FLOAT u2,v2;

	~MULTITEXVERTEX(){}
	MULTITEXVERTEX(){}

	const static DWORD FVF; 
}MultiTexVertex;
