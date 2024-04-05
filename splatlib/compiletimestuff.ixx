/*
* Yes this is a C++ module! I love it!
* 
* It's intended to be a single file for the whole implementation for the moment, step by step!
*/

module;


#include <array>
#include <cmath>
#include <mdspan>
#include <print>

#include <bit>
#include <bitset>
export module splat.math;


export constexpr std::array<float, 16> getProjectionMatrix(float fx, float fy, int width, int height)
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

export constexpr void invertMatrix(std::mdspan<float, std::extents<std::size_t, 4, 4> > matrix)
{
	float b00 = matrix[std::array { 0, 0 }] * matrix[std::array { 1, 1 }] - matrix[std::array { 0, 1 }] * matrix[std::array { 1, 0 }];
	float b01 = matrix[std::array { 0, 0 }] * matrix[std::array { 1, 2 }] - matrix[std::array { 0, 2 }] * matrix[std::array { 1, 0 }];
	float b02 = matrix[std::array { 0, 0 }] * matrix[std::array { 1, 3 }] - matrix[std::array { 0, 3 }] * matrix[std::array { 1, 0 }];
	float b03 = matrix[std::array { 0, 1 }] * matrix[std::array { 1, 2 }] - matrix[std::array { 0, 2 }] * matrix[std::array { 1, 1 }];

	float b04 = matrix[std::array { 0, 1 }] * matrix[std::array { 1, 3 }] - matrix[std::array { 0, 3 }] * matrix[std::array { 1, 1 }];
	float b05 = matrix[std::array { 0, 2 }] * matrix[std::array { 1, 3 }] - matrix[std::array { 0, 3 }] * matrix[std::array { 1, 2 }];
	float b06 = matrix[std::array { 2, 0 }] * matrix[std::array { 3, 1 }] - matrix[std::array { 2, 1 }] * matrix[std::array { 3, 0 }];
	float b07 = matrix[std::array { 2, 0 }] * matrix[std::array { 3, 2 }] - matrix[std::array { 2, 2 }] * matrix[std::array { 3, 0 }];

	float b08 = matrix[std::array { 2, 0 }] * matrix[std::array { 3, 3 }] - matrix[std::array { 2, 3 }] * matrix[std::array { 3, 0 }];
	float b09 = matrix[std::array { 2, 1 }] * matrix[std::array { 3, 2 }] - matrix[std::array { 2, 2 }] * matrix[std::array { 3, 1 }];
	float b10 = matrix[std::array { 2, 1 }] * matrix[std::array { 3, 3 }] - matrix[std::array { 2, 3 }] * matrix[std::array { 3, 1 }];
	float b11 = matrix[std::array { 2, 2 }] * matrix[std::array { 3, 3 }] - matrix[std::array { 2, 3 }] * matrix[std::array { 3, 2 }];
	float det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
	if (det < 0.0000001) {
		return;
	}

	std::array<float, 16> arr { (matrix[std::array { 1, 1 }] * b11 - matrix[std::array { 1, 2 }] * b10 + matrix[std::array { 1, 3 }] * b09) / det,
		(matrix[std::array { 0, 2 }] * b10 - matrix[std::array { 0, 1 }] * b11 - matrix[std::array { 0, 3 }] * b09) / det,
		(matrix[std::array { 3, 1 }] * b05 - matrix[std::array { 3, 2 }] * b04 + matrix[std::array { 3, 3 }] * b03) / det,
		(matrix[std::array { 2, 2 }] * b04 - matrix[std::array { 2, 1 }] * b05 - matrix[std::array { 2, 3 }] * b03) / det,

		(matrix[std::array { 1, 2 }] * b08 - matrix[std::array { 1, 0 }] * b11 - matrix[std::array { 1, 3 }] * b07) / det,
		(matrix[std::array { 0, 0 }] * b11 - matrix[std::array { 0, 2 }] * b08 + matrix[std::array { 0, 3 }] * b07) / det,
		(matrix[std::array { 3, 2 }] * b02 - matrix[std::array { 3, 0 }] * b05 - matrix[std::array { 3, 3 }] * b01) / det,
		(matrix[std::array { 2, 0 }] * b05 - matrix[std::array { 2, 2 }] * b02 + matrix[std::array { 2, 3 }] * b01) / det,

		(matrix[std::array { 1, 0 }] * b10 - matrix[std::array { 1, 1 }] * b08 + matrix[std::array { 1, 3 }] * b06) / det,
		(matrix[std::array { 0, 1 }] * b08 - matrix[std::array { 0, 0 }] * b10 - matrix[std::array { 0, 3 }] * b06) / det,
		(matrix[std::array { 3, 0 }] * b04 - matrix[std::array { 3, 1 }] * b02 + matrix[std::array { 3, 3 }] * b00) / det,
		(matrix[std::array { 2, 1 }] * b02 - matrix[std::array { 2, 0 }] * b04 - matrix[std::array { 2, 3 }] * b00) / det,

		(matrix[std::array { 1, 1 }] * b07 - matrix[std::array { 1, 0 }] * b09 - matrix[std::array { 1, 2 }] * b06) / det,
		(matrix[std::array { 0, 0 }] * b09 - matrix[std::array { 0, 1 }] * b07 + matrix[std::array { 0, 2 }] * b06) / det,
		(matrix[std::array { 3, 1 }] * b01 - matrix[std::array { 3, 0 }] * b03 - matrix[std::array { 3, 2 }] * b00) / det,
		(matrix[std::array { 2, 0 }] * b03 - matrix[std::array { 2, 1 }] * b01 + matrix[std::array { 2, 2 }] * b00) / det };

	std::memcpy(matrix.data_handle(), arr.data(), sizeof(float) * 16);
}

