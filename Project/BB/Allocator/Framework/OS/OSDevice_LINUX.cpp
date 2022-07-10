#include "pch.h"
#include "OSDevice.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <cstdint>
#include <unistd.h>

#include "Storage/Dynamic_Array.h"
#include "Storage/Pool.h"

using namespace BB;

typedef FreeListAllocator_t OSAllocator_t;
typedef LinearAllocator_t OSTempAllocator_t;

OSAllocator_t OSAllocator{ mbSize * 8 };
OSTempAllocator_t OSTempAllocator{ mbSize * 4 };

static OSDevice osDevice;

//The OS window for Windows.
class OSWindow
{
public:
	void Init(OS_WINDOW_STYLE a_Style, int a_X, int a_Y, int a_Width, int a_Height, const char* a_WindowName)
	{
		windowName = a_WindowName;

		display = XOpenDisplay(NULL);
		BB_ASSERT(display != NULL, "Linux XOpenDisplay failed!");
		screen = DefaultScreen(display);

		uint64_t t_Foreground_color = WhitePixel(display, screen);
		uint64_t t_Background_color = BlackPixel(display, screen);

		//Initialize the window with a white foreground and a black background.
		window = XCreateSimpleWindow(display,
			DefaultRootWindow(display),
			a_X,
			a_Y,
			a_Width,
			a_Height,
			5,
			t_Foreground_color,
			t_Background_color);

		switch (a_Style)
		{
		case BB::OS_WINDOW_STYLE::MAIN:
			// TO DO
			break;
		case BB::OS_WINDOW_STYLE::CHILD:
			//TO DO
			break;
		default:
			BB_ASSERT(false, "Tried to create a window with a OS_WINDOW_STYLE it does not accept.");
			break;
		}

		//Set window properties
		XSetStandardProperties(display, window, a_WindowName, "EXAMPLE", None, NULL, 0, NULL);

		//The input rules on what is allowed in the input.
		XSelectInput(display, window, ExposureMask | ButtonPressMask | KeyPressMask);

		//Create the graphics context
		graphicContext = XCreateGC(display, window, 0, 0);

		//Set the foreground and background. Wtf is x11.
		XSetForeground(display, graphicContext, t_Foreground_color);
		XSetBackground(display, graphicContext, t_Background_color);

		/* clear the window and bring it on top of the other windows */
		XClearWindow(display, window);
		XMapRaised(display, window);

	}

	void Destroy()
	{
		XFreeGC(display, graphicContext);
		XDestroyWindow(display, window);
		XCloseDisplay(display);
	}

	_XDisplay* display; //XDisplay, also known as Window.
	const char* windowName;
	int screen; //the current user window.
	XID window; //X11 window ID.
	_XGC* graphicContext; //Graphic context of X11


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

FrameworkHandle OSDevice::CreateOSWindow(OS_WINDOW_STYLE a_Style, int a_X, int a_Y, int a_Width, int a_Height, const char* a_WindowName)
{
	OSWindow t_OSWindow;
	t_OSWindow.Init(a_Style, a_X, a_Y, a_Width, a_Height, a_WindowName);

	size_t t_OSWindowsSize = m_OSDevice->OSWindows.size();

	for (size_t i = 0; i < t_OSWindowsSize; i++)
	{
		if (m_OSDevice->OSWindows[i].display == nullptr)
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
	m_OSDevice->OSWindows[a_Handle.index].display = nullptr;
}

void OSDevice::ExitApp() const
{
	exit(EXIT_FAILURE);
}

bool BB::OSDevice::ProcessMessages() const
{
	KeySym t_Key;
	char t_Text[255];

	for (size_t i = 0; i < m_OSDevice->OSWindows.size(); i++)
	{
		if (m_OSDevice->OSWindows[i].display == nullptr)
		{
			continue;
		}

		while (XPending(m_OSDevice->OSWindows[i].display))
		{
			XEvent t_Event;
			XNextEvent(m_OSDevice->OSWindows[i].display, &t_Event);

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
						m_OSDevice->OSWindows[i].BackOnce();
						XClearWindow(m_OSDevice->OSWindows[i].display, 
							m_OSDevice->OSWindows[i].window);
						XDrawString(m_OSDevice->OSWindows[i].display,
							m_OSDevice->OSWindows[i].window,
							m_OSDevice->OSWindows[i].graphicContext,
							10,
							50,
							m_OSDevice->OSWindows[i].m_WindowTextBuffer,
							m_OSDevice->OSWindows[i].m_CurrentTextPos);
						break;
					default:
						m_OSDevice->OSWindows[i].AddChar(t_Key);
						XClearWindow(m_OSDevice->OSWindows[i].display, 
							m_OSDevice->OSWindows[i].window);
						XDrawString(m_OSDevice->OSWindows[i].display,
							m_OSDevice->OSWindows[i].window,
							m_OSDevice->OSWindows[i].graphicContext,
							10,
							50,
							m_OSDevice->OSWindows[i].m_WindowTextBuffer,
							m_OSDevice->OSWindows[i].m_CurrentTextPos);
						break;
					}
				}
				break;
			}
		}
	}

	return true;
}