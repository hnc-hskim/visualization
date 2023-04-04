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

		// ù ��° ���� �׳� �׸���
		PointPressure point = m_points->at(0);
		path.AddLine(point.GetPoint(), point.GetPoint());

		// �� ��° ������ �ε巯�� ���� �׸���
		for (int i = 1; i < m_points->size(); ++i)
		{
			PointPressure pp1 = m_points->at(i - 1);
			PointPressure pp2 = m_points->at(i);

			PointF p1 = pp1.GetPoint();
			PointF p2 = pp2.GetPoint();

			DrawBezier(&graphics, &pen, p1, p2);

			//// �߰��� ���ϱ�
			//PointF midPoint = GetMidPoint(p1, p2);

			//// ��Ʈ�� ����Ʈ ����ϱ�
			//PointF controlPoint1(midPoint.X, p1.Y);
			//PointF controlPoint2(midPoint.X, p2.Y);

			//// ������ � �׸���
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
};

#endif // !POINT_VECTOR_H