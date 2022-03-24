#version 460 core

in vec3 fColor;

out vec4 FragColor;

void main()
{
    FragColor = vec4(fColor, 0.2);
}