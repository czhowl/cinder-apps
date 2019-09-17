#version 150

uniform sampler2DRect uLifeTex;

in vec2		vTexCoord0;
in vec2 	vPosition;

out vec4	oColor;

void main()
{	
	// retrieve normal from texture
	vec4 smpColor = texture( uLifeTex, vPosition );

	// output color
    oColor = smpColor;
//    oColor = vec4(1.0,vTexCoord0.x,vTexCoord0.y,1.0);
}
