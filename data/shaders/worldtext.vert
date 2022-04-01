#version 330 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 in_tex_coord;

uniform mat3 modelMatrix;
uniform mat3 viewMatrix;
uniform mat3 projectionMatrix;

out vec2 tex_coord;

void main()
{
    tex_coord = in_tex_coord;
    vec3 transformedPos = projectionMatrix * viewMatrix * modelMatrix * vec3(pos.xy, 1.0);
    gl_Position = vec4(transformedPos.xy, 0.0, 1.0); // transformedPos.z
}
