#version 330 core
#define MAX_LIGHTS 16

struct Light {
    vec3 pos;
    vec3 kA;
    vec3 kD;
    vec3 kS;
};

struct Material {
    vec3 kA;
    vec3 kD;
    vec3 kS;
    float shine;
};

in vec3 chNor;
in vec3 chFragPos;
in vec2 chUV;

out vec4 outCol;

uniform Material uMaterial;
uniform vec3 uViewPos;
uniform int uLightCount;
uniform Light uLights[MAX_LIGHTS];

uniform bool uUseTexture;
uniform sampler2D uDiffMap;

void main()
{
    vec3 normal = normalize(chNor);
    vec3 viewDir = normalize(uViewPos - chFragPos);

    vec3 baseColor = uMaterial.kD;
    if (uUseTexture)
        baseColor *= texture(uDiffMap, chUV).rgb;

    vec3 result = vec3(0.0);

    for (int i = 0; i < uLightCount; i++)
    {
        Light light = uLights[i];

        vec3 ambient = light.kA * uMaterial.kA * baseColor;

        vec3 lightDir = normalize(light.pos - chFragPos);
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = light.kD * diff * baseColor;

        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), uMaterial.shine);
        vec3 specular = light.kS * spec * uMaterial.kS;

        result += ambient + diffuse + specular;
    }

    outCol = vec4(result, 1.0);
}
