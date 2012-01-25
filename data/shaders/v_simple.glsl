precision highp float; 



//Provided in camera space
uniform vec3 lightPosition;

attribute vec3 a_vertex; 
attribute vec3 a_normal; 
attribute vec3 a_tangent; 
attribute vec2 a_texcoord0; 


varying vec3 lightVec; 
varying vec3 eyeVec;
varying vec2 v_texcoord; 
varying vec3 v_normal;


uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelViewMatrix;

void main(void) 
{

/*  
//#### BUMP_MAPPING
	//Move tangent space into camera space
	vec3 n = normalize(modelViewMatrix * vec4(a_normal ,1.0)).xyz;
	vec3 t = normalize(modelViewMatrix * vec4(a_tangent,1.0)).xyz;
	vec3 b = cross(n, t);
   
	vec3 vVertex = (modelViewMatrix * vec4(a_vertex,1.0)).xyz;
	
	
	vec3 tmpVec =  lightPosition - vVertex;
	lightVec.x = dot(tmpVec, t);
	lightVec.y = dot(tmpVec, b);
	lightVec.z = dot(tmpVec, n);

	tmpVec = -vVertex;
	eyeVec.x = dot(tmpVec, t);
	eyeVec.y = dot(tmpVec, b);
	eyeVec.z = dot(tmpVec, n);
*/


	vec3 vVertex = (modelViewMatrix * vec4(a_vertex,1.0)).xyz;

	lightVec = normalize(lightPosition - vVertex);
	v_normal = (modelViewMatrix * vec4(a_normal ,0.0)).xyz;



   // Pass through texture coordinate 
   v_texcoord = a_texcoord0.xy; 
      
      // Transform output position 
   gl_Position =  modelViewProjectionMatrix *    vec4(a_vertex,1.0) ;

}