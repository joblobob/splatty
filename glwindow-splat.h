
#ifndef GLWIDGETSPLAT_H
#define GLWIDGETSPLAT_H

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWindow>
#include <limits>
#include <print>
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
	QOpenGLTexture* m_texture       = nullptr;
	QOpenGLShaderProgram* m_program = nullptr;
	QOpenGLVertexArrayObject* m_vao = nullptr;
	bool gotTexture                 = false;

	QOpenGLBuffer m_indexBuffer;
	QOpenGLBuffer m_vertexBuffer;

	int m_projMatrixLoc = 0;
	int m_viewPortLoc   = 0;
	int m_focalLoc      = 0;
	int m_viewLoc       = 0;

	worker m_worker;
	std::vector<float> m_projectionMatrix;
};

#endif
