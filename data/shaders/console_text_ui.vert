#version 330 core

// Shader for Rendering Text on Textured Quads

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 in_tex_coord;

out vec2 tex_coord;

uniform mat4 matrix_model;

void main()
{
    gl_Position = matrix_model * vec4(pos, 0.0, 1.0);
    tex_coord = in_tex_coord;
}