export void rotateMatrix(std::mdspan<float, std::extents<std::size_t, 4, 4> > matrix, float rad, float x, float y, float z)
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

	matrix[std::array { 0, 0 }] = matrix[std::array { 0, 0 }] * b00 + matrix[std::array { 1, 0 }] * b01 + matrix[std::array { 2, 0 }] * b02;
	matrix[std::array { 0, 1 }] = matrix[std::array { 0, 1 }] * b00 + matrix[std::array { 1, 1 }] * b01 + matrix[std::array { 2, 1 }] * b02;
	matrix[std::array { 0, 2 }] = matrix[std::array { 0, 2 }] * b00 + matrix[std::array { 1, 2 }] * b01 + matrix[std::array { 2, 2 }] * b02;
	matrix[std::array { 0, 3 }] = matrix[std::array { 0, 3 }] * b00 + matrix[std::array { 1, 3 }] * b01 + matrix[std::array { 2, 3 }] * b02;

	matrix[std::array { 1, 0 }] = matrix[std::array { 0, 0 }] * b10 + matrix[std::array { 1, 0 }] * b11 + matrix[std::array { 2, 0 }] * b12;
	matrix[std::array { 1, 1 }] = matrix[std::array { 0, 1 }] * b10 + matrix[std::array { 1, 1 }] * b11 + matrix[std::array { 2, 1 }] * b12;
	matrix[std::array { 1, 2 }] = matrix[std::array { 0, 2 }] * b10 + matrix[std::array { 1, 2 }] * b11 + matrix[std::array { 2, 2 }] * b12;
	matrix[std::array { 1, 3 }] = matrix[std::array { 0, 3 }] * b10 + matrix[std::array { 1, 3 }] * b11 + matrix[std::array { 2, 3 }] * b12;

	matrix[std::array { 2, 0 }] = matrix[std::array { 0, 0 }] * b20 + matrix[std::array { 1, 0 }] * b21 + matrix[std::array { 2, 0 }] * b22;
	matrix[std::array { 2, 1 }] = matrix[std::array { 0, 1 }] * b20 + matrix[std::array { 1, 1 }] * b21 + matrix[std::array { 2, 1 }] * b22;
	matrix[std::array { 2, 2 }] = matrix[std::array { 0, 2 }] * b20 + matrix[std::array { 1, 2 }] * b21 + matrix[std::array { 2, 2 }] * b22;
	matrix[std::array { 2, 3 }] = matrix[std::array { 0, 3 }] * b20 + matrix[std::array { 1, 3 }] * b21 + matrix[std::array { 2, 3 }] * b22;
}


export constexpr void translateMatrix(std::mdspan<float, std::extents<std::size_t, 4, 4> > matrix, float x, float y, float z)
{
	matrix[std::array { 3, 0 }] += matrix[std::array { 0, 0 }] * x + matrix[std::array { 1, 0 }] * y + matrix[std::array { 2, 0 }] * z;
	matrix[std::array { 3, 1 }] += matrix[std::array { 0, 1 }] * x + matrix[std::array { 1, 1 }] * y + matrix[std::array { 2, 1 }] * z;
	matrix[std::array { 3, 2 }] += matrix[std::array { 0, 2 }] * x + matrix[std::array { 1, 2 }] * y + matrix[std::array { 2, 2 }] * z;
	matrix[std::array { 3, 3 }] += matrix[std::array { 0, 3 }] * x + matrix[std::array { 1, 3 }] * y + matrix[std::array { 2, 3 }] * z;
}

export int floatToHalf(float val)
{
	unsigned int f;
	memcpy(&f, &val, 4);

	int sign = (f >> 31) & 0x0001;
	int exp  = (f >> 23) & 0x00ff;
	int frac = f & 0x007fffff;

	int newExp = 0;
	if (exp < 113) {
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

export unsigned int packHalf2x16(float x, float y)
{
	return std::bitset<32>(floatToHalf(x) | floatToHalf(y) << 16).to_ulong();
}

export constexpr unsigned int to_uints(float v)
{
	return std::bit_cast<unsigned int>(v);
}