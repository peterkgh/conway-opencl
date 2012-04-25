#include "stdafx.h"

#ifdef _WIN32

HWND Window;
struct dispatcher_context *global_c;

//Enable slow drawing (disable conversion from byte buffer to GDI bitmap)
//#define DRAW_BY_PIXEL

int _tmain(int argc, _TCHAR* argv[])
{
	return start_simulation();
}

unsigned long long
get_time(void)
{
	return GetTickCount();
}

int
start_dispatcher(struct dispatcher_context *c)
{
	MSG msg;

    global_c = c;

	if (c->gui_enabled)
	{
		CreateThread(NULL,
			0,
			dispatcher,
			c,
			0,
			NULL);

		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	else
	{
		dispatcher(c);
	}

	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_ERASEBKGND:
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

bool initialize_window(void)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GetModuleHandle(NULL);
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"CComputeCLWindowClass";
    wcex.hIconSm = NULL;
    if( !RegisterClassEx( &wcex ) )
        return false;

    // Create window
    RECT rc = { 0, 0, 800, 600 };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    Window = CreateWindow( L"CComputeCLWindowClass", L"Life Simulation (OpenCL Accelerated)", WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wcex.hInstance,
                           NULL );
    if( !Window )
        return false;

    ShowWindow( Window, SW_SHOWMAXIMIZED );

    return true;
}

#ifdef DRAW_BY_PIXEL
void
update_gui(unsigned int *buff, size_t width, size_t height)
{
	size_t x, y;
	HDC hDC = GetDC(Window);
	RECT WindRect;

	if (!hDC) return;

	GetClientRect(Window, &WindRect);

    global_c->win_height = WindRect.bottom - WindRect.top;
	if (global_c->rows < (unsigned int)global_c->win_height)
		global_c->win_height = global_c->rows;

	for (y = 0; y < height && y < (unsigned int)(WindRect.bottom - WindRect.top); y++)
    {
        for (x = 0; x < width && x < (unsigned int)(WindRect.right - WindRect.left); x++)
        {
           if (buff[(x + y * width) >> 5] & (1 << (31 - ((x + y * width) & 31))))
               SetPixelV(hDC, x, y, RGB(0,0,0));
		   else
			   SetPixelV(hDC, x, y, RGB(255,255,255));
        }
    }

	ReleaseDC(Window, hDC);
}
#else
void
update_gui(unsigned int *buff, size_t width, size_t height)
{
	HDC hWindowDC = GetDC(Window);
	HDC hBufferDC;
	HBITMAP hBitmap;
	RECT WindRect;

	if (!hWindowDC) return;

	for (unsigned int i = 0; i < (height * (width / 8)) / 4; i++)
	{
		buff[i] = ((buff[i] & 0xFF000000) >> 24) |
				  ((buff[i] & 0x00FF0000) >> 8) |
				  ((buff[i] & 0x0000FF00) << 8) |
				  ((buff[i] & 0x000000FF) << 24);
	}

	hBitmap = CreateBitmap(width, height, 1, 1, buff);
	if (hBitmap)
	{
		hBufferDC = CreateCompatibleDC(NULL);
		if (hBufferDC)
		{
			SelectObject(hBufferDC, hBitmap);

			GetClientRect(Window, &WindRect);

            global_c->win_height = WindRect.bottom - WindRect.top;
			if (global_c->rows < (unsigned int)global_c->win_height)
				global_c->win_height = global_c->rows;

			InvalidateRgn(Window, NULL, TRUE);

			BitBlt(hWindowDC,
				0,
				0,
				WindRect.right - WindRect.left,
				WindRect.bottom - WindRect.top,
				hBufferDC,
				0,
				0,
				NOTSRCCOPY);

			ValidateRgn(Window, NULL);

			DeleteDC(hBufferDC);
		}

		DeleteObject(hBitmap);
	}

	ReleaseDC(Window, hWindowDC);
}
#endif

#endif