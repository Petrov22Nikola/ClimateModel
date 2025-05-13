#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D thermalTexture;
uniform sampler2D physicalTexture;

void main()
{
    FragColor = mix(texture(thermalTexture, TexCoord), texture(physicalTexture, TexCoord), 0.5);
}