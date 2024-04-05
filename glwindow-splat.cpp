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

std::array<float, 16> GLWindowSplat::worldInteraction(std::array<float, 16>& view)
{
	auto md = std::mdspan<float, std::extents<std::size_t, 4, 4> >(view.data(), 4, 4);

	invertMatrix(md);

	rotateMatrix(md, std::sin(16.0f / 2000.5f), 1, -1, 1);
	translateMatrix(md, 0.05, 0.05, -0.5);

	invertMatrix(md);

	return view;
}
