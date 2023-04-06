#pragma once
#ifndef POINT_VECTOR_H
#define POINT_VECTOR_H 

#include <gdiplus.h>
#include <iostream>
#include <vector> 

using namespace std;
using namespace Gdiplus;

enum PointType
{
	Line = 0,
	Bezier = 1,
};

// 포인트와 압력값을 저장하는 클래스
class PointPressure
{
public:
	PointPressure(PointType type, PointF pt, float pressure)
	{
		m_type = type; 
		m_point = pt;
		m_pressure = pressure;
	} 

	PointType GetMyType()
	{
		return m_type;
	}

	PointF GetPoint()
	{
		return m_point;
	}

	void SetPoint(PointF pt)
	{
		m_point = pt;
	} 

	float GetPressure()
	{
		return m_pressure;
	}

	float GetDistance(PointF target)
	{
		/*float dx = target.X - GetEndPoint().X;
		float dy = target.Y - GetEndPoint().Y;*/

		float dx = target.X - m_point.X;
		float dy = target.Y - m_point.Y;

		return sqrt(dx * dx + dy * dy);
	}

	float GetAngle(PointF target)
	{
		float dx = target.X - m_point.X;
		float dy = target.Y - m_point.Y;
		float angle = atan2(dy, dx);
		return angle < 0 ? angle + 2 * M_PI : angle;
	}
	 

private:
	PointF m_point; 
	float m_pressure;
	PointType m_type;
};

// 마우스 이동 경로를 포인트 벡터로 저장하는 클래스
class PointVector
{
public:
	PointVector()
	{
		m_points = new vector<PointPressure>(); 
	}

	~PointVector()
	{
		delete m_points; 
	}

	void AddPoint(PointPressure point)
	{
		m_points->push_back(point);
	}

	void Clear()
	{
		m_points->clear();
	}

	vector<PointPressure>* GetPoints()
	{
		return m_points;
	}

	int GetPointCount()
	{
		return m_points->size();
	}

	PointPressure GetPoint(int index)
	{
		return m_points->at(index);
	}

	PointPressure back()
	{
		return m_points->back();
	}

	PointPressure GetLastPoint()
	{
		return m_points->back();
	}

	void SetLastPoint(PointPressure point)
	{
		// vector의 마지막 원소를 바꾼다.
		m_points->back() = point; 
	} 

	void CreateRectanglePath(PointF ptRect[4], GraphicsPath* pPath)
	{
		pPath->Reset();
		pPath->AddLine(ptRect[0], ptRect[1]);
		pPath->AddLine(ptRect[1], ptRect[2]);
		pPath->AddLine(ptRect[2], ptRect[3]);
		pPath->AddLine(ptRect[3], ptRect[0]);
	}

	CustomLineCap* CreateRectCap(int capSizeWidth, int capSizeHeight)
	{
		//const int capSize = 10;
		PointF capPoints[4];
		capPoints[0] = PointF(-capSizeWidth / 2, capSizeHeight / 2);
		capPoints[1] = PointF(capSizeWidth / 2, capSizeHeight / 2);
		capPoints[2] = PointF(capSizeWidth / 2, -capSizeHeight / 2);
		capPoints[3] = PointF(-capSizeWidth / 2, -capSizeHeight / 2);
		GraphicsPath capPath;
		CreateRectanglePath(capPoints, &capPath);

		PointF origin(-capSizeWidth / 2, -capSizeHeight / 2); // 회전중심
		Matrix matrix;
		matrix.RotateAt(0, origin);
		capPath.Transform(&matrix); // Path를 회전

		CustomLineCap* pCap = new CustomLineCap(NULL, &capPath);
		/*pCap->SetStrokeJoin(LineJoinRound);
		pCap->SetStrokeCap(LineCapRound);*/

		pCap->SetStrokeJoin(LineJoinMiter);
		pCap->SetStrokeCap(LineCapNoAnchor);
		//pCap->SetBaseInset(0);
		return pCap;
	}  

