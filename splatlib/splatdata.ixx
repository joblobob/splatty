/*
* Yes this is a C++ module! I love it!
* 
* It's intended to be a single file for the whole implementation for the moment, step by step!
*/

module;

#include <ranges>
#include <vector>

#include <battery/embed.hpp>

export module splat.data;

constexpr unsigned int to_uints(float v)
{
	return std::bit_cast<unsigned int>(v);
}

//static inline const auto plushData = b::embed<"res/plush.splat">();

export struct SplatData {
	SplatData(int splatCount, const std::vector<unsigned char> splatBuffer) : m_ucharBuffer(splatBuffer)
	{
		m_floatBuffer.resize(m_ucharBuffer.size() / 4);
		//copy binary to float  with our friend memcpy!
		std::memcpy(m_floatBuffer.data(), m_ucharBuffer.data(), m_ucharBuffer.size());

		m_uintBuffer = m_floatBuffer | std::views::transform(to_uints) | std::ranges::to<std::vector<unsigned int> >();
	}

	int m_vertexCount;
	std::vector<float> m_floatBuffer;
	std::vector<unsigned char> m_ucharBuffer;
	std::vector<unsigned int> m_uintBuffer;


	// 6*4 + 4 + 4 = 8*4
	// XYZ - Position (Float32) 1,2,3
	// XYZ - Scale (Float32)	4,5,6
	// RGBA - colors (uint8)	7,8,9,10
	// IJKL - quaternion/rot (uint8) 11,12,13,14
};
