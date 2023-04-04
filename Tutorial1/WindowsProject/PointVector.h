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
	PointPressure(PointType type, PointF start, PointF end, float pressure)
	{
		m_type = type;
		m_start = start;
		m_End = end;
		m_pressure = pressure;
	} 

	PointType GetMyType()
	{
		return m_type;
	}

	PointF GetStartPoint()
	{
		return m_start;
	}

	void SetStartPoint(PointF pt)
	{
		m_start = pt;
	}

	PointF GetEndPoint()
	{
		return m_End;
	}

	void SetEndPoint(PointF pt)
	{
		m_End = pt;
	}

	float GetPressure()
	{
		return m_pressure;
	}

	float GetDistance(PointF target)
	{
		/*float dx = target.X - GetEndPoint().X;
		float dy = target.Y - GetEndPoint().Y;*/

		float dx = target.X - GetStartPoint().X;
		float dy = target.Y - GetStartPoint().Y;

		return sqrt(dx * dx + dy * dy);
	}

	float GetAngle(PointF target)
	{
		float dx = target.X - GetEndPoint().X;
		float dy = target.Y - GetEndPoint().Y;
		float angle = atan2(dy, dx);
		return angle < 0 ? angle + 2 * M_PI : angle;
	}
	 

private:
	PointF m_start;
	PointF m_End;
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

	void DrawPath(Graphics& graphics, Pen& pen)
	{
		for (int i = 0; i < m_points->size(); i++)
		{
			
			PointPressure point = m_points->at(i);

			pen.SetWidth(point.GetPressure());

			if (point.GetMyType() == PointType::Line)
			{
				graphics.DrawLine(&pen, point.GetStartPoint(), point.GetEndPoint());
			}
			else
			{
				DrawBezier(graphics, pen, point.GetStartPoint(), point.GetEndPoint());
			}
		} 
	}

	void DrawSmoothLines(Graphics& graphics, Pen& pen)
	{
		if (m_points->size() < 2) 
			return;

		GraphicsPath path;
		path.StartFigure();

		// 첫 번째 점은 그냥 그린다
		PointPressure point = m_points->at(0);
		path.AddLine(point.GetStartPoint(), point.GetEndPoint());

		// 두 번째 점부터 부드러운 라인 그리기
		for (int i = 1; i < m_points->size(); ++i)
		{
			PointPressure pp1 = m_points->at(i - 1);
			PointPressure pp2 = m_points->at(i);

			PointF p1 = pp1.GetEndPoint();
			PointF p2 = pp2.GetEndPoint();

			// 중간점 구하기
			PointF midPoint = GetMidPoint(p1, p2);

			// 컨트롤 포인트 계산하기
			PointF controlPoint1(midPoint.X, p1.Y);
			PointF controlPoint2(midPoint.X, p2.Y);

			// 베지어 곡선 그리기
			path.AddBezier(p1, controlPoint1, controlPoint2, p2);
		}

		path.CloseFigure();

		graphics.DrawPath(&pen, &path);
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

private:
	vector<PointPressure>* m_points;
};

#endif // !POINT_VECTOR_H