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
	// *** interact with the world ON
	auto inv = invert4(viewMatrix);

	// code a propos des activeskeys pas ré-écrit
	//inv = rotate4(inv, std::sin(16.0f / 2000.5f), 1, -1, 1);
	//inv        = translate4(inv, 0.05, 0.05, -0.5);

	viewMatrix = invert4(inv);

	auto viewProj = multiply4(m_worker.m_projectionMatrix, viewMatrix);
	// *** interact with the world OFF

	// set the view to the new coordinates
	m_worker.setView(viewProj);

	//update canvas when we need to ^_^
	update();
}
