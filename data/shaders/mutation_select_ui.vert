#version 330 core

layout (location = 0) in vec2 pos;

out vec2 fragPos;

uniform float xOffset;

void main()
{
    vec2 offsetedPos = pos;
    offsetedPos.x += xOffset;
    fragPos = offsetedPos;
    gl_Position = vec4(offsetedPos, 0.0, 1.0);
}