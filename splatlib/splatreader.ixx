/*
* Yes this is a C++ module! I love it!
* 
* It's intended to be a single file for the whole implementation for the moment, step by step!
*/

module;

#include <vector>

#include <filesystem>
#include <fstream>

export module splat.reader;


export namespace Splat {

std::vector<unsigned char> readFromFile(const std::filesystem::path& path)
{
	// file size
	auto length = std::filesystem::file_size(path);

	std::vector<unsigned char> u_buffer;
	u_buffer.resize(length);

	// read file data
	std::basic_ifstream<unsigned char, std::char_traits<unsigned char> > inputFile(path, std::ios_base::binary);
	inputFile.read(u_buffer.data(), length);
	inputFile.close();

	return u_buffer;
}

}; // namespace Splat
