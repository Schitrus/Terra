#version 460 core

layout (vertices = 4) out;

in vec2 tescUV[];

out vec2 teseUV[];

void main(){
	if(gl_InvocationID == 0){
		gl_TessLevelInner[0] = 256.0;
		gl_TessLevelInner[1] = 256.0;
		gl_TessLevelOuter[0] = 256.0;
		gl_TessLevelOuter[1] = 256.0;
		gl_TessLevelOuter[2] = 256.0;
		gl_TessLevelOuter[3] = 256.0;
	}
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

	teseUV[gl_InvocationID] = tescUV[gl_InvocationID];
}