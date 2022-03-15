#version 460 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in mat4 instanceModel;

out vec3 fColor;

uniform mat4 projection;

void main()
{
    gl_Position = projection * instanceModel * vec4(aPos * -1, 0.0, 1.0);
    fColor = aColor;
}