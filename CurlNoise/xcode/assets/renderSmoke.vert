#version 150 core

in vec3 VertexPosition;
in vec2 VertexStartTime;
in vec4 VertexColor;

out float agePct; // To Fragment Shader
out vec4  Pcolor;
uniform float MinParticleSize;
uniform float MaxParticleSize;

uniform float Time;
//uniform float ParticleLifetime;

uniform mat4 ciModelViewProjection;

void main() {
    Pcolor = VertexColor;
    float age = VertexStartTime.x;
//    Transp = 0.0;
    gl_Position = ciModelViewProjection * vec4(VertexPosition, 1.0);
    //    if( Time >= VertexStartTime ) {
    agePct = VertexStartTime.x / VertexStartTime.y;
//    Transp = 1.0 - agePct;
    //        gl_PointSize = mix( MinParticleSize, MaxParticleSize, Transp );
    gl_PointSize = mix( 0.0, 3.0, agePct );
//            gl_PointSize = 1.0;
    //    }
}
