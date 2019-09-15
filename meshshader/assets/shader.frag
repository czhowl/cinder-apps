#version 150

in vec4		vertPosition;
in vec3		vertNormal;
in vec4		vertColor;
in vec4		vertColorBack;
in vec4     vertUp;

out vec4	fragColor;

void main()
{	
	const vec4  kAmbient = vec4(0.1, 0.1, 0.1, 1);
	const vec4	kSpecular = vec4(0.7, 0.7, 0.4, 1);
	const float kShininess = 50.0;
	
	vec3 N = normalize( vertNormal );
	vec3 L = normalize( -vertPosition.xyz );   
	vec3 E = normalize( -vertPosition.xyz ); 
	vec3 H = normalize( L + E ); 
	vec4 diffuse = vec4(1.0);
	// ambient term
	vec4 ambient = kAmbient;
	if (gl_FrontFacing){
		diffuse = vertColor;
	}else{
		diffuse = vertColorBack;
		//diffuse = vertColor;
		//N *= -0.7;
	}
	// diffuse term

	diffuse *= max( dot( N, L ), 0.0 );    

	// specular term
	vec4 specular = kSpecular; 
	specular *= pow( max( dot( N, H ), 0.0 ), kShininess );

	// reflection term (fake ground reflection)
	float f = max( 0.0, dot( -N, normalize( vertUp.xyz ) ) );
	vec4 reflection = vec4(f, f, f, 1.0) * kSpecular;

	// final color 
	fragColor = ambient + reflection + diffuse + specular;
}