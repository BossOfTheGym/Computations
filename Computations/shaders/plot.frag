#version 460 core

out vec4 color_out;

in float height;


void main()
{
	color_out = vec4(vec3(height), 1.0);
} 