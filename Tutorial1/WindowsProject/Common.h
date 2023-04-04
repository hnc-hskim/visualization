#pragma once

#ifndef COMMON_H
#define COMMON_H

#include <windows.h>
#include <debugapi.h>
#include <objidl.h>
#include <gdiplus.h>  
#include "PointVector.h" 

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp)) 

#define M_PI 3.14159265358979323846   // pi

#define MIN_DISTANCE 30

#endif // !COMMON_H

