precision highp float; 

uniform sampler2D s_baseMap;
uniform sampler2D s_bumpMap;

uniform vec3 lightColorAmbient;
uniform vec3 lightColorDiffuse;


varying vec3 lightVec; 
varying vec3 eyeVec;
varying vec2 v_texcoord; 
varying vec3 v_normal;

void main(void) 
{
	// Fetch basemap color 
	vec3  color = texture2D(s_baseMap, v_texcoord).rgb;
//	vec3 bump =  texture2D(s_bumpMap, v_texcoord).xyz * -2.0 + 1.0;
   
   float lamberFactor  = max( dot(lightVec, v_normal), 0.0 ) ;//max( dot(lightVec, bump), 0.0 );
   
   
   
	   // Output final color 
	vec3 ambientComponent = lightColorAmbient *color;
	vec3 diffuseComponent = lightColorDiffuse * color *  lamberFactor;
   gl_FragColor =  vec4(ambientComponent + diffuseComponent,1.0)  ;
}