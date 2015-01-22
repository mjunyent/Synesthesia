#version 330 core

in vec2 UV;

layout(location = 0) out vec4 color;

uniform sampler2DRect tex;
uniform vec3 mean;
uniform vec2 size;

void main(){
    if(UV.x*size.x < 20) color.xyz = mean;
    else color = texture(tex, UV*size-vec2(20,0));

//    if(UV.x*1920 > 1800) color.rgb = mean;

//    color = texelFetch(tex, UV);
    color.a = 1.0;
}