// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef GLWIDGETSPLAT_H
#define GLWIDGETSPLAT_H

#include <QMatrix4x4>
#include <QOpenGLWindow>
#include <QVector3D>
#include <vector>

QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)
QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)
QT_FORWARD_DECLARE_CLASS(QOpenGLBuffer)
QT_FORWARD_DECLARE_CLASS(QOpenGLVertexArrayObject)

// let's get crazy and copy everything here!

struct position {
	float x, y, z;
};

struct rotation {
	position yaw, pitch, roll;
};

struct camera {
	int id, witdh, height;
	position position;
	rotation rotation;
	float fy, fx;
};

static std::vector<float> getProjectionMatrix(float fx, float fy, int width, int height) {
	constexpr float znear = 0.2f;
	constexpr float zfar = 200;

	return { (2.0f * fx) / width, 0.f, 0.f,  0.f,
			  0.f, -(2 * fy) / height, 0.f,  0.f,
			  0.f, 0.f, zfar / (zfar - znear), 1.f,
			  0.f, 0.f, -(zfar * znear) / (zfar - znear), 0.f
	};
}







class GLWindowSplat : public QOpenGLWindow {
	Q_OBJECT
		Q_PROPERTY(float z READ z WRITE setZ)
		Q_PROPERTY(float r READ r WRITE setR)
		Q_PROPERTY(float r2 READ r2 WRITE setR2)

public:
	GLWindowSplat();
	~GLWindowSplat();

	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

	float z() const { return m_eye.z(); }
	void setZ(float v);

	float r() const { return m_r; }
	void setR(float v);
	float r2() const { return m_r2; }
	void setR2(float v);
private slots:
	void startSecondStage();

private:
	QOpenGLTexture* m_texture = nullptr;
	QOpenGLShaderProgram* m_program = nullptr;
	QOpenGLBuffer* m_vbo = nullptr;
	QOpenGLVertexArrayObject* m_vao = nullptr;

	int m_projMatrixLoc = 0;
	int m_camMatrixLoc = 0;
	int m_worldMatrixLoc = 0;
	int m_myMatrixLoc = 0;
	int m_lightPosLoc = 0;
	QMatrix4x4 m_proj;
	QMatrix4x4 m_world;
	QVector3D m_eye;
	QVector3D m_target = { 0, 0, -1 };
	bool m_uniformsDirty = true;
	float m_r = 0;
	float m_r2 = 0;
};

#endif
