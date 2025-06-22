#pragma once

struct Point2D
{
	double x;
	double y;

	Point2D()
		:x(0.), y(0.)
	{
	}
};

struct Circle
{
	Point2D center;
	double radius;

	Circle()
		:center(), radius(0.)
	{
	}

	void clear() 
	{
		center.x = 0;
		center.y = 0;
		radius = 0;
	}
};

class PointView : public CView
{
public:
	PointView();
	~PointView();

	void clearAllPoints();
	void randomizePoints();
	void setRadius(double radius);
	void setThickness(double thickness);

private:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg LRESULT OnRandomizeStep(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseLeave();

	void OnDraw(CDC* pDC) final;
	bool calcCircumcircle();
	CPoint* findPointAt(CPoint point);

	std::vector<Gdiplus::PointF> calcPolygonPoints(double centerX, double centerY, double radius);
	void drawFilledCircle(Gdiplus::Graphics& graphics, double centerX, double centerY, double radius, const Gdiplus::Color& color);
	void drawEmptyCircle(Gdiplus::Graphics& graphics, double centerX, double centerY, double radius, double thickness, const Gdiplus::Color& color);

private:
	std::vector<CPoint> m_points;
	CPoint* m_draggingPoint;
	const CPoint* m_hoveredPoint;
	double m_radius;
	double m_thickness;
	bool m_circleValid;
	Circle m_circumcircle;

	std::atomic<bool> m_isRandomizing;
	std::atomic<bool> m_stopRandomizingFlag;
	std::mt19937 m_randomGenerator;
};

