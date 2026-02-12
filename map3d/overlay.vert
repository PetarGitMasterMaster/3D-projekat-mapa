#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

out vec2 vUV;

uniform vec2 uPos;
uniform vec2 uSize;
uniform vec2 uUVScale;
uniform vec2 uUVOffset;

void main()
{
    vec2 scaled = aPos * uSize;
    vec2 finalPos = scaled + uPos;

    gl_Position = vec4(finalPos, 0.0, 1.0);

    vUV = aUV * uUVScale + uUVOffset;
}