	void DrawSmoothLines(Graphics& graphics, Pen& pen)
	{
		if (m_points->size() < 2) 
			return;

		GraphicsPath path;
		path.StartFigure();

		// 첫 번째 점은 그냥 그린다
		PointPressure point = m_points->at(0);
		path.AddLine(point.GetPoint(), point.GetPoint());

		graphics.SetPageUnit(UnitPixel);

		// 비트맵 생성
		Bitmap bitmap(10, 20, PixelFormat32bppARGB);

		// Graphics 객체 생성
		Graphics bitmapGraphics(&bitmap);

		// 검은색 브러시 생성
		SolidBrush blackBrush(Color::Black);

		// 비트맵 전체를 검은색으로 채움
		bitmapGraphics.FillRectangle(&blackBrush, 0, 0, 10, 20);

		TextureBrush textureBrush(&bitmap);

		graphics.SetSmoothingMode(SmoothingModeAntiAlias);

		// 두 번째 점부터 부드러운 라인 그리기
		for (int i = 1; i < m_points->size(); ++i)
		{
			PointPressure pp1 = m_points->at(i - 1);
			PointPressure pp2 = m_points->at(i);

			PointF p1 = pp1.GetPoint();
			PointF p2 = pp2.GetPoint(); 
			 
			// custom brush로 사각형 그리기 
			float start_x = p1.X;
			float start_y = p1.Y;
			float end_x = p2.X;
			float end_y = p2.Y;

			if (p1.X > p2.X)
			{
				start_x = p2.X;
				end_x = p1.X;
			}

			if (p1.Y > p2.Y)
			{
				start_y = p2.Y;
				end_y = p1.Y;
			}
				 
			graphics.FillRectangle(&textureBrush, start_x, start_y, 10 + (end_x - start_x), 20 + (end_y - start_y)); 
		}

		path.CloseFigure();

		graphics.DrawPath(&pen, &path);
	}   

	void DrawSmoothPathEx(HWND hwnd)
	{
		if (m_points->size() < 2)
			return;

		// 1. 메모리DC 생성
		HDC hdc = GetDC(hwnd);
		HDC memDC = CreateCompatibleDC(hdc);
		RECT rc;
		GetClientRect(hwnd, &rc);
		int width = rc.right - rc.left;
		int height = rc.bottom - rc.top;
		HBITMAP bitmap = CreateCompatibleBitmap(hdc, width, height);
		HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bitmap);

		// 2. 패스를 메모리DC에 그림
		Graphics graphics(memDC);  

		CustomLineCap* cap = CreateRectCap(1, 3);

		Pen myPen(Color(255, 255, 255, 0), 40);
		myPen.SetLineCap(LineCap::LineCapCustom, LineCap::LineCapCustom, DashCap::DashCapFlat);
		myPen.SetCustomStartCap(cap);
		myPen.SetCustomEndCap(cap);  

		myPen.SetAlignment(PenAlignment::PenAlignmentCenter);

		GraphicsPath path;

		Matrix matrix;
		myPen.SetTransform(&matrix); // 단위 매트릭스를 설정하여 회전을 막음

		// 두 번째 점부터 부드러운 라인 그리기
		for (int i = 1; i < m_points->size(); ++i)
		{
			PointPressure pp1 = m_points->at(i - 1);
			PointPressure pp2 = m_points->at(i);

			PointF p1 = pp1.GetPoint();
			PointF p2 = pp2.GetPoint();

			float start_x = p1.X;
			float start_y = p1.Y;
			float end_x = p2.X;
			float end_y = p2.Y; 

			path.AddLine(p1, p2);
		} 

		//Matrix matrix;
		//matrix.Rotate(45); // 45도 회전
		//graphics.SetTransform(&matrix);

		graphics.DrawPath(&myPen, &path);

		// 3. 메모리DC의 내용을 화면에 출력
		BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

