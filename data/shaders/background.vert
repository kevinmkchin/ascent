#version 330

// Input attributes
layout (location = 0) in vec2 in_position;
layout (location = 1) in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;
out vec2 position;

uniform float bg_offset;

void main()
{
	position = in_texcoord;
	texcoord.x = in_texcoord.x + bg_offset;
	texcoord.y = in_texcoord.y;
	gl_Position = vec4(in_position, 0.0, 1.0);
}