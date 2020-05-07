#version 460 core

layout (vertices = 4) out;

uniform float res;

in vec2 tescUV[];

out vec2 teseUV[];

void main(){
	if(gl_InvocationID == 0){
		gl_TessLevelInner[0] = res;
		gl_TessLevelInner[1] = res;
		gl_TessLevelOuter[0] = res;
		gl_TessLevelOuter[1] = res;
		gl_TessLevelOuter[2] = res;
		gl_TessLevelOuter[3] = res;
	}
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

	teseUV[gl_InvocationID] = tescUV[gl_InvocationID];
}