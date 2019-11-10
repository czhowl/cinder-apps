#version 330

layout (points) in;
//layout (points, max_vertices = 1) out;
layout (line_strip, max_vertices = 2) out;

uniform mat4 ciModelViewProjection;

in vec3 vEnd[];
//in float agePct[];
//in vec4  Pcolor[];
//out float age;
//out vec4  color;
//out vec3 gEnd;

void main() {
//    color = Pcolor[0];
//    age = agePct[0];
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + ciModelViewProjection * vec4(vEnd[0], 0.0);
    EmitVertex();
    
    EndPrimitive();
}
