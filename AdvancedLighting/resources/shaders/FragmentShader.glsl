#version 430 core

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float shininess;
}; 

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

void main()
{
	//ambient lighting
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));

	//diffused lighting
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(light.position - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = (diff * light.diffuse) * vec3(texture(material.texture_diffuse1, TexCoords));

	//specular lighting
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = (spec * light.specular) * vec3(texture(material.texture_specular1, TexCoords));  
	
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);


	FragColor = vec4(vec3(texture(material.texture_diffuse1, TexCoords)), 1.0);
}