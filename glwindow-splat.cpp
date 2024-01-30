// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "glwindow-splat.h"

#include <QImage>
#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGlContext>
#include <QOpenGLDebugLogger>
#include <QOpenGLVertexArrayObject>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QTimer>
#include <QFile>

#include <print>
#include <format>
#include <vector>

#include "happly.h"

GLWindowSplat::GLWindowSplat() : m_worker(this)
{
	m_world.setToIdentity();
	m_world.translate(0, 0, -1);
	m_world.rotate(180, 1, 0, 0);

	QSequentialAnimationGroup* animGroup = new QSequentialAnimationGroup(this);
	animGroup->setLoopCount(-1);
	QPropertyAnimation* zAnim0 = new QPropertyAnimation(this, QByteArrayLiteral("z"));
	zAnim0->setStartValue(1.5f);
	zAnim0->setEndValue(10.0f);
	zAnim0->setDuration(2000);
	animGroup->addAnimation(zAnim0);
	QPropertyAnimation* zAnim1 = new QPropertyAnimation(this, QByteArrayLiteral("z"));
	zAnim1->setStartValue(10.0f);
	zAnim1->setEndValue(50.0f);
	zAnim1->setDuration(4000);
	zAnim1->setEasingCurve(QEasingCurve::OutElastic);
	animGroup->addAnimation(zAnim1);
	QPropertyAnimation* zAnim2 = new QPropertyAnimation(this, QByteArrayLiteral("z"));
	zAnim2->setStartValue(50.0f);
	zAnim2->setEndValue(1.5f);
	zAnim2->setDuration(2000);
	animGroup->addAnimation(zAnim2);
	animGroup->start();

	QPropertyAnimation* rAnim = new QPropertyAnimation(this, QByteArrayLiteral("r"));
	rAnim->setStartValue(0.0f);
	rAnim->setEndValue(360.0f);
	rAnim->setDuration(2000);
	rAnim->setLoopCount(-1);
	rAnim->start();

	QTimer::singleShot(4000, this, &GLWindowSplat::startSecondStage);




	//example struff finished

	// Construct a data object by reading from file
	constexpr int rowLength = 3 * 4 + 3 * 4 + 4 + 4;

	/*const int downsample =
		splatData.size() / rowLength > 500000 ? 1 : 1 / 2/*devicePixelRatio*/;
		/*
		qCritical("start init");
		qCritical() << downsample;
		qCritical() << splatData.size() / rowLength;
		qCritical() << splatData.size(); */

		std::vector<float> projectionMatrix;

		std::vector<float> newData;
		std::vector<unsigned char> originalData;

		QFile splatFile("plush.splat");
		splatFile.open(QIODevice::ReadOnly);
		qCritical() << newData.size();
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
	makeCurrent();
	//delete m_texture;
	delete m_program;
	delete m_vbo;
	delete m_vao;
}

void GLWindowSplat::startSecondStage()
{
	QPropertyAnimation* r2Anim = new QPropertyAnimation(this, QByteArrayLiteral("r2"));
	r2Anim->setStartValue(0.0f);
	r2Anim->setEndValue(360.0f);
	r2Anim->setDuration(20000);
	r2Anim->setLoopCount(-1);
	r2Anim->start();
}

void GLWindowSplat::setZ(float v)
{
	m_eye.setZ(v);
	m_uniformsDirty = true;
	update();
}

void GLWindowSplat::setR(float v)
{
	m_r = v;
	m_uniformsDirty = true;
	update();
}

void GLWindowSplat::setR2(float v)
{
	m_r2 = v;
	m_uniformsDirty = true;
	update();
}

