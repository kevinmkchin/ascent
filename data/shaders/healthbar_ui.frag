#version 330 core

out vec4 colour;

uniform float hpNormalized; // 0 to 1

in vec2 fragPos;

#define LEFTBOUND -0.96
#define RIGHTBOUND -0.2

void main()
{
    float hpNDC = LEFTBOUND + (hpNormalized*(RIGHTBOUND - LEFTBOUND));

    if(fragPos.x < hpNDC)
    {
        // float f = (abs(fragPos.y) - 0.935) / (1.0 - 0.935);
        // f = sin(f * 3.14159265);

        // vec3 col1 = vec3(124.0/255.0, 216.0/255.0, 235.0/255.0);
        // vec3 col2 = vec3(66.0/255.0, 91.0/255.0, 189.0/255.0);

        colour = vec4(1.0, 0.0, 0.0, 1.0);
    }
    else
    {
        colour = vec4(0.2,0.2,0.2,1.0);
    }
}