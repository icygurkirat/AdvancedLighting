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
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(Normal);
    // and the diffuse per-fragment color
    gAlbedo.rgb = texture(material.texture_diffuse1,TexCoords).rgb;
}