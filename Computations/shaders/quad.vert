#version 460 core

out vec2 uv;

const vec4 positions[4] = 
{
	  vec4(+1.0, +1.0, +0.0, +1.0)
	, vec4(-1.0, +1.0, +0.0, +1.0)
	, vec4(+1.0, -1.0, +0.0, +1.0)
	, vec4(-1.0, -1.0, +0.0, +1.0)
};

const vec2 uvs[4] = 
{
	  vec2(1.0, 1.0)
	, vec2(0.0, 1.0)
	, vec2(1.0, 0.0)
	, vec2(0.0, 0.0)
};

void main()
{
	int i = gl_VertexID % 4;

	uv = uvs[i];
	gl_Position = positions[i];
}