#version 330

// Input attributes
layout (location = 0) in vec2 in_position;
layout (location = 1) in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;
out vec2 position;

// Application data
uniform mat3 transform;
uniform mat3 projection;
uniform mat3 cameraTransform;

void main()
{
	texcoord = in_texcoord;
	position = in_position / 6.0;
	vec3 pos = projection * cameraTransform * transform * vec3(in_position, 1.0);
	gl_Position = vec4(pos.xy, 0.0, 1.0);
}