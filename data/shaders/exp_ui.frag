#version 330 core

out vec4 colour;

uniform float expProgress; // 0 to 1

in vec2 fragPos;

void main()
{
    // TODO(Kevin): Use frag pos to decorate exp bar
    // if(fragPos.y < -0.99)
    // {
    //     discard;
    // }

    float ep = (expProgress*2.0-1.0);

    if(fragPos.x < ep)
    {
        colour = vec4(1.0, 0.843, 0.0, 1.0);
    }
    else
    {
        colour = vec4(0.4,0.4,0.4,1.0);
    }
}