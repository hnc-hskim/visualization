#include "Common.h"
#include "Util.h"

using namespace std; 

using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")  

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

void CreateRectangularLineCap(GraphicsPath& path, float width, float height)
{
	// Create a rectangular path
	path.Reset();
	path.AddRectangle(RectF(-width / 2, -height / 2, width, height));

	// Move the path to the center
	Matrix matrix;
	matrix.Translate(width / 2, height / 2);
	path.Transform(&matrix);
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
	static PointF lastPoint = PointF(0, 0);
	static PointF currentPoint = PointF(0, 0);
	GraphicsPath path;

	static float pen_pressure = 1.0F; 
	static float default_pen_width = 20;
	static float MinLength = 0;

	static vector<PointVector*> PathList = vector<PointVector*>();
	static PointVector* currentPoints = NULL;

	switch (message)
	{
	case WM_PAINT:
	{
		// 그리기 준비
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);  

		// GDI+ Graphics 객체 생성
		Graphics graphics(hdc); 
		graphics.SetPageUnit(UnitPixel);

		// 형광펜 효과를 위한 펜 생성 및 속성 설정
		Pen pen(Color(255, 0, 0, 0), default_pen_width);
		
		if (isOldPathDrawing)
		{
			for (vector<PointVector*>::iterator it = PathList.begin(); it != PathList.end(); it++)
			{
				//(*it)->DrawSmoothLines(graphics, pen);
				//(*it)->DrawSmoothPath(graphics, pen);
			}
			isOldPathDrawing = false;
		}

		// 마우스 이동 경로를 그리기
		if (isDrawing)
		{ 
			if (pen_pressure != 1.0F)
			{ 
				default_pen_width = default_pen_width * pen_pressure;
			} 

			//currentPoints->DrawSmoothLines(graphics, pen);  
//			currentPoints->DrawSmoothPath(hdc, pen); 
			currentPoints->DrawSmoothPathEx(hWnd);
		} 

		// 그리기 종료
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_MOUSEMOVE:  
		// 마우스 이동 시 경로 업데이트
		if (isDrawing)
		{ 
			currentPoint = PointF(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); 

			if (currentPoint.X == currentPoints->GetLastPoint().GetPoint().X && currentPoint.Y == currentPoints->GetLastPoint().GetPoint().Y)
			{
				break;
			}

			//currentPoints의 마지막 저장된 Point와 currentPoint의 거리가 MinLength보다 작다면, currentPoints의 
			// 마지막 원소의 값을 currentPoint로 변경한다. 
			/*if (currentPoints->GetPointCount() > 0 && currentPoints->GetLastPoint().GetDistance(currentPoint) < default_pen_width)
			{ 
				PointPressure last = currentPoints->GetLastPoint();
				last.SetPoint(currentPoint);

				currentPoints->SetLastPoint(last);
				InvalidateRect(hWnd, NULL, FALSE);
				break; 
			}*/

			float thickness = default_pen_width* pen_pressure;

			if (currentPoints->GetPointCount() > 0)
			{
				float dist = currentPoints->GetLastPoint().GetDistance(currentPoint);
				float angle = currentPoints->GetLastPoint().GetAngle(currentPoint);
				float currentWidth = default_pen_width * pen_pressure;
				thickness = (float)(currentWidth / currentWidth + fabs(cos(angle) * default_pen_width));
			}

			// 두 점의 각도가 1도 이상이면 Bezier 타입으로 추가한다. 
			/*if (currentPoints->GetPointCount() > 0 && currentPoints->GetLastPoint().GetAngle(currentPoint) > 1)
			{
				float dist = currentPoints->GetLastPoint().GetDistance(currentPoint);
				float angle = currentPoints->GetLastPoint().GetAngle(currentPoint);
				float currentWidth = default_pen_width* pen_pressure;

				float thickness = (float)(currentWidth / currentWidth + fabs(cos(angle) * 2));

				currentPoints->AddPoint(PointPressure(PointType::Bezier, lastPoint, currentPoint, thickness));
			}
			else*/
			{
				currentPoints->AddPoint(PointPressure(PointType::Line, currentPoint, thickness));
			}

			InvalidateRect(hWnd, NULL, FALSE);

			lastPoint = currentPoint;
		}
		break;
	case WM_LBUTTONDOWN:
		// 마우스 왼쪽 버튼 클릭 시 그리기 시작
		isDrawing = true;
		currentPoint = lastPoint = PointF(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); 
		currentPoints = new PointVector(); 
		PathList.push_back(currentPoints);
		currentPoints->AddPoint(PointPressure(PointType::Line, currentPoint, default_pen_width * pen_pressure));
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

				// PathList의 원소들을 모두 삭제한다.  
				PathList.clear();

				delete currentPoints;
				currentPoints = new PointVector(); 

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