#version 460 core

uniform vec3 light_color;

out vec4 out_color;

void main(){

	gl_FragDepth = 1.0f;

	out_color = vec4(light_color, 1.0f);
}