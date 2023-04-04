#ifndef _MY_UTIL_H
#define _MY_UTIL_H

#include "Common.h"

void SavePoint(Point stPoint, Point endPoint, float pen_pressure, PointVector& myPoints);
float GetDistance(PointF point1, PointF point2); 
float GetAngle(PointF point1, PointF point2); 
float GetThickness(float distance, float angle, float defaultThickness); 
void DrawPathWithVariableThickness(Graphics* graphics, vector<PointF> points, int count, Pen* pen, float default_pen_width); 
std::vector<PointF> SplitPointsByDistance(PointF startPoint, PointF endPoint, float distance); 
void DrawLineEx(Graphics* graphics, Pen* pen, PointF prevPt, PointF currentPt, float default_pen_width);  
void DrawLine(Graphics* graphics, Pen* pen, PointF prevPt, PointF currentPt, float default_pen_width);  
void DrawMyPath(HDC hdc, PointVector& myPoints); 

#endif	//_MY_UTIL_H