#include "pch.h"
#include "OSDevice.h"

#include <Windows.h>
#include "Storage/Dynamic_Array.h"
#include "HID.h"

#include <hidsdi.h>

using namespace BB;

typedef FreeListAllocator_t OSAllocator_t;
typedef LinearAllocator_t OSTempAllocator_t;

OSAllocator_t OSAllocator{ mbSize * 8 };
OSTempAllocator_t OSTempAllocator{ mbSize * 4 };

static OSDevice osDevice;

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
	void Init(OS_WINDOW_STYLE a_Style, int a_X, int a_Y, int a_Width, int a_Height, const char* a_WindowName)
	{
		m_WindowName = a_WindowName;

		WNDCLASS t_WndClass = {};
		t_WndClass.lpszClassName = m_WindowName;
		t_WndClass.hInstance = m_HInstance;
		t_WndClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
		t_WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		t_WndClass.lpfnWndProc = WindowProc;

		RegisterClass(&t_WndClass);
		//DWORD t_Style = WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;

		DWORD t_Style;
		switch (a_Style)
		{
		case BB::OS_WINDOW_STYLE::MAIN:
			t_Style = WS_OVERLAPPEDWINDOW;
			break;
		case BB::OS_WINDOW_STYLE::CHILD:
			t_Style = WS_OVERLAPPED | WS_THICKFRAME;
			break;
		default:
			t_Style = 0;
			BB_ASSERT(false, "Tried to create a window with a OS_WINDOW_STYLE it does not accept.");
			break;
		}

		RECT t_Rect{};
		t_Rect.left = a_X;
		t_Rect.top = a_Y;
		t_Rect.right = t_Rect.left + a_Width;
		t_Rect.bottom = t_Rect.top + a_Height;

		AdjustWindowRect(&t_Rect, t_Style, false);

		hwnd = CreateWindowEx(
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

		ShowWindow(hwnd, SW_SHOW);
	}

	void Destroy()
	{
		//Delete the window before you unregister the class.
		if (!DestroyWindow(hwnd))
			osDevice.LatestOSError();

		if (!UnregisterClassA(m_WindowName, m_HInstance))
			osDevice.LatestOSError();
	}

	HWND hwnd = nullptr;

private:
	const char* m_WindowName = nullptr;
	HINSTANCE m_HInstance = nullptr;
};


struct BB::OSDevice_o
{
	//Special array for all the windows. Stored seperately 
	Dynamic_Array<OSWindow> OSWindows{ OSAllocator, 8 };
};


OSDevice& BB::AppOSDevice()
{
	return osDevice;
}

OSDevice::OSDevice()
{
	m_OSDevice = BBalloc<OSDevice_o>(OSAllocator);
}

OSDevice::~OSDevice()
{
	BBfree(OSAllocator, m_OSDevice);
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

FrameworkHandle OSDevice::CreateOSWindow(OS_WINDOW_STYLE a_Style, int a_X, int a_Y, int a_Width, int a_Height, const char* a_WindowName)
{
	OSWindow t_OSWindow;
	t_OSWindow.Init(a_Style, a_X, a_Y, a_Width, a_Height, a_WindowName);

	size_t t_OSWindowsSize = m_OSDevice->OSWindows.size();

	for (size_t i = 0; i < t_OSWindowsSize; i++)
	{
		if (m_OSDevice->OSWindows[i].hwnd == nullptr)
		{
			m_OSDevice->OSWindows[i] = t_OSWindow;
			return FrameworkHandle(FRAMEWORK_RESOURCE_TYPE::WINDOW, static_cast<uint32_t>(i));
		}
	}

	m_OSDevice->OSWindows.push_back(t_OSWindow);

	return FrameworkHandle(FRAMEWORK_RESOURCE_TYPE::WINDOW, static_cast<uint32_t>(t_OSWindowsSize));
}

void BB::OSDevice::DestroyOSWindow(FrameworkHandle a_Handle)
{
	BB_ASSERT(a_Handle.type == FRAMEWORK_RESOURCE_TYPE::WINDOW, "Framework handle is not of type WINDOW when calling DestroyOSWindow!");

	//Don't delete it from the array but call the deconstructor.
	m_OSDevice->OSWindows[a_Handle.index].Destroy();

	//Instead of deleting the entry mark it as empty, so that it may be used again.
	m_OSDevice->OSWindows[a_Handle.index].hwnd = nullptr;
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