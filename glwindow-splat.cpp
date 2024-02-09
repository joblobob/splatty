#include "glwindow-splat.h"

GLWindowSplat::GLWindowSplat() : m_worker(std::filesystem::path { "plush.splat" }) {}


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

	auto viewProj = multiply4(m_worker.gl.m_projectionMatrix, viewMatrix);
	// *** interact with the world OFF

	// set the view to the new coordinates
	m_worker.setView(viewProj);

	//update canvas when we need to ^_^
	update();
}
