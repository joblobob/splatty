/*
* Splatty main window creation!
*/

#include <QGuiApplication>

#include "GlWindow-splat.h"

int main(int argc, char* argv[])
{
	QGuiApplication app(argc, argv);

	GLWindowSplat window { std::filesystem::path { "plush.splat" } };
	window.showMaximized();

	return app.exec();
}
