#ifndef _VIDEOCAPTURE_H_
#define _VIDEOCAPTURE_H_ 

#include <string.h>  
#include <string>  
#include <windows.h>
#include <dshow.h>

#include "Qedit.h"
#include "Direct3DDevice.h"
using  namespace  std;  

#define WM_GRAPHNOTIFY  WM_APP+1

enum PLAYSTATE {Stopped, Paused, Running, Init};
// Hook to redirect stream data

typedef struct  DEVICE_DESC
{
	string deviceName;
	string deviceDesc;
	string devicePath;
	
	int id;

	string ToString()
	{

		return "Device Description is " + deviceDesc + "/r/n"
			+ "Device Path is " + devicePath + "/r/n";
	}
}DeviceDesc,*PDeviceDesc;

class VideoCapture
{
public:
	LPCWSTR m_errorMessage; 
	PLAYSTATE currentPlayState;
	// The preview window instance handler
	HWND hwndOwner;	
	IVideoWindow  * pVideoWindow;
	ISampleGrabber *pSampleGrabber;
	IBaseFilter *pCaptureFilter;
	IMemInputPin * pOutMemPin;

	LONG lWidth;
	LONG lHeight;

	HRESULT CreateInstance(HWND hwmd);
	int CreateDeviceList(HWND hwndList);
	void BindDevice(LPCWSTR deviceDesc);
	// Setup video window to preview video
	HRESULT SetupVideoWindow();
	// Change the video preview mode when the preview windows focused or not
	HRESULT ChangePreviewState(int nShow);
	// Resize the preview window size when the window resized
	void ResizeVideoWindow(void);
	// Handle the graph events
	HRESULT HandleGraphEvent(void);
	// Receive data 
	HRESULT ReceiveData(BYTE *buffer,DWORD len);
	VideoCapture();
	~VideoCapture();
private:
	IGraphBuilder *pGraph;
	ICaptureGraphBuilder2 *pBuild;
	IEnumMoniker *pEnum;
	IMediaEventEx * pMediaEvent;
	IMediaControl * pMediaControl;
	bool deviceSelected;
	HRESULT EnumerateDevices(REFGUID category, IEnumMoniker **ppEnum);
	HRESULT RedirectOutputData(IBaseFilter *pFilter);
	HRESULT DeleteMediaType(AM_MEDIA_TYPE *pmtConfig);
};
#endif // !_VIDEOCAPTURE_H_
