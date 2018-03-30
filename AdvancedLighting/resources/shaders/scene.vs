#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform bool invertedNormals;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	mat4 trans = view * model;

    vec4 viewPos = trans * vec4(aPos, 1.0);
    FragPos = viewPos.xyz; 
    
    //mat3 normalMatrix = transpose(inverse(mat3(view * model)));
    Normal = mat3(trans) * (invertedNormals ? -aNormal : aNormal);

	TexCoords = aTexCoords;
    
    gl_Position = projection * viewPos;
}