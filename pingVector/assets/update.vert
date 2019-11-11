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
uniform vec2 Tester;
uniform vec2 Mouse;
uniform float Click;

void main() {
    
    // Update position & velocity for next frame
    Position = VertexPosition;
    Velocity = VertexVelocity;
    vec3 acce;
    
    
    acce = (curlNoise(vec3((VertexPosition.x * 0.02), (VertexPosition.z * 0.02), Time * 0.02)) + 0) * VertexRandom * 0.1;
    
    vec2 m = VertexTexCoord - Mouse;
//    acce += smoothstep(10.0, 0.0, Time - Click) *
    acce += smoothstep(10.0, 0.0, Time - Click) * vec3(0.0,1000.0,0.0) * smoothstep(0.1, 0.0, length(m));
    vec2 dir = VertexTexCoord - Tester;

    Velocity += acce * H;
    if(length(Velocity) > 200.0) Velocity = 200.0 * normalize(Velocity);
    EndPosition += Velocity * H;
    EndPosition = EndPosition * 2.0 + snoiseVec3(vec3(VertexPosition.xz, Time * (500/VertexRandom))) * 0.8 * length(vec2(EndPosition.x,EndPosition.z));
    EndPosition += (1.0-smoothstep(0.0, 0.1, length(dir))) * vec3(dir.x, 0.0, dir.y) * length(vec2(EndPosition.x,EndPosition.z)) * 10.0;
    if(length(EndPosition) > 10.0) EndPosition = 10.0 * normalize(EndPosition);
    EndPosition *= smoothstep(0.0, 5.0, EndPosition.y);
    
    Color = VertexColor;
}
