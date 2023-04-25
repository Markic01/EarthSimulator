#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    vec3 specular;
    vec3 diffuse;
    vec3 ambient;

    float constant;
    float linear;
    float quadratic;
};

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;
out vec3 TangentSunLightPos;
out vec3 TangentMoonLightPos;
out vec3 TangentViewPos;
out vec3 TangentFragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform SpotLight moonLight;
uniform SpotLight sunLight;
uniform vec3 viewPosition;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    TexCoords = aTexCoords;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    mat3 TBN = transpose(mat3(T, B, N));
    TangentSunLightPos = TBN * sunLight.position;
    TangentMoonLightPos = TBN * moonLight.position;
    TangentViewPos  = TBN * viewPosition;
    TangentFragPos  = TBN * FragPos;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}