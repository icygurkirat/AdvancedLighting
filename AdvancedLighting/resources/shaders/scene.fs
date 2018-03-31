#version 430 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gPositionModel;
layout (location = 4) out vec3 gNormalModel;

struct Material {
    sampler2D texture_diffuse1;
};

in vec3 FragPosModel;
in vec3 FragPos;
in vec3 Normal;
in vec3 NormalModel;
in vec2 TexCoords;


uniform Material material;

void main()
{    
    // per-fragment position vector
    gPosition = FragPos;
    gPositionModel = FragPosModel;
    // per-fragment normal vector
    gNormal = normalize(Normal);
    gNormalModel = normalize(NormalModel);
    // per-fragment diffuse color
    gAlbedo.rgb = texture(material.texture_diffuse1,TexCoords).rgb;
}