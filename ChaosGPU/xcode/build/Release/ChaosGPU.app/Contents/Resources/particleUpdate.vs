#version 150 core

uniform float  uDeltaTime;
uniform float  uConstantA;
uniform float  uConstantB;
uniform float  uConstantC;

in vec3   iPosition;
in vec4   iColor;

out vec3  position;
out vec4  color;

void main()
{
    position =  iPosition;
    //color =     iColor;
    
    float x = position.x;
    float y = position.y;
    float z = position.z;
    float dx = (uConstantA * x - y * z) * uDeltaTime;
    float dy = (uConstantB * y + x * z) * uDeltaTime;
    float dz = (uConstantC * z + x * y) * uDeltaTime;
    position += vec3(dx, dy, dz);
    color = vec4(dx + 0.5f, dy + 0.1f, dz + 0.3f, 1.0f);
    if(length(position) > 100.0f){
        position = vec3(1.0f, 0.0f, 4.5f);
    }
}
