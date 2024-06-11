/*
* Splatty main window 
*/

#ifndef GLWIDGETSPLAT_H
#define GLWIDGETSPLAT_H

#include <QMouseEvent>
#include <QOpenGLWindow>
#include <mdspan>

#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <print>

import splatty;
import splat.math;

class GLWindowSplat : public QOpenGLWindow
{
	Q_OBJECT

public:
	GLWindowSplat(const std::filesystem::path& splatFilePath) : m_splatty(splatFilePath), mousepress(false) { installEventFilter(this); }

	void worldInteraction(std::mdspan<float, std::extents<std::size_t, 4, 4> > view);

protected:
	bool event(QEvent* event) override;
	void mouseMoveEvent(QMouseEvent* e) override
	{
		if (mousepress) {
			std::println("{} x:{} y:{}", "mouse move!", startX, startY);
			invertMatrix(viewMatrix);

			float dx = (float)(5.0 * (float)(e->x() - startX)) / (float)width();
			float dy = (float)(5.0 * (float)(e->y() - startY)) / (float)height();
			auto d   = 4;
			std::println("{} x:{} y:{}", dx, dy, d);
			translateMatrix(viewMatrix, 0, 0, d);
			rotateMatrix(viewMatrix, dx, 0, 1, 0);
			rotateMatrix(viewMatrix, -dy, 1, 0, 0);
			translateMatrix(viewMatrix, 0, 0, -d);
			// let postAngle = Math.atan2(inv[0], inv[10])
			// inv = rotate4(inv, postAngle - preAngle, 0, 0, 1)
			// console.log(postAngle)

			startX = e->x();
			startY = e->y();
			invertMatrix(viewMatrix);
			e->ignore();
		}
	}
	void mousePressEvent(QMouseEvent* ev) override
	{
		startX = ev->x();
		startY = ev->y();
		std::println("{} x:{} y:{}", "mouse press!", startX, startY);
		mousepress = true;
		ev->ignore();
	}
	void mouseReleaseEvent(QMouseEvent* ev) override
	{
		std::println("{}", "mouse release!");
		startX     = 0;
		startY     = 0;
		mousepress = false;
		ev->ignore();
	}

private:
	Splatty m_splatty;

	int startX, startY;
	bool mousepress;

	void initializeGL() { m_splatty.initializeGL(); }
	void resizeGL(int w, int h) { m_splatty.resizeGL(w, h); }
	void paintGL();
};

#endif
