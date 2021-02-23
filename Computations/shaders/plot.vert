#version 460 core

layout(location = 0) in vec2 pos_in; 

out float height;

layout(binding = 0) uniform sampler2D heightMap;

layout(binding = 0, std430) uniform MatrixBlock
{
	mat4 pvm;
	mat4 p;
	mat4 v;
	mat4 m;
}

void main()
{
	float h = texture2D(heightMap, pos_in);

	height = h;
	gl_Position = pvm * vec4(pos_in, h, 1.0);
}
