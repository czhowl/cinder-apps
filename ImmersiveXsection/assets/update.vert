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
//uniform vec2 Tester;
uniform vec2 Mouse;
uniform float Click;
uniform float Sit;
uniform vec2 FishPos[30];

const vec3 green = vec3(0.4, 1.0, 0.0);
const vec3 red = vec3(3.0, 0.8, 0.2);
const vec3 blue = vec3(0.2, 0.8, 3.0);
const vec3 white = vec3(1.0, 1.0, 1.0);

void main() {
    
    // Update position & velocity for next frame
    Position = VertexPosition;
    Velocity = VertexVelocity;
    Color = VertexColor;
    EndPosition = VertexEndPosition;
    vec2 m = VertexTexCoord - Mouse;
//    Color = vec4(1.0);
//    Color += vec4(1.0);
    Color += (vec4(green, 1.0) - Color) * 0.05 * smoothstep(0.0, 0.1, length(m));
    Color += (vec4((red * (1.0 - Sit)) + (blue * Sit), 1.0) - Color) * smoothstep(0.1, 0.0, length(m)) * 0.05;
    
    vec3 acce;
    acce = (curlNoise(vec3((VertexPosition.x * 0.02), (VertexPosition.z * 0.02), Time * 0.005)) + 0) * VertexRandom * 0.1;
    
    acce += vec3(0.0,1000.0,-1000.0) * smoothstep(0.1, 0.0, length(m));
    acce += smoothstep(1.0, 0.0, Time - Click) * smoothstep(0.1, 0.0, length(m)) * vec3(m.x,0.0,m.y) * 100000.0;
    
    Velocity += acce * H;
    if(length(Velocity) > 200.0) Velocity = 200.0 * normalize(Velocity);
    EndPosition += Velocity * H;
    EndPosition = EndPosition * 5.0 + snoiseVec3(vec3(VertexPosition.xz, Time * (500/VertexRandom))) * 0.8 * length(vec2(EndPosition.x,EndPosition.z));
    
    for(int i = 0; i < 30; i++){
        vec2 dir = VertexTexCoord - (FishPos[i] + vec2(0.5));
        EndPosition += (1.0-smoothstep(0.0, 0.1, length(dir))) * vec3(dir.x, 0.0, dir.y) * length(vec2(EndPosition.x,EndPosition.z)) * 10.0;
    }
    
    if(length(EndPosition) > 6.0) EndPosition = 6.0 * normalize(EndPosition);
    EndPosition *= smoothstep(0.0, 5.0, EndPosition.y);
//    smoothstep(10.0, 0.0, Time - Click) *
}
