#version 300 es

uniform sampler2D tex0;
uniform sampler2D tex1;

uniform highp float time;

in highp vec2        vTexCoord0;
in highp vec4    vPos;
out highp vec4     oColor;

void main( void )
{
    highp vec2 ORIGIN = vec2(0.5, 0.5);
    highp vec2 coord = vTexCoord0 - ORIGIN;
    highp float c, s;
    c = cos(time);    s = sin(time);
    highp vec2 coord1 = vec2(coord.x * c - coord.y * s, coord.y * c + coord.x * s) + ORIGIN;
    c = cos(time * -0.7);    s = sin(time * -0.7);
    highp vec2 coord2 = vec2(coord.x * c - coord.y * s, coord.y * c + coord.x * s) + ORIGIN;
    
    highp vec4 corona = mix(texture(tex1, coord1), texture(tex1, coord2), 0.5) * vec4(1.0);
    
    // mix with star sample
    c = cos(time * 0.1);    s = sin(time * 0.1);
    highp vec2 coord3 = vec2(coord.x * c - coord.y * s, coord.y * c + coord.x * s) + ORIGIN;
    
    highp vec4 star = pow( texture(tex0, coord3), vec4( 2.0 ) ) * vec4(1.0);
    highp vec4 color = star + corona;
    oColor = vec4(vec3(1.0), color.x);
}
