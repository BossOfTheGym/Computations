#version 460 core

in vec2 uv;

out vec4 color;

layout(binding = 0) uniform sampler2D test;

void main()
{
	color = vec4(texture2D(test, uv).xyz, 1.0);
}