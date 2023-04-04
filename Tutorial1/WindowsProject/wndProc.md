LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL isDrawing = false;
	static BOOL isOldPathDrawing = false;
	static Point lastPoint = Point(0, 0);
	static Point currentPoint = Point(0, 0);
	GraphicsPath path;

	static float pen_pressure = 1.0F; 

	static vector<PointF> Points = vector<PointF>();

	switch (message)
	{
	case WM_PAINT:
	{
		// 그리기 준비
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps); 

		if (isOldPathDrawing)
		{
			DrawMyPath(hdc, myPoints);
		}

		// GDI+ Graphics 객체 생성
		Graphics graphics(hdc);

		float default_pen_width = 20;

		// 형광펜 효과를 위한 펜 생성 및 속성 설정
		Pen pen(Color(255, 0, 0, 0), default_pen_width);
		pen.SetStartCap(LineCapRound);
		pen.SetEndCap(LineCapRound);

		// 마우스 이동 경로를 그리기
		if (isDrawing)
		{
			// 현재 마우스 위치를 경로에 추가
			//path.AddLine(lastPoint, currentPoint); 
			 
			if (pen_pressure != 1.0F)
			{
				//pen.SetWidth(default_pen_width * pen_pressure);
				default_pen_width = default_pen_width * pen_pressure;
			}

			// 경로를 그리기
			graphics.SetSmoothingMode(SmoothingModeAntiAlias);

#ifdef Test0 
			DrawLine(&graphics, &pen, PointF(lastPoint.X, lastPoint.Y), PointF(currentPoint.X, currentPoint.Y), default_pen_width);
#endif 

#ifdef Test1
			DrawLineEx(&graphics, &pen, PointF(lastPoint.X, lastPoint.Y), PointF(currentPoint.X, currentPoint.Y), default_pen_width); 
#endif  

#ifdef Test2
			{
				std::vector<PointF> points = SplitPointsByDistance(PointF(lastPoint.X, lastPoint.Y), PointF(currentPoint.X, currentPoint.Y), default_pen_width);
				//if (points.size() > 2)
				{
					DrawPathWithVariableThickness(&graphics, points, points.size(), &pen, default_pen_width);
				}
				/*else
				{
					pen.SetWidth(default_pen_width);
					path.AddLine(lastPoint, currentPoint);
					graphics.DrawPath(&pen, &path);
				}*/
			} 
#endif
#ifdef Test3
			{
				Gdiplus::Graphics graphics(hWnd);

				OnErase(hWnd, hdc);

				if (Points.size() == 0)
				{
					Points.push_back(PointF(currentPoint.X, currentPoint.Y)); 
					graphics.DrawLines(&pen, &Points[0], Points.size());

					return 0;
				}

				if (isDrawing) 
				{
					PointF prevPoint = Points.back();
					float distance = GetDistance(PointF(prevPoint.X, prevPoint.Y), PointF(currentPoint.X, currentPoint.Y));

					if (distance >= MIN_DISTANCE) { 
						float angle = GetAngle(PointF(prevPoint.X, prevPoint.Y), PointF(currentPoint.X, currentPoint.Y));
						float thickness = GetThickness(distance, angle, 10);

						Points.push_back(PointF(currentPoint.X, currentPoint.Y));
						pen.SetWidth(thickness); 

						graphics.DrawLines(&pen, &Points[0], Points.size());
					}
					else {
						Points.pop_back();
						Points.push_back(PointF(currentPoint.X, currentPoint.Y)); 

						graphics.DrawLines(&pen, &Points[0], Points.size());

					}
				}
			}
#endif
		}

		// GraphicsPath의 Point를 저장하는 함수 호출
		SavePoint(lastPoint, currentPoint, pen_pressure, myPoints);

		// 그리기 종료
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_MOUSEMOVE:  
		// 마우스 이동 시 경로 업데이트
		if (isDrawing)
		{
			lastPoint = currentPoint;
			currentPoint = Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	case WM_LBUTTONDOWN:
		// 마우스 왼쪽 버튼 클릭 시 그리기 시작
		isDrawing = true;
		currentPoint = lastPoint = Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		path.Reset();
		path.StartFigure();
		break;
	case WM_LBUTTONUP:
		// 마우스 왼쪽 버튼 떼면 그리기 종료
		isDrawing = false;
		break; 
	case WM_RBUTTONDOWN:
	{
		// 현재 커서 위치 얻기
		POINT pt;
		GetCursorPos(&pt);

		// 컨텍스트 메뉴 생성
		HMENU hMenu = CreatePopupMenu();
		AppendMenu(hMenu, MF_STRING, 1, L"화면 지우기");
		AppendMenu(hMenu, MF_STRING, 2, L"패스 불러오기");
		AppendMenu(hMenu, MF_STRING, 3, L"저장된 패스데이터 삭제");

		// 컨텍스트 메뉴 보이기
		TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);

		// 컨텍스트 메뉴 핸들 해제
		DestroyMenu(hMenu); 
	}
	break;
	// 메뉴 항목 선택 이벤트 처리
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case 1:
			{
				isOldPathDrawing = false;

				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				OnErase(hWnd, hdc);
				// 그리기 종료
				EndPaint(hWnd, &ps);
			}
			break;

		case 2:
			// 메뉴 항목 2 선택 시 처리할 코드 
			isOldPathDrawing = true;
			InvalidateRect(hWnd, NULL, FALSE);
			break;

		case 3:
			{
				// 메뉴 항목 3 선택 시 처리할 코드
				myPoints.Clear();
				isOldPathDrawing = false;

				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				OnErase(hWnd, hdc);
				// 그리기 종료
				EndPaint(hWnd, &ps);
			}
			break;

		default:
			break;
		}

		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_POINTERUPDATE:
	{
		// IsPenMessage 함수를 호출해 pen 입력인지 확인하고, 펜 입력일 경우 펜 압력을 가져옴 
		if (IsPenMessage(wParam))
		{
			UINT32 pressure = GetPenPressure(wParam);

			// 펜 압력은 0에서 1024 사이의 범위로 정규화됩니다.
			// 디바이스에서 압력을 보고하지 않는 경우 기본값은 0입니다.
			if(pressure != 0)
			{ 
				pen_pressure = (float)pressure / 1024.0F;
			}
			else
			{
				pen_pressure = 1.0F;
			} 
		} 
	}  
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
} 