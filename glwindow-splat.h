/*
* Splatty main window 
*/

#ifndef GLWIDGETSPLAT_H
#define GLWIDGETSPLAT_H

#include <QOpenGLWindow>

import splatty;

class GLWindowSplat : public QOpenGLWindow
{
	Q_OBJECT

public:
	GLWindowSplat(const std::filesystem::path& splatFilePath) : m_splatty(splatFilePath) {}

	void initializeGL() { m_splatty.initializeGL(); }
	void resizeGL(int w, int h) { m_splatty.resizeGL(w, h); }
	void paintGL();

	std::vector<float> worldInteraction(std::vector<float>& viewMatrix);

private:
	splatdata m_splatty;
};

#endif
