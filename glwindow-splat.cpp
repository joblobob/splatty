// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "glwindow-splat.h"

#include <QImage>
#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QTimer>

GLWindowSplat::GLWindowSplat()
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
	QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();

	delete m_program;
	m_program = new QOpenGLShaderProgram;
	// Prepend the correct version directive to the sources. The rest is the
	// same, thanks to the common GLSL syntax.
	m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, versionedShaderCodehere(vertexShaderSource));
	m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, versionedShaderCodehere(fragmentShaderSource));
	m_program->link();

	m_projMatrixLoc = m_program->uniformLocation("projMatrix");
	m_camMatrixLoc = m_program->uniformLocation("camMatrix");
	m_worldMatrixLoc = m_program->uniformLocation("worldMatrix");
	m_myMatrixLoc = m_program->uniformLocation("myMatrix");
	m_lightPosLoc = m_program->uniformLocation("lightPos");

	// Create a VAO. Not strictly required for ES 3, but it is for plain OpenGL.
	delete m_vao;
	m_vao = new QOpenGLVertexArrayObject;
	if (m_vao->create())
		m_vao->bind();

	m_program->bind();
	delete m_vbo;
	m_vbo = new QOpenGLBuffer;
	m_vbo->create();
	m_vbo->bind();
	//m_vbo->allocate(m_logo.constData(), m_logo.count() * sizeof(GLfloat));
	f->glEnableVertexAttribArray(0);
	f->glEnableVertexAttribArray(1);
	f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
		nullptr);
	f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
		reinterpret_cast<void*>(3 * sizeof(GLfloat)));
	m_vbo->release();

	f->glEnable(GL_DEPTH_TEST);
	f->glEnable(GL_CULL_FACE);
}

void GLWindowSplat::resizeGL(int w, int h)
{
	m_proj.setToIdentity();
	m_proj.perspective(45.0f, GLfloat(w) / h, 0.01f, 100.0f);
	m_uniformsDirty = true;
}

void GLWindowSplat::paintGL()
{
	// Now use QOpenGLExtraFunctions instead of QOpenGLFunctions as we want to
	// do more than what GL(ES) 2.0 offers.
	QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();

	f->glClearColor(0, 0, 0, 1);
	f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_program->bind();
	//m_texture->bind();

	if (m_uniformsDirty) {
		m_uniformsDirty = false;
		QMatrix4x4 camera;
		camera.lookAt(m_eye, m_eye + m_target, QVector3D(0, 1, 0));
		m_program->setUniformValue(m_projMatrixLoc, m_proj);
		m_program->setUniformValue(m_camMatrixLoc, camera);
		QMatrix4x4 wm = m_world;
		wm.rotate(m_r, 1, 1, 0);
		m_program->setUniformValue(m_worldMatrixLoc, wm);
		QMatrix4x4 mm;
		mm.setToIdentity();
		mm.rotate(-m_r2, 1, 0, 0);
		m_program->setUniformValue(m_myMatrixLoc, mm);
		m_program->setUniformValue(m_lightPosLoc, QVector3D(0, 0, 70));
	}

	// Now call a function introduced in OpenGL 3.1 / OpenGL ES 3.0. We
	// requested a 3.3 or ES 3.0 context, so we know this will work.
	//  f->glDrawArraysInstanced(GL_TRIANGLES, 0, m_logo.vertexCount(), 32 * 36);
}
