/*
* Yes this is a C++ module! I love it!
* 
* It's intended to be a single file for the whole implementation for the moment, step by step!
*/
module;

#include <string>

export module shaders;

export namespace ShaderSource {

constexpr inline auto vertex =
    "#version 330\n"
    "#extension GL_ARB_shading_language_packing : enable\n"
    "uniform highp usampler2D u_texture;\n"
    "uniform mat4 projection, view;\n"
    "uniform vec2 focal;\n"
    "uniform vec2 viewport;\n"
    "in vec2 position;\n"
    "in int index;\n"
    "out vec4 vColor;\n"
    "out vec2 vPosition;\n"
    "void main () {\n"
    " uvec4 cen = texelFetch(u_texture, ivec2((uint(index) & 0x3ffu) << 1, uint(index) >> 10), 0);\n"
    "  vec4 cam = view * vec4(uintBitsToFloat(cen.xyz), 1);\n"
    "  vec4 pos2d = projection * cam;\n"
    "  highp float clip = 1.2 * pos2d.w;\n"
    "  if (pos2d.z < -clip || pos2d.x < -clip || pos2d.x > clip || pos2d.y < -clip || pos2d.y > clip) {\n"
    "    gl_Position = vec4(0.0, 0.0, 2.0, 1.0);\n"
    "    return;\n"
    "  }\n"
    "  uvec4 cov = texelFetch(u_texture, ivec2(((uint(index) & 0x3ffu) << 1) | 1u, uint(index) >> 10), 0);\n"
    "  vec2 u1 = unpackHalf2x16(cov.x), u2 = unpackHalf2x16(cov.y), u3 = unpackHalf2x16(cov.z);\n"
    "  mat3 Vrk = mat3(u1.x, u1.y, u2.x, u1.y, u2.y, u3.x, u2.x, u3.x, u3.y);\n"
    "  mat3 J = mat3(\n"
    "      focal.x / cam.z, 0., -(focal.x * cam.x) / (cam.z * cam.z), \n"
    "      0., -focal.y / cam.z, (focal.y * cam.y) / (cam.z * cam.z), \n"
    "      0., 0., 0.\n"
    "      );\n"
    "  mat3 T = transpose(mat3(view)) * J;\n"
    "  mat3 cov2d = transpose(T) * Vrk * T;\n"
    "  float mid = (cov2d[0][0] + cov2d[1][1]) / 2.0;\n"
    "  float radius = length(vec2((cov2d[0][0] - cov2d[1][1]) / 2.0, cov2d[0][1]));\n"
    "  float lambda1 = mid + radius, lambda2 = mid - radius;\n"
    "  if(lambda2 < 0.0) return;\n"
    "  vec2 diagonalVector = normalize(vec2(cov2d[0][1], lambda1 - cov2d[0][0]));\n"
    "  vec2 majorAxis = min(sqrt(2.0 * lambda1), 1024.0) * diagonalVector;\n"
    "  vec2 minorAxis = min(sqrt(2.0 * lambda2), 1024.0) * vec2(diagonalVector.y, -diagonalVector.x);\n"
    "  vColor = clamp(pos2d.z/pos2d.w+1.0, 0.0, 1.0) * vec4((cov.w) & 0xffu, (cov.w >> 8) & 0xffu, (cov.w >> 16) & 0xffu, (cov.w >> 24) & 0xffu) / 255.0;\n"
    "  vPosition = position;\n"
    "  vec2 vCenter = vec2(pos2d) / pos2d.w;\n"
    "  gl_Position = vec4(\n"
    "      vCenter \n"
    "	  + position.x * majorAxis / viewport \n"
    "	  + position.y * minorAxis / viewport, 0.0, 1.0);\n"
    "}\n";
;

constexpr inline auto fragment = "#version 330\n"
                                 "in highp vec4 vColor;\n"
                                 "in highp vec2 vPosition;\n"
                                 "out highp vec4 fragColor;\n"
                                 "void main () {\n"
                                 "  highp float A = -dot(vPosition, vPosition);\n"
                                 "  if (A < -4.0) discard;\n"
                                 "  highp float B = exp(A) * vColor.a;\n"
                                 "  fragColor = vec4(B * vColor.rgb, B);\n"
                                 "}\n";

} // namespace ShaderSource
