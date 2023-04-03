#include <windows.h>
#include <debugapi.h>
#include <objidl.h>
#include <gdiplus.h>  
#include "PointVector.h" 

using namespace std; 

using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib") 

#define Test2

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp)) 

#define M_PI 3.14159265358979323846   // pi

#define MIN_DISTANCE 10


static PointVector myPoints = PointVector();

void SavePoint(Point stPoint, Point endPoint, float pen_pressure, PointVector& myPoints)
{
	PointPressure pt = PointPressure(stPoint, endPoint, pen_pressure);
	myPoints.AddPoint(pt);
}

float GetDistance(PointF point1, PointF point2)
{
	float dx = point2.X - point1.X;
	float dy = point2.Y - point1.Y;
	return sqrt(dx * dx + dy * dy);
}

float GetAngle(PointF point1, PointF point2)
{
	float dx = point2.X - point1.X;
	float dy = point2.Y - point1.Y;
	float angle = atan2(dy, dx);
	return angle < 0 ? angle + 2 * M_PI : angle;
}

float GetThickness(float distance, float angle, float defaultThickness)
{
	// Calculate the thickness based on the distance and angle
	float thickness = (float)(30 / defaultThickness + fabs(cos(angle) * 5.0)); 

	return thickness;
} 

void DrawPathWithVariableThickness(Graphics* graphics, vector<PointF> points, int count, Pen* pen, float default_pen_width)
{
	if (count <= 1) 
		return;

	PointF previousPoint = points[0];
	PointF currentPoint;
	float previousThickness = default_pen_width;
	float currentThickness;
	float angle;
	float distance;

	for (int i = 1; i < count; i++)
	{
		currentPoint = points[i];
		distance = GetDistance(previousPoint, currentPoint);
		angle = GetAngle(previousPoint, currentPoint);

		// Calculate the thickness based on the angle and distance
		currentThickness = GetThickness(distance, angle, default_pen_width);

		// Create a new pen with the current thickness
		Pen currentPen(Color(255, 0, 0, 0), currentThickness); 

		// Draw a line between the previous and current points with the current pen
		graphics->DrawLine(&currentPen, previousPoint, currentPoint);

		// Update the previous values for the next iteration
		previousPoint = currentPoint;
		previousThickness = currentThickness;
	}
}



std::vector<PointF> SplitPointsByDistance(PointF startPoint, PointF endPoint, float distance)
{
	std::vector<PointF> points;

	float dx = endPoint.X - startPoint.X;
	float dy = endPoint.Y - startPoint.Y;
	float length = std::sqrt(dx * dx + dy * dy);

	if (length <= distance) // 거리가 일정 거리 이하이면 두 점을 반환
	{
		points.push_back(startPoint);
		points.push_back(endPoint);
		return points;
	}

	int numSplits = std::ceil(length / distance); // 일정 거리로 나누어야 하는 횟수

	float xStep = dx / numSplits; // x 좌표의 증가량
	float yStep = dy / numSplits; // y 좌표의 증가량

	float x = startPoint.X;
	float y = startPoint.Y;

	points.push_back(startPoint);
	for (int i = 0; i < numSplits - 1; i++)
	{
		x += xStep;
		y += yStep;
		points.push_back(PointF(x, y));
	}
	points.push_back(endPoint);

	return points;
}

void DrawLineEx(Graphics* graphics, Pen* pen, PointF prevPt, PointF currentPt, float default_pen_width)
{ 
	float maxPenWidth = 50;

	std::vector<PointF> points = SplitPointsByDistance(prevPt, currentPt, default_pen_width);

	wchar_t debug_buffer[256];
	swprintf_s(debug_buffer, L"Value: %d \n", points.size());
	OutputDebugStringW(debug_buffer);

	PointF prevPoint = points[0];

	for (int i = 1; i < points.size(); i++)
	{
		PointF curPoint = points[i];
		float dx = curPoint.X - prevPoint.X;
		float dy = curPoint.Y - prevPoint.Y;
		float distance = sqrt(dx * dx + dy * dy);
		float angle = atan2(dy, dx);
		float thickness = default_pen_width * (1.0f - distance / default_pen_width);

		if (thickness < 1.0f)
		{
			thickness = 1.0f;
		}

		pen->SetWidth(thickness);

		wchar_t debug_buffer[256];  
		swprintf_s(debug_buffer, L"Value: %f \n", default_pen_width);
		OutputDebugStringW(debug_buffer);

		pen->SetStartCap(LineCapRound);
		pen->SetEndCap(LineCapRound);
		pen->SetLineJoin(LineJoinRound);

		graphics->DrawLine(pen, prevPoint, curPoint);

		prevPoint = curPoint;
	}
	 
} 

