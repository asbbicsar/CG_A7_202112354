#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

out vec3 wPosition;
out vec3 wNormal;
out vec3 wColor;

uniform mat4 modeling;
uniform mat4 modeling_inv_tr; 
uniform mat4 camera; 
uniform mat4 projection; 

void main()
{
    gl_Position = projection * camera * modeling * vec4(aPos, 1.0);
    wPosition = vec3(modeling * vec4(aPos, 1.0));
    wNormal = vec3(modeling_inv_tr * vec4(aNormal, 1.0)); // Transform normal to world space
    wColor = aColor;
}