#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform float uAlpha;

void main()
{
    vec4 c = texture(uTexture, vUV);
    if (c.a < 0.1) discard;
    FragColor = vec4(c.rgb, c.a * uAlpha);
}
