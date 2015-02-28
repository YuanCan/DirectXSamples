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
	LPDIRECT3DVERTEXBUFFER9 Vb;
	LPDIRECT3DINDEXBUFFER9 Ib;

	~ Direct3DDevice(); 

	static Direct3DDevice * GetInstance();
	static void DestroyInstance();
	HRESULT CreateDevice(HWND window,bool windowed,int windowWidth,int windowHeight);
	int CheckDeviceCap();
	void InitPresentParamater(HWND window,bool windowed,int windowWidth,int windowHeight);
	void InitRenderPara();
	void Direct3DDevice::ResetRenderPara(int windowWidth,int windowHeight);
	void RenderTarget();
	HRESULT InitGeometry();
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
	VERTEX(float x,float y,float z)
	{
		_x = x;
		_y = y;
		_z = z;

	}
	FLOAT _x, _y, _z; // The transformed position for the vertex.

	const static DWORD FVF;
}Vertex,PVertex;
