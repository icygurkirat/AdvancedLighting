#version 430 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;   //6 faces * 3 vertices

uniform mat4 shadowMatrices[6];

out vec4 FragPos;

void main()
{
    for(int i = 0; i < 6; i++)
    {
        gl_Layer = i; // The face to which we wish to render
        for(int j = 0; j < 3; j++)
        {
            FragPos = gl_in[j].gl_Position;
            gl_Position = shadowMatrices[i] * FragPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
}  