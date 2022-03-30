#version 330 core

out vec4 colour;

uniform float expProgress; // 0 to 1

in vec2 fragPos;

void main()
{
    float epNDC = (expProgress*2.0-1.0);

    if(fragPos.x < epNDC)
    {
        float f = (abs(fragPos.y) - 0.935) / (1.0 - 0.935);
        f = sin(f * 3.14159265);

        vec3 col1 = vec3(124.0/255.0, 216.0/255.0, 235.0/255.0);
        vec3 col2 = vec3(66.0/255.0, 91.0/255.0, 189.0/255.0);

        colour = vec4(mix(col1,col2,f), 1.0);
    }
    else
    {
        colour = vec4(0.2,0.2,0.2,1.0);
    }
}