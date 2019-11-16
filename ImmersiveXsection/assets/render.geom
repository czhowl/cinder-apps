#version 330

layout (points) in;
//layout (points, max_vertices = 1) out;
layout (triangle_strip, max_vertices = 6) out;
//layout (line_strip, max_vertices = 2) out;

uniform mat4 ciModelViewProjection;

in vec3 vEnd[];
in float vRandom[];
in vec3 vColor[];
//in float agePct[];
//in vec4  Pcolor[];
out VertexData{
    vec2 mTexCoord;
    float len;
    float width;
    float random;
    vec3 color;
//    vec3 mColor;
} VertexOut;
//out vec4  color;
//out vec3 gEnd;

void main() {
    float width = 0.1;
    
    vec4 pos = gl_in[0].gl_Position;
    vec4 pos1 = pos - vec4(-vEnd[0].z, 0.0, vEnd[0].x, 0.0) * (width / 2);
    vec4 pos2 = pos + vec4(-vEnd[0].z, 0.0, vEnd[0].x, 0.0) * (width / 2);
    vec4 pos3 = pos + vec4(vEnd[0].x, 0.0, vEnd[0].z, 0.0) - vec4(-vEnd[0].z, 0.0, vEnd[0].x, 0.0) * (width / 2);
    vec4 pos4 = pos + vec4(vEnd[0].x, 0.0, vEnd[0].z, 0.0) + vec4(-vEnd[0].z, 0.0, vEnd[0].x, 0.0) * (width / 2);
    VertexOut.len = length(vec4(vEnd[0].x, 0.0, vEnd[0].z, 0.0));
    VertexOut.width = width;
    VertexOut.random = 0.4 + vRandom[0];
    VertexOut.color = vColor[0];
    VertexOut.mTexCoord = vec2( 0, 0 );
    gl_Position = ciModelViewProjection * pos1;
    EmitVertex();
    
    VertexOut.mTexCoord = vec2( 0, 1 );
    gl_Position = ciModelViewProjection * pos2;
    EmitVertex();
    
    VertexOut.mTexCoord = vec2( 1, 0 );
    gl_Position = ciModelViewProjection * pos3;
    EmitVertex();
    
    VertexOut.mTexCoord = vec2( 0, 1 );
    gl_Position = ciModelViewProjection * pos2;
    EmitVertex();
    
    VertexOut.mTexCoord = vec2( 1, 0 );
    gl_Position = ciModelViewProjection * pos3;
    EmitVertex();
    
    VertexOut.mTexCoord = vec2( 1, 1 );
    gl_Position = ciModelViewProjection * pos4;
    EmitVertex();
    
    EndPrimitive();
}
