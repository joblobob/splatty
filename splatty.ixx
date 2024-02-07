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

#include <QFile>
#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>

export module splatty;

// when you import splatty, you also get the shaders, because they go together, but could be loaded independently
export import shaders;

import splatmath;


export struct worker {
	worker() : m_texture(QOpenGLTexture::Target::Target2D)
	{
		// Construct a data object by reading from file
		constexpr int rowLength = 3 * 4 + 3 * 4 + 4 + 4;


		QFile splatFile("plush.splat");
		splatFile.open(QIODevice::ReadOnly);
		QByteArray splatData = splatFile.readAll();
		splatFile.close();
		for (const unsigned char data : splatData) {
			u_buffer.push_back(data);
		}

		for (int i = 0; i < splatData.size(); i += 4) {
			float f;
			uchar b[] = { splatData[i + 0], splatData[i + 1], splatData[i + 2], splatData[i + 3] };
			memcpy(&f, &b, sizeof(f));
			buffer.push_back(f);
		}
		vertexCount = (u_buffer.size() / rowLength);
	}
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

		setTextureData();
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

		setDepthIndex();
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


	void setView(const std::vector<float>& newviewProj)
	{
		viewProj = newviewProj;
		throttledSort();

		QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();

		// fps calculations (from paintGL)
		if (vertexCount > 0) {
			f->glUniformMatrix4fv(m_viewLoc, 1, false, viewMatrix.data());
			f->glClear(GL_COLOR_BUFFER_BIT);
			QOpenGLContext::currentContext()->extraFunctions()->glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, vertexCount);
		} else {
			f->glClear(GL_COLOR_BUFFER_BIT);
		}
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

	void setTextureData()
	{
		QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();
		f->glBindTexture(GL_TEXTURE_2D, m_texture.textureId());
		f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32UI, texwidth, texheight, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, texdata.data());
		f->glActiveTexture(GL_TEXTURE0);
		f->glBindTexture(GL_TEXTURE_2D, m_texture.textureId());
	}

	void setDepthIndex()
	{
		QOpenGLExtraFunctions* f = QOpenGLContext::currentContext()->extraFunctions();

		f->glBindBuffer(GL_ARRAY_BUFFER, m_indexBuffer.bufferId());
		f->glBufferData(GL_ARRAY_BUFFER, depthIndex.size() * 4, depthIndex.data(), GL_DYNAMIC_DRAW);
	}
};
