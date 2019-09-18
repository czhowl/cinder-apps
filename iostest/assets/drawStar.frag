#version 300 es

uniform sampler2D tex0;

uniform highp float time;

in highp vec2        vTexCoord0;
in highp vec4    vPos;
out highp vec4     oColor;

void main( void )
{
    highp vec2 ORIGIN = vec2(0.5, 0.5);
    highp vec2 coord = (vTexCoord0 - ORIGIN) * 2.0;
    
    const float kFallOff = 8.0;
    float opacity = clamp( pow( 1.0 - length( coord ), kFallOff ), 0.0, 1.0 );
    
    oColor = opacity * texture(tex0, coord) * vColor;
}
