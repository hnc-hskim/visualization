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
		// �׸��� �غ�
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps); 

		if (isOldPathDrawing)
		{
			DrawMyPath(hdc, myPoints);
		}

		// GDI+ Graphics ��ü ����
		Graphics graphics(hdc);

		float default_pen_width = 20;

		// ������ ȿ���� ���� �� ���� �� �Ӽ� ����
		Pen pen(Color(255, 0, 0, 0), default_pen_width);
		pen.SetStartCap(LineCapRound);
		pen.SetEndCap(LineCapRound);

		// ���콺 �̵� ��θ� �׸���
		if (isDrawing)
		{
			// ���� ���콺 ��ġ�� ��ο� �߰�
			//path.AddLine(lastPoint, currentPoint); 
			 
			if (pen_pressure != 1.0F)
			{
				//pen.SetWidth(default_pen_width * pen_pressure);
				default_pen_width = default_pen_width * pen_pressure;
			}

			// ��θ� �׸���
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

		// GraphicsPath�� Point�� �����ϴ� �Լ� ȣ��
		SavePoint(lastPoint, currentPoint, pen_pressure, myPoints);

		// �׸��� ����
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_MOUSEMOVE:  
		// ���콺 �̵� �� ��� ������Ʈ
		if (isDrawing)
		{
			lastPoint = currentPoint;
			currentPoint = Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	case WM_LBUTTONDOWN:
		// ���콺 ���� ��ư Ŭ�� �� �׸��� ����
		isDrawing = true;
		currentPoint = lastPoint = Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		path.Reset();
		path.StartFigure();
		break;
	case WM_LBUTTONUP:
		// ���콺 ���� ��ư ���� �׸��� ����
		isDrawing = false;
		break; 
	case WM_RBUTTONDOWN:
	{
		// ���� Ŀ�� ��ġ ���
		POINT pt;
		GetCursorPos(&pt);

		// ���ؽ�Ʈ �޴� ����
		HMENU hMenu = CreatePopupMenu();
		AppendMenu(hMenu, MF_STRING, 1, L"ȭ�� �����");
		AppendMenu(hMenu, MF_STRING, 2, L"�н� �ҷ�����");
		AppendMenu(hMenu, MF_STRING, 3, L"����� �н������� ����");

		// ���ؽ�Ʈ �޴� ���̱�
		TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);

		// ���ؽ�Ʈ �޴� �ڵ� ����
		DestroyMenu(hMenu); 
	}
	break;
	// �޴� �׸� ���� �̺�Ʈ ó��
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
				// �׸��� ����
				EndPaint(hWnd, &ps);
			}
			break;

		case 2:
			// �޴� �׸� 2 ���� �� ó���� �ڵ� 
			isOldPathDrawing = true;
			InvalidateRect(hWnd, NULL, FALSE);
			break;

		case 3:
			{
				// �޴� �׸� 3 ���� �� ó���� �ڵ�
				myPoints.Clear();
				isOldPathDrawing = false;

				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				OnErase(hWnd, hdc);
				// �׸��� ����
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
		// IsPenMessage �Լ��� ȣ���� pen �Է����� Ȯ���ϰ�, �� �Է��� ��� �� �з��� ������ 
		if (IsPenMessage(wParam))
		{
			UINT32 pressure = GetPenPressure(wParam);

			// �� �з��� 0���� 1024 ������ ������ ����ȭ�˴ϴ�.
			// ����̽����� �з��� �������� �ʴ� ��� �⺻���� 0�Դϴ�.
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