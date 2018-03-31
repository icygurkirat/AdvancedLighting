#version 430 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D ssao;
uniform samplerCube shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float farPlane;

void main()
{             
    // retrieve data from gbuffer (deferred shading pass)
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 albedo = texture(gAlbedo, TexCoords).rgb;

    //occlusion from ssao pass
    float AmbientOcclusion = texture(ssao, TexCoords).r;
    

    // blinn-phong:-
    vec3 ambient = vec3(vec3(0.5f, 0.5f, 0.5f) * albedo * AmbientOcclusion);

    vec3 lightDir = lightPos - FragPos;
    float currentDepth = length(lightDir);
    lightDir = normalize(lightDir);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * albedo * vec3(0.5f, 0.5f, 0.5f);

    vec3 viewDir  = normalize(viewPos - FragPos); 
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
    vec3 specular = vec3(0.2f, 0.2f, 0.2f) * spec;


    //shadow calculation:-
    float closestDepth = texture(shadowMap, -lightDir).r;
    closestDepth *= farPlane;
    float bias = 0.05; 
    float shadow = (currentDepth -  bias > closestDepth) ? 0.0 : 1.0; 
    FragColor = vec4( ambient + ((diffuse /*+ specular*/)*(shadow)), 1.0);
}