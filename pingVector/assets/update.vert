#version 330

#include "util.glsl"

in vec3 VertexPosition;
in vec3 VertexEndPosition;
in vec3 VertexVelocity;
in vec4 VertexColor;
in float VertexRandom;
in vec2 VertexTexCoord;

out vec3 Position; // To Transform Feedback
out vec3 EndPosition; // To Transform Feedback
out vec3 Velocity; // To Transform Feedback
out vec4 Color; // To Transform Feedback


uniform float Time; // Time
uniform float H;    // Elapsed time between frames
uniform vec3 Accel; // Particle Acceleration
uniform float ParticleLifetime; // Particle lifespan
uniform vec2 Mouse;

void main() {
    
    // Update position & velocity for next frame
    Position = VertexPosition;
    Velocity = VertexVelocity;
    vec3 acce;
    
    
    acce = (curlNoise(vec3((VertexTexCoord.x * 1.5), (VertexTexCoord.y * 1.5), Time * 0.05)) + 0) * VertexRandom;
    vec2 dir = VertexTexCoord - Mouse;
//    acce += (1.0-smoothstep(0.0, 0.2, length(dir))) * vec3(dir.x, 0.0, dir.y) * 10000000.0;
//    acce = vec3(0);
    Velocity += acce * H;
    if(length(Velocity) > 200.0) Velocity = 200.0 * normalize(Velocity);
    EndPosition += Velocity * H;
    EndPosition = EndPosition * 2.0 + snoiseVec3(vec3(VertexPosition.xz, Time * (500/VertexRandom))) * 0.8 * length(vec2(EndPosition.x,EndPosition.z));
    EndPosition += (1.0-smoothstep(0.0, 0.1, length(dir))) * vec3(dir.x, 0.0, dir.y) * length(vec2(EndPosition.x,EndPosition.z)) * 10.0;
    if(length(EndPosition) > 10.0) EndPosition = 10.0 * normalize(EndPosition);
    
    Color = VertexColor;
}
