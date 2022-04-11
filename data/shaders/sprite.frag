#version 330

// From vertex shader
in vec2 texcoord;
in vec2 position;
uniform int lightSize;
uniform vec2 lightSources[25];

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;

// Output color
layout(location = 0) out vec4 color;

void main()
{
	float lightFactor = 0.0;

    float closestDistance = 1.0 / 0.0;
    for (int i = 0; i < lightSize; i++) {
        float dist = distance(position, lightSources[i]);
        if (dist < closestDistance) {
            closestDistance = dist;
        }
    }
    
    if (lightSize == 0) {
        lightFactor = 1.0;
    }
    else if (closestDistance < 150.0) {
        float exp = closestDistance;
        lightFactor = max(1.0 * pow(0.98, exp), 0.3);
    }
    else {
        lightFactor = 0.3;
    }

	color = vec4(lightFactor * fcolor, 1.0) * texture(sampler0, texcoord);
}
