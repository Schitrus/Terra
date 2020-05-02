#version 460 core

layout (quads, equal_spacing, cw) in;

uniform mat4 MVP;

uniform sampler2D noise;

in vec2 teseUV[];

out vec2 fragUV;

out vec3 normal;

void main(){
	vec4 pos    = mix(mix(gl_in[0].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x),
					  mix(gl_in[1].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x),
					                                                  gl_TessCoord.y);
	
	
	fragUV      = mix(mix(teseUV[0], teseUV[3], gl_TessCoord.x),
					  mix(teseUV[1], teseUV[2], gl_TessCoord.x),
					                            gl_TessCoord.y);

	float dt = 1.0/256.0;

	vec3 p0 = vec3(fragUV.x, texture(noise, fragUV).x, fragUV.y);
	vec3 p1 = vec3(fragUV.x + dt, texture(noise, fragUV + vec2(dt, 0)).x, fragUV.y);
	vec3 p2 = vec3(fragUV.x, texture(noise, fragUV + vec2(0, dt)).x, fragUV.y + dt);

	normal = normalize(cross(p2 - p0, p1 - p0));

	float height = texture(noise, fragUV).x;


	

	pos = vec4(pos.x, height, pos.z, 1.0f);

	gl_Position = MVP * pos; 
}