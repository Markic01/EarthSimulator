#version 330 core
out vec4 FragColor;

struct DirectionalLight {
    vec3 direction;

    vec3 specular;
    vec3 diffuse;
    vec3 ambient;
};

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

struct Material {
    sampler2D texture_diffuse1;
    vec3 specular;

    float shininess;
};
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform DirectionalLight directionalLight;
uniform SpotLight moonLight;
uniform SpotLight sunLight;
uniform Material material;

uniform vec3 viewPosition;

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

       vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));

       // diffuse
       float diff = max(dot(normal, lightDir), 0.0);
       vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));

       // specular
       vec3 halfwayDir = normalize(lightDir + viewDir);
       float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
       vec3 specular = light.specular * spec * material.specular;

       // spotlight (soft edges)
       float theta = dot(lightDir, normalize(-light.direction));
       float epsilon = (light.cutOff - light.outerCutOff);
       float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
       diffuse  *= intensity;
       specular *= intensity;

       // attenuation
       float distance    = length(light.position - FragPos);
       float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

       ambient  *= attenuation;
       diffuse  *= attenuation;
       specular *= attenuation;

       return (ambient + diffuse + specular);
}
// calculates the color when using a point light.
vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * material.specular;
    return (ambient + diffuse + specular);
}

void main()
{
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPosition - FragPos);
    vec3 result = CalcDirectionalLight(directionalLight, normal, FragPos, viewDir);
    result += CalcSpotLight(sunLight, normal, FragPos, viewDir);
    result += CalcSpotLight(moonLight, normal, FragPos, viewDir);
    FragColor = vec4(result, 1.0);
}