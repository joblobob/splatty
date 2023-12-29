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

static std::vector<float> getViewMatrix(camera camera) {
	const rotation R = camera.rotation;
	const position t = camera.position;
	const std::vector<float> camToWorld = {
		R.yaw.x, R.yaw.y, R.yaw.z, 0 ,
		R.pitch.x, R.pitch.y, R.pitch.z, 0,
		R.roll.x, R.roll.y, R.roll.z, 0,

		-t.x * R.yaw.x - t.y * R.pitch.x - t.z * R.roll.x,
		-t.x * R.yaw.y - t.y * R.pitch.y - t.z * R.roll.y,
		-t.x * R.yaw.z - t.y * R.pitch.z - t.z * R.roll.z,
		1
	};

	return camToWorld;
}

static std::vector<float> multiply4(std::vector<float> a, std::vector<float> b) {
	return{
		b[0] * a[0] + b[1] * a[4] + b[2] * a[8] + b[3] * a[12],
			b[0] * a[1] + b[1] * a[5] + b[2] * a[9] + b[3] * a[13],
			b[0] * a[2] + b[1] * a[6] + b[2] * a[10] + b[3] * a[14],
			b[0] * a[3] + b[1] * a[7] + b[2] * a[11] + b[3] * a[15],
			b[4] * a[0] + b[5] * a[4] + b[6] * a[8] + b[7] * a[12],
			b[4] * a[1] + b[5] * a[5] + b[6] * a[9] + b[7] * a[13],
			b[4] * a[2] + b[5] * a[6] + b[6] * a[10] + b[7] * a[14],
			b[4] * a[3] + b[5] * a[7] + b[6] * a[11] + b[7] * a[15],
			b[8] * a[0] + b[9] * a[4] + b[10] * a[8] + b[11] * a[12],
			b[8] * a[1] + b[9] * a[5] + b[10] * a[9] + b[11] * a[13],
			b[8] * a[2] + b[9] * a[6] + b[10] * a[10] + b[11] * a[14],
			b[8] * a[3] + b[9] * a[7] + b[10] * a[11] + b[11] * a[15],
			b[12] * a[0] + b[13] * a[4] + b[14] * a[8] + b[15] * a[12],
			b[12] * a[1] + b[13] * a[5] + b[14] * a[9] + b[15] * a[13],
			b[12] * a[2] + b[13] * a[6] + b[14] * a[10] + b[15] * a[14],
			b[12] * a[3] + b[13] * a[7] + b[14] * a[11] + b[15] * a[15],
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
