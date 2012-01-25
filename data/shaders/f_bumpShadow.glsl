precision highp float; 

uniform sampler2D s_baseMap;
uniform sampler2D s_bumpMap;
uniform sampler2D s_shadowpMap;
											uniform sampler2D s_specularMap;

uniform vec3 lightColorAmbient;
uniform vec3 lightColorDiffuse;

uniform vec3 lightColorSpecular;
uniform float materialShininess;
											uniform vec3 matColorSpecular;
varying vec3 lightVec; 
varying vec3 halfVec;
varying vec2 v_texcoord; 
//varying float distance;
varying vec4 lightPOVPosition;

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

	return float(unpackedZValue > lightZ.z);
}

void main(void) 
{
	
	vec3 ambientComponent;
	vec3 diffuseComponent= vec3(0.0);
	vec3 specularComponent= vec3(0.0);
	
	float lamberFactor ;
	float specularFactor=0.0;
	float shadowFactor=1.0;
	
	vec3 color = texture2D(s_baseMap, v_texcoord).rgb;
	vec3 bump =  texture2D(s_bumpMap, v_texcoord).rgb * 2.0 - 1.0;
	vec3 matTextColor = texture2D(s_specularMap, v_texcoord).rgb; 

   ambientComponent  = lightColorAmbient  * color ;


   lamberFactor  =  max(0.0,dot(lightVec, bump) );

	specularFactor = max(0.0,pow(dot(halfVec,bump),materialShininess)) ;
   //if (lamberFactor >= 0.0)
   //{
		
		diffuseComponent  = lightColorDiffuse  * color *  lamberFactor;
		specularComponent = lightColorSpecular * matColorSpecular * specularFactor;
		//specularComponent = lightColorSpecular * matTextColor * specularFactor;
		
		
		vec4 lightZ = lightPOVPosition / lightPOVPosition.w;
		lightZ = (lightZ + 1.0) /2.0;
		shadowFactor = getShadowFactor(lightZ);
   //}
	

	gl_FragColor = vec4( ambientComponent + (diffuseComponent + specularComponent) *shadowFactor ,1.0)  ; //
	//gl_FragColor = vec4(vec3(color),1.0);
}