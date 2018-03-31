#version 430 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float farPlane;

void main()
{
    float lightDistance = length(FragPos.xyz - lightPos);
    
    // map to [0,1] range 
    gl_FragDepth = lightDistance / farPlane;
}  