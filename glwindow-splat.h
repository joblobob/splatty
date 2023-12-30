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

static
std::vector<float> invert4(std::vector<float> a) {
	float b00 = a[0] * a[5] - a[1] * a[4];
	float b01 = a[0] * a[6] - a[2] * a[4];
	float b02 = a[0] * a[7] - a[3] * a[4];
	float b03 = a[1] * a[6] - a[2] * a[5];
	float b04 = a[1] * a[7] - a[3] * a[5];
	float b05 = a[2] * a[7] - a[3] * a[6];
	float b06 = a[8] * a[13] - a[9] * a[12];
	float b07 = a[8] * a[14] - a[10] * a[12];
	float b08 = a[8] * a[15] - a[11] * a[12];
	float b09 = a[9] * a[14] - a[10] * a[13];
	float b10 = a[9] * a[15] - a[11] * a[13];
	float b11 = a[10] * a[15] - a[11] * a[14];
	float det =
		b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
	if (det < 0.0000001) return {};

	return{
			(a[5] * b11 - a[6] * b10 + a[7] * b09) / det,
			(a[2] * b10 - a[1] * b11 - a[3] * b09) / det,
			(a[13] * b05 - a[14] * b04 + a[15] * b03) / det,
			(a[10] * b04 - a[9] * b05 - a[11] * b03) / det,
			(a[6] * b08 - a[4] * b11 - a[7] * b07) / det,
			(a[0] * b11 - a[2] * b08 + a[3] * b07) / det,
			(a[14] * b02 - a[12] * b05 - a[15] * b01) / det,
			(a[8] * b05 - a[10] * b02 + a[11] * b01) / det,
			(a[4] * b10 - a[5] * b08 + a[7] * b06) / det,
			(a[1] * b08 - a[0] * b10 - a[3] * b06) / det,
			(a[12] * b04 - a[13] * b02 + a[15] * b00) / det,
			(a[9] * b02 - a[8] * b04 - a[11] * b00) / det,
			(a[5] * b07 - a[4] * b09 - a[6] * b06) / det,
			(a[0] * b09 - a[1] * b07 + a[2] * b06) / det,
			(a[13] * b01 - a[12] * b03 - a[14] * b00) / det,
			(a[8] * b03 - a[9] * b01 + a[10] * b00) / det
	};
}

static
std::vector<float> rotate4(std::vector<float> a, float rad, float x, float y, float z) {
	float len = std::hypot(x, y, z);
	x /= len;
	y /= len;
	z /= len;
	float s = std::sin(rad);
	float c = std::cos(rad);
	float t = 1 - c;
	float b00 = x * x * t + c;
	float b01 = y * x * t + z * s;
	float b02 = z * x * t - y * s;
	float b10 = x * y * t - z * s;
	float b11 = y * y * t + c;
	float b12 = z * y * t + x * s;
	float b20 = x * z * t + y * s;
	float b21 = y * z * t - x * s;
	float b22 = z * z * t + c;
	return {
			a[0] * b00 + a[4] * b01 + a[8] * b02,
			a[1] * b00 + a[5] * b01 + a[9] * b02,
			a[2] * b00 + a[6] * b01 + a[10] * b02,
			a[3] * b00 + a[7] * b01 + a[11] * b02,
			a[0] * b10 + a[4] * b11 + a[8] * b12,
			a[1] * b10 + a[5] * b11 + a[9] * b12,
			a[2] * b10 + a[6] * b11 + a[10] * b12,
			a[3] * b10 + a[7] * b11 + a[11] * b12,
			a[0] * b20 + a[4] * b21 + a[8] * b22,
			a[1] * b20 + a[5] * b21 + a[9] * b22,
			a[2] * b20 + a[6] * b21 + a[10] * b22,
			a[3] * b20 + a[7] * b21 + a[11] * b22,
			a[12], a[13], a[14], a[15]
			//...a.slice(12, 16)
	};
}

static std::vector<float> translate4(std::vector<float> a, float x, float y, float z) {
	return{
		//...a.slice(0, 12),
			a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9], a[10], a[11],
			a[0] * x + a[4] * y + a[8] * z + a[12],
			a[1] * x + a[5] * y + a[9] * z + a[13],
			a[2] * x + a[6] * y + a[10] * z + a[14],
			a[3] * x + a[7] * y + a[11] * z + a[15]
	};
}

struct worker {
	float buffer;
	int vertexCount = 0;
	float viewProj;
	// 6*4 + 4 + 4 = 8*4
	// XYZ - Position (Float32)
	// XYZ - Scale (Float32)
	// RGBA - colors (uint8)
	// IJKL - quaternion/rot (uint8)
	const int rowLength = 3 * 4 + 3 * 4 + 4 + 4;
	std::vector<float> lastProj;
	std::vector<int> depthIndex;
	int lastVertexCount = 0;


private:

	int floatToHalf(float val) {
		int f = std::floor(val);

		int sign = (f >> 31) & 0x0001;
		int exp = (f >> 23) & 0x00ff;
		int frac = f & 0x007fffff;

		int newExp;
		if (exp == 0) {
			newExp = 0;
		}
		else if (exp < 113) {
			newExp = 0;
			frac |= 0x00800000;
			frac = frac >> (113 - exp);
			if (frac & 0x01000000) {
				newExp = 1;
				frac = 0;
			}
		}
		else if (exp < 142) {
			newExp = exp - 112;
		}
		else {
			newExp = 31;
			frac = 0;
		}

		return (sign << 15) | (newExp << 10) | (frac >> 13);
	}

	int packHalf2x16(float x, float y) {
		return unsigned int(floatToHalf(x) | (floatToHalf(y) << 16)) >> 0;
	}


};

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
