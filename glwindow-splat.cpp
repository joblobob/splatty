/*
* Splatty main window definition!
*/

#include "glwindow-splat.h"

#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <print>

void GLWindowSplat::paintGL()
{
	// set the view to the new coordinates
	worldInteraction(viewMatrix);
	m_splatty.setView(viewMatrix[std::array { 0, 2 }], viewMatrix[std::array { 1, 2 }], viewMatrix[std::array { 2, 2 }]);
	//update canvas when we need to ^_^
	update();
}

void GLWindowSplat::worldInteraction(std::mdspan<float, std::extents<std::size_t, 4, 4> > view)
{
	invertMatrix(view);

	//rotateMatrix(view, 0.01f, 0.0f, -0.2f, 0.0f);
	//translateMatrix(view, 0.05, 0.05, -0.5);

	invertMatrix(view);
}

bool GLWindowSplat::event(QEvent* event)
{
	//buttons
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent* ke = static_cast<QKeyEvent*>(event);
		invertMatrix(viewMatrix);
		std::println("{}{}", "key!", ke->key());
		if (ke->key() == Qt::Key_Up) {
			translateMatrix(viewMatrix, 0, 0, 0.5);


		} else if (ke->key() == Qt::Key_Down) {
			translateMatrix(viewMatrix, 0, 0, -0.5);
		} else if (ke->key() == Qt::Key_Left) {
			rotateMatrix(viewMatrix, 0.1, -0.05, 0, 0);
		} else if (ke->key() == Qt::Key_Right) {
			rotateMatrix(viewMatrix, 0.1, 0.05, 0, 0);
		}
		invertMatrix(viewMatrix);
		return true;
	}

	return QOpenGLWindow::event(event);
}

//
//if (activeKeys.includes("ArrowUp")) {
//	if (shiftKey) {
//		inv = translate4(inv, 0, -0.03, 0);
//	} else {
//		inv = translate4(inv, 0, 0, 0.1);
//	}
//}
//if (activeKeys.includes("ArrowDown")) {
//	if (shiftKey) {
//		inv = translate4(inv, 0, 0.03, 0);
//	} else {
//		inv = translate4(inv, 0, 0, -0.1);
//	}
//}
//if (activeKeys.includes("ArrowLeft"))
//	inv = translate4(inv, -0.03, 0, 0);
//
//if (activeKeys.includes("ArrowRight"))
//	inv = translate4(inv, 0.03, 0, 0);
