#include "pch.h"
#include "OSDevice.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <cstdint>
#include <unistd.h>

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

//The OS window for Windows.
class BB::OSWindow
{
public:
	OSWindow(int a_X, int a_Y, int a_Width, int a_Height, const char* a_WindowName)
		: m_WindowName(a_WindowName)
	{
		m_Display = XOpenDisplay(NULL);
		BB_ASSERT(m_Display != NULL, "Linux XOpenDisplay failed!");
		m_Screen = DefaultScreen(m_Display);

		uint64_t t_Foreground_color = WhitePixel(m_Display, m_Screen);
		uint64_t t_Background_color = BlackPixel(m_Display, m_Screen);

		//Initialize the window with a white foreground and a black background.
		m_Window = XCreateSimpleWindow(m_Display,
			DefaultRootWindow(m_Display),
			a_X,
			a_Y,
			a_Width,
			a_Height,
			5,
			t_Foreground_color,
			t_Background_color);

		//Set window properties
		XSetStandardProperties(m_Display, m_Window, a_WindowName, "EXAMPLE", None, NULL, 0, NULL);

		//The input rules on what is allowed in the input.
		XSelectInput(m_Display, m_Window, ExposureMask | ButtonPressMask | KeyPressMask);

		//Create the graphics context
		m_GraphicContext = XCreateGC(m_Display, m_Window, 0, 0);

		//Set the foreground and background. Wtf is x11.
		XSetForeground(m_Display, m_GraphicContext, t_Foreground_color);
		XSetBackground(m_Display, m_GraphicContext, t_Background_color);

		/* clear the window and bring it on top of the other windows */
		XClearWindow(m_Display, m_Window);
		XMapRaised(m_Display, m_Window);
	};

	~OSWindow()
	{
		XFreeGC(m_Display, m_GraphicContext);
		XDestroyWindow(m_Display, m_Window);
		XCloseDisplay(m_Display);
	}

	_XDisplay* m_Display; //XDisplay, also known as Window.
	const char* m_WindowName;
	int m_Screen; //the current user window.
	XID m_Window; //X11 window ID.
	_XGC* m_GraphicContext; //Graphic context of X11


	//For fun functions.
	void AddChar(char a_Char)
	{
		if (m_CurrentTextPos > 31)
			return;
		m_WindowTextBuffer[m_CurrentTextPos] = a_Char;
		m_CurrentTextPos++;
	}

	void BackOnce()
	{
		if (m_CurrentTextPos > 0)
			m_CurrentTextPos--;
	}

	void FlushChar()
	{
		m_CurrentTextPos = 0;
	}

	uint32_t m_CurrentTextPos = 0;
	char m_WindowTextBuffer[31]; //Just a test to have some fun.
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
	return sysconf(_SC_PAGE_SIZE);
}

const size_t BB::OSDevice::VirtualMemoryMinimumAllocation() const
{
	return sysconf(_SC_PAGE_SIZE);
}

const uint32_t OSDevice::LatestOSError() const
{
	int t_LatestError = errno;
	if (errno != 0)
	{
		printf("OSDevice Error:");
		printf("%i", t_LatestError);
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
	KeySym t_Key;
	char t_Text[255];

	while (XPending(window->m_Display))
	{
		XEvent t_Event;
		XNextEvent(window->m_Display, &t_Event);

		switch (t_Event.type)
		{
		case Expose:

			break;
		case KeyPress:
			if (XLookupString(&t_Event.xkey, t_Text, 255, &t_Key, 0))
			{
				switch (t_Key)
				{
				case XK_Escape:
					return false;
					break;
				case XK_BackSpace:
					window->BackOnce();
					XClearWindow(window->m_Display, window->m_Window);
					XDrawString(window->m_Display,
						window->m_Window,
						window->m_GraphicContext,
						10,
						50,
						window->m_WindowTextBuffer,
						window->m_CurrentTextPos);
					break;
				default:
					window->AddChar(t_Key);
					XClearWindow(window->m_Display, window->m_Window);
					XDrawString(window->m_Display,
						window->m_Window,
						window->m_GraphicContext,
						10,
						50,
						window->m_WindowTextBuffer,
						window->m_CurrentTextPos);
					break;
				}
			}
			break;
		}
	}

	return true;
}