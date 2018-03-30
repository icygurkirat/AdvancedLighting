#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;

struct Material {
    sampler2D texture_diffuse1;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;


uniform Material material;

void main()
{    
    // per-fragment position vector
    gPosition = FragPos;
    // per-fragment normal vector
    gNormal = normalize(Normal);
    // per-fragment diffuse color
    gAlbedo.rgb = texture(material.texture_diffuse1,TexCoords).rgb;
}