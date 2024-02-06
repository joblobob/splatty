/*
* Yes this is a C++ module! I love it!
* 
* It's intended to be a single file for the whole implementation for the moment, step by step!
*/

module;


#include <cmath>
#include <limits>
#include <print>
#include <vector>

#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>

export module splatty;

// when you import splatty, you also get the shaders, because they go together, but could be loaded independently
export import shaders;

export std::vector<float> getProjectionMatrix(float fx, float fy, int width, int height)
{
	constexpr float znear = 0.2f;
	constexpr float zfar  = 200;

	return { (2.0f * fx) / width,
		0.f,
		0.f,
		0.f,
		0.f,
		-(2 * fy) / height,
		0.f,
		0.f,
		0.f,
		0.f,
		zfar / (zfar - znear),
		1.f,
		0.f,
		0.f,
		-(zfar * znear) / (zfar - znear),
		0.f };
}

export std::vector<float> multiply4(const std::vector<float>& a, const std::vector<float>& b)
{
	return {
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

export std::vector<float> invert4(const std::vector<float>& a)
{
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
	float det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
	if (det < 0.0000001)
		return std::vector<float>(16);

	return { (a[5] * b11 - a[6] * b10 + a[7] * b09) / det,
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
		(a[8] * b03 - a[9] * b01 + a[10] * b00) / det };
}

export std::vector<float> rotate4(const std::vector<float>& a, float rad, float x, float y, float z)
{
	float len = std::hypot(x, y, z);
	x /= len;
	y /= len;
	z /= len;
	float s   = std::sin(rad);
	float c   = std::cos(rad);
	float t   = 1 - c;
	float b00 = x * x * t + c;
	float b01 = y * x * t + z * s;
	float b02 = z * x * t - y * s;
	float b10 = x * y * t - z * s;
	float b11 = y * y * t + c;
	float b12 = z * y * t + x * s;
	float b20 = x * z * t + y * s;
	float b21 = y * z * t - x * s;
	float b22 = z * z * t + c;
	return { a[0] * b00 + a[4] * b01 + a[8] * b02,
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
		a[12],
		a[13],
		a[14],
		a[15] };
}

export std::vector<float> translate4(const std::vector<float>& a, float x, float y, float z)
{
	return { a[0],
		a[1],
		a[2],
		a[3],
		a[4],
		a[5],
		a[6],
		a[7],
		a[8],
		a[9],
		a[10],
		a[11],
		a[0] * x + a[4] * y + a[8] * z + a[12],
		a[1] * x + a[5] * y + a[9] * z + a[13],
		a[2] * x + a[6] * y + a[10] * z + a[14],
		a[3] * x + a[7] * y + a[11] * z + a[15] };
}



static const std::vector<float> defaultViewMatrix = { 0.47, 0.04, 0.88, 0, -0.11, 0.99, 0.02, 0, -0.88, -0.11, 0.47, 0, 0.07, 0.03, 6.55, 1 };

export std::vector<float> viewMatrix = defaultViewMatrix;

export constexpr int focalWidth  = 1500;
export constexpr int focalHeight = 1500;


export struct worker {
	worker() : m_texture(QOpenGLTexture::Target::Target2D) {}
	std::vector<float> buffer;
	std::vector<unsigned char> u_buffer;
	int vertexCount = 0;
	std::vector<float> viewProj;
	// 6*4 + 4 + 4 = 8*4
	// XYZ - Position (Float32)
	// XYZ - Scale (Float32)
	// RGBA - colors (uint8)
	// IJKL - quaternion/rot (uint8)
	const int rowLength = 3 * 4 + 3 * 4 + 4 + 4;
	std::vector<float> lastProj;
	int lastVertexCount = 0;
	std::vector<unsigned int> texdata;
	int texwidth;
	int texheight;
	std::vector<unsigned int> depthIndex;

	QOpenGLTexture m_texture;
	QOpenGLShaderProgram m_program;
	QOpenGLVertexArrayObject m_vao;
	bool gotTexture = false;

	int m_projMatrixLoc = 0;
	int m_viewPortLoc   = 0;
	int m_focalLoc      = 0;
	int m_viewLoc       = 0;
	QOpenGLBuffer m_indexBuffer;
	QOpenGLBuffer m_vertexBuffer;

	std::vector<float> m_projectionMatrix;

	int floatToHalf(float val)
	{
		unsigned int f;
		memcpy(&f, &val, 4);

		int sign = (f >> 31) & 0x0001;
		int exp  = (f >> 23) & 0x00ff;
		int frac = f & 0x007fffff;

		int newExp;
		if (exp == 0) {
			newExp = 0;
		} else if (exp < 113) {
			newExp = 0;
			frac |= 0x00800000;
			frac = frac >> (113 - exp);
			if (frac & 0x01000000) {
				newExp = 1;
				frac   = 0;
			}
		} else if (exp < 142) {
			newExp = exp - 112;
		} else {
			newExp = 31;
			frac   = 0;
		}

		return (sign << 15) | (newExp << 10) | (frac >> 13);
	}

	int packHalf2x16(float x, float y) { return unsigned int(floatToHalf(x) | (floatToHalf(y) << 16)) >> 0; }


	void generateTexture()
	{
		if (buffer.empty())
			return;

		texwidth  = 1024 * 2;                                              // Set to your desired width
		texheight = std::ceil((float)(2 * vertexCount) / (float)texwidth); // Set to your desired height
		texdata.resize(texwidth * texheight * 4 + 1);                      // 4 components per pixel (RGBA)

		// Here we convert from a .splat file buffer into a texture
		// With a little bit more foresight perhaps this texture file
		// should have been the native format as it'd be very easy to
		// load it into webgl.
		for (int i = 0; i < vertexCount; i++) {
			// x, y, z
			unsigned int valx, valy, valz;
			memcpy(&valx, &buffer[8 * i + 0], 4);
			memcpy(&valy, &buffer[8 * i + 1], 4);
			memcpy(&valz, &buffer[8 * i + 2], 4);
			texdata[8 * i + 0] = valx;
			texdata[8 * i + 1] = valy;
			texdata[8 * i + 2] = valz;

			// r, g, b, a
			unsigned int valrgba;
			memcpy(&valrgba, &u_buffer[32 * i + 24 + 0], 4);
			texdata[(8 * i + 7) + 0] = valrgba;

			// quaternions
			std::vector<float> scale = {
				buffer[8 * i + 3 + 0],
				buffer[8 * i + 3 + 1],
				buffer[8 * i + 3 + 2],
			};

			std::vector<float> rot = { (float)(u_buffer[32 * i + 28 + 0] - 128.0f) / 128.0f,
				(float)(u_buffer[32 * i + 28 + 1] - 128.0f) / 128.0f,
				(float)(u_buffer[32 * i + 28 + 2] - 128.0f) / 128.0f,
				(float)(u_buffer[32 * i + 28 + 3] - 128.0f) / 128.0f };

			// Compute the matrix product of S and R (M = S * R)
			std::vector<float> M = {
				1.0f - 2.0f * (rot[2] * rot[2] + rot[3] * rot[3]),
				2.0f * (rot[1] * rot[2] + rot[0] * rot[3]),
				2.0f * (rot[1] * rot[3] - rot[0] * rot[2]),

				2.0f * (rot[1] * rot[2] - rot[0] * rot[3]),
				1.0f - 2.0f * (rot[1] * rot[1] + rot[3] * rot[3]),
				2.0f * (rot[2] * rot[3] + rot[0] * rot[1]),

				2.0f * (rot[1] * rot[3] + rot[0] * rot[2]),
				2.0f * (rot[2] * rot[3] - rot[0] * rot[1]),
				1.0f - 2.0f * (rot[1] * rot[1] + rot[2] * rot[2]),
			};
			for (int i = 0; i < M.size(); i++) {
				M[i] = M[i] * scale[std::floor(i / 3)];
			}

			const std::vector<float> sigma = {
				M[0] * M[0] + M[3] * M[3] + M[6] * M[6],
				M[0] * M[1] + M[3] * M[4] + M[6] * M[7],
				M[0] * M[2] + M[3] * M[5] + M[6] * M[8],
				M[1] * M[1] + M[4] * M[4] + M[7] * M[7],
				M[1] * M[2] + M[4] * M[5] + M[7] * M[8],
				M[2] * M[2] + M[5] * M[5] + M[8] * M[8],
			};

			texdata[8 * i + 4] = packHalf2x16(4 * sigma[0], 4 * sigma[1]);
			texdata[8 * i + 5] = packHalf2x16(4 * sigma[2], 4 * sigma[3]);
			texdata[8 * i + 6] = packHalf2x16(4 * sigma[4], 4 * sigma[5]);
		}
	}

	void runSort(const std::vector<float>& viewProj)
	{
		if (buffer.empty())
			return;

		if (lastVertexCount == vertexCount) {
			float dot = lastProj[2] * viewProj[2] + lastProj[6] * viewProj[6] + lastProj[10] * viewProj[10];
			if (std::abs(dot - 1) < 0.01) {
				return;
			}
		} else {
			generateTexture();
			lastVertexCount = vertexCount;
		}

		//console.time("sort");
		int maxDepth = INT_MIN;
		int minDepth = INT_MAX;
		std::vector<int> sizeList(vertexCount);
		for (int i = 0; i < vertexCount; i++) {
			int depth   = (viewProj[2] * buffer[8 * i + 0] + viewProj[6] * buffer[8 * i + 1] + viewProj[10] * buffer[8 * i + 2]) * 4096;
			sizeList[i] = depth;
			if (depth > maxDepth)
				maxDepth = depth;
			if (depth < minDepth)
				minDepth = depth;
		}

		// This is a 16 bit single-pass counting sort
		float depthInv = (256 * 256) / (maxDepth - minDepth);
		std::vector<int> counts0(256 * 256);
		for (int i = 0; i < vertexCount; i++) {
			sizeList[i] = std::floor((sizeList[i] - minDepth) * depthInv);
			counts0[sizeList[i]]++;
		}
		std::vector<int> starts0(256 * 256);
		for (int i = 1; i < 256 * 256; i++)
			starts0[i] = starts0[i - 1] + counts0[i - 1];
		depthIndex.resize(vertexCount + 1);
		for (int i = 0; i < vertexCount; i++)
			depthIndex[starts0[sizeList[i]]++] = i;

		//console.timeEnd("sort");

		lastProj = viewProj;
	}


	bool sortRunning = false;

	void throttledSort()
	{
		if (!sortRunning) {
			sortRunning                       = true;
			const std::vector<float> lastView = viewProj;
			runSort(lastView);
			sortRunning = false;
			// when the view changes, we should re-do the sort
		}
	};

	void setBuffer(const std::vector<float>& newbuffer, const std::vector<unsigned char>& orignialSplatData, int newvertexCount)
	{
		buffer      = newbuffer;
		u_buffer    = orignialSplatData;
		vertexCount = newvertexCount;
	}

	void setVertexCount(int newvertexCount) { vertexCount = newvertexCount; }

	void setView(const std::vector<float>& newviewProj)
	{
		viewProj = newviewProj;
		throttledSort();
	}


	void initializeGL()
	{
		//QOpenGLDebugLogger* logger = new QOpenGLDebugLogger(this);
		//connect(logger, &QOpenGLDebugLogger::messageLogged, [&](const QOpenGLDebugMessage& debugMessage) { qCritical() << debugMessage; });
		//logger->initialize(); // initializes in the current context, i.e. ctx
		//logger->startLogging(QOpenGLDebugLogger::SynchronousLogging);

		QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();


		m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, ShaderSource::vertex);
		m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, ShaderSource::fragment);
		m_program.link();

		m_program.bind();

		// Create a VAO. Not strictly required for ES 3, but it is for plain OpenGL.

		if (m_vao.create())
			m_vao.bind();

		f->glDisable(GL_DEPTH_TEST); // Disable depth testing

		f->glEnable(GL_BLEND);
		f->glBlendFuncSeparate(GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_ONE_MINUS_DST_ALPHA, GL_ONE);

		m_projMatrixLoc = m_program.uniformLocation("projection");
		m_viewPortLoc   = m_program.uniformLocation("viewport");
		m_focalLoc      = m_program.uniformLocation("focal");
		m_viewLoc       = m_program.uniformLocation("view");

		// positions
		const std::vector<float> triangleVertices = { -2, -2, 2, -2, 2, 2, -2, 2 };

		m_vertexBuffer.create();

		f->glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer.bufferId());
		f->glBufferData(GL_ARRAY_BUFFER, 8 * 4, triangleVertices.data(), GL_STATIC_DRAW);
		const int a_position = m_program.attributeLocation("position");
		f->glEnableVertexAttribArray(a_position);
		f->glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer.bufferId());
		f->glVertexAttribPointer(a_position, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

		//m_texture = new QOpenGLTexture(QOpenGLTexture::Target::Target2D);
		f->glBindTexture(GL_TEXTURE_2D, m_texture.textureId());

		auto u_textureLocation = m_program.uniformLocation("u_texture");
		f->glUniform1i(u_textureLocation, 0);

		m_indexBuffer.create();
		const int a_index = m_program.attributeLocation("index");
		f->glEnableVertexAttribArray(a_index);
		f->glBindBuffer(GL_ARRAY_BUFFER, m_indexBuffer.bufferId());
		f->glVertexAttribIPointer(a_index, 1, GL_INT, false, 0);
		f->glVertexAttribDivisor(a_index, 1);
	}

	void resizeGL(int w, int h)
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
};
