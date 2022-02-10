#version 330

// Input attributes
layout (location = 0) in vec2 in_position;
layout (location = 1) in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;

void main()
{
	texcoord = in_texcoord;
	gl_Position = vec4(in_position, 0.0, 1.0);
}