#version 150 core

in vec3 VertexPosition;
in float VertexStartTime;
in vec4 VertexColor;

out float Transp; // To Fragment Shader
out vec4  Pcolor;
uniform float MinParticleSize;
uniform float MaxParticleSize;

uniform float Time;
uniform float ParticleLifetime;

uniform mat4 ciModelViewProjection;

void main() {
    Pcolor = VertexColor;
    float age = Time - VertexStartTime;
    Transp = 0.0;
    gl_Position = ciModelViewProjection * vec4(VertexPosition, 1.0);
    if( Time >= VertexStartTime ) {
        float agePct = age / ParticleLifetime;
        Transp = 1.0 - agePct;
        gl_PointSize = mix( MinParticleSize, MaxParticleSize, Transp );
        gl_PointSize = mix( 1.0, 5.0, Transp );
//        gl_PointSize = 1.0;
    }
}
