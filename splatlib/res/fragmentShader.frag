#version 330
in highp vec4 vColor;
in highp vec2 vPosition;
out highp vec4 fragColor;
void main () {
  highp float A = -dot(vPosition, vPosition);
  if (A < -4.0) discard;
  highp float B = exp(A) * vColor.a;
  fragColor = vec4(B * vColor.rgb, B);
}