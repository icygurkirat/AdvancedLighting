#version 430 core

in vec2 TexCoords;
in float cam2center;

uniform sampler2D smoke;
uniform sampler2D gPosition;
uniform int screenWidth;
uniform int screenHeight;

out vec4 FragColor;

void main(){
	vec2 FragPos = vec2(float(gl_FragCoord.x)/screenWidth, float(gl_FragCoord.y)/screenHeight);
	vec3 bufferPos = texture(gPosition, FragPos).xyz;
	float z = dot(bufferPos, bufferPos);
	if(z < cam2center)
		discard;

	vec4 col = texture( smoke, TexCoords );
	if(col.a<0.1)
		discard;
	FragColor = col;
}