#version 430 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

layout (std430, binding = 1) buffer ubo{
	vec4 kernels[];
};
uniform mat4 projection;
uniform int kernelSize;
uniform vec2 noiseScale;
uniform float radius;
uniform float eps;

void main()
{
    vec3 fragPos   = texture(gPosition, TexCoords).xyz;
	vec3 normal    = texture(gNormal, TexCoords).rgb;
	vec3 randomVec = texture(texNoise, TexCoords * noiseScale).xyz;  

	vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN       = mat3(tangent, bitangent, normal);  

	float occlusion = 0.0;
	for(int i = 0; i < kernelSize; ++i){
		// get sample position
		vec3 kernel = TBN * kernels[i].xyz; // From tangent to view-space
		kernel = fragPos + kernel * radius; 
    
		vec4 offset = vec4(kernel, 1.0);
		offset      = projection * offset;    // from view to clip-space
		offset.xyz /= offset.w;               // perspective divide
		offset.xyz  = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0  

		float sampleDepth = texture(gPosition, offset.xy).z; 

		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= kernel.z + eps ? 1.0 : 0.0) * rangeCheck; 
	}  

	occlusion = 1.0 - (occlusion / kernelSize);
	FragColor = occlusion;
}