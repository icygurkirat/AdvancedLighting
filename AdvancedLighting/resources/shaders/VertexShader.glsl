#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

out vec3 exColor;

uniform float offset;

void main() {
	gl_Position = vec4(aPos.x + offset, aPos.yz, 1.0);
	exColor = gl_Position.xyz;
}