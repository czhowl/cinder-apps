#version 330

#include "util.glsl"

in vec3 VertexPosition;
in vec3 VertexEndPosition;
in vec3 VertexVelocity;
in vec4 VertexColor;
in float VertexRandom;

out vec3 Position; // To Transform Feedback
out vec3 EndPosition; // To Transform Feedback
out vec3 Velocity; // To Transform Feedback
out vec4 Color; // To Transform Feedback


uniform float Time; // Time
uniform float H;    // Elapsed time between frames
uniform vec3 Accel; // Particle Acceleration
uniform float ParticleLifetime; // Particle lifespan

void main() {
    
    // Update position & velocity for next frame
    Position = VertexPosition;
    Velocity = VertexVelocity;
    vec3 acce;
    acce = (curlNoise(vec3((VertexPosition.x) * 0.02, (VertexPosition.z) * 0.02, Time * 0.05)) + 0) * VertexRandom;
    Velocity += acce * H;
    if(length(Velocity) > 100.0) Velocity = 100.0 * normalize(Velocity);
    EndPosition += Velocity * H;
    EndPosition = EndPosition * 2.0 + snoiseVec3(vec3(VertexPosition.xz, Time * (500/VertexRandom))) * 0.5 * length(EndPosition);
//    if(length(EndPosition) > 100.0) EndPosition = 100.0 * normalize(EndPosition);
    Color = VertexColor;
}
