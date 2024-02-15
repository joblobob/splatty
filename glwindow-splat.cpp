/*
* Splatty main window definition!
*/

#include "glwindow-splat.h"


void GLWindowSplat::paintGL()
{
	// set the view to the new coordinates
	m_splatty.setView(worldInteraction(viewMatrix));

	//update canvas when we need to ^_^
	update();
}

std::vector<float> GLWindowSplat::worldInteraction(std::vector<float>& view)
{
	auto inv = invert4(view);

	inv = rotate4(inv, std::sin(16.0f / 2000.5f), 1, -1, 1);
	inv = translate4(inv, 0.05, 0.05, -0.5);

	view = invert4(inv);

	auto viewProj = multiply4(m_splatty.gl.m_projectionMatrix, view);

	return viewProj;
}
