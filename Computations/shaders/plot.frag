#version 460 core

#define PI 3.14159265359

out vec4 color_out;

in float height;
in vec4 pos_out;

void main()
{
	float l = length(pos_out.xy);
	float c = 1.0 - smoothstep(0.25, 0.27, l);
	color_out = vec4(height, c, 0.0, 1.0);
} 