//void BuildPath(GraphicsPath path, PointF curPoint, PointF previousPoint)
//{
//	if (previousPoint.IsEmpty()) {
//		previousPoint = curPoint;
//		return;
//	}
//
//	float distance = GetDistance(previousPoint, curPoint);
//
//	if (distance < MIN_DISTANCE) {
//		return;
//	}
//
//	float angle = GetAngle(previousPoint, curPoint);
//	float thickness = GetThickness(angle, distance);
//
//	if (path.IsEmpty()) {
//		path.StartFigure();
//		path.AddLine(previousPoint, curPoint);
//		previousPoint = curPoint;
//		previousThickness = thickness;
//		return;
//	}
//
//	if (abs(previousThickness - thickness) < minThicknessDifference) {
//		path.AddLine(previousPoint, curPoint);
//	}
//	else {
//		AddPointToPath(previousPoint, curPoint, previousThickness);
//		previousThickness = thickness;
//	}
//
//	previousPoint = curPoint;
//}

void DrawLine(Graphics* graphics, Pen* pen, PointF prevPt, PointF currentPt, float default_pen_width)
{ 
	//float penWidth = 5;
	float maxPenWidth = 10;
	//if (!startPoint.IsEmpty()) 
	{
		// 이전 점과의 거리와 각도 계산
		float dx = currentPt.X - prevPt.X;
		float dy = currentPt.Y - prevPt.Y;
		float dist = sqrt(dx * dx + dy * dy);
		float angle = atan2(dy, dx) * 180 / M_PI;

		// 형광펜 모양을 위한 끝점 모양 설정
		pen->SetStartCap(LineCap::LineCapRound);
		pen->SetEndCap(LineCap::LineCapRound); 

		// 거리와 각도에 따라 Pen 객체의 두께와 색상을 조절
		if (dist > 1) {
			if (angle > 0 && angle <= 90) {
				default_pen_width = maxPenWidth + dist / default_pen_width;
			}
			else if (angle > 90 && angle <= 180) {
				default_pen_width = maxPenWidth + dist / default_pen_width;
			}
			else if (angle > -180 && angle <= -90) {
				default_pen_width = maxPenWidth + dist / default_pen_width;
			}
			else if (angle > -90 && angle <= 0) {
				default_pen_width = maxPenWidth + dist / default_pen_width;
			}
		}

		pen->SetWidth(default_pen_width);

		// penWidth 디버그 출력  
		/*wchar_t debug_buffer[256];  
		swprintf_s(debug_buffer, L"Value: %f \n", default_pen_width);
		OutputDebugStringW(debug_buffer);*/

		// 선 그리기
		graphics->DrawLine(pen, prevPt, currentPt); 
	} 
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

	static vector<PointF> Points = vector<PointF>();

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

		float default_pen_width = 20;

		// 형광펜 효과를 위한 펜 생성 및 속성 설정
		Pen pen(Color(255, 0, 0, 0), default_pen_width);
		pen.SetStartCap(LineCapRound);
		pen.SetEndCap(LineCapRound);

		// 마우스 이동 경로를 그리기
		if (isDrawing)
		{
			// 현재 마우스 위치를 경로에 추가
			//path.AddLine(lastPoint, currentPoint); 
			 
			if (pen_pressure != 1.0F)
			{
				//pen.SetWidth(default_pen_width * pen_pressure);
				default_pen_width = default_pen_width * pen_pressure;
			}

			// 경로를 그리기
			graphics.SetSmoothingMode(SmoothingModeAntiAlias);

#ifdef Test0 
			DrawLine(&graphics, &pen, PointF(lastPoint.X, lastPoint.Y), PointF(currentPoint.X, currentPoint.Y), default_pen_width);
#endif 

#ifdef Test1
			DrawLineEx(&graphics, &pen, PointF(lastPoint.X, lastPoint.Y), PointF(currentPoint.X, currentPoint.Y), default_pen_width); 
#endif  

#ifdef Test2
			{
				std::vector<PointF> points = SplitPointsByDistance(PointF(lastPoint.X, lastPoint.Y), PointF(currentPoint.X, currentPoint.Y), default_pen_width);
				//if (points.size() > 2)
				{
					DrawPathWithVariableThickness(&graphics, points, points.size(), &pen, default_pen_width);
				}
				/*else
				{
					pen.SetWidth(default_pen_width);
					path.AddLine(lastPoint, currentPoint);
					graphics.DrawPath(&pen, &path);
				}*/
			} 
#endif
#ifdef Test3
			/*{
				if (isDrawing) {
					PointF prevPoint = Points.back();
					float distance = GetDistance(PointF(lastPoint.X, lastPoint.Y), PointF(currentPoint.X, currentPoint.Y));

					if (distance >= MIN_DISTANCE) { 
						float angle = GetAngle(PointF(lastPoint.X, lastPoint.Y), PointF(currentPoint.X, currentPoint.Y));
						float thickness = GetThickness(distance, angle, 10);

						Points.push_back(PointF(currentPoint.X, currentPoint.Y));
						pen.SetWidth(thickness);

						Gdiplus::Graphics graphics(hWnd);
						graphics.DrawPath(&pen, BuildPath(PointF(currentPoint.X, currentPoint.Y)));
					}
					else {
						Points.back() = PointF(currentPoint.X, currentPoint.Y);

						Gdiplus::Graphics graphics(hWnd);
						graphics.DrawLines(pen, &Points[0], Points.size());
					}
				}
			}*/
#endif
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