/*
* Yes this is a C++ module! I love it!
* 
* It's intended to be a single file for the whole implementation for the moment, step by step!
*/

module;

#include <ranges>
#include <vector>

export module splat.data;

import splat.math;

// 6*4 + 4 + 4 = 8*4
// XYZ - Position (Float32)
// XYZ - Scale (Float32)
// RGBA - colors (uint8)
// IJKL - quaternion/rot (uint8)

export struct SplatData {
	SplatData(const std::vector<unsigned char> splatBuffer) : m_ucharBuffer(splatBuffer), m_floatBuffer(m_ucharBuffer.size() / 4)
	{
		//copy binary to float with our friend memcpy!
		std::memcpy(m_floatBuffer.data(), m_ucharBuffer.data(), m_ucharBuffer.size());

		//convert float to uint using ranges!
		m_uintBuffer = m_floatBuffer | std::views::transform(to_uints) | std::ranges::to<std::vector<unsigned int> >();
	}

	std::vector<unsigned char> m_ucharBuffer;
	std::vector<float> m_floatBuffer;
	std::vector<unsigned int> m_uintBuffer;
};
