
#ifndef GLWIDGETSPLAT_H
#define GLWIDGETSPLAT_H

#include <QOpenGLWindow>

import splatty;

class GLWindowSplat : public QOpenGLWindow
{
	Q_OBJECT

public:
	GLWindowSplat();

	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

private:
	worker m_worker;
};

#endif
