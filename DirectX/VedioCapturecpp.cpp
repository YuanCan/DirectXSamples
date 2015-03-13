#include "stdafx.h"
#include "VideoCaptue.h"
#include "atlconv.h"

VideoCapture::VideoCapture()
{
	deviceSelected = false;
}

HRESULT VideoCapture::CreateInstance(HWND hwmd)
{
	// Create the Capture Graph Builder.
	CoInitialize(NULL);
	hwndOwner = hwmd;
	HRESULT hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, 
		CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuild );
	if (SUCCEEDED(hr))
	{
		// Create the Filter Graph Manager.
		hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER,
			IID_IGraphBuilder, (void**)&pGraph);
		if (SUCCEEDED(hr))
		{
			// Initialize the Capture Graph Builder.
			pBuild->SetFiltergraph(pGraph);
		}
		else
		{
			pBuild->Release();
		}
	}
	else
	{
		m_errorMessage = L"Create  Filter Graph Manager Com instance fail/r/n";
		return hr;
	}

	// Obtain interfaces for media control and Video Window
	hr = pGraph->QueryInterface(IID_IMediaControl,(LPVOID *) &pMediaControl);
	if (FAILED(hr))
	{
		m_errorMessage = L"Create  query media control interface fail/r/n";
		return hr;
	}

	hr = pGraph->QueryInterface(IID_IVideoWindow, (LPVOID *) &pVideoWindow);
	if (FAILED(hr))
	{
		m_errorMessage = L"Create  query video window interface fail/r/n";
		return hr;
	}
	hr = pGraph->QueryInterface(IID_IMediaEventEx, (LPVOID *) &pMediaEvent);
	if (FAILED(hr))
	{
		m_errorMessage = L"Create  query media event interface fail/r/n";
		return hr;
	}

	// Set the window handle used to process graph events
	hr = pMediaEvent->SetNotifyWindow((OAHWND)hwmd, WM_GRAPHNOTIFY, 0);
	if (FAILED(hr))
	{
		m_errorMessage = L"Set media notify window fail/r/n";
		return hr;
	}
	return hr;
}

VideoCapture::~VideoCapture()
{
	// Stop previewing data
	if (pMediaControl)
		pMediaControl->StopWhenReady();

	currentPlayState = Stopped;

	// Stop receiving events
	if (pMediaEvent)
		pMediaEvent->SetNotifyWindow(NULL, WM_GRAPHNOTIFY, 0);

	// Relinquish ownership (IMPORTANT!) of the video window.
	// Failing to call put_Owner can lead to assert failures within
	// the video renderer, as it still assumes that it has a valid
	// parent window.
	if(pVideoWindow)
	{
		pVideoWindow->put_Visible(OAFALSE);
		pVideoWindow->put_Owner(NULL);
	}
	SAFE_RELEASE(pOutMemPin);
	SAFE_RELEASE(pCaptureFilter);
	SAFE_RELEASE(pMediaControl);
	SAFE_RELEASE(pMediaEvent);
	SAFE_RELEASE(pVideoWindow);
	SAFE_RELEASE(pEnum);
	SAFE_RELEASE(pBuild);
	SAFE_RELEASE(pGraph);
	CoUninitialize();
}

int VideoCapture::CreateDeviceList(HWND hwndList)
{
	IMoniker *pMoniker = NULL;
	if(pMoniker == NULL)
	{
		HRESULT hr = EnumerateDevices(CLSID_VideoInputDeviceCategory, &pEnum);
		if(FAILED(hr))
		{
			m_errorMessage = L"Create Devices enumerator com instance fail /r/n";
			return -1;
		}
	}
	while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
	{
		IPropertyBag *pPropBag;
		HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
		if (FAILED(hr))
		{
			pMoniker->Release();
			continue;  
		} 

		VARIANT var;
		VariantInit(&var);

		// Get description or friendly name.
		hr = pPropBag->Read(L"Description", &var, 0);
		if (FAILED(hr))
		{
			hr = pPropBag->Read(L"FriendlyName", &var, 0);
		}
		if (SUCCEEDED(hr))
		{
			// Add it to the application's list box.
			USES_CONVERSION;
			(long)SendMessage(hwndList, CB_ADDSTRING, 0, 
				(LPARAM)OLE2T(var.bstrVal));
			VariantClear(&var);  
		}

		hr = pPropBag->Write(L"FriendlyName", &var);

		hr = pPropBag->Read(L"DevicePath", &var, 0);
		if (SUCCEEDED(hr))
		{
			VariantClear(&var); 
		}
		pPropBag->Release();
		pMoniker->Release();
	}
	return 0;
}

