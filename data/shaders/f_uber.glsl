precision mediump float; 

uniform sampler2D s_baseMap;

uniform vec3 lightColorAmbient;
uniform vec3 lightColorDiffuse;

uniform vec3 lightColorSpecular;
uniform float materialShininess;
uniform vec3 matColorSpecular;

varying vec3 lightVec; 
varying vec3 halfVec;
varying vec2 v_texcoord; 

#ifdef SHADO_MAPPING
	varying vec4 lightPOVPosition;
	uniform sampler2D s_shadowpMap;
#endif

#ifdef BUMP_MAPPING
	uniform sampler2D s_bumpMap;
#else
	varying vec3 v_normal ;
#endif

#ifdef SPEC_MAPPING
	uniform sampler2D s_specularMap;
#endif


#ifdef SHADO_MAPPING
float unpack(vec4 packedZValue)
{
	//const vec4 multCoef = vec4(1.0,1.0/255.0, 1.0/(255.0 * 255.0),1.0/(255.0 * 255.0 * 255.0));
	//return dot(packedZValue,multCoef);
	//return packedZValue.r;
	
	
	const vec4 unpackFactors = vec4( 1.0 / (256.0 * 256.0 * 256.0), 1.0 / (256.0 * 256.0), 1.0 / 256.0, 1.0 );
	return dot(packedZValue,unpackFactors);
}

float getShadowFactor(vec4 lightZ)
{

	vec4 packedZValue = texture2D(s_shadowpMap, lightZ.st);

	
	float unpackedZValue = unpack(packedZValue);

	return float((unpackedZValue+0.0001) > lightZ.z);
}
#endif



void main(void) 
{
	
	vec3 ambientComponent;
	vec3 diffuseComponent= vec3(0.0);
	vec3 specularComponent= vec3(0.0);
	
	float lamberFactor =0.0;
	float specularFactor=0.0;
	float shadowFactor=1.0;
	
	vec3 bump ;
	
	vec4 colorSample  = texture2D(s_baseMap, v_texcoord) ;
	vec3 color = colorSample.rgb;
	float alpha = colorSample.a ;
	
#ifdef BUMP_MAPPING
		bump		=  texture2D(s_bumpMap, v_texcoord).rgb * 2.0 - 1.0;
		lamberFactor  =  max(0.0,dot(lightVec, bump) );
		specularFactor = max(0.0,pow(dot(halfVec,bump),materialShininess)) ;
#else
		lamberFactor  =  max(0.0,dot(lightVec, v_normal) );
		specularFactor = max(0.0,pow(dot(halfVec,v_normal),materialShininess)) ;
		
#endif
	
#ifdef SPEC_MAPPING
		vec3 matTextColor = texture2D(s_specularMap, v_texcoord).rgb; 
#else
		vec3 matTextColor = matColorSpecular;
#endif
	
	ambientComponent  = lightColorAmbient  * color ;
	diffuseComponent  = lightColorDiffuse  * color *  lamberFactor;
	specularComponent = lightColorSpecular * matTextColor * specularFactor;
		
	
#ifdef SHADO_MAPPING
		vec4 lightZ = lightPOVPosition / lightPOVPosition.w;
		lightZ = (lightZ + 1.0) /2.0;
		shadowFactor = getShadowFactor(lightZ);
		gl_FragColor = vec4( ambientComponent + (diffuseComponent + specularComponent) *shadowFactor ,colorSample.a)  ; 
#else
		gl_FragColor = vec4( ambientComponent + (diffuseComponent + specularComponent) ,colorSample.a)  ; 
#endif
	

	
	
	
	// 
	//gl_FragColor = vec4(vec3(matTextColor),1.0);
	//gl_FragColor = vec4(halfVec,1.0);
}