static const char* vertexShaderSource = "#extension GL_ARB_shading_language_packing : enable\n"
"uniform highp usampler2D u_texture;\n"
"uniform mat4 projection, view;\n"
"uniform vec2 focal;\n"
"uniform vec2 viewport;\n"
"in vec2 position;\n"
"in int index;\n"
"out vec4 vColor;\n"
"out vec2 vPosition;\n"
"void main () {\n"
" uvec4 cen = texelFetch(u_texture, ivec2((uint(index) & 0x3ffu) << 1, uint(index) >> 10), 0);\n"
"  vec4 cam = view * vec4(uintBitsToFloat(cen.xyz), 1);\n"
"  vec4 pos2d = projection * cam;\n"
"  highp float clip = 1.2 * pos2d.w;\n"
"  if (pos2d.z < -clip || pos2d.x < -clip || pos2d.x > clip || pos2d.y < -clip || pos2d.y > clip) {\n"
"    gl_Position = vec4(0.0, 0.0, 2.0, 1.0);\n"
"    return;\n"
"  }\n"
"  uvec4 cov = texelFetch(u_texture, ivec2(((uint(index) & 0x3ffu) << 1) | 1u, uint(index) >> 10), 0);\n"
"  vec2 u1 = unpackHalf2x16(cov.x), u2 = unpackHalf2x16(cov.y), u3 = unpackHalf2x16(cov.z);\n"
"  mat3 Vrk = mat3(u1.x, u1.y, u2.x, u1.y, u2.y, u3.x, u2.x, u3.x, u3.y);\n"
"  mat3 J = mat3(\n"
"      focal.x / cam.z, 0., -(focal.x * cam.x) / (cam.z * cam.z), \n"
"      0., -focal.y / cam.z, (focal.y * cam.y) / (cam.z * cam.z), \n"
"      0., 0., 0.\n"
"      );\n"
"  mat3 T = transpose(mat3(view)) * J;\n"
"  mat3 cov2d = transpose(T) * Vrk * T;\n"
"  float mid = (cov2d[0][0] + cov2d[1][1]) / 2.0;\n"
"  float radius = length(vec2((cov2d[0][0] - cov2d[1][1]) / 2.0, cov2d[0][1]));\n"
"  float lambda1 = mid + radius, lambda2 = mid - radius;\n"
"  if(lambda2 < 0.0) return;\n"
"  vec2 diagonalVector = normalize(vec2(cov2d[0][1], lambda1 - cov2d[0][0]));\n"
"  vec2 majorAxis = min(sqrt(2.0 * lambda1), 1024.0) * diagonalVector;\n"
"  vec2 minorAxis = min(sqrt(2.0 * lambda2), 1024.0) * vec2(diagonalVector.y, -diagonalVector.x);\n"
"  vColor = clamp(pos2d.z/pos2d.w+1.0, 0.0, 1.0) * vec4((cov.w) & 0xffu, (cov.w >> 8) & 0xffu, (cov.w >> 16) & 0xffu, (cov.w >> 24) & 0xffu) / 255.0;\n"
"  vPosition = position;\n"
"  vec2 vCenter = vec2(pos2d) / pos2d.w;\n"
"  gl_Position = vec4(\n"
"      vCenter \n"
"	  + position.x * majorAxis / viewport \n"
"	  + position.y * minorAxis / viewport, 0.0, 1.0);\n"
"}\n";
;

static const char* fragmentShaderSource = "in highp vec4 vColor;\n"
"in highp vec2 vPosition;\n"
"out highp vec4 fragColor;\n"
"void main () {\n"
"  highp float A = -dot(vPosition, vPosition);\n"
"  if (A < -4.0) discard;\n"
"  highp float B = exp(A) * vColor.a;\n"
"  fragColor = vec4(B * vColor.rgb, B);\n"
"}\n";

QByteArray versionedShaderCodehere(const char* src)
{
	QByteArray versionedSrc;

	if (QOpenGLContext::currentContext()->isOpenGLES())
		versionedSrc.append(QByteArrayLiteral("#version 300 es\n"));
	else
		versionedSrc.append(QByteArrayLiteral("#version 330\n"));

	versionedSrc.append(src);
	return versionedSrc;
}

