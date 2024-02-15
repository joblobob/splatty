/*
* Yes this is a C++ module! I love it!
* 
* It's intended to be a single file for the whole implementation for the moment, step by step!
*/

module;


#include <cmath>
#include <limits>
#include <print>
#include <vector>

#include <QFile>
#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>

export module splat.opengl;

import splat.shaders;

import splat.math;

static const std::vector<float> defaultViewMatrix = { 0.47, 0.04, 0.88, 0, -0.11, 0.99, 0.02, 0, -0.88, -0.11, 0.47, 0, 0.07, 0.03, 6.55, 1 };

export std::vector<float> viewMatrix = defaultViewMatrix;

export constexpr int focalWidth  = 1500;
export constexpr int focalHeight = 1500;

export struct glsplat {
	glsplat() : m_texture(QOpenGLTexture::Target::Target2D) {}

	int vertexCount = 0;


	QOpenGLTexture m_texture;
	QOpenGLShaderProgram m_program;
	QOpenGLVertexArrayObject m_vao;

	int m_projMatrixLoc = 0;
	int m_viewPortLoc   = 0;
	int m_focalLoc      = 0;
	int m_viewLoc       = 0;
	QOpenGLBuffer m_indexBuffer;
	QOpenGLBuffer m_vertexBuffer;

	std::vector<float> m_projectionMatrix;



	void viewChanged()
	{
		QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();

		// fps calculations (from paintGL)
		f->glUniformMatrix4fv(m_viewLoc, 1, false, viewMatrix.data());
		f->glClear(GL_COLOR_BUFFER_BIT);
		QOpenGLContext::currentContext()->extraFunctions()->glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, vertexCount);
	}


	void initializeGL()
	{
		//QOpenGLDebugLogger* logger = new QOpenGLDebugLogger(this);
		//connect(logger, &QOpenGLDebugLogger::messageLogged, [&](const QOpenGLDebugMessage& debugMessage) { qCritical() << debugMessage; });
		//logger->initialize(); // initializes in the current context, i.e. ctx
		//logger->startLogging(QOpenGLDebugLogger::SynchronousLogging);

		QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();


		m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, ShaderSource::vertex);
		m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, ShaderSource::fragment);
		m_program.link();

		m_program.bind();

		// Create a VAO. Not strictly required for ES 3, but it is for plain OpenGL.

		if (m_vao.create())
			m_vao.bind();

		f->glDisable(GL_DEPTH_TEST); // Disable depth testing

		f->glEnable(GL_BLEND);
		f->glBlendFuncSeparate(GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_ONE_MINUS_DST_ALPHA, GL_ONE);

		m_projMatrixLoc = m_program.uniformLocation("projection");
		m_viewPortLoc   = m_program.uniformLocation("viewport");
		m_focalLoc      = m_program.uniformLocation("focal");
		m_viewLoc       = m_program.uniformLocation("view");

		// positions
		const std::vector<float> triangleVertices = { -2, -2, 2, -2, 2, 2, -2, 2 };

		m_vertexBuffer.create();

		f->glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer.bufferId());
		f->glBufferData(GL_ARRAY_BUFFER, 8 * 4, triangleVertices.data(), GL_STATIC_DRAW);
		const int a_position = m_program.attributeLocation("position");
		f->glEnableVertexAttribArray(a_position);
		f->glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer.bufferId());
		f->glVertexAttribPointer(a_position, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

		f->glBindTexture(GL_TEXTURE_2D, m_texture.textureId());

		auto u_textureLocation = m_program.uniformLocation("u_texture");
		f->glUniform1i(u_textureLocation, 0);

		m_indexBuffer.create();
		const int a_index = m_program.attributeLocation("index");
		f->glEnableVertexAttribArray(a_index);
		f->glBindBuffer(GL_ARRAY_BUFFER, m_indexBuffer.bufferId());
		f->glVertexAttribIPointer(a_index, 1, GL_INT, false, 0);
		f->glVertexAttribDivisor(a_index, 1);
	}

	void resizeGL(int w, int h)
	{
		QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();

		GLfloat tabFloat[] = { focalWidth, focalHeight };
		f->glUniform2fv(m_focalLoc, 1, tabFloat);
		m_projectionMatrix = getProjectionMatrix(focalWidth, focalHeight, w, h);

		GLfloat innerTab[] = { w, h };
		f->glUniform2fv(m_viewPortLoc, 1, innerTab);

		f->glViewport(0, 0, w, h);
		f->glUniformMatrix4fv(m_projMatrixLoc, 1, false, m_projectionMatrix.data());
	}

	void setTextureData(const std::vector<unsigned int>& texdata, int texwidth, int texheight)
	{
		QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();
		f->glBindTexture(GL_TEXTURE_2D, m_texture.textureId());
		f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32UI, texwidth, texheight, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, texdata.data());
		f->glActiveTexture(GL_TEXTURE0);
		f->glBindTexture(GL_TEXTURE_2D, m_texture.textureId());
	}

	void setDepthIndex(const std::vector<unsigned int>& depthIndex)
	{
		QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();

		f->glBindBuffer(GL_ARRAY_BUFFER, m_indexBuffer.bufferId());
		f->glBufferData(GL_ARRAY_BUFFER, depthIndex.size() * 4, depthIndex.data(), GL_DYNAMIC_DRAW);
	}
};