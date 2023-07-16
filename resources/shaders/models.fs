#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

struct Material {
    sampler2D texture_diffuse1;
    vec3 specular;

    float shininess;
};
in vec2 TexCoords;
uniform Material material;
uniform vec3 ambientLight;

void main()
{
    //only has ambient component since it is a light source itself
    vec3 ambient = ambientLight * vec3(texture(material.texture_diffuse1, TexCoords));
    FragColor = vec4(ambient, 1.0);
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}