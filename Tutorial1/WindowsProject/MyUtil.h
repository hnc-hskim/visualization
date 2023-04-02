#include <gdiplus.h>
#include <iostream>
#include <vector>
#include "PointVector.h"

using namespace std;
using namespace Gdiplus;


void SavePoint(Point stPoint, Point endPoint, float pen_pressure, PointVector& myPoints)
{
	PointPressure pt = PointPressure(stPoint, endPoint, pen_pressure);
	myPoints.AddPoint(pt);
}