#pragma  once
#include <d3d9.h>
#include <D3DX10Math.h>

class  Direct3DDevice
{
public:
	// device parameter
	D3DPRESENT_PARAMETERS d3dpp;
	int windowWidth;
	int windowHeight;

	~ Direct3DDevice(); 

	static Direct3DDevice * GetInstance();
	static void DestroyInstance();
	HRESULT CreateDevice(HWND window,bool windowed,int windowWidth,int windowHeight);
	int CheckDeviceCap();
	void InitPresentParamater(HWND window,bool windowed,int windowWidth,int windowHeight);
	void InitRenderPara();
	void RenderTarget(LPDIRECT3DVERTEXBUFFER9 vertexBuffer,LPDIRECT3DINDEXBUFFER9 indexBuffer);
	void InitGeometry(LPDIRECT3DVERTEXBUFFER9  vb,LPDIRECT3DINDEXBUFFER9 ib);
	void RenderTutorial();
private:
	IDirect3D9 *_d3d9;
	D3DCAPS9 caps;
	IDirect3DDevice9 *device;
	static Direct3DDevice *Instance;

	Direct3DDevice();
};

typedef struct VERTEX
{
	~VERTEX(){}
	VERTEX(float x,float y,float z,DWORD color)
	{
		_x = x;
		_y = y;
		_z = z;
		_color = color;

	}
	FLOAT _x, _y, _z; // The transformed position for the vertex.
	DWORD _color;        // The vertex color.

	const static DWORD FVF;
}Vertex,PVertex;
