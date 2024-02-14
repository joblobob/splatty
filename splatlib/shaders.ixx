/*
* Yes this is a C++ module! I love it!
* 
* It's intended to be a single file for the whole implementation for the moment, step by step!
*/
module;

#include <battery/embed.hpp>
#include <string>

export module splat.shaders;

//Please Put My Data In My Gotdang Executable And Stop Doing Crazy Nonsense, The Feature

export namespace ShaderSource {

const char* vertex = b::embed<"res/vertexShader.vert">().data();

const char* fragment = b::embed<"res/fragmentShader.frag">().data();

} // namespace ShaderSource
