
#include <windows.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
//	PSTR szCmdLine, int iCmdShow)
int main()
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	static TCHAR szAppName[] = TEXT("NoBorderWnd");
	HWND         hwnd;
	MSG          msg;
	WNDCLASS     wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"),
			szAppName, MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(szAppName,                  // window class name
		TEXT("Win32无边框窗口"), // window caption
		WS_POPUP | WS_MINIMIZEBOX,        // window style
		100,              // initial x position
		100,              // initial y position
		500,              // initial x size
		500,              // initial y size
		NULL,                       // parent window handle
		NULL,                       // window menu handle
		NULL,                  // program instance handle
		NULL);                     // creation parameters

	ShowWindow(hwnd, 1);
	//UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

#define BorderWidth 5
#define CaptionHeight 20
bool bBorder;
void OnMouseMove(HWND hwnd, int x, int y, int action)
{
	RECT rc;
	GetClientRect(hwnd, &rc);
	int w = rc.right;
	int h = rc.bottom;
	if (action == MK_LBUTTON)
	{
		if (!bBorder)
			return;
		if (x<BorderWidth && y>h - BorderWidth)//左下
		{
			SetCursor(LoadCursor(NULL, IDC_SIZENESW));
			SendMessage(hwnd, WM_SYSCOMMAND, SC_SIZE | WMSZ_BOTTOMLEFT, 0);
		}
		else if (x > w - BorderWidth && y < BorderWidth)//右上
		{
			SetCursor(LoadCursor(NULL, IDC_SIZENESW));
			SendMessage(hwnd, WM_SYSCOMMAND, SC_SIZE | WMSZ_TOPRIGHT, 0);
		}
		else if (x < BorderWidth && y < BorderWidth)//左上
		{
			SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
			SendMessage(hwnd, WM_SYSCOMMAND, SC_SIZE | WMSZ_TOPLEFT, 0);
		}
		else if (x > w - BorderWidth && y > h - BorderWidth)//右下
		{
			SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
			SendMessage(hwnd, WM_SYSCOMMAND, SC_SIZE | WMSZ_BOTTOMRIGHT, 0);
		}
		else if (x<BorderWidth && y>BorderWidth && y < h - BorderWidth)//左
		{
			SetCursor(LoadCursor(NULL, IDC_SIZEWE));
			SendMessage(hwnd, WM_SYSCOMMAND, SC_SIZE | WMSZ_LEFT, 0);
		}
		else if (x > w - BorderWidth && y > BorderWidth && y < h - BorderWidth)//右
		{
			SetCursor(LoadCursor(NULL, IDC_SIZEWE));
			SendMessage(hwnd, WM_SYSCOMMAND, SC_SIZE | WMSZ_RIGHT, 0);
		}
		else if (y < BorderWidth && x > BorderWidth && x < w - BorderWidth)//上
		{
			SetCursor(LoadCursor(NULL, IDC_SIZENS));
			SendMessage(hwnd, WM_SYSCOMMAND, SC_SIZE | WMSZ_TOP, 0);
		}
		else if (y > h - BorderWidth && x > BorderWidth && x < w - BorderWidth)//下
		{
			SetCursor(LoadCursor(NULL, IDC_SIZENS));
			SendMessage(hwnd, WM_SYSCOMMAND, SC_SIZE | WMSZ_BOTTOM, 0);
		}
		else if (x > BorderWidth && x<w - BorderWidth && y>BorderWidth && y < BorderWidth + CaptionHeight)//标题栏移动
			SendMessage(hwnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
		else //if (x > BorderWidth && x<w - BorderWidth && y>BorderWidth && y < h - BorderWidth)
			SetCursor(LoadCursor(NULL, IDC_ARROW));
	}
	else
	{
		if ((x<BorderWidth && y>h - BorderWidth) || (x > w - BorderWidth && y < BorderWidth))
			SetCursor(LoadCursor(NULL, IDC_SIZENESW));
		else if ((x < BorderWidth && y < BorderWidth) || (x > w - BorderWidth && y > h - BorderWidth))
			SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
		else if (y > BorderWidth && y < h - BorderWidth && (x<BorderWidth || x>w - BorderWidth))
			SetCursor(LoadCursor(NULL, IDC_SIZEWE));
		else if (x > BorderWidth && x < w - BorderWidth && (y<BorderWidth || y>h - BorderWidth))
			SetCursor(LoadCursor(NULL, IDC_SIZENS));
		else if (x > BorderWidth && x<w - BorderWidth && y>BorderWidth && y < h - BorderWidth)
			SetCursor(LoadCursor(NULL, IDC_ARROW));
	}
}

void OnLButtonDown(HWND hwnd, int x, int y, int action)
{
	RECT rc;
	GetClientRect(hwnd, &rc);
	int w = rc.right;
	int h = rc.bottom;
	if (x > BorderWidth && x<w - BorderWidth && y>BorderWidth + CaptionHeight && y < h - BorderWidth)
	{
		bBorder = false;
		return;
	}
	bBorder = true;
	OnMouseMove(hwnd, x, y, action);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC         hdc;
	PAINTSTRUCT ps;

	switch (message)
	{
	case WM_CREATE:
		bBorder = false;
		return 0;

	case WM_MOUSEMOVE:
		OnMouseMove(hwnd, LOWORD(lParam), HIWORD(lParam), wParam);
		return 0;

	case WM_LBUTTONDOWN:
		OnLButtonDown(hwnd, LOWORD(lParam), HIWORD(lParam), wParam);
		return 0;

	case WM_LBUTTONUP:
		bBorder = false;
		return 0;

	case WM_PAINT:
		//hdc = BeginPaint(hwnd, &ps);
		//EndPaint(hwnd, &ps);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}