#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPosModel;
out vec3 FragPos;
out vec3 Normal;
out vec3 NormalModel;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vec4 viewPos = model * vec4(aPos, 1.0);
    FragPosModel = (viewPos.xyz)/viewPos.w; 

	mat4 trans = view * model;

    viewPos = trans * vec4(aPos, 1.0);
    FragPos = (viewPos.xyz)/viewPos.w; 
    
    //as all transformations are global scaling or translation or rotation, we can directly
    //multiply by this matrix to get normal vector. else multiply by transpose((trans)^-1)
    Normal = mat3(trans) * aNormal;
    NormalModel = mat3(model) * aNormal;

	TexCoords = aTexCoords;
    
    gl_Position = projection * viewPos;
}