void GLWindowSplat::initializeGL()
{

	QOpenGLContext* gl = QOpenGLContext::currentContext();
	QOpenGLDebugLogger* logger = new QOpenGLDebugLogger(this);
	connect(logger, &QOpenGLDebugLogger::messageLogged, [&](const QOpenGLDebugMessage& debugMessage) { qCritical() << "OpenGLDebug: " << debugMessage; });
	logger->initialize(); // initializes in the current context, i.e. ctx
	logger->startLogging(QOpenGLDebugLogger::SynchronousLogging);

	QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();

	delete m_program;
	m_program = new QOpenGLShaderProgram;
	// Prepend the correct version directive to the sources. The rest is the
	// same, thanks to the common GLSL syntax.
	bool isVertexOk = m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, versionedShaderCodehere(vertexShaderSource));
	bool isFragmentOk = m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, versionedShaderCodehere(fragmentShaderSource));
	bool isLinked = m_program->link();

	bool isBoundProgram = m_program->bind();

	qCritical() << "hola" << isVertexOk << isFragmentOk << isLinked << isBoundProgram;

	// Create a VAO. Not strictly required for ES 3, but it is for plain OpenGL.
	delete m_vao;
	m_vao = new QOpenGLVertexArrayObject(gl);
	if (m_vao->create())
		m_vao->bind();

	f->glDisable(GL_DEPTH_TEST); // Disable depth testing

	f->glEnable(GL_BLEND);
	f->glBlendFuncSeparate(GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_ONE_MINUS_DST_ALPHA, GL_ONE);

	m_projMatrixLoc = m_program->uniformLocation("projection");
	m_viewPortLoc = m_program->uniformLocation("viewport");
	m_focalLoc = m_program->uniformLocation("focal");
	m_viewLoc = m_program->uniformLocation("view");

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
	//m_proj.setToIdentity();
	//m_proj.perspective(45.0f, GLfloat(w) / h, 0.01f, 100.0f);
	m_uniformsDirty = true;

	QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();

	GLfloat tabFloat[] = { baseCamera.fx,  baseCamera.fy };
	f->glUniform2fv(m_focalLoc, 1, tabFloat);

	m_projectionMatrix = getProjectionMatrix(baseCamera.fx, baseCamera.fy, w, h);

	GLfloat innerTab[] = { static_cast<float>(w),  static_cast<float>(h) };
	f->glUniform2fv(m_viewPortLoc, 1, innerTab);

	f->glViewport(0, 0, w, h);
	f->glUniformMatrix4fv(m_projMatrixLoc, 1, false, m_projectionMatrix.data());
}

void GLWindowSplat::paintGL()
{
	// Now use QOpenGLExtraFunctions instead of QOpenGLFunctions as we want to
	// do more than what GL(ES) 2.0 offers.
	QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();

	auto inv = invert4(viewMatrix);

	// code a propos des activeskeys pas ré-écrit
	//inv = rotate4(inv, -0.6f * std::sin(16.0f / 5000.5f), 0, 1, 0);

	viewMatrix = invert4(inv);

	auto viewProj = multiply4(m_projectionMatrix, viewMatrix);
	m_worker.setView(viewProj);

	// fps calculations
	if (m_worker.vertexCount > 0) {
		f->glUniformMatrix4fv(m_viewLoc, 1, false, viewMatrix.data());
		f->glClear(GL_COLOR_BUFFER_BIT);
		QOpenGLContext::currentContext()->extraFunctions()->glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, m_worker.vertexCount);
	}
	else {
		f->glClear(GL_COLOR_BUFFER_BIT);
	}

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



void GLWindowSplat::setDepthIndex(const std::vector<unsigned int>& depthIndex, const std::vector<float>& viewProj, int vertexCount) {
	QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();

	f->glBindBuffer(GL_ARRAY_BUFFER, m_indexBuffer.bufferId());
	f->glBufferData(GL_ARRAY_BUFFER, depthIndex.size() * 4, depthIndex.data(), GL_DYNAMIC_DRAW);
}