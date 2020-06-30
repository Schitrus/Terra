#version 460 core

uniform ivec3 seed;

uniform float freq;

uniform vec3 offset;

uniform float size;

layout(binding = 0)uniform sampler2D noise;

layout(binding = 1)uniform sampler1D gradient;

in vec2 UV;

out float height;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
 
vec3 grad(ivec3 pos){
	int p = (pos.x*pos.x + pos.y*pos.y + pos.z*pos.z)%1024;
	return texelFetch(gradient, p, 0).xyz;
}

float fade(float x){
	return x * x * x * (x * (x * 6 - 15) + 10);
}

void main(){

	vec3 pos = freq * ((1.0f + 2.0f/size) * (vec3(UV.x, 0.0f, UV.y) + vec3(-0.5)) + offset);
	vec3 in_pos = fract(abs(pos));
	in_pos.x = pos.x >= 0 ? in_pos.x : 1 - in_pos.x;
	in_pos.y = pos.y >= 0 ? in_pos.y : 1 - in_pos.y;
	in_pos.z = pos.z >= 0 ? in_pos.z : 1 - in_pos.z;
	ivec3 ex_pos = ivec3(floor(pos)) + seed;
	
	vec3 g0, g1, g2, g3, g4, g5, g6, g7;

	ivec3 dx = ivec3(1,0,0);
	ivec3 dy = ivec3(0,1,0);
	ivec3 dz = ivec3(0,0,1);

	g0 = grad(ex_pos);
	g1 = grad(ex_pos+dx);
	g2 = grad(ex_pos+dy);
	g3 = grad(ex_pos+dx+dy);
	g4 = grad(ex_pos+dz);
	g5 = grad(ex_pos+dx+dz);
	g6 = grad(ex_pos+dy+dz);
	g7 = grad(ex_pos+dx+dy+dz);

	float d0, d1, d2, d3, d4, d5, d6, d7;

	d0 = dot(g0, in_pos);
	d1 = dot(g1, in_pos-vec3(dx));
	d2 = dot(g2, in_pos-vec3(dy));
	d3 = dot(g3, in_pos-vec3(dx)-vec3(dy));
	d4 = dot(g4, in_pos-vec3(dz));
	d5 = dot(g5, in_pos-vec3(dx)-vec3(dz));
	d6 = dot(g6, in_pos-vec3(dy)-vec3(dz));
	d7 = dot(g7, in_pos-vec3(dx)-vec3(dy)-vec3(dz));

	float x0, x1, x2, x3, y0, y1, z0;

	x0 = mix(d0, d1, fade(in_pos.x));
	x1 = mix(d2, d3, fade(in_pos.x));
	x2 = mix(d4, d5, fade(in_pos.x));
	x3 = mix(d6, d7, fade(in_pos.x));
	y0 = mix(x0, x1, fade(in_pos.y));
	y1 = mix(x2, x3, fade(in_pos.y));
	z0 = mix(y0, y1, fade(in_pos.z));

	height = 0.5f*texture(noise, UV).x + z0 / freq;
}