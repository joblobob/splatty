#include "glwindow-splat.h"

#include <QFile>
#include <QOpenGLDebugLogger>
#include <QOpenGLExtraFunctions>
#include <QOpenGlContext>
#include <QTimer>

#include <format>


GLWindowSplat::GLWindowSplat()
{
	// Construct a data object by reading from file
	constexpr int rowLength = 3 * 4 + 3 * 4 + 4 + 4;

	std::vector<float> newData;
	std::vector<unsigned char> originalData;

	QFile splatFile("plush.splat");
	splatFile.open(QIODevice::ReadOnly);
	qCritical() << "File size:" << newData.size();
	QByteArray splatData = splatFile.readAll();
	splatFile.close();

	for (const unsigned char data : splatData) {
		originalData.push_back(data);
	}

	for (int i = 0; i < splatData.size(); i += 4) {
		float f;
		uchar b[] = { splatData[i + 0], splatData[i + 1], splatData[i + 2], splatData[i + 3] };
		memcpy(&f, &b, sizeof(f));
		newData.push_back(f);
	}

	m_worker.setBuffer(newData, originalData, (originalData.size() / rowLength));
}

GLWindowSplat::~GLWindowSplat() {}


void GLWindowSplat::initializeGL()
{
	m_worker.initializeGL();
}

void GLWindowSplat::resizeGL(int w, int h)
{
	m_worker.resizeGL(w, h);
}

void GLWindowSplat::paintGL()
{
	QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();

	auto inv = invert4(viewMatrix);

	// code a propos des activeskeys pas ré-écrit
	//inv = rotate4(inv, std::sin(16.0f / 2000.5f), 1, -1, 1);
	//inv        = translate4(inv, 0.05, 0.05, -0.5);
	viewMatrix = invert4(inv);

	auto viewProj = multiply4(m_worker.m_projectionMatrix, viewMatrix);
	m_worker.setView(viewProj);
	if (!m_worker.gotTexture) {
		m_worker.gotTexture = true;
		setTextureData(m_worker.texdata, m_worker.texwidth, m_worker.texheight);
		setDepthIndex(m_worker.depthIndex, m_worker.viewProj, m_worker.vertexCount);
	}


	// fps calculations
	if (m_worker.vertexCount > 0) {
		f->glUniformMatrix4fv(m_worker.m_viewLoc, 1, false, viewMatrix.data());
		f->glClear(GL_COLOR_BUFFER_BIT);
		QOpenGLContext::currentContext()->extraFunctions()->glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, m_worker.vertexCount);
	} else {
		f->glClear(GL_COLOR_BUFFER_BIT);
	}

	//update canvas when we need to ^_^
	update();
}

void GLWindowSplat::setTextureData(const std::vector<unsigned int>& texdata, int texwidth, int texheight)
{
	QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();
	f->glBindTexture(GL_TEXTURE_2D, m_worker.m_texture.textureId());
	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32UI, texwidth, texheight, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, texdata.data());
	f->glActiveTexture(GL_TEXTURE0);
	f->glBindTexture(GL_TEXTURE_2D, m_worker.m_texture.textureId());
}



void GLWindowSplat::setDepthIndex(const std::vector<unsigned int>& depthIndex, const std::vector<float>& viewProj, int vertexCount)
{
	QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();

	f->glBindBuffer(GL_ARRAY_BUFFER, m_worker.m_indexBuffer.bufferId());
	f->glBufferData(GL_ARRAY_BUFFER, depthIndex.size() * 4, depthIndex.data(), GL_DYNAMIC_DRAW);
}