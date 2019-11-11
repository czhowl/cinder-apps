#version 330

in vec3 VertexPosition;
in vec3 VertexEndPosition;
in vec4 VertexColor;
in float VertexRandom;

out vec3 vEnd;
out float vRandom;
uniform float MinParticleSize;
uniform float MaxParticleSize;

uniform float Time;
uniform float ParticleLifetime;

uniform mat4 ciModelViewProjection;

void main() {
    vRandom = VertexRandom / 2000.0;
    vEnd = VertexEndPosition;
//    gl_Position = ciModelViewProjection * vec4(VertexPosition, 1.0);
    gl_Position = vec4(VertexPosition, 1.0);
//    if( Time >= VertexStartTime ) {
//        float agePct = age / ParticleLifetime;
//        Transp = 1.0 - agePct;
//        gl_PointSize = mix( MinParticleSize, MaxParticleSize, agePct );
//        gl_PointSize = 10.0;
//    }
}