HRESULT VideoCapture::EnumerateDevices(REFGUID category, IEnumMoniker **ppEnum)
{
	// Create the System Device Enumerator.
	ICreateDevEnum *pDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,  
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));

	if (SUCCEEDED(hr))
	{
		// Create an enumerator for the category.
		hr = pDevEnum->CreateClassEnumerator(category, ppEnum, 0);
		if (hr == S_FALSE)
		{
			hr = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
		}
		pDevEnum->Release();
	}
	return hr;
}

void VideoCapture::BindDevice(LPCWSTR deviceDesc)
{
	SAFE_RELEASE(pOutMemPin);
	IMoniker *pMoniker = NULL;
	if(pMoniker == NULL)
	{
		HRESULT hr = EnumerateDevices(CLSID_VideoInputDeviceCategory, &pEnum);
		if(FAILED(hr))
		{
			m_errorMessage = L"Create Devices enumerator com instance fail /r/n";
		}
	}
	LPCWSTR desc;
	int offset = 0;
	bool eqauls = false;
	while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
	{
		IPropertyBag *pPropBag;
		HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
		if (FAILED(hr))
		{
			pMoniker->Release();
			continue;  
		} 

		VARIANT var;
		VariantInit(&var);

		// Get description or friendly name.
		hr = pPropBag->Read(L"Description", &var, 0);
		if (FAILED(hr))
		{
			hr = pPropBag->Read(L"FriendlyName", &var, 0);
		}
		if (SUCCEEDED(hr))
		{
			USES_CONVERSION;
			desc = (LPCWSTR)OLE2T(var.bstrVal);
			while (desc[offset])
			{
				if(deviceDesc[offset] != desc[offset])
				{
					eqauls = false;
					break;
				}
				offset++;
				eqauls = true;
			}
			VariantClear(&var); 
		}
		hr = pPropBag->Write(L"FriendlyName", &var);
		hr = pPropBag->Read(L"DevicePath", &var, 0);
		if (SUCCEEDED(hr))
		{
			VariantClear(&var); 
		}
		// Copy the found filter pointer to the output parameter.
		IBaseFilter * src = NULL; 
		if(eqauls)
		{
			hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&src);
			if (SUCCEEDED(hr))
			{
				if(pCaptureFilter)
				{
					pGraph->RemoveFilter(pCaptureFilter);
					pCaptureFilter->Release();
				}
				pCaptureFilter = src;
				src->AddRef();

				hr = pGraph->AddFilter(src, L"Capture Filter");

				IEnumPins*      pins = 0; 
				hr = src?src->EnumPins(&pins):0;   // we need output pin to auto generate rest of the graph
				IPin*           pin  = 0;  
				hr = pins?pins->Next(1,&pin, 0):0; // via graph->Render
 				IAMStreamConfig* pConfig = 0; 
				hr = pin->QueryInterface( IID_IAMStreamConfig, (void **)&pConfig);  // (Those are optional steps to set better resolution)
				int iCount = 0, iSize = 0;
				hr = pConfig->GetNumberOfCapabilities(&iCount, &iSize);

				// Check the size to make sure we pass in the correct structure.
				if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
				{
					// Use the video capabilities structure.
					int widthMax = 0;
					AM_MEDIA_TYPE *pmtConfig;
					VIDEO_STREAM_CONFIG_CAPS scc;
					hr = pConfig->GetStreamCaps(0, &pmtConfig, (BYTE*)&scc);
					if (SUCCEEDED(hr))
					{
						/* Examine the format, and possibly use it. */

						// Delete the media type when you are done.
 						VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER*>(pmtConfig->pbFormat);
 						// pVih contains the detailed format information.
						lWidth = pVih->bmiHeader.biWidth;
						lHeight = pVih->bmiHeader.biHeight;
						// Delete a media type structure that was allocated on the heap.
						DeleteMediaType(pmtConfig);
					}
				}

				hr = pBuild->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, 
					src, NULL, NULL);
//				hr = pGraph->Render(pin); // graph builder now builds whole filter chain including MJPG decompression on some webcams

// 				// Redirect the data stream
 				RedirectOutputData(src);

				SetupVideoWindow();
				currentPlayState = Stopped;
				deviceSelected = true;

				//SAFE_RELEASE(pin);
			}
			src->Release();
			pPropBag->Release();
			pMoniker->Release();
			break;
		}
		pPropBag->Release();
		pMoniker->Release();
	}
}

