#version 330 core

layout (location = 0) in vec2 pos;

out vec2 fragPos;

void main()
{
    fragPos = pos;
    gl_Position = vec4(pos, 0.0, 1.0);
}