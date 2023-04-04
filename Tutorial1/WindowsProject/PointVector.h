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

	/*void DrawPath(Graphics& graphics, Pen& pen)
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
	}*/

	void DrawPath(Graphics& graphics, Pen& pen)
	{
		if (m_points->size() < 2)
			return;

		for (int i = 1; i < m_points->size(); i++)
		{
			PointPressure pp1 = m_points->at(i - 1);
			PointPressure pp2 = m_points->at(i);

			PointF p1 = pp1.GetPoint();
			PointF p2 = pp2.GetPoint();

			pen.SetWidth(pp2.GetPressure()); 

			graphics.DrawLine(&pen, p1, p2);
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
		path.AddLine(point.GetPoint(), point.GetPoint());

		// 두 번째 점부터 부드러운 라인 그리기
		for (int i = 1; i < m_points->size(); ++i)
		{
			PointPressure pp1 = m_points->at(i - 1);
			PointPressure pp2 = m_points->at(i);

			PointF p1 = pp1.GetPoint();
			PointF p2 = pp2.GetPoint();

			DrawBezier(&graphics, &pen, p1, p2);

			//// 중간점 구하기
			//PointF midPoint = GetMidPoint(p1, p2);

			//// 컨트롤 포인트 계산하기
			//PointF controlPoint1(midPoint.X, p1.Y);
			//PointF controlPoint2(midPoint.X, p2.Y);

			//// 베지어 곡선 그리기
			//path.AddBezier(p1, controlPoint1, controlPoint2, p2);
		}

		path.CloseFigure();

		graphics.DrawPath(&pen, &path);
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
};

#endif // !POINT_VECTOR_H