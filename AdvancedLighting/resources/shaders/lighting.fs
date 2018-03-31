#version 430 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D ssao;

uniform vec3 lightPos;

void main()
{             
    // retrieve data from gbuffer (deferred shading pass)
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 albedo = texture(gAlbedo, TexCoords).rgb;

    //occlusion from ssao pass
    float AmbientOcclusion = texture(ssao, TexCoords).r;
    

    // blinn-phong:-
    vec3 ambient = vec3(vec3(0.35f, 0.35f, 0.35f) * albedo * AmbientOcclusion);

    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * albedo * vec3(0.7f, 0.7f, 0.7f);

    vec3 viewDir  = normalize(-FragPos); //in view-space
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
    vec3 specular = vec3(0.2f, 0.2f, 0.2f) * spec;

    FragColor = vec4(ambient + diffuse /*+ specular*/, 1.0);
}