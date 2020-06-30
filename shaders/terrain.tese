#version 460 core

layout (quads, fractional_odd_spacing, cw) in;

uniform mat4 MVP;

uniform mat4 model;

uniform float res;

uniform mat4 modelViewMatrix;

uniform mat4 lightMatrix;

layout(binding = 0) uniform sampler2D noise;

in vec2 teseUV[];

out vec2 fragUV;

out vec3 normal_vector;

out vec3 fragment_position;

out vec4 shadowMapCoord;

out float h;

void main(){
	vec4 pos    = mix(mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x),
					  mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x),
					                                                  gl_TessCoord.y);
	
	
	fragUV      = mix(mix(teseUV[0], teseUV[1], gl_TessCoord.x),
					  mix(teseUV[2], teseUV[3], gl_TessCoord.x),
					                            gl_TessCoord.y);



	float terrain_length = res + 1;

	vec2 p0 = ivec2(gl_TessCoord.xy * terrain_length + vec2(0,0)  + vec2(1,1)) / (terrain_length + 2);
	vec2 p1 = ivec2(gl_TessCoord.xy * terrain_length + vec2(1,0)  + vec2(1,1)) / (terrain_length + 2);
	vec2 p2 = ivec2(gl_TessCoord.xy * terrain_length + vec2(0,1)  + vec2(1,1)) / (terrain_length + 2);
	vec2 p3 = ivec2(gl_TessCoord.xy * terrain_length + vec2(-1,0) + vec2(1,1)) / (terrain_length + 2);
	vec2 p4 = ivec2(gl_TessCoord.xy * terrain_length + vec2(0,-1) + vec2(1,1)) / (terrain_length + 2);

	vec3 center_point = vec3(p0.x, texture(noise, p0, 0).x, p0.y);
	vec3 v1 = normalize(vec3(p1.x, texture(noise, p1, 0).x, p1.y) - center_point);
	vec3 v2 = normalize(vec3(p2.x, texture(noise, p2, 0).x, p2.y) - center_point);
	vec3 v3 = normalize(vec3(p3.x, texture(noise, p3, 0).x, p3.y) - center_point);
	vec3 v4 = normalize(vec3(p4.x, texture(noise, p4, 0).x, p4.y) - center_point);

	vec3 n1 = normalize(cross(v2, v1));
	vec3 n2 = normalize(cross(v3, v2));
	vec3 n3 = normalize(cross(v4, v3));
	vec3 n4 = normalize(cross(v1, v4));

	normal_vector = normalize(n1 + n2 + n3 + n4);

	float height = texture(noise, p0, 0).x;

	h = height;
	

	pos = vec4(pos.x, height, pos.z, 1.0f);

	fragment_position = vec3(model * pos);

	shadowMapCoord = lightMatrix * modelViewMatrix * pos;

	gl_Position = MVP * pos; 
}