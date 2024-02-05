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

	std::vector<float> projectionMatrix;

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

GLWindowSplat::~GLWindowSplat()
{
	delete m_texture;
	delete m_vao;
	delete m_program;
}


void GLWindowSplat::initializeGL()
{
	QOpenGLContext* gl         = QOpenGLContext::currentContext();
	QOpenGLDebugLogger* logger = new QOpenGLDebugLogger(this);
	connect(logger, &QOpenGLDebugLogger::messageLogged, [&](const QOpenGLDebugMessage& debugMessage) { qCritical() << debugMessage; });
	logger->initialize(); // initializes in the current context, i.e. ctx
	logger->startLogging(QOpenGLDebugLogger::SynchronousLogging);

	QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();

	delete m_program;
	m_program = new QOpenGLShaderProgram;
	m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, ShaderSource::vertex);
	m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, ShaderSource::fragment);
	m_program->link();

	m_program->bind();

	// Create a VAO. Not strictly required for ES 3, but it is for plain OpenGL.
	delete m_vao;
	m_vao = new QOpenGLVertexArrayObject(gl);
	if (m_vao->create())
		m_vao->bind();

	f->glDisable(GL_DEPTH_TEST); // Disable depth testing

	f->glEnable(GL_BLEND);
	f->glBlendFuncSeparate(GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_ONE_MINUS_DST_ALPHA, GL_ONE);

	m_projMatrixLoc = m_program->uniformLocation("projection");
	m_viewPortLoc   = m_program->uniformLocation("viewport");
	m_focalLoc      = m_program->uniformLocation("focal");
	m_viewLoc       = m_program->uniformLocation("view");

	// positions
	const std::vector<float> triangleVertices = { -2, -2, 2, -2, 2, 2, -2, 2 };

	m_vertexBuffer.create();

	f->glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer.bufferId());
	f->glBufferData(GL_ARRAY_BUFFER, 8 * 4, triangleVertices.data(), GL_STATIC_DRAW);
	const int a_position = m_program->attributeLocation("position");
	f->glEnableVertexAttribArray(a_position);
	f->glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer.bufferId());
	f->glVertexAttribPointer(a_position, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	m_texture = new QOpenGLTexture(QOpenGLTexture::Target::Target2D);
	f->glBindTexture(GL_TEXTURE_2D, m_texture->textureId());

	auto u_textureLocation = m_program->uniformLocation("u_texture");
	f->glUniform1i(u_textureLocation, 0);

	m_indexBuffer.create();
	const int a_index = m_program->attributeLocation("index");
	f->glEnableVertexAttribArray(a_index);
	f->glBindBuffer(GL_ARRAY_BUFFER, m_indexBuffer.bufferId());
	gl->extraFunctions()->glVertexAttribIPointer(a_index, 1, GL_INT, false, 0);
	gl->extraFunctions()->glVertexAttribDivisor(a_index, 1);
}

void GLWindowSplat::resizeGL(int w, int h)
{
	QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();

	GLfloat tabFloat[] = { focalWidth, focalHeight };
	f->glUniform2fv(m_focalLoc, 1, tabFloat);
	m_projectionMatrix = getProjectionMatrix(focalWidth, focalHeight, w, h);

	GLfloat innerTab[] = { w, h };
	f->glUniform2fv(m_viewPortLoc, 1, innerTab);

	f->glViewport(0, 0, w, h);
	f->glUniformMatrix4fv(m_projMatrixLoc, 1, false, m_projectionMatrix.data());
}

void GLWindowSplat::paintGL()
{
	QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();

	auto inv = invert4(viewMatrix);

	// code a propos des activeskeys pas ré-écrit
	//inv = rotate4(inv, std::sin(16.0f / 2000.5f), 1, -1, 1);
	//inv        = translate4(inv, 0.05, 0.05, -0.5);
	viewMatrix = invert4(inv);

	auto viewProj = multiply4(m_projectionMatrix, viewMatrix);
	m_worker.setView(viewProj);
	if (!gotTexture) {
		gotTexture = true;
		setTextureData(m_worker.texdata, m_worker.texwidth, m_worker.texheight);
		setDepthIndex(m_worker.depthIndex, m_worker.viewProj, m_worker.vertexCount);
	}


	// fps calculations
	if (m_worker.vertexCount > 0) {
		f->glUniformMatrix4fv(m_viewLoc, 1, false, viewMatrix.data());
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
	f->glBindTexture(GL_TEXTURE_2D, m_texture->textureId());
	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32UI, texwidth, texheight, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, texdata.data());
	f->glActiveTexture(GL_TEXTURE0);
	f->glBindTexture(GL_TEXTURE_2D, m_texture->textureId());
}



void GLWindowSplat::setDepthIndex(const std::vector<unsigned int>& depthIndex, const std::vector<float>& viewProj, int vertexCount)
{
	QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();

	f->glBindBuffer(GL_ARRAY_BUFFER, m_indexBuffer.bufferId());
	f->glBufferData(GL_ARRAY_BUFFER, depthIndex.size() * 4, depthIndex.data(), GL_DYNAMIC_DRAW);
}