#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>  
#include "PointVector.h" 

using namespace std; 

using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib") 


#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp)) 


static PointVector myPoints = PointVector();

void SavePoint(Point stPoint, Point endPoint, float pen_pressure, PointVector& myPoints)
{
	PointPressure pt = PointPressure(stPoint, endPoint, pen_pressure);
	myPoints.AddPoint(pt);
}

void DrawMyPath(HDC hdc)
{ 
	if (myPoints.GetPointCount() == 0)
		return;

	// GDI+ Graphics 객체 생성
	Graphics graphics(hdc);

	float default_pen_width = 40;

	for (int i = 0; i < myPoints.GetPointCount(); i++)
	{
		PointPressure pt = myPoints.GetPoint(i);

		GraphicsPath path;

		// 형광펜 효과를 위한 펜 생성 및 속성 설정
		Pen pen(Color(255, 0, 0, 0), default_pen_width);
		pen.SetStartCap(LineCapRound);
		pen.SetEndCap(LineCapRound); 
			 
		path.AddLine(pt.GetStartPoint(), pt.GetEndPoint());

		if (pt.GetPressure() != 1.0F)
		{
			pen.SetWidth(default_pen_width * pt.GetPressure());
		}

		// 경로를 그리기
		graphics.SetSmoothingMode(SmoothingModeAntiAlias);
		graphics.DrawPath(&pen, &path);
	}  
}

// 마우스 우클릭 발생시 화면을 지우는 함수
VOID OnErase(HWND hWnd, HDC hdc)
{
	RECT rect;
	GetClientRect(hWnd, &rect);
	FillRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
	::InvalidateRect(hWnd, NULL, true); 
}

VOID OnPaint(HDC hdc)
{
	Graphics graphics(hdc);
	Pen      pen(Color(255, 0, 0, 255));
	graphics.DrawLine(&pen, 0, 0, 200, 100);
} 

// wparm을 확인해 pen 입력인지 확인하는 함수
BOOL IsPenMessage(WPARAM wParam)
{
	UINT32 pointerId = GET_POINTERID_WPARAM(wParam);
	UINT32 penMask = (1 << (pointerId - 1));
	return (penMask & penMask) != 0;
} 

UINT32 GetPenPressure(LPARAM wParam)
{
	UINT32 id = GET_POINTERID_WPARAM(wParam);

	POINTER_PEN_INFO penInfo;
	if (GetPointerPenInfo(id, &penInfo))
	{ 
		return penInfo.pressure; 
	}

	return 0;
} 


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow)
{
	HWND                hWnd;
	MSG                 msg;
	WNDCLASS            wndClass;
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	// Initialize GDI+.
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = TEXT("GettingStarted");

	RegisterClass(&wndClass);

	hWnd = CreateWindow(
		TEXT("GettingStarted"),   // window class name
		TEXT("Getting Started"),  // window caption
		WS_OVERLAPPEDWINDOW,      // window style
		CW_USEDEFAULT,            // initial x position
		CW_USEDEFAULT,            // initial y position
		CW_USEDEFAULT,            // initial x size
		CW_USEDEFAULT,            // initial y size
		NULL,                     // parent window handle
		NULL,                     // window menu handle
		hInstance,                // program instance handle
		NULL);                    // creation parameters

	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	GdiplusShutdown(gdiplusToken);
	return msg.wParam;
}  // WinMain


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL isDrawing = false;
	static BOOL isOldPathDrawing = false;
	static Point lastPoint = Point(0, 0);
	static Point currentPoint = Point(0, 0);
	GraphicsPath path;

	static float pen_pressure = 1.0F; 

	switch (message)
	{
	case WM_PAINT:
	{
		// 그리기 준비
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps); 

		if (isOldPathDrawing)
		{
			DrawMyPath(hdc);
		}

		// GDI+ Graphics 객체 생성
		Graphics graphics(hdc);

		float default_pen_width = 40;

		// 형광펜 효과를 위한 펜 생성 및 속성 설정
		Pen pen(Color(255, 0, 0, 0), default_pen_width);
		pen.SetStartCap(LineCapRound);
		pen.SetEndCap(LineCapRound);

		// 마우스 이동 경로를 그리기
		if (isDrawing)
		{
			// 현재 마우스 위치를 경로에 추가
			path.AddLine(lastPoint, currentPoint); 
			 
			if (pen_pressure != 1.0F)
			{
				pen.SetWidth(default_pen_width * pen_pressure);
			}

			// 경로를 그리기
			graphics.SetSmoothingMode(SmoothingModeAntiAlias);
			graphics.DrawPath(&pen, &path);
		}

		// GraphicsPath의 Point를 저장하는 함수 호출
		SavePoint(lastPoint, currentPoint, pen_pressure, myPoints);

		// 그리기 종료
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_MOUSEMOVE:  
		// 마우스 이동 시 경로 업데이트
		if (isDrawing)
		{
			lastPoint = currentPoint;
			currentPoint = Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	case WM_LBUTTONDOWN:
		// 마우스 왼쪽 버튼 클릭 시 그리기 시작
		isDrawing = true;
		currentPoint = lastPoint = Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		path.Reset();
		path.StartFigure();
		break;
	case WM_LBUTTONUP:
		// 마우스 왼쪽 버튼 떼면 그리기 종료
		isDrawing = false;
		break; 
	case WM_RBUTTONDOWN:
	{
		// 현재 커서 위치 얻기
		POINT pt;
		GetCursorPos(&pt);

		// 컨텍스트 메뉴 생성
		HMENU hMenu = CreatePopupMenu();
		AppendMenu(hMenu, MF_STRING, 1, L"화면 지우기");
		AppendMenu(hMenu, MF_STRING, 2, L"패스 불러오기");
		AppendMenu(hMenu, MF_STRING, 3, L"저장된 패스데이터 삭제");

		// 컨텍스트 메뉴 보이기
		TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);

		// 컨텍스트 메뉴 핸들 해제
		DestroyMenu(hMenu); 
	}
	break;
	// 메뉴 항목 선택 이벤트 처리
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case 1:
			{
				isOldPathDrawing = false;

				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				OnErase(hWnd, hdc);
				// 그리기 종료
				EndPaint(hWnd, &ps);
			}
			break;

		case 2:
			// 메뉴 항목 2 선택 시 처리할 코드 
			isOldPathDrawing = true;
			InvalidateRect(hWnd, NULL, FALSE);
			break;

		case 3:
			{
				// 메뉴 항목 3 선택 시 처리할 코드
				myPoints.Clear();
				isOldPathDrawing = false;

				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				OnErase(hWnd, hdc);
				// 그리기 종료
				EndPaint(hWnd, &ps);
			}
			break;

		default:
			break;
		}

		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_POINTERUPDATE:
	{
		// IsPenMessage 함수를 호출해 pen 입력인지 확인하고, 펜 입력일 경우 펜 압력을 가져옴 
		if (IsPenMessage(wParam))
		{
			UINT32 pressure = GetPenPressure(wParam);

			// 펜 압력은 0에서 1024 사이의 범위로 정규화됩니다.
			// 디바이스에서 압력을 보고하지 않는 경우 기본값은 0입니다.
			if(pressure != 0)
			{ 
				pen_pressure = (float)pressure / 1024.0F;
			}
			else
			{
				pen_pressure = 1.0F;
			} 
		} 
	}  
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
} 