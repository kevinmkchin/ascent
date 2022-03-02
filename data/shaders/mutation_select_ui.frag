#version 330 core

out vec4 colour;

in vec2 fragPos;

uniform bool bSelected;
uniform bool bBox;
uniform float time;

void main()
{
    colour = vec4(0.7, 0.7, 0.7, 1.0);
    if(bSelected)
    {
        colour = vec4((sin(time + fragPos.x) + 1.0) * 0.5, (cos(time + fragPos.y) + 1.0) * 0.5, 1.0, 1.0);
    }
    if(bBox)
    {
        colour = vec4(0.0, 0.0, 0.0, 0.9);
    }
}