#version 330

uniform sampler2D screen_texture;

in vec2 texcoord;

out vec4 color;

void main()
{
    vec4 in_color = texture(screen_texture, texcoord);
	color = in_color;
}