#version 150

uniform sampler2DRect uLifeTex;

in vec2		vTexCoord0;
in vec2 	vPosition;
//in vec4		vertPosition;
//in vec3		vertNormal;
//in vec4		vertColor;
//in vec4		vertColorBack;
//in vec4     vertUp;
//in vec3		viewDir;

out vec4	oColor;

void main()
{	
	// retrieve normal from texture
	vec4 smpColor = texture( uLifeTex, vPosition );

	// output color
	oColor = smpColor;
	//oColor = vec4(1.0,vPosition.x,vPosition.y,1.0);
}