#version 460 core

out vec4 color_out;

in float height;
in vec4 pos_model;
in vec4 pos_view;

void main()
{
	float dist = length(pos_view.xy) + 1e-6;
	float distCoef = log2(dist) / log(5.0);

	vec2 pos2 = pos_model.xy;
	float gridCoef = 0.0;
	float multCoef = 0.5;
	for (int i = 0; i < 4; i++)
	{
		vec2 grid = abs(fract(pos2 - 0.5) - 0.5);
		grid = vec2(1.0) - smoothstep(0.08, 0.10, grid);

		gridCoef += multCoef * max(grid.x, grid.y);
		pos2 *= 5.0;
		multCoef *= 0.5;
	}

	float len = length(pos_model.xy);
	float centerCoef = 1.0 - smoothstep(0.10, 0.30, len);

	color_out = vec4(height, centerCoef, gridCoef, 1.0);
} 