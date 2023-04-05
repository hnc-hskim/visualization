#include "Util.h"

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

void DrawMyPath(HDC hdc, PointVector& myPoints)
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

		path.AddLine(pt.GetPoint(), pt.GetPoint());

		if (pt.GetPressure() != 1.0F)
		{
			pen.SetWidth(default_pen_width * pt.GetPressure());
		}

		// 경로를 그리기
		graphics.SetSmoothingMode(SmoothingModeAntiAlias);
		graphics.DrawPath(&pen, &path);
	}
} 