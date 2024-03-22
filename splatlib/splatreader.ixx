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

// unsigned char filestream
using ucfstream = std::basic_ifstream<unsigned char, std::char_traits<unsigned char> >;

std::vector<unsigned char> readFromFile(const std::filesystem::path& path)
{
	// file size
	auto length = std::filesystem::file_size(path);

	// file buffer
	std::vector<unsigned char> u_buffer(length);

	// read file data to buffer
	ucfstream inputFile(path, std::ios_base::binary);
	inputFile.read(u_buffer.data(), length);
	inputFile.close();

	return u_buffer;
}

}; // namespace Splat
