#version 460 core

layout(location = 0) in vec2 pos_in; 

out float height;
out vec4 pos_model;
out vec4 pos_view;

layout(binding = 0) uniform sampler2D heightMap;

layout(binding = 0, std140) uniform MatrixBlock
{
	mat4 pvm;
	mat4 p;
	mat4 v;
	mat4 m;
};

void main()
{
	float h = texture2D(heightMap, pos_in).x;
	height = h;

	vec4 pos = m * vec4(pos_in, h, 1.0); 
	pos_model = pos;

	pos = v * pos;
	pos_view = pos;

	gl_Position = p * pos_view;
}