HRESULT VideoCapture::SetupVideoWindow()
{
	HRESULT hr;

	// Set the video window to be a child of the main window
	hr = pVideoWindow->put_Owner((OAHWND)hwndOwner);
	if (FAILED(hr))
		return hr;

	// Set video window style
	hr = pVideoWindow->put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN);
	if (FAILED(hr))
		return hr;

	// Use helper function to position video window in client rect 
	// of main application window
	ResizeVideoWindow();

	// Make the video window visible, now that it is properly positioned
	hr = pVideoWindow->put_Visible(OATRUE);
	if (FAILED(hr))
		return hr;

	return hr;
}

void VideoCapture::ResizeVideoWindow(void)
{
	// Resize the video preview window to match owner window size
	if (pVideoWindow)
	{
		RECT rc;

		// Make the preview video fill our window
		GetClientRect(hwndOwner, &rc);
		pVideoWindow->SetWindowPosition(0, 0, lWidth, lHeight);
	}
}

HRESULT VideoCapture::ChangePreviewState(int nShow)
{
	HRESULT hr=S_OK;
	// no device selected
	if(!deviceSelected)
	{
		return hr;
	}
	// If the media control interface isn't ready, don't call it
	if (!pMediaControl)
		return S_OK;

	if (nShow)
	{
		if (currentPlayState != Running)
		{
			// Start previewing video data
			hr = pMediaControl->Run();
			currentPlayState = Running;
		}
	}
	else
	{
		// Stop previewing video data
		hr = pMediaControl->StopWhenReady();
		currentPlayState = Stopped;
	}

	return hr;
}

HRESULT VideoCapture::HandleGraphEvent(void)
{
	LONG evCode;
	LONG_PTR evParam1, evParam2;
	HRESULT hr=S_OK;

	if (!pMediaEvent)
		return E_POINTER;

	while(SUCCEEDED(pMediaEvent->GetEvent(&evCode, &evParam1, &evParam2, 0)))
	{
		//
		// Free event parameters to prevent memory leaks associated with
		// event parameter data.  While this application is not interested
		// in the received events, applications should always process them.
		//
		hr = pMediaEvent->FreeEventParams(evCode, evParam1, evParam2);

		// Insert event processing code here, if desired
	}

	return hr;
}

HRESULT VideoCapture::RedirectOutputData(IBaseFilter *pFilter)
{
	HRESULT hr = S_OK;
	IEnumFilters * pFilterEnum = NULL;  
	IBaseFilter * pLastFilter = NULL;
	IEnumPins * pPinEnum = NULL;
	IPin *pPin = NULL;
	IMemInputPin*   pMemInputPin  = NULL; 
	IAMStreamConfig *pASMConfig = NULL;
	DWORD no = 0;
	hr = pGraph->EnumFilters(&pFilterEnum); // from all newly added filters
	if(FAILED(hr))
	{
		goto Done;
	}
	pFilterEnum->Next(1,&pLastFilter,0);
	hr = pLastFilter->EnumPins(&pPinEnum);  // because data we are interested in are pumped to render input pin 

	if(FAILED(hr))
	{
		goto Done;
	}
	pPinEnum->Next(1,&pPin, 0);
	hr = pPin->QueryInterface(IID_IMemInputPin,(void**)&pMemInputPin);
	if(FAILED(hr))
	{
		goto Done;
	}
	else
	{
		pOutMemPin = pMemInputPin;
		pMemInputPin->AddRef();
	}

Done:
	// Release instance
	SAFE_RELEASE(pFilterEnum);
	SAFE_RELEASE(pLastFilter);
	SAFE_RELEASE(pPin);
	SAFE_RELEASE(pPinEnum);
	SAFE_RELEASE(pMemInputPin);
	SAFE_RELEASE(pASMConfig);
	return hr;
}

// Get the stream data from hook proc
HRESULT VideoCapture::ReceiveData(BYTE *buffer,DWORD len)
{
	Direct3DDevice::GetInstance()->UpdateTexture(buffer,lWidth,lHeight);
	return S_OK;
}

HRESULT VideoCapture::DeleteMediaType(AM_MEDIA_TYPE *pmtConfig)
{
	if (pmtConfig != NULL)
	{
		if (pmtConfig->cbFormat != 0)
		{
			CoTaskMemFree((PVOID)pmtConfig->pbFormat);
			pmtConfig->cbFormat = 0;
			pmtConfig->pbFormat = NULL;
		}
		if (pmtConfig->pUnk != NULL)
		{
			// Unecessary because pUnk should not be used, but safest.
			pmtConfig->pUnk->Release();
			pmtConfig->pUnk = NULL;
		}
		CoTaskMemFree(pmtConfig);
	}

	return S_OK;
}