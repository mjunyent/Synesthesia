#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition;

// Output data ; will be interpolated for each fragment.
out vec2 UV;

void main(){
	gl_Position =  vec4(vertexPosition,1);
	UV = vec2(1+vertexPosition.x, 1-vertexPosition.y)/2.0;
}

