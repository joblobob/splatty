
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
	~GLWindowSplat();

	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

	void setTextureData(const std::vector<unsigned int>& texdata, int texwidth, int texheight);
	void setDepthIndex(const std::vector<unsigned int>& depthIndex, const std::vector<float>& viewProj, int vertexCount);

private:
	worker m_worker;
};

#endif
