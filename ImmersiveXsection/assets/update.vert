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
uniform vec2 ChairA;
uniform float ClickA;
uniform float SitA;
uniform vec2 ChairB;
uniform float ClickB;
uniform float SitB;
uniform vec2 FishPos[50];

const vec3 green = vec3(0.4, 1.0, 0.0);
const vec3 red = vec3(3.0, 0.8, 0.2);
const vec3 blue = vec3(0.0, 2.8, 0.2);
const vec3 white = vec3(1.0, 1.0, 1.0);

void main() {
    
    // Update position & velocity for next frame
    Position = VertexPosition;
    Velocity = VertexVelocity;
    Color = VertexColor;
    EndPosition = VertexEndPosition;
    vec2 mA = VertexTexCoord - ChairA;
    vec2 mB = VertexTexCoord - ChairB;
//    Color = vec4(1.0);
//    Color += vec4(1.0);
    Color += (vec4(green, 1.0) - Color) * 0.05 * smoothstep(0.0, 0.1, length(mA));
    Color += (vec4((red * (1.0 - SitA)) + (blue * SitA), 1.0) - Color) * smoothstep(0.1, 0.0, length(mA)) * 0.05;
    
    Color += (vec4(green, 1.0) - Color) * 0.05 * smoothstep(0.0, 0.1, length(mB));
    Color += (vec4((red * (1.0 - SitB)) + (blue * SitB), 1.0) - Color) * smoothstep(0.1, 0.0, length(mB)) * 0.05;

    
    vec3 acce;
    float multiplier = 2.0 / ( 2.0 * PI );
    float nx = cos( VertexPosition.x * 0.02 * PI ) * multiplier;
    float ny = VertexPosition.z * 0.02;
    float nz = Time * 0.05 + 100;

//    acce = (curlNoise(vec3(nx, ny, nz)) + 0) * VertexRandom * 0.1;
    acce = (curlNoise(vec3((VertexPosition.x * 0.02), (VertexPosition.z * 0.02), Time * 0.005)) + 0) * VertexRandom * 0.1;
//    acce = (curlNoise(vec4((VertexPosition.x * 0.02), (VertexPosition.z * 0.02), Time * 0.005, 0).xyz) + 0) * VertexRandom * 0.1;
    
    
    acce += vec3(0.0,1000.0,-1000.0) * smoothstep(0.1, 0.0, length(mA));
    acce += smoothstep(1.0, 0.0, Time - ClickA) * smoothstep(0.1, 0.0, length(mA)) * vec3(mA.x,0.0,mA.y) * 100000.0;
    
    acce += vec3(0.0,1000.0,-1000.0) * smoothstep(0.1, 0.0, length(mB));
    acce += smoothstep(1.0, 0.0, Time - ClickB) * smoothstep(0.1, 0.0, length(mB)) * vec3(mB.x,0.0,mB.y) * 100000.0;
    
    Velocity += acce * H;
    if(length(Velocity) > 200.0) Velocity = 200.0 * normalize(Velocity);
    EndPosition += Velocity * H;
    EndPosition = EndPosition * 5.0 + snoiseVec3(vec3(VertexPosition.xz, Time * (500/VertexRandom))) * 0.8 * length(vec2(EndPosition.x,EndPosition.z));
    
    vec2 coord = vec2(VertexTexCoord.x, VertexTexCoord.y);
    for(int i = 0; i < 50; i++){
        vec2 dir = VertexTexCoord - (FishPos[i] + vec2(0.5));
        EndPosition += (1.0-smoothstep(0.0, 0.03, length(dir))) * vec3(dir.x, 0.0, dir.y) * length(vec2(EndPosition.x,EndPosition.z)) * 10.0;
    }
    
    if(length(EndPosition) > 6.0) EndPosition = 6.0 * normalize(EndPosition);
    EndPosition *= smoothstep(0.0, 5.0, EndPosition.y);
//    smoothstep(10.0, 0.0, Time - Click) *
}
