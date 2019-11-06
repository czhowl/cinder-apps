#version 330

uniform sampler2D uTex0;
//uniform mat4 ciModelViewProjection;
uniform mat4 ciModelView;

in vec4 ciPosition;
//in vec3 ciNormal;
in vec3 ciColor;
in vec2 ciTexCoord0;

//out vec3 vNormal;
//out vec4 vPosition;
out vec3 vColor;
out vec2 vTexCoord0;
out vec3 flowDir;

void main( void )
{
    flowDir = texture( uTex0, ciTexCoord0.st ).xyz;
    gl_Position = ciPosition;
//    gl_PointSize   = (flowDir.z + 1.0) * 30.0;
//    gl_PointSize   = 20.0;
    vColor          = ciColor;
    vTexCoord0      =ciTexCoord0;
//    vPosition = ciPosition;
}
