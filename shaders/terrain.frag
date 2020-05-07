#version 460 core

out vec4 out_color;

in vec2 fragUV;

in vec3 normal;

in float h;

in vec3 fragPos;

uniform vec3 light_position;

uniform vec3 view_position;

void main(){

	vec3 light_dir = -normalize(light_position - fragPos);

	vec3 view_dir = -normalize(view_position - fragPos);

	vec3 reflect_dir = reflect(-light_dir, normal);

	vec3 light_color = vec3(1.0f, 1.0f, 1.0f);

	float ambi = 0.1f;

	float diff = max(dot(-light_dir, normal), 0.0);

	float spec = 0.5 * pow(max(dot(view_dir, reflect_dir), 0.0), 32);

	vec3 object_color = mix(vec3(0.0f, 0.7f, 0.2f), vec3(1.0f, 1.0f, 1.0f), 2*h);

	out_color = vec4((ambi + diff + spec) * object_color, 1.0f);
}