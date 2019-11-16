#version 330
#extension all : warn

#include "util.glsl"

//uniform sampler2D ParticleTex;
uniform float Time;
//in float Transp;
in VertexData{
    vec2 mTexCoord;
    float len;
    float width;
    float random;
    vec3 color;
//    vec3 mColor;
} VertexIn;
out vec4 FragColor;

const vec3 blue = vec3(0.1, 0.2, 0.8);
const vec3 green = vec3(0.4, 1.0, 0.3);

float circle(in vec2 _st, in float _radius){
    vec2 dist = _st-vec2(0.5);
    return 1.-smoothstep(_radius-(_radius*0.01),
                         _radius+(_radius*0.01),
                         dot(dist,dist)*4.0);
}

float box(in vec2 _st, in vec2 _size){
    _size = vec2(0.5) - _size*0.5;
    vec2 uv = smoothstep(_size,
                        _size+vec2(0.001),
                        _st);
    uv *= smoothstep(_size,
                    _size+vec2(0.001),
                    vec2(1.0)-_st);
    return uv.x*uv.y;
}

void main() {
    float ratio = VertexIn.len / VertexIn.width;
//    FragColor = texture( ParticleTex, gl_PointCoord );
//    FragColor = vec4((vec3(1 - FragColor.r, 1 - FragColor.g, 1 - FragColor.b) * pink), FragColor.a);
//    FragColor.a = 1.0;
//    FragColor.a -= .1;
//    float box = box(VertexIn.mTexCoord, vec2(1.0));
    float circle0 = circle(vec2((VertexIn.mTexCoord.x - 0.9) * 10.0, VertexIn.mTexCoord.y), 0.5);
    float box = box(vec2(VertexIn.mTexCoord.x + 0.05, VertexIn.mTexCoord.y), vec2(1.0));
    float circle = circle(vec2((VertexIn.mTexCoord.x - 0.9) * 10.0, VertexIn.mTexCoord.y), 1.0);
    float shape = max(box, circle);
    FragColor = vec4(mix(blue, VertexIn.color, VertexIn.len/12.5)
                     + circle0 * vec3(sin(Time*VertexIn.random) * 0.5 + 0.5),
                     0.9 * VertexIn.mTexCoord.x)* shape;
}