		// 4. 메모리DC 및 관련 객체 해제
		SelectObject(memDC, oldBitmap);
		DeleteObject(bitmap);
		DeleteDC(memDC);
		ReleaseDC(hwnd, hdc); 
	}

	void DrawSmoothPath(HDC hdc, Pen& pen)
	{
		if (m_points->size() < 2)
			return; 

		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);

		// 메모리 DC와 비트맵 생성
		HDC memDc = CreateCompatibleDC(hdc);
		HBITMAP bitmap = CreateCompatibleBitmap(hdc, screenWidth, screenHeight);

		// 메모리 DC와 비트맵 연결
		HGDIOBJ oldBitmap = SelectObject(memDc, bitmap);


		Pen myPen(Color(255, 255, 255), 20);
		myPen.SetStartCap(LineCapSquare); // 시작 부분 모양 설정
		myPen.SetEndCap(LineCapSquare); // 끝 부분 모양 설정
		// CustomLineCap 객체 생성
		CustomLineCap* rectCap = CreateRectCap(30, 50);
		myPen.SetCustomEndCap(rectCap); 

		myPen.SetDashCap(DashCap::DashCapFlat);

		Color pencolor;
		myPen.GetColor(&pencolor);

		COLORREF cr = RGB(pencolor.GetR(), pencolor.GetG(), pencolor.GetB());
		HPEN hPen = CreatePen(myPen.GetDashStyle(), myPen.GetWidth(), cr); 
		

		//// LOGPEN 구조체 생성
		//LOGPEN logPen;
		//logPen.lopnColor = cr;
		//logPen.lopnStyle = myPen.GetDashStyle();
		//logPen.lopnWidth.x = static_cast<LONG>(myPen.GetWidth());
		//logPen.lopnWidth.y = 0;

		//// GDI Pen 핸들 생성
		//HPEN hPen = CreatePenIndirect(&logPen);

		 

		// 라인 그리기
		HGDIOBJ oldPen = SelectObject(memDc, hPen);

		// 두 번째 점부터 부드러운 라인 그리기
		for (int i = 1; i < m_points->size(); ++i)
		{
			PointPressure pp1 = m_points->at(i - 1);
			PointPressure pp2 = m_points->at(i);

			PointF p1 = pp1.GetPoint();
			PointF p2 = pp2.GetPoint(); 

			float start_x = p1.X;
			float start_y = p1.Y;
			float end_x = p2.X;
			float end_y = p2.Y;

			/*if (p1.X > p2.X)
			{
				start_x = p2.X;
				end_x = p1.X;
			}

			if (p1.Y > p2.Y)
			{
				start_y = p2.Y;
				end_y = p1.Y;
			}*/

			//DrawLineWithROP(hdc, start_x, start_y, end_x, end_y, 0x00330008);
			MoveToEx(memDc, start_x, start_y, NULL);
			LineTo(memDc, end_x, end_y);
		}   


		SelectObject(memDc, oldPen);

		// 화면에 비트맵 출력
		BitBlt(hdc, 0, 0, screenWidth, screenHeight, memDc, 0, 0, 0x00330008);

		// 해제
		SelectObject(memDc, oldBitmap);
		DeleteObject(hPen);
		DeleteObject(bitmap);
		DeleteDC(memDc);
	} 


	/*BitBlt 함수의 rop 값은 비트맵 출력을 다양한 방법으로 처리하기 위한 Raster Operation Code(Rop Code)를 지정합니다.
	rop 값은 DWORD 형식으로, 하위 16비트는 출력 대상, 상위 16비트는 출력 소스를 나타냅니다.
	다음은 rop 값의 종류와 각각의 의미에 대한 설명입니다.

	R2_BLACK(0x00000042) - 전경색을 무시하고 검정색으로 출력합니다.
	R2_NOTMERGEPEN(0x0002AFFF) - 전경색과 배경색을 XOR 연산하여 출력합니다.
	R2_MASKNOTPEN(0x00330008) - 전경색을 마스크로 사용하고, 배경색을 XOR 연산하여 출력합니다.
	R2_NOTCOPYPEN(0x00330008) - 전경색을 마스크로 사용하고, XOR 연산한 결과를 출력합니다.
	R2_MASKPENNOT(0x0005A5F0) - 배경색을 마스크로 사용하고, 전경색을 XOR 연산하여 출력합니다.
	R2_NOT(0x0002AFFF) - 전경색과 배경색을 모두 XOR 연산하여 출력합니다.
	R2_XORPEN(0x00112233) - 전경색을 XOR 연산하여 출력합니다.
	R2_NOTMASKPEN(0x00330008) - 배경색을 마스크로 사용하고, XOR 연산한 결과를 출력합니다.
	R2_MASKPEN(0x000F00FF) - 전경색을 마스크로 사용하고, 배경색을 AND 연산하여 출력합니다.
	R2_NOTXORPEN(0x00112233) - 전경색을 XOR 연산하고, 결과를 NOT 연산하여 출력합니다.
	R2_NOP(0x00) - 출력을 하지 않습니다.
	R2_MERGENOTPEN(0x00CA0749) - 전경색을 마스크로 사용하고, 배경색을 OR 연산하여 출력합니다.
	R2_COPYPEN(0x00FA0089) - 전경색을 그대로 출력합니다.
	R2_MERGEPENNOT(0x00F0A041) - 배경색을 마스크로 사용하고, 전경색을 OR 연산하여 출력합니다.
	R2_MERGEPEN(0x00BA0F92) - 전경색과 배경색을 OR 연산하여 출력합니다.
	R2_WHITE(0x00FF0062) - 전경색을 무시하고 흰색으로 출력합니다.
	
	예를 들어, rop 값이 0xCC0020인 경우, 상위 16비트인 0xCC는 R2_XORPEN을 나타내고, 하위 16비트인 0x0020은 배경색을 무시하고 전경색을 XOR 연산하여 
	출력한다는 의미입니다.따라서 출력 결과는 전경색을 XOR 연산하여 출력됩니다. */

	void DrawLineWithROP(HDC hdc, float x1, float y1, float x2, float y2, DWORD rop)
	{
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);

		// 메모리 DC와 비트맵 생성
		HDC memDc = CreateCompatibleDC(hdc);
		HBITMAP bitmap = CreateCompatibleBitmap(hdc, screenWidth, screenHeight);

		// 메모리 DC와 비트맵 연결
		HGDIOBJ oldBitmap = SelectObject(memDc, bitmap);

		// Pen 생성
		HPEN pen = CreatePen(PS_SOLID, 10, RGB(255, 0, 0)); 

		// 라인 그리기
		HGDIOBJ oldPen = SelectObject(memDc, pen);
		MoveToEx(memDc, 0, 0, NULL);
		LineTo(memDc, x2 - x1, y2 - y1);
		SelectObject(memDc, oldPen);

		// 화면에 비트맵 출력
		BitBlt(hdc, 0, 0, screenWidth, screenHeight, memDc, 0, 0, rop);

		// 해제
		SelectObject(memDc, oldBitmap);
		DeleteObject(pen);
		DeleteObject(bitmap);
		DeleteDC(memDc);
	}


	void DrawBezier(Gdiplus::Graphics* graphics, Gdiplus::Pen* pen, Gdiplus::PointF p0, Gdiplus::PointF p1)
	{
		Gdiplus::PointF startPoint = p0;
		Gdiplus::PointF endPoint = p1;
		Gdiplus::PointF startCtrlPoint = Gdiplus::PointF(p0.X + (p1.X - p0.X) / 3, p0.Y + (p1.Y - p0.Y) / 3);
		Gdiplus::PointF endCtrlPoint = Gdiplus::PointF(p0.X + 2 * (p1.X - p0.X) / 3, p0.Y + 2 * (p1.Y - p0.Y) / 3);

		Gdiplus::PointF points[4] = { startPoint, startCtrlPoint, endCtrlPoint, endPoint };
		graphics->DrawBezier(pen, points[0], points[1], points[2], points[3]);
	}

	Gdiplus::PointF GetMidPoint(Gdiplus::PointF curPoint, Gdiplus::PointF prevPoint) 
	{
		float x = (curPoint.X + prevPoint.X) / 2.0f;
		float y = (curPoint.Y + prevPoint.Y) / 2.0f;
		return Gdiplus::PointF(x, y);
	}

	void DrawBezier(Graphics& graphics, Pen& pen, PointF start, PointF end)
	{
		Gdiplus::PointF midPoint = GetMidPoint(end, start);
		Gdiplus::PointF controlPoint = GetMidPoint(midPoint, start);
		Gdiplus::PointF endPoint = GetMidPoint(end, midPoint);

		Gdiplus::PointF bezierPoints[4] = { start, controlPoint, endPoint, end };
		Gdiplus::GraphicsPath bezierPath;
		bezierPath.AddBezier(bezierPoints[0], bezierPoints[1], bezierPoints[2], bezierPoints[3]); 
		graphics.DrawPath(&pen, &bezierPath);
	}

	// 벡터 A와 벡터 B의 거리를 계산하는 함수
	double Distance(PointPressure& A, PointPressure& B) {
		double dx = A.GetPoint().X - B.GetPoint().X;
		double dy = A.GetPoint().Y - B.GetPoint().Y;
		return sqrt(dx * dx + dy * dy);
	}

	void MakeSimple()
	{
		if (m_points->size() < 3)
			return;

		/*vector<PointPressure> temp = DouglasPeucker(*m_points, 12);
		m_points->clear(); 
		m_points->assign(temp.begin(), temp.end());*/

		vector<PointPressure> temp = simplify(*m_points);
		m_points->clear();
		m_points->assign(temp.begin(), temp.end());
	}

	vector<PointPressure> simplify(vector<PointPressure>& points)
	{
		vector<PointPressure> result;
		result.push_back(points[0]); // 첫번째 포인트는 무조건 포함

		// 3개의 연속된 포인트를 선택하여 한 직선상에 있는지 판단
		for (int i = 1; i < points.size() - 1; i++) {
			if ((points[i - 1].GetPoint().Y - points[i].GetPoint().Y) * (points[i + 1].GetPoint().X - points[i].GetPoint().X) ==
				(points[i - 1].GetPoint().X - points[i].GetPoint().X) * (points[i + 1].GetPoint().Y - points[i].GetPoint().Y)) {
				// 한 직선상에 있다면 중간 포인트는 제거
				continue;
			}
			result.push_back(points[i]);
		}

		result.push_back(points.back()); // 마지막 포인트는 무조건 포함

		return result;
	}

	// 연속된 포인트들을 단순화하는 함수
	vector<PointPressure> DouglasPeucker(vector<PointPressure>& points, double epsilon) 
	{
		int n = points.size();
		vector<bool> mark(n, false); // 포인트가 마크되었는지 여부
		vector<PointPressure> result; // 결과 포인트 리스트

		// 시작점과 끝점을 결과 리스트에 추가
		result.push_back(points.front());
		result.push_back(points.back());

		// 시작점과 끝점을 제외한 포인트들을 검사
		DouglasPeuckerRecursive(points, epsilon, 0, n - 1, mark, result);

		return result;
	}

	// 재귀적으로 연속된 포인트들을 단순화하는 함수
	void DouglasPeuckerRecursive(vector<PointPressure>& points, double epsilon, int start, int end,
		vector<bool>& mark, vector<PointPressure>& result)
	{
		if (end <= start + 1) 
			return; // 더 이상 분할할 수 없는 경우

		double dmax = 0.0; // 최대 거리
		int index = start + 1; // 최대 거리를 가지는 포인트의 인덱스

		// 시작점과 끝점을 제외한 포인트들 중에서 최대 거리를 구함
		for (int i = start + 1; i < end; ++i) {
			double d = Distance(points[i], points[start]) + Distance(points[i], points[end]);
			if (Distance(result[result.size() - 2], points[i]) < epsilon && d < 2 * epsilon) 
				continue;

			if (d > dmax) 
			{
				index = i;
				dmax = d;
			}
		}

		// 최대 거리가 epsilon보다 큰 경우에만 분할
		if (dmax > epsilon) {
			mark[index] = true;
			DouglasPeuckerRecursive(points, epsilon, start, index, mark, result);
			DouglasPeuckerRecursive(points, epsilon, index, end, mark, result);
		}
		else {
			for (int i = start + 1; i < end; ++i) {
				if (!mark[i]) {
					result.push_back(points[i]);
				}
			}
		}
	}

private:
	vector<PointPressure>* m_points;
	Graphics* m_pGraphics;
}; 

#endif // !POINT_VECTOR_H