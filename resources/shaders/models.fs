#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse1;
    vec3 specular;

    float shininess;
};
in vec2 TexCoords;
uniform Material material;

void main()
{
    //only has ambient component since it is a light source itself
    vec3 ambient = vec3(texture(material.texture_diffuse1, TexCoords));
    FragColor = vec4(ambient, 1.0);
}