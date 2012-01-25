precision highp float; 

uniform mat4 modelViewProjectionMatrix;
uniform mat4 lightPOVPVMMatrix;
//Provided in model space
uniform vec3 lightPosition;

//Provided in model space
											uniform vec3 cameraPosition;


attribute vec3 a_vertex; 
attribute vec3 a_normal; 
attribute vec3 a_tangent; 
attribute vec2 a_texcoord0; 


varying vec3 lightVec; 
varying vec3 halfVec;
varying vec2 v_texcoord; 
//varying float distance;
varying vec4 lightPOVPosition;



void main(void) 
{

	vec3 a_bitangent = cross(a_normal,a_tangent);
	
	vec3 tmpVec =  lightPosition - a_vertex  ;
	
	//distance = length(tmpVec);
	
	lightVec.x = dot(tmpVec, a_tangent);
	lightVec.y = dot(tmpVec, a_bitangent);
	lightVec.z = dot(tmpVec, a_normal);
	lightVec = normalize(lightVec);
	

	tmpVec = cameraPosition - a_vertex ;
	//tmpVec = normalize(tmpVec);	
	
	halfVec.x = dot(tmpVec, a_tangent);
	halfVec.y = dot(tmpVec, a_bitangent);
	halfVec.z = dot(tmpVec, a_normal);

	halfVec = normalize(halfVec);
	
	halfVec = (halfVec + lightVec) /2.0;

	halfVec = normalize(halfVec);
	
   // Pass through texture coordinate 
   v_texcoord = a_texcoord0.xy; 
      
      // Transform output position 
   gl_Position =   modelViewProjectionMatrix*    vec4(a_vertex,1.0) ;
   lightPOVPosition  = lightPOVPVMMatrix    *    vec4(a_vertex,1.0) ;
}