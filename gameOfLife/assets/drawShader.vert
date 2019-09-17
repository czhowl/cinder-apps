#version 150

uniform mat4	ciModelViewProjection;
uniform mat4	ciModelView;
uniform mat3	ciNormalMatrix;

in vec4 ciPosition;
in vec3 ciColor;
in vec2 ciTexCoord0;

out vec3 vColor;
out vec2 vTexCoord0;
out vec2 vPosition;

void main()
{
	vPosition = ciPosition.xy;
	vColor = ciColor;
	vTexCoord0 = ciTexCoord0;

	// simply pass position and vertex color on to the fragment shader
	gl_Position = ciModelViewProjection * ciPosition;
}
