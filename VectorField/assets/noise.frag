#version 330

#include "util.glsl"

uniform float        uTime;

in vec2 vTexCoord0;

out vec4 oColor;

void main() {

    // output pixel color
    oColor = vec4( curlNoise(vec3(vTexCoord0.x, vTexCoord0.y, uTime * 0.1)), 1.0 );
//    oColor = vec4( snoiseVec3(vec3(vTexCoord0.x, vTexCoord0.y, uTime * 0.1)), 1.0 );
}
