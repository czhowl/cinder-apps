#version 330

layout (points) in;
//layout (points, max_vertices = 1) out;
layout (line_strip, max_vertices = 2) out;

uniform mat4 ciModelViewProjection;

in float agePct[];
in vec4  Pcolor[];
out float age;
out vec4  color;

void main() {
    color = Pcolor[0];
    age = agePct[0];
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + ciModelViewProjection * vec4(5.0, 0.0, 5.0, 0.0);
    EmitVertex();
    
    EndPrimitive();
}
