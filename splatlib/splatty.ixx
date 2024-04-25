/*
* Yes this is a C++ module! I love it!
* 
* It's intended to be a single file for the whole implementation for the moment, step by step!
*/

module;


#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <mdspan>
#include <print>
#include <ranges>
#include <vector>

#include <filesystem>

#include <QDebug>

export module splatty;

import splat.coroutine;
import splat.data;
import splat.opengl;
import splat.math;
import splat.reader;

// a splat is:
// 6*4 + 4 + 4 = 8*4
// XYZ - Position (Float32)
// XYZ - Scale (Float32)
// RGBA - colors (uint8)
// IJKL - quaternion/rot (uint8)
constexpr int rowLength = 3 * 4 + 3 * 4 + 4 + 4;

export struct Splatty {
	std::unique_ptr<SplatData> m_data;
	std::unique_ptr<glsplat> m_gl;

	int vertexCount = 0;

	float lastProjX, lastProjY, lastProjZ;


	static constexpr int texwidth = 2048;
	int texheight;
	std::vector<unsigned int> texdata;
	std::vector<unsigned int> depthIndex;
	Chat chat = Fun(); // #E Creation of the coroutine
	Splatty(const std::filesystem::path& path)
	{
		std::vector<unsigned char> data = Splat::readFromFile(path);

		// resize data folowwing the vertexCount
		vertexCount = (data.size() / rowLength);

		depthIndex.resize(vertexCount + 1);

		texheight = std::ceil((float)(2 * vertexCount) / (float)texwidth); // Set to your desired height
		texdata.resize(texwidth * texheight * 4 + 1);                      // 4 components per pixel (RGBA)

		m_data = std::make_unique<SplatData>(data);
		m_gl   = std::make_unique<glsplat>(vertexCount);
	}

	void generateTexture()
	{
		// Here we convert from a .splat file buffer into a texture
		// With a little bit more foresight perhaps this texture file
		// should have been the native format as it'd be very easy to
		// load it into webgl.

		std::array<float, 4> rot;
		std::array<float, 9> M;
		std::array<float, 6> sigma;

		auto spanny = std::mdspan(m_data->m_ucharBuffer.data(), 32, m_data->m_floatBuffer.size());

		for (unsigned int i : std::views::iota(0u, m_data->m_floatBuffer.size()) | std::views::stride(8)) {
			// x, y, z from float to binary
			//texdata[i]     = uintBuffer[i];
			//texdata[i + 1] = uintBuffer[i + 1];
			//texdata[i + 2] = uintBuffer[i + 2];
			std::ranges::copy_n(m_data->m_uintBuffer.begin() + i, 3, texdata.begin() + i);

			// r, g, b, a
			std::memcpy(&texdata[i + 7], &m_data->m_ucharBuffer[4 * i + 24], 4);

			// quaternions

			rot[0] = std::bit_cast<float>(m_data->m_ucharBuffer[4 * i + 28 + 0] - 128.0f) / 128.0f;
			rot[1] = std::bit_cast<float>(m_data->m_ucharBuffer[4 * i + 28 + 1] - 128.0f) / 128.0f;
			rot[2] = std::bit_cast<float>(m_data->m_ucharBuffer[4 * i + 28 + 2] - 128.0f) / 128.0f;
			rot[3] = std::bit_cast<float>(m_data->m_ucharBuffer[4 * i + 28 + 3] - 128.0f) / 128.0f;

			// Compute the matrix product of S and R (M = S * R)
			M[0] = m_data->m_floatBuffer[i + 3 + 0] * (1.0f - 2.0f * (rot[2] * rot[2] + rot[3] * rot[3]));
			M[1] = m_data->m_floatBuffer[i + 3 + 1] * (2.0f * (rot[1] * rot[2] + rot[0] * rot[3]));
			M[2] = m_data->m_floatBuffer[i + 3 + 2] * (2.0f * (rot[1] * rot[3] - rot[0] * rot[2]));

			M[3] = m_data->m_floatBuffer[i + 3 + 0] * (2.0f * (rot[1] * rot[2] - rot[0] * rot[3]));
			M[4] = m_data->m_floatBuffer[i + 3 + 1] * (1.0f - 2.0f * (rot[1] * rot[1] + rot[3] * rot[3]));
			M[5] = m_data->m_floatBuffer[i + 3 + 2] * (2.0f * (rot[2] * rot[3] + rot[0] * rot[1]));

			M[6] = m_data->m_floatBuffer[i + 3 + 0] * (2.0f * (rot[1] * rot[3] + rot[0] * rot[2]));
			M[7] = m_data->m_floatBuffer[i + 3 + 1] * (2.0f * (rot[2] * rot[3] - rot[0] * rot[1]));
			M[8] = m_data->m_floatBuffer[i + 3 + 2] * (1.0f - 2.0f * (rot[1] * rot[1] + rot[2] * rot[2]));

			sigma[0] = M[0] * M[0] + M[3] * M[3] + M[6] * M[6];
			sigma[1] = M[0] * M[1] + M[3] * M[4] + M[6] * M[7];
			sigma[2] = M[0] * M[2] + M[3] * M[5] + M[6] * M[8];
			sigma[3] = M[1] * M[1] + M[4] * M[4] + M[7] * M[7];
			sigma[4] = M[1] * M[2] + M[4] * M[5] + M[7] * M[8];
			sigma[5] = M[2] * M[2] + M[5] * M[5] + M[8] * M[8];


			texdata[i + 4] = packHalf2x16(4 * sigma[0], 4 * sigma[1]);
			texdata[i + 5] = packHalf2x16(4 * sigma[2], 4 * sigma[3]);
			texdata[i + 6] = packHalf2x16(4 * sigma[4], 4 * sigma[5]);
		}



		m_gl->setTextureData(texdata, texwidth, texheight);
	}

