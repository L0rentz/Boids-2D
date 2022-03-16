#version 460 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec4 aWorldPosAndScale;
layout (location = 3) in float angleDeg;

out vec3 fColor;

uniform mat4 projection;

mat4 translate(vec2 transform)
{
    return mat4(1.0, 0.0, 0.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                transform.x, transform.y, 1.0, 1.0);
}

void main()
{
    float angleRad = angleDeg * 3.14159265358979323846 / 180 * -1;
    float s = sin(angleRad);
	float c = cos(angleRad);

	mat4 rotMat = mat4(c, -s, 0.0, 0.0,
		               s, c, 0.0, 0.0,
                       0.0, 0.0, 1.0, 0.0,
                       0.0, 0.0, 0.0, 1.0
	);

    mat4 scaleMat = mat4(aWorldPosAndScale.z, 0.0, 0.0, 0.0,
                         0.0, aWorldPosAndScale.w, 0.0, 0.0,
                         0.0, 0.0, 1.0, 0.0,
                         0.0, 0.0, 0.0, 1.0);

    mat4 model = translate(aWorldPosAndScale.xy);
    model *= rotMat;
    model *= scaleMat;

    gl_Position = projection * model * vec4(aPos * -1, 0.0, 1.0);
    fColor = aColor;
}