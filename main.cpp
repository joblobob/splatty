
#include <QGuiApplication>
#include <QOpenGLContext>
#include <QSurfaceFormat>

#include "GlWindow-splat.h"

int main(int argc, char* argv[])
{
	QGuiApplication app(argc, argv);

	QSurfaceFormat fmt;
	fmt.setDepthBufferSize(24);
	fmt.setOption(QSurfaceFormat::DebugContext);

	// Request OpenGL 3.3 core or OpenGL ES 3.0.
	if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL) {
		fmt.setVersion(3, 3);
		fmt.setProfile(QSurfaceFormat::CoreProfile);
	}
	else {
		fmt.setVersion(3, 0);
	}

	QSurfaceFormat::setDefaultFormat(fmt);

	GLWindowSplat glWindowSplat;
	glWindowSplat.resize(1024, 768);
	glWindowSplat.show();

	return app.exec();
}
