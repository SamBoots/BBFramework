#include "pch.h"
#include "OSDevice.h"
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include "Storage/Dynamic_Array.h"

using namespace BB;

typedef FreeListAllocator_t OSAllocator_t;
typedef LinearAllocator_t OSTempAllocator_t;

OSAllocator_t OSAllocator{ mbSize * 8 };
OSTempAllocator_t OSTempAllocator{ mbSize * 4 };

static OSDevice osDevice;

struct BB::OSDevice_o
{
	//Allocate 64 elements that the OS might keep for the user.
	Dynamic_Array<void*, FreeListAllocator_t> OSResources{ OSAllocator, 64 };
};

//Custom callback for the Windows proc.
LRESULT CALLBACK WindowProc(HWND a_Hwnd, UINT a_Msg, WPARAM a_WParam, LPARAM a_LParam)
{
	switch (a_Msg)
	{
	case WM_CLOSE:
		DestroyWindow(a_Hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(a_Hwnd, a_Msg, a_WParam, a_LParam);
}

//The OS window for Windows.
class OSWindow
{
public:
	OSWindow(int a_X, int a_Y, int a_Width, int a_Height, const char* a_WindowName)
		: m_WindowName(a_WindowName)
	{
		WNDCLASS t_WndClass = {};
		t_WndClass.lpszClassName = m_WindowName;
		t_WndClass.hInstance = m_HInstance;
		t_WndClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
		t_WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		t_WndClass.lpfnWndProc = WindowProc;

		RegisterClass(&t_WndClass);
		//DWORD t_Style = WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;

		DWORD t_Style = WS_OVERLAPPEDWINDOW;

		RECT t_Rect{};
		t_Rect.left = a_X;
		t_Rect.top = a_Y;
		t_Rect.right = t_Rect.left + a_Width;
		t_Rect.bottom = t_Rect.top + a_Height;

		AdjustWindowRect(&t_Rect, t_Style, false);

		m_Hwnd = CreateWindowEx(
			0,
			m_WindowName,
			"Memory Studies",
			t_Style,
			t_Rect.left,
			t_Rect.top,
			t_Rect.right - t_Rect.left,
			t_Rect.bottom - t_Rect.top,
			NULL,
			NULL,
			m_HInstance,
			NULL
		);

		ShowWindow(m_Hwnd, SW_SHOW);
	};

	~OSWindow()
	{
		//Delete the window before you unregister the class.
		if (!DestroyWindow(m_Hwnd))
			osDevice.LatestOSError();

		if (!UnregisterClassA(m_WindowName, m_HInstance))
			osDevice.LatestOSError();
	}

private:
	const char* m_WindowName;
	HINSTANCE m_HInstance = nullptr;
	HWND m_Hwnd;
};


OSDevice& BB::AppOSDevice()
{
	return osDevice;
}

OSDevice::OSDevice()
{
	m_OSDevice = BBalloc<OSDevice_o, OSAllocator_t>(OSAllocator);
}

OSDevice::~OSDevice()
{
	BBFree<OSAllocator_t>(OSAllocator, m_OSDevice);
}

const size_t BB::OSDevice::VirtualMemoryPageSize() const
{
	SYSTEM_INFO t_Info;
	GetSystemInfo(&t_Info);
	return t_Info.dwPageSize;
}

const size_t BB::OSDevice::VirtualMemoryMinimumAllocation() const
{
	SYSTEM_INFO t_Info;
	GetSystemInfo(&t_Info);
	return t_Info.dwAllocationGranularity;
}

const uint32_t OSDevice::LatestOSError() const
{
	DWORD t_LatestError = GetLastError();
	if (t_LatestError != 0x00)
	{
		printf("OSDevice Error:");
		printf("%u", t_LatestError);
	}

	return static_cast<uint32_t>(t_LatestError);
}

framework_handle OSDevice::CreateOSWindow(int a_X, int a_Y, int a_Width, int a_Height, const char* a_WindowName)
{
	void* t_OSWindow = BBalloc<OSWindow, OSAllocator_t>(OSAllocator, a_X, a_Y, a_Width, a_Height, a_WindowName);
	m_OSDevice->OSResources.push_back(t_OSWindow);

	return framework_handle(FRAMEWORK_RESOURCE_TYPE::WINDOW, m_OSDevice->OSResources.size() - 1);
}

void BB::OSDevice::DestroyOSWindow(framework_handle a_Handle)
{
	BB_ASSERT(a_Handle.type == FRAMEWORK_RESOURCE_TYPE::WINDOW, "Framework handle is not of type WINDOW when calling DestroyOSWindow!");
	BBFree<OSAllocator_t>(OSAllocator, m_OSDevice->OSResources[a_Handle.index]);
}

void OSDevice::ExitApp() const
{
	exit(EXIT_FAILURE);
}

bool BB::OSDevice::ProcessMessages() const
{
	MSG t_Msg{};

	while (PeekMessage(&t_Msg, nullptr, 0u, 0u, PM_REMOVE))
	{
		if (t_Msg.message == WM_QUIT)
		{
			return false;
		}

		TranslateMessage(&t_Msg);
		DispatchMessage(&t_Msg);
	}

	return true;
}