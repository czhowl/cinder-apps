#version 300 es

uniform mat4    ciModelViewProjection;

in vec4            ciPosition;
in vec4            ciColor;
in vec2             ciTexCoord0;
out lowp vec4       vPos;
out highp vec2        vTexCoord0;

void main( void )
{
    vTexCoord0 = ciTexCoord0;
    gl_Position    = ciModelViewProjection * ciPosition;
    gl_PointSize = 1.0;
    vPos = ciPosition;
}
