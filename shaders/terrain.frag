#version 460 core

out vec4 out_color;

in vec2 fragUV;

in vec3 normal;

void main(){

	vec3 light_dir = vec3(0.0f,-1.0f,0.0);

	float diff = dot(normal, -light_dir);

	vec3 object_color = vec3(0.0f, 0.7f, 0.2f);

	out_color = vec4(0.0f, 0.7f, 0.2f, 1.0f) * diff;
}