/*
* Splatty main window 
*/

#ifndef GLWIDGETSPLAT_H
#define GLWIDGETSPLAT_H

#include <QOpenGLWindow>
#include <mdspan>

import splatty;

class GLWindowSplat : public QOpenGLWindow
{
	Q_OBJECT

public:
	GLWindowSplat(const std::filesystem::path& splatFilePath) : m_splatty(splatFilePath) {}

	void worldInteraction(std::mdspan<float, std::extents<std::size_t, 4, 4> > view);

private:
	Splatty m_splatty;

	void initializeGL() { m_splatty.initializeGL(); }
	void resizeGL(int w, int h) { m_splatty.resizeGL(w, h); }
	void paintGL();
};

#endif
