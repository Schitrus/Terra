#version 460 core

layout (quads, fractional_odd_spacing, cw) in;

uniform mat4 MVP;

uniform float res;

uniform sampler2D noise;

in vec2 teseUV[];

out vec2 fragUV;

out vec3 normal;

out vec3 fragPos;

out float h;

void main(){
	vec4 pos    = mix(mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x),
					  mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x),
					                                                  gl_TessCoord.y);
	
	
	fragUV      = mix(mix(teseUV[0], teseUV[1], gl_TessCoord.x),
					  mix(teseUV[2], teseUV[3], gl_TessCoord.x),
					                            gl_TessCoord.y);

	float dt = 1.0/res;

	vec2 p0 = gl_TessCoord.xy;
	vec2 p1 = gl_TessCoord.xy + vec2(dt,0);
	vec2 p2 = gl_TessCoord.xy + vec2(0,dt);
	vec2 p3 = gl_TessCoord.xy + vec2(-dt,0);
	vec2 p4 = gl_TessCoord.xy + vec2(0,-dt);

	vec3 v0 = vec3(p0.x, texture(noise, p0).x, p0.y);
	vec3 v1 = vec3(p1.x, texture(noise, p1).x, p1.y) - v0;
	vec3 v2 = vec3(p2.x, texture(noise, p2).x, p2.y) - v0;
	vec3 v3 = vec3(p3.x, texture(noise, p3).x, p3.y) - v0;
	vec3 v4 = vec3(p4.x, texture(noise, p4).x, p4.y) - v0;

	vec3 n1 = normalize(cross(v1, v2));
	vec3 n2 = normalize(cross(v2, v3));
	vec3 n3 = normalize(cross(v3, v4));
	vec3 n4 = normalize(cross(v4, v1));

	normal = -normalize(n1 + n2 + n3 + n4);

	float height = texture(noise, p0).x;

	h = height;
	

	pos = vec4(pos.x, height, pos.z, 1.0f);

	fragPos = vec3(pos);

	gl_Position = MVP * pos; 
}