/*
* Splatty main window definition!
*/

#include "glwindow-splat.h"

void GLWindowSplat::paintGL()
{
	// set the view to the new coordinates
	worldInteraction(viewMatrix);
	m_splatty.setView(viewMatrix[std::array { 0, 2 }], viewMatrix[std::array { 1, 2 }], viewMatrix[std::array { 2, 2 }]);

	//update canvas when we need to ^_^
	update();
}

void GLWindowSplat::worldInteraction(std::mdspan<float, std::extents<std::size_t, 4, 4> > view)
{
	invertMatrix(view);

	rotateMatrix(view, 0.002f, 0.0f, -0.04f, 0.0f);
	translateMatrix(view, 0.01, 0.01, -0.1);

	invertMatrix(view);
}
