#version 430 core

uniform sampler2D albedo; 

in vec2 TexCoords;

out vec4 FragColor;

void main(){
	FragColor=vec4(texture(albedo, TexCoords).rrr,1.0);
	//FragColor=vec4(1.0);
}