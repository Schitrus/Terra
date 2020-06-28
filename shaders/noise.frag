#version 460 core

uniform ivec2 seed;

uniform float freq;

uniform vec2 offset;

uniform float size;

layout(binding = 0)uniform sampler2D noise;

layout(binding = 1)uniform sampler1D gradient;

in vec2 UV;

out float height;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
 
vec2 grad(ivec2 pos){
	int p = (pos.x*pos.x + pos.y*pos.y)%1024;
	return texelFetch(gradient, p, 0).xy;
}

float fade(float x){
	return x * x * x * (x * (x * 6 - 15) + 10);
}

void main(){

	vec2 pos = freq * ((1.0f + 2.0f/size) * (UV + vec2(-0.5)) + offset);
	vec2 in_pos = fract(abs(pos));
	in_pos.x = pos.x >= 0 ? in_pos.x : 1 - in_pos.x;
	in_pos.y = pos.y >= 0 ? in_pos.y : 1 - in_pos.y;
	ivec2 ex_pos = ivec2(floor(pos)) + seed;
	
	vec2 g0, g1, g2, g3;

	ivec2 dx = ivec2(1,0);
	ivec2 dy = ivec2(0,1);

	g0 = grad(ex_pos);
	g1 = grad(ex_pos+dx);
	g2 = grad(ex_pos+dy);
	g3 = grad(ex_pos+dx+dy);

	float d0, d1, d2 ,d3;

	d0 = dot(g0, in_pos);
	d1 = dot(g1, in_pos-vec2(dx));
	d2 = dot(g2, in_pos-vec2(dy));
	d3 = dot(g3, in_pos-vec2(dx)-vec2(dy));

	float x0, x1, y0;

	x0 = mix(d0, d1, fade(in_pos.x));
	x1 = mix(d2, d3, fade(in_pos.x));
	y0 = mix(x0, x1, fade(in_pos.y));

	height = texture(noise, UV).x + y0 / freq;
}