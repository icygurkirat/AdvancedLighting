#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords; 
layout (location = 2) in vec3 aCenter; 
layout (location = 3) in float aSize;
layout (location = 4) in float aTime;

out vec2 TexCoords;
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


	//vec4 Pos = proj * view * vec4((aCenter + aSize * aPos), 1.0);
	vec4 Pos = vec4((aCenter + aSize * aPos), 1.0);
	Pos.z = aCenter.z<=-0.9 ? 1.0 : 0.2;
	gl_Position = Pos; 
}