#version 460 core

layout(location = 0) in vec3 pos;

out vec2 tescUV;

void main(){

	tescUV = 0.5 * vec2(pos.x, pos.z) + vec2(0.5); 

	gl_Position = vec4(pos, 1.0f);
}