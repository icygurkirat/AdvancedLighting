#version 430 core
out float FragColor;
  
in vec2 TexCoords;
  
uniform sampler2D ssao;
uniform int blurKernal;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(ssao, 0));
    float result = 0.0;
    for (int x = -blurKernal/2; x < blurKernal/2; x++) 
    {
        for (int y = -blurKernal/2; y < blurKernal/2; y++) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssao, TexCoords + offset).r;
        }
    }
    FragColor = result / (blurKernal * blurKernal * 1.0);
}