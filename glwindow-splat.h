
#ifndef GLWIDGETSPLAT_H
#define GLWIDGETSPLAT_H

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWindow>
#include <vector>

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
