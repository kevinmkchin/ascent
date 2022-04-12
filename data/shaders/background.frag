#version 330

// From vertex shader
in vec2 texcoord;
in vec2 position;

// Application data
uniform sampler2D sampler0;
uniform int lightSize;
uniform vec2 lightSources[25];
uniform vec2 cameraPosition;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	float lightFactor = 0.0;

    vec2 offset = (position * vec2(320.0, 180.0)) - (0.5 * vec2(320.0, 180.0));
    offset += cameraPosition;

    float closestDistance = 1.0 / 0.0;
    for (int i = 0; i < lightSize; i++) {
        float dist = distance(offset, lightSources[i]);
        if (dist < closestDistance) {
            closestDistance = dist;
        }
    }
    
    if (lightSize == 0) {
        lightFactor = 1.0;
    }
    // else if (closestDistance < 180.0) {
    //     float exp = closestDistance;
    //     lightFactor = max(1.0 * pow(0.98, exp), 0.1);
    // }
    else {
        lightFactor = 0.2;
    }

	color = vec4(lightFactor * vec3(1.0), 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y));
}
