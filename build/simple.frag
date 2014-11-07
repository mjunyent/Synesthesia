#version 330 core

in vec2 UV;

layout(location = 0) out vec4 color;

uniform sampler2D tex;

void main(){
//	color = vec4(UV, 1.0, 1.0);
    color = texture(tex, UV);
    color.a = 1.0;
//    color.b = 1.0;
}