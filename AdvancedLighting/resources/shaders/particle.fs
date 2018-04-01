#version 430 core

in vec2 TexCoords;

uniform sampler2D smoke;
uniform sampler2D gPosition;

out vec4 FragColor;

void main(){
	vec4 col = texture( smoke, TexCoords );
	if(col.a<0.1)
		discard;
	FragColor = col;
}