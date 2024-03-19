/*
* Yes this is a C++ module! I love it!
* 
* It's intended to be a single file for the whole implementation for the moment, step by step!
*/

module;


#include <cmath>
#include <limits>
#include <mdspan>
#include <print>
#include <ranges>
#include <vector>

#include <filesystem>
#include <fstream>
#include <ranges>

export module splatty;

import splat.data;
import splat.opengl;
import splat.math;


export struct splatdata {
	std::unique_ptr<SplatData> m_data;

	splatdata(const std::filesystem::path& path)
	{
		fileRead(path);

		// set vertex count in our openGL helper
		gl.vertexCount = vertexCount;
		depthIndex.resize(vertexCount + 1);
	}

	void fileRead(const std::filesystem::path& path)
	{
		//runtime to put compilee time^
		// file size
		auto length = std::filesystem::file_size(path);

		std::vector<unsigned char> u_buffer;
		u_buffer.resize(length);

		// read file data
		std::basic_ifstream<unsigned char, std::char_traits<unsigned char> > inputFile(path, std::ios_base::binary);
		inputFile.read(u_buffer.data(), length);
		inputFile.close();


		constexpr int rowLength = 3 * 4 + 3 * 4 + 4 + 4;
		vertexCount             = (u_buffer.size() / rowLength);

		m_data = std::make_unique<SplatData>(vertexCount, u_buffer);
	}

	int vertexCount = 0;
	std::vector<float> viewProj;
	// 6*4 + 4 + 4 = 8*4
	// XYZ - Position (Float32)
	// XYZ - Scale (Float32)
	// RGBA - colors (uint8)
	// IJKL - quaternion/rot (uint8)

	std::vector<float> lastProj;
	int lastVertexCount = 0;

	std::vector<unsigned int> texdata;
	int texwidth;
	int texheight;
	std::vector<unsigned int> depthIndex;

	glsplat gl;

	unsigned int rendu = 1;

	void generateTexture()
	{
		texwidth  = 2048;                                                  // Set to your desired width
		texheight = std::ceil((float)(2 * vertexCount) / (float)texwidth); // Set to your desired height
		texdata.resize(texwidth * texheight * 4 + 1);                      // 4 components per pixel (RGBA)

		// Here we convert from a .splat file buffer into a texture
		// With a little bit more foresight perhaps this texture file
		// should have been the native format as it'd be very easy to
		// load it into webgl.

		if (rendu >= m_data->m_floatBuffer.size())
			rendu = m_data->m_floatBuffer.size();
		else
			rendu += 4096;

		std::array<float, 4> rot;
		std::array<float, 9> M;
		std::array<float, 6> sigma;

		for (unsigned int i : std::views::iota(0u, rendu) | std::views::stride(8)) {
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



		gl.setTextureData(texdata, texwidth, texheight);
	}

	void runSort(const std::vector<float>& viewProj)
	{
		//if (lastVertexCount == vertexCount) {
		//	float dot = lastProj[2] * viewProj[2] + lastProj[6] * viewProj[6] + lastProj[10] * viewProj[10];
		//	if (std::abs(dot - 1) < 0.01) {
		//		return;
		//	}
		//} else {
		generateTexture();
		lastVertexCount = vertexCount;
		//}

		//console.time("sort");
		int maxDepth = INT_MIN;
		int minDepth = INT_MAX;
		std::vector<unsigned int> sizeList(vertexCount);
		for (int i = 0; i < vertexCount; i++) {
			int depth = (viewProj[2] * m_data->m_floatBuffer[8 * i + 0] + viewProj[6] * m_data->m_floatBuffer[8 * i + 1] +
			                viewProj[10] * m_data->m_floatBuffer[8 * i + 2]) *
			            4096;
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

		//console.timeEnd("sort");

		lastProj = viewProj;

		gl.setDepthIndex(depthIndex);
	}

	void setView(const std::vector<float>& newviewProj)
	{
		viewProj = newviewProj;
		runSort(viewProj);

		gl.viewChanged();
	}


	void initializeGL() { gl.initializeGL(); }

	void resizeGL(int w, int h) { gl.resizeGL(w, h); }
};
