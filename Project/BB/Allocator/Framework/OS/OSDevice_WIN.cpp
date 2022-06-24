#include "pch.h"
#include "OSDevice.h"
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

using namespace BB;

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
class BB::OSWindow
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
	HINSTANCE m_HInstance;
	HWND m_Hwnd;
};


OSDevice& BB::AppOSDevice()
{
	return osDevice;
}

OSDevice::OSDevice()
{
	SYSTEM_INFO t_SystemInfo;
	GetSystemInfo(&t_SystemInfo);
	virtualMemoryPageSize = t_SystemInfo.dwAllocationGranularity;
}

OSDevice::~OSDevice()
{
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
	window = new OSWindow(a_X, a_Y, a_Width, a_Height, a_WindowName);
	return FRAMEWORK_NULL_HANDLE;
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