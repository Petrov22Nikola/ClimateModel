#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoord;
uniform mat4 MVP;
uniform bool isPoint;

out vec2 TexCoord;

void main()
{
    gl_Position = MVP * vec4(pos.x, pos.y, pos.z, 1.0);
    if (isPoint) {
        gl_PointSize = 10.0f;
    }
    TexCoord = texCoord;
}