#version 330 core

in vec3 exColor;
out vec4 FragColor;

void main(){
	FragColor = vec4(exColor, 1.0);
}