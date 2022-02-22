#version 330 core

// Shader for Rendering Text on Textured Quads

in vec2 tex_coord;

out vec4 colour;

uniform sampler2D fontAtlas;
uniform vec4 textColour;

void main()
{
    float text_alpha = texture(fontAtlas, tex_coord).x; // x because bitmap is passed as a GL_R value only
    // Make sure alpha blending is on
    colour = vec4(textColour.xyz, textColour.w * text_alpha);
}