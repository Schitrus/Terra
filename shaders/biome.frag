#version 460 core

uniform vec2 offset;

uniform float size;

layout(binding = 0)uniform sampler2D noise;

in vec2 UV;

out float height;

void main(){

	vec2 pos = ((1.0f + 2.0f/size) * (UV + vec2(-0.5)) + offset);


	height = (abs(pos.y*pos.y) - 100.0f)/200.0f - (abs(pos.x)/50.0f) + (75.0f + abs(pos.y))/100.0f * (1000.0f + abs(pos.x*pos.x))/1000.0f * texture(noise, UV).x;
}