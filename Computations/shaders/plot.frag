#version 460 core

out vec4 color_out;

in float height;
in vec4 pos_model;
in vec4 pos_view;

void main()
{
	float dist = length(pos_view);

	vec2 grid = abs(fract(5.0 * (pos_model.xy + vec2(0.5))));
	grid = vec2(1.0) - smoothstep(0.1, 0.11, grid);

	float gridCoef = max(grid.x, grid.y);

	float len = length(pos_model.xy);
	float centerCoef = 1.0 - smoothstep(0.25, 0.27, len);

	color_out = vec4(height, centerCoef, gridCoef, 1.0);
} 