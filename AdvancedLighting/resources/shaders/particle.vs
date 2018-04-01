#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords; 
layout (location = 2) in vec3 aCenter; 
layout (location = 3) in float aSize;
layout (location = 4) in float aTime;

out vec2 TexCoords;
out float cam2center;

uniform vec3 camPos;
uniform float maxLife;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	int tile = int((aTime/maxLife) * 64.0);
	tile = min(tile,63);
	int row = tile/8;
	int col = tile%8;
	TexCoords.x = (col + aTexCoords.x) / 8.0;
	TexCoords.y = (row + aTexCoords.y) / 8.0;

	vec3 temp = camPos - aCenter;
	cam2center = dot(temp, temp);

	vec4 Pos = proj * view * vec4((aCenter + aSize * aPos), 1.0);
	//vec4 Pos = proj * view * vec4(aCenter, 1.0);
	//Pos /= Pos.w;
	//Pos = vec4(Pos.xyz + inversesqrt(cam2center) * aSize * aPos, 1.0);

	Pos.z = aCenter.z<=-24.0 ? 1.0*Pos.w: 0.2*Pos.w;
	gl_Position = Pos; 
}