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

// ����Ʈ�� �з°��� �����ϴ� Ŭ����
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

// ���콺 �̵� ��θ� ����Ʈ ���ͷ� �����ϴ� Ŭ����
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
		// vector�� ������ ���Ҹ� �ٲ۴�.
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

		PointF origin(-capSizeWidth / 2, -capSizeHeight / 2); // ȸ���߽�
		Matrix matrix;
		matrix.RotateAt(0, origin);
		capPath.Transform(&matrix); // Path�� ȸ��

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

		// ù ��° ���� �׳� �׸���
		PointPressure point = m_points->at(0);
		path.AddLine(point.GetPoint(), point.GetPoint());

		graphics.SetPageUnit(UnitPixel);

		// ��Ʈ�� ����
		Bitmap bitmap(10, 20, PixelFormat32bppARGB);

		// Graphics ��ü ����
		Graphics bitmapGraphics(&bitmap);

		// ������ �귯�� ����
		SolidBrush blackBrush(Color::Black);

		// ��Ʈ�� ��ü�� ���������� ä��
		bitmapGraphics.FillRectangle(&blackBrush, 0, 0, 10, 20);

		TextureBrush textureBrush(&bitmap);

		graphics.SetSmoothingMode(SmoothingModeAntiAlias);

		// �� ��° ������ �ε巯�� ���� �׸���
		for (int i = 1; i < m_points->size(); ++i)
		{
			PointPressure pp1 = m_points->at(i - 1);
			PointPressure pp2 = m_points->at(i);

			PointF p1 = pp1.GetPoint();
			PointF p2 = pp2.GetPoint(); 
			 
			// custom brush�� �簢�� �׸��� 
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

		// 1. �޸�DC ����
		HDC hdc = GetDC(hwnd);
		HDC memDC = CreateCompatibleDC(hdc);
		RECT rc;
		GetClientRect(hwnd, &rc);
		int width = rc.right - rc.left;
		int height = rc.bottom - rc.top;
		HBITMAP bitmap = CreateCompatibleBitmap(hdc, width, height);
		HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bitmap);

		// 2. �н��� �޸�DC�� �׸�
		Graphics graphics(memDC);  

		CustomLineCap* cap = CreateRectCap(1, 3);

		Pen myPen(Color(255, 255, 255, 0), 40);
		myPen.SetLineCap(LineCap::LineCapCustom, LineCap::LineCapCustom, DashCap::DashCapFlat);
		myPen.SetCustomStartCap(cap);
		myPen.SetCustomEndCap(cap);  

		myPen.SetAlignment(PenAlignment::PenAlignmentCenter);

		GraphicsPath path;

		Matrix matrix;
		myPen.SetTransform(&matrix); // ���� ��Ʈ������ �����Ͽ� ȸ���� ����

		// �� ��° ������ �ε巯�� ���� �׸���
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
		//matrix.Rotate(45); // 45�� ȸ��
		//graphics.SetTransform(&matrix);

		graphics.DrawPath(&myPen, &path);

		// 3. �޸�DC�� ������ ȭ�鿡 ���
		BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

		// 4. �޸�DC �� ���� ��ü ����
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

		// �޸� DC�� ��Ʈ�� ����
		HDC memDc = CreateCompatibleDC(hdc);
		HBITMAP bitmap = CreateCompatibleBitmap(hdc, screenWidth, screenHeight);

		// �޸� DC�� ��Ʈ�� ����
		HGDIOBJ oldBitmap = SelectObject(memDc, bitmap);


		Pen myPen(Color(255, 255, 255), 20);
		myPen.SetStartCap(LineCapSquare); // ���� �κ� ��� ����
		myPen.SetEndCap(LineCapSquare); // �� �κ� ��� ����
		// CustomLineCap ��ü ����
		CustomLineCap* rectCap = CreateRectCap(30, 50);
		myPen.SetCustomEndCap(rectCap); 

		myPen.SetDashCap(DashCap::DashCapFlat);

		Color pencolor;
		myPen.GetColor(&pencolor);

		COLORREF cr = RGB(pencolor.GetR(), pencolor.GetG(), pencolor.GetB());
		HPEN hPen = CreatePen(myPen.GetDashStyle(), myPen.GetWidth(), cr); 
		

		//// LOGPEN ����ü ����
		//LOGPEN logPen;
		//logPen.lopnColor = cr;
		//logPen.lopnStyle = myPen.GetDashStyle();
		//logPen.lopnWidth.x = static_cast<LONG>(myPen.GetWidth());
		//logPen.lopnWidth.y = 0;

		//// GDI Pen �ڵ� ����
		//HPEN hPen = CreatePenIndirect(&logPen);

		 

		// ���� �׸���
		HGDIOBJ oldPen = SelectObject(memDc, hPen);

		// �� ��° ������ �ε巯�� ���� �׸���
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

		// ȭ�鿡 ��Ʈ�� ���
		BitBlt(hdc, 0, 0, screenWidth, screenHeight, memDc, 0, 0, 0x00330008);

		// ����
		SelectObject(memDc, oldBitmap);
		DeleteObject(hPen);
		DeleteObject(bitmap);
		DeleteDC(memDc);
	} 


	/*BitBlt �Լ��� rop ���� ��Ʈ�� ����� �پ��� ������� ó���ϱ� ���� Raster Operation Code(Rop Code)�� �����մϴ�.
	rop ���� DWORD ��������, ���� 16��Ʈ�� ��� ���, ���� 16��Ʈ�� ��� �ҽ��� ��Ÿ���ϴ�.
	������ rop ���� ������ ������ �ǹ̿� ���� �����Դϴ�.

	R2_BLACK(0x00000042) - ������� �����ϰ� ���������� ����մϴ�.
	R2_NOTMERGEPEN(0x0002AFFF) - ������� ������ XOR �����Ͽ� ����մϴ�.
	R2_MASKNOTPEN(0x00330008) - ������� ����ũ�� ����ϰ�, ������ XOR �����Ͽ� ����մϴ�.
	R2_NOTCOPYPEN(0x00330008) - ������� ����ũ�� ����ϰ�, XOR ������ ����� ����մϴ�.
	R2_MASKPENNOT(0x0005A5F0) - ������ ����ũ�� ����ϰ�, ������� XOR �����Ͽ� ����մϴ�.
	R2_NOT(0x0002AFFF) - ������� ������ ��� XOR �����Ͽ� ����մϴ�.
	R2_XORPEN(0x00112233) - ������� XOR �����Ͽ� ����մϴ�.
	R2_NOTMASKPEN(0x00330008) - ������ ����ũ�� ����ϰ�, XOR ������ ����� ����մϴ�.
	R2_MASKPEN(0x000F00FF) - ������� ����ũ�� ����ϰ�, ������ AND �����Ͽ� ����մϴ�.
	R2_NOTXORPEN(0x00112233) - ������� XOR �����ϰ�, ����� NOT �����Ͽ� ����մϴ�.
	R2_NOP(0x00) - ����� ���� �ʽ��ϴ�.
	R2_MERGENOTPEN(0x00CA0749) - ������� ����ũ�� ����ϰ�, ������ OR �����Ͽ� ����մϴ�.
	R2_COPYPEN(0x00FA0089) - ������� �״�� ����մϴ�.
	R2_MERGEPENNOT(0x00F0A041) - ������ ����ũ�� ����ϰ�, ������� OR �����Ͽ� ����մϴ�.
	R2_MERGEPEN(0x00BA0F92) - ������� ������ OR �����Ͽ� ����մϴ�.
	R2_WHITE(0x00FF0062) - ������� �����ϰ� ������� ����մϴ�.
	
	���� ���, rop ���� 0xCC0020�� ���, ���� 16��Ʈ�� 0xCC�� R2_XORPEN�� ��Ÿ����, ���� 16��Ʈ�� 0x0020�� ������ �����ϰ� ������� XOR �����Ͽ� 
	����Ѵٴ� �ǹ��Դϴ�.���� ��� ����� ������� XOR �����Ͽ� ��µ˴ϴ�. */

	void DrawLineWithROP(HDC hdc, float x1, float y1, float x2, float y2, DWORD rop)
	{
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);

		// �޸� DC�� ��Ʈ�� ����
		HDC memDc = CreateCompatibleDC(hdc);
		HBITMAP bitmap = CreateCompatibleBitmap(hdc, screenWidth, screenHeight);

		// �޸� DC�� ��Ʈ�� ����
		HGDIOBJ oldBitmap = SelectObject(memDc, bitmap);

		// Pen ����
		HPEN pen = CreatePen(PS_SOLID, 10, RGB(255, 0, 0)); 

		// ���� �׸���
		HGDIOBJ oldPen = SelectObject(memDc, pen);
		MoveToEx(memDc, 0, 0, NULL);
		LineTo(memDc, x2 - x1, y2 - y1);
		SelectObject(memDc, oldPen);

		// ȭ�鿡 ��Ʈ�� ���
		BitBlt(hdc, 0, 0, screenWidth, screenHeight, memDc, 0, 0, rop);

		// ����
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

	// ���� A�� ���� B�� �Ÿ��� ����ϴ� �Լ�
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
		result.push_back(points[0]); // ù��° ����Ʈ�� ������ ����

		// 3���� ���ӵ� ����Ʈ�� �����Ͽ� �� ������ �ִ��� �Ǵ�
		for (int i = 1; i < points.size() - 1; i++) {
			if ((points[i - 1].GetPoint().Y - points[i].GetPoint().Y) * (points[i + 1].GetPoint().X - points[i].GetPoint().X) ==
				(points[i - 1].GetPoint().X - points[i].GetPoint().X) * (points[i + 1].GetPoint().Y - points[i].GetPoint().Y)) {
				// �� ������ �ִٸ� �߰� ����Ʈ�� ����
				continue;
			}
			result.push_back(points[i]);
		}

		result.push_back(points.back()); // ������ ����Ʈ�� ������ ����

		return result;
	}

	// ���ӵ� ����Ʈ���� �ܼ�ȭ�ϴ� �Լ�
	vector<PointPressure> DouglasPeucker(vector<PointPressure>& points, double epsilon) 
	{
		int n = points.size();
		vector<bool> mark(n, false); // ����Ʈ�� ��ũ�Ǿ����� ����
		vector<PointPressure> result; // ��� ����Ʈ ����Ʈ

		// �������� ������ ��� ����Ʈ�� �߰�
		result.push_back(points.front());
		result.push_back(points.back());

		// �������� ������ ������ ����Ʈ���� �˻�
		DouglasPeuckerRecursive(points, epsilon, 0, n - 1, mark, result);

		return result;
	}

	// ��������� ���ӵ� ����Ʈ���� �ܼ�ȭ�ϴ� �Լ�
	void DouglasPeuckerRecursive(vector<PointPressure>& points, double epsilon, int start, int end,
		vector<bool>& mark, vector<PointPressure>& result)
	{
		if (end <= start + 1) 
			return; // �� �̻� ������ �� ���� ���

		double dmax = 0.0; // �ִ� �Ÿ�
		int index = start + 1; // �ִ� �Ÿ��� ������ ����Ʈ�� �ε���

		// �������� ������ ������ ����Ʈ�� �߿��� �ִ� �Ÿ��� ����
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

		// �ִ� �Ÿ��� epsilon���� ū ��쿡�� ����
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