	void sortByDepth(float x, float y, float z)
	{
		int maxDepth = std::numeric_limits<int>::min();
		int minDepth = std::numeric_limits<int>::max();
		std::vector<unsigned int> sizeList(vertexCount);
		for (int i = 0; i < vertexCount; i++) {
			int depth   = (x * m_data->m_floatBuffer[8 * i + 0] + y * m_data->m_floatBuffer[8 * i + 1] + z * m_data->m_floatBuffer[8 * i + 2]) * 4096;
			sizeList[i] = depth;

			if (depth > maxDepth)
				maxDepth = depth;
			if (depth < minDepth)
				minDepth = depth;
		}

		constexpr int sizeSort = 256 * 256;
		// This is a 16 bit single-pass counting sort
		float depthInv = (sizeSort) / (maxDepth - minDepth);
		std::vector<int> counts0(sizeSort);

		//normalize depth

		sizeList | std::views::transform([&](unsigned int& val) {
			val = std::floor((val - minDepth) * depthInv);
			counts0[val]++; //count occurrences
			return val;
		}) | std::ranges::to<std::vector>();

		std::vector<int> starts0(sizeSort);
		for (int i = 1; i < sizeSort; i++)
			starts0[i] = starts0[i - 1] + counts0[i - 1];

		for (int i = 0; i < vertexCount; i++) {
			depthIndex[starts0[sizeList[i]]++] = i;
		}


		m_gl->setDepthIndex(depthIndex);
	}

	void setView(float x, float y, float z)
	{
		chat.answer("Where are you?\n"); // #G Send data into the coroutine

		float dot = lastProjX * x + lastProjY * y + lastProjZ * z;
		if (std::abs(dot - 1) > 0.01) {
			std::cout << chat.listen(); // #H Wait for more data from the coroutine "here"
			generateTexture();

			sortByDepth(x, y, z);

			lastProjX = x;
			lastProjY = y;
			lastProjZ = z;
		}


		m_gl->viewChanged();
	}


	void initializeGL() { m_gl->initializeGL(); }

	void resizeGL(int w, int h) { m_gl->resizeGL(w, h); }
};
