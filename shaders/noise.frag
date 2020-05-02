#version 460 core

in vec2 UV;

out vec4 height;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main(){

	height = vec4(rand(UV));
}