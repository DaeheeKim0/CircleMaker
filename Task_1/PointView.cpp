#include "pch.h"
#include "PointView.h"
#include "resource.h"
#include <gdiplus.h>

BEGIN_MESSAGE_MAP(PointView, CView)
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_ERASEBKGND()
    ON_MESSAGE(WM_APP_RANDOMIZE_STEP, &PointView::OnRandomizeStep)
    ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()

constexpr float PI = 3.14159f;

PointView::PointView()
    :CView(), m_radius(5), m_thickness(2), m_circleValid(false), m_draggingPoint(nullptr), m_hoveredPoint(nullptr), m_isRandomizing(false), m_stopRandomizingFlag(false)
{
    m_randomGenerator.seed(std::random_device()());
}

PointView::~PointView()
{
    m_stopRandomizingFlag = true;
    while (m_isRandomizing.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void PointView::clearAllPoints()
{
    m_points.clear();
    m_circleValid = false;
    if (m_isRandomizing)
    {
        m_stopRandomizingFlag = true;
    }

    if (auto pParent = GetParent())
    {
        pParent->SendMessage(WM_APP_POINT_COUNT_CHANGED, m_points.size(), 0);
    }
    Invalidate();
}

void PointView::randomizePoints()
{
    if (m_points.size() != THREE)
    {
        return;
    }

    if (m_isRandomizing.exchange(true))
    {
        return;
    }

    m_stopRandomizingFlag = false;

    // [요구사항] 랜덤한 위치로 이동 및 정원 그리기 동작을 초당 2회, 총 10번 자동으로 반복하되 메인UI가 프리징 상태가 되지 않도록 별도 쓰레드로 구현해야 합니다.
    std::thread([this]() {
        HWND hwnd = GetSafeHwnd();
        
        for (int i = 0; i < 10; ++i)
        {
            if (!hwnd || !::IsWindow(hwnd))
            {
                break;
            }

            if (m_points.size() != THREE)
            {
                break;
            }

            if (m_stopRandomizingFlag)
            {
                break;
            }

            if (::PostMessage(hwnd, WM_APP_RANDOMIZE_STEP, 0, 0) == 0)
            {
                break;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        m_isRandomizing = false;
        }).detach();
}

void PointView::setRadius(double radius)
{
    auto newRadius = max(radius, 0.1);
    if (abs(m_radius - newRadius) > 0.1)
    {
        m_radius = newRadius;
        Invalidate();
    }
}

void PointView::setThickness(double thickness)
{
    auto newThickness = max(thickness, 0.1);
    if (abs(m_thickness - newThickness) > 0.1)
    {
        m_thickness = newThickness;
        Invalidate();
    }
}

void PointView::OnLButtonDown(UINT nFlags, CPoint point)
{
    // [요구사항] 클릭 지점 3개 중 하나를 클릭하고 드래그 했을 때 정원을 다시 그립니다.
    if (m_points.size() == THREE)
    {
        if (auto* p = findPointAt(point))
        {
            m_draggingPoint = p;
            SetCapture();
            Invalidate();
            CView::OnLButtonDown(nFlags, point);
            return;
        }
    }

    // [요구사항] 네 번째 클릭부터는 클릭 지점 원을 그리지 않습니다.
    if (m_points.size() < THREE)
    {
        m_points.emplace_back(point);

        // [요구사항] 세 번째 클릭 이후에 클릭 지점 3개를 모두 지나가는 정원 1개를 그립니다.
        if (m_points.size() == THREE)
        {
            m_circleValid = calcCircumcircle();

            if (auto pParent = GetParent())
            {
                pParent->SendMessage(WM_APP_POINT_COUNT_CHANGED, m_points.size(), 0);
            }
        }
        Invalidate();
    }

    CView::OnLButtonDown(nFlags, point);
}

LRESULT PointView::OnRandomizeStep(WPARAM wParam, LPARAM lParam)
{
	if (m_points.size() != THREE)
	{
		m_stopRandomizingFlag = true;
		return 1;
	}

	CRect rect;
	GetClientRect(&rect);

	if (rect.Width() <= 0 || rect.Height() <= 0)
	{
		m_stopRandomizingFlag = true;
		return 1;
	}

	std::uniform_int_distribution<int> distX(rect.left, rect.right);
	std::uniform_int_distribution<int> distY(rect.top, rect.bottom);

	// [요구사항] 이 때, 정원 또한 마찬가지로 다시 그려져야 합니다.
	for (auto& p : m_points)
	{
		p = { distX(m_randomGenerator), distY(m_randomGenerator) };
	}

	m_circleValid = calcCircumcircle();
	Invalidate();

	return 0;
}

void PointView::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (m_draggingPoint)
    {
        m_draggingPoint = nullptr;
        ReleaseCapture();
    }

    CView::OnLButtonUp(nFlags, point);
}

void PointView::OnMouseMove(UINT nFlags, CPoint point)
{
	// [요구사항] 마우스 커서 좌표가 바뀌는 동안 즉, 마우스 드래그 상태가 끝날 때 까지 정원이 계속해서 이동하며 그려져야 합니다.
	if (m_draggingPoint)
	{
		*m_draggingPoint = point;
		m_circleValid = calcCircumcircle();
		Invalidate();
	}
	else
	{
		const CPoint* previouslyHovered = m_hoveredPoint;
		m_hoveredPoint = findPointAt(point);

		if (m_hoveredPoint != previouslyHovered)
		{
			Invalidate();

			// tracking leave window
			if (m_hoveredPoint)
			{
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = GetSafeHwnd();
				tme.dwHoverTime = 0;
				_TrackMouseEvent(&tme);
			}
		}
	}

	CView::OnMouseMove(nFlags, point);
}

void PointView::OnMouseLeave()
{
    if (m_hoveredPoint)
    {
        m_hoveredPoint = nullptr;
        Invalidate();
    }
    CView::OnMouseLeave();
}

void PointView::OnDraw(CDC* pDC)
{
    CRect rect;
    GetClientRect(&rect);
    int width = rect.Width();
    int height = rect.Height();

    Gdiplus::Bitmap offscreenBitmap(width, height);

    Gdiplus::Graphics ofScreenGraphics(&offscreenBitmap);
    ofScreenGraphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    ofScreenGraphics.Clear(Gdiplus::Color(255, 255, 255, 255));

    Gdiplus::Color commonColor(Gdiplus::Color::DarkOrange);
    Gdiplus::Color hoverColor(255, 200, 200, 200);
    Gdiplus::Color circumcircleColor(255, 0, 0, 0);
    Gdiplus::Color alertMessageColor(255, 255, 100, 100);

    Gdiplus::Font font(L"Arial", 10);
    Gdiplus::SolidBrush commonBrush(commonColor);
    Gdiplus::SolidBrush hoverBrush(hoverColor);
    Gdiplus::SolidBrush alertMessageBrush(alertMessageColor);

    for (const auto& point : m_points)
    {
        bool isHoverPoint = &point == m_hoveredPoint;
        drawFilledCircle(ofScreenGraphics, point.x, point.y, m_radius, isHoverPoint ? hoverColor : commonColor);

        // [요구사항] 각 클릭 지점 원의 중심 좌표를 UI에 표시합니다.
        // draw coord
        {
            CString strCoord;
            strCoord.Format(L"(x: %d, y: %d)", point.x, point.y);

            Gdiplus::RectF textRect;
            ofScreenGraphics.MeasureString(strCoord, -1, &font, Gdiplus::PointF(0, 0), &textRect);

            float textX = static_cast<float>(point.x) - textRect.Width / 2.f;
            float textY = static_cast<float>(point.y) - static_cast<float>(m_radius) - textRect.Height - 5.f;

            ofScreenGraphics.DrawString(strCoord, -1, &font, Gdiplus::PointF{ textX, textY }, isHoverPoint ? &hoverBrush : &commonBrush);
        }
    }

    if (m_points.size() == THREE)
    {
        // draw circumcircle
        if (m_circleValid)
        {
            // [요구사항] 세 클릭 지점을 지나가는 정원의 내부는 채워지지 않아야 합니다.
            drawEmptyCircle(ofScreenGraphics, m_circumcircle.center.x, m_circumcircle.center.y, m_circumcircle.radius, m_thickness, circumcircleColor);
        }
        // alert invalid circumcircle
        else
        {
            CString alert = L"Can not draw circumcircle";
            Gdiplus::RectF textRect;
            ofScreenGraphics.MeasureString(alert, -1, &font, Gdiplus::PointF(0, 0), &textRect);

            float textX = width / 2.f - textRect.Width / 2.f;
            float textY = textRect.Height;

            ofScreenGraphics.DrawString(alert, -1, &font, Gdiplus::PointF{ textX, textY }, &alertMessageBrush);
        }
    }

    Gdiplus::Graphics onScreenGraphics(pDC->GetSafeHdc());
    onScreenGraphics.DrawImage(&offscreenBitmap, 0, 0);
}

CPoint* PointView::findPointAt(CPoint point)
{
	int th = max(static_cast<int>(m_radius), 10);
	for (auto& pt : m_points)
	{
		CRect rect(pt.x - th, pt.y - th, pt.x + th, pt.y + th);
		if (rect.PtInRect(point))
		{
			return &pt;
		}
	}
	return nullptr;
}

bool PointView::calcCircumcircle()
{
    if (m_points.size() != THREE)
    {
        return false;
    }

    CPoint& p1 = m_points[0];
    CPoint& p2 = m_points[1];
    CPoint& p3 = m_points[2];

    double d = 2.0 * (static_cast<double>(p1.x) * (p2.y - p3.y) + static_cast<double>(p2.x) * (p3.y - p1.y) + static_cast<double>(p3.x) * (p1.y - p2.y));

    // check validation
	{
		double s1_length_sq = static_cast<double>(p1.x - p2.x) * (p1.x - p2.x) + static_cast<double>(p1.y - p2.y) * (p1.y - p2.y);
		double s2_length_sq = static_cast<double>(p2.x - p3.x) * (p2.x - p3.x) + static_cast<double>(p2.y - p3.y) * (p2.y - p3.y);
		double s3_length_sq = static_cast<double>(p3.x - p1.x) * (p3.x - p1.x) + static_cast<double>(p3.y - p1.y) * (p3.y - p1.y);

		double max_len_sq = max(max(s1_length_sq, s2_length_sq), s3_length_sq);
		double longest_side = sqrt(max_len_sq);

		if (longest_side < 0.1)
		{
			return false;
		}

		double height = abs(d) / longest_side;
		if (height < 3.0)
		{
			return false;
		}
    }

    // calc center
    {
        double p1_sq = static_cast<double>(p1.x) * p1.x + static_cast<double>(p1.y) * p1.y;
        double p2_sq = static_cast<double>(p2.x) * p2.x + static_cast<double>(p2.y) * p2.y;
        double p3_sq = static_cast<double>(p3.x) * p3.x + static_cast<double>(p3.y) * p3.y;

        double centerX = (p1_sq * (p2.y - p3.y) + p2_sq * (p3.y - p1.y) + p3_sq * (p1.y - p2.y)) / d;
        double centerY = (p1_sq * (p3.x - p2.x) + p2_sq * (p1.x - p3.x) + p3_sq * (p2.x - p1.x)) / d;

        m_circumcircle.center.x = centerX;
        m_circumcircle.center.y = centerY;
    }

    // calc radius
    {
        double dx = m_circumcircle.center.x - p1.x;
        double dy = m_circumcircle.center.y - p1.y;
        m_circumcircle.radius = sqrt(dx * dx + dy * dy);
    }

    return true;
}

// prevent flicker when redraw
BOOL PointView::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}

void PointView::drawFilledCircle(Gdiplus::Graphics& graphics, double centerX, double centerY, double radius, const Gdiplus::Color& color)
{
    auto points = calcPolygonPoints(centerX, centerY, radius);
    Gdiplus::SolidBrush brush(color);

    // [요구사항] 클릭 지점 원을 그릴 때 Ellipse류 함수를 사용하면 안됩니다.
    graphics.FillPolygon(&brush, points.data(), static_cast<int>(points.size()));
}

void PointView::drawEmptyCircle(Gdiplus::Graphics& graphics, double centerX, double centerY, double radius, double thickness, const Gdiplus::Color& color)
{
    auto points = calcPolygonPoints(centerX, centerY, radius);
    Gdiplus::Pen pen(color, static_cast<float>(thickness));
    pen.SetLineJoin(Gdiplus::LineJoinRound);   

    // [요구사항] 정원을 그릴 때 Ellipse류 함수를 사용하면 안됩니다.
    graphics.DrawPolygon(&pen, points.data(), static_cast<int>(points.size()));
}

std::vector<Gdiplus::PointF> PointView::calcPolygonPoints(double centerX, double centerY, double radius)
{
    const int segments = static_cast<int>(radius * 2);
    const float angleStep = 2.0f * PI / segments;

    std::vector<Gdiplus::PointF> points;
    for (int i = 0; i <= segments; ++i)
    {
        float angle = i * angleStep;
        float x = static_cast<float>(centerX) + static_cast<float>(radius) * cosf(angle);
        float y = static_cast<float>(centerY) + static_cast<float>(radius) * sinf(angle);
        points.emplace_back(x, y);
    }

    return points;
}
