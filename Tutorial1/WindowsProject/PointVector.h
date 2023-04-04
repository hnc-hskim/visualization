#pragma once
#ifndef POINT_VECTOR_H
#define POINT_VECTOR_H 

#include <gdiplus.h>
#include <iostream>
#include <vector>

using namespace std;
using namespace Gdiplus;

// 포인트와 압력값을 저장하는 클래스
class PointPressure
{
public:
	PointPressure(PointF start, PointF end, float pressure)
	{
		m_start = start;
		m_End = end;
		m_pressure = pressure;
	} 

	PointF GetStartPoint()
	{
		return m_start;
	}

	PointF GetEndPoint()
	{
		return m_End;
	}

	float GetPressure()
	{
		return m_pressure;
	}
	 

private:
	PointF m_start;
	PointF m_End;
	float m_pressure;
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

	void DrawPath(Graphics& graphics, Pen& pen)
	{
		for (int i = 0; i < m_points->size(); i++)
		{
			
			PointPressure point = m_points->at(i);

			pen.SetWidth(point.GetPressure());

			graphics.DrawLine(&pen, point.GetStartPoint(), point.GetEndPoint());
		} 
	}

private:
	vector<PointPressure>* m_points;
};

#endif // !POINT_VECTOR_H