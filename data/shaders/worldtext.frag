#version 330 core

in vec2 tex_coord;

out vec4 colour;

uniform sampler2D fontAtlas;

void main()
{
    float text_alpha = texture(fontAtlas, tex_coord).x; // x because bitmap is passed as a GL_R value only
    // Make sure alpha blending is on
    colour = vec4(vec3(1.0), text_alpha);
}
