#version 430 core

uniform sampler2D albedo; 
uniform bool gray;

in vec2 TexCoords;

out vec4 FragColor;

void main(){
	FragColor = gray? vec4(texture(albedo, TexCoords).rrr,1.0) : vec4(texture(albedo, TexCoords).rgb,1.0);
}