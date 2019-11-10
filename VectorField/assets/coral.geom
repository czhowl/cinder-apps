#version 330

layout (points) in;
layout (line_strip, max_vertices = 2) out;

uniform mat4 ciModelViewProjection;

in vec3 flowDir[];
out vec3 gFlow;

void main() {
    gFlow = flowDir[0];
    gl_Position = ciModelViewProjection * gl_in[0].gl_Position;
    EmitVertex();

    gl_Position = ciModelViewProjection * (gl_in[0].gl_Position + vec4(gFlow.x, 0.0, gFlow.y, 0.0) * 5.0);
    EmitVertex();
    
    EndPrimitive();
}
