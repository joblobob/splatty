/*
* Yes this is a C++ module! I love it!
* 
* It's intended to be a single file for the whole implementation for the moment, step by step!
*/

module;


#include <cmath>
#include <limits>
#include <print>
#include <ranges>
#include <vector>

#include <filesystem>
#include <fstream>


export module splatty;

import splat.opengl;
import splat.math;


export struct splatdata {
	splatdata(const std::filesystem::path& path)
	{
		fileRead(path);

		// set vertex count in our openGL helper
		gl.vertexCount = vertexCount;
	}

	void fileRead(const std::filesystem::path& path)
	{
		// file size
		auto length = std::filesystem::file_size(path);

		u_buffer.resize(length);
		buffer.resize(length / 4);

		// read file data
		std::ifstream inputFile(path, std::ios_base::binary);
		inputFile.read(reinterpret_cast<char*>(u_buffer.data()), length);
		inputFile.close();

		//copy binary to float  with our friend memcpy!
		memcpy(buffer.data(), u_buffer.data(), length);

		constexpr int rowLength = 3 * 4 + 3 * 4 + 4 + 4;
		vertexCount             = (u_buffer.size() / rowLength);
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

	std::vector<float> lastProj;
	int lastVertexCount = 0;

	std::vector<unsigned int> texdata;
	int texwidth;
	int texheight;
	std::vector<unsigned int> depthIndex;

	glsplat gl;

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
			// x, y, z from float to binary
			memcpy(&texdata[8 * i + 0], &buffer[8 * i + 0], 12);

			// r, g, b, a
			memcpy(&texdata[(8 * i + 7) + 0], &u_buffer[32 * i + 24 + 0], 4);



			std::vector<float> rot = { (float)(u_buffer[32 * i + 28 + 0] - 128.0f) / 128.0f,
				(float)(u_buffer[32 * i + 28 + 1] - 128.0f) / 128.0f,
				(float)(u_buffer[32 * i + 28 + 2] - 128.0f) / 128.0f,
				(float)(u_buffer[32 * i + 28 + 3] - 128.0f) / 128.0f };

			// quaternions
			std::vector<float> scale = { buffer[8 * i + 3 + 0], buffer[8 * i + 3 + 1], buffer[8 * i + 3 + 2] };

			// Compute the matrix product of S and R (M = S * R)
			std::vector<float> M = { scale[0] * (1.0f - 2.0f * (rot[2] * rot[2] + rot[3] * rot[3])),
				scale[1] * (2.0f * (rot[1] * rot[2] + rot[0] * rot[3])),
				scale[2] * (2.0f * (rot[1] * rot[3] - rot[0] * rot[2])),

				scale[0] * (2.0f * (rot[1] * rot[2] - rot[0] * rot[3])),
				scale[1] * (1.0f - 2.0f * (rot[1] * rot[1] + rot[3] * rot[3])),
				scale[2] * (2.0f * (rot[2] * rot[3] + rot[0] * rot[1])),

				scale[0] * (2.0f * (rot[1] * rot[3] + rot[0] * rot[2])),
				scale[1] * (2.0f * (rot[2] * rot[3] - rot[0] * rot[1])),
				scale[2] * (1.0f - 2.0f * (rot[1] * rot[1] + rot[2] * rot[2])) };


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

		gl.setTextureData(texdata, texwidth, texheight);
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
