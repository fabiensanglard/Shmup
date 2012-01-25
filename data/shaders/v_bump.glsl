precision highp float; 

uniform mat4 modelViewProjectionMatrix;
//Provided in model space
uniform vec3 lightPosition;

attribute vec3 a_vertex; 
attribute vec3 a_normal; 
attribute vec3 a_tangent; 
attribute vec2 a_texcoord0; 


varying vec3 lightVec; 
varying vec3 eyeVec;
varying vec2 v_texcoord; 
varying float distance;




void main(void) 
{

	vec3 a_bitangent = cross(a_normal,a_tangent);
	
	vec3 tmpVec =  lightPosition - a_vertex;
	
	distance = length(tmpVec);
	
	lightVec.x = dot(tmpVec, a_tangent);
	lightVec.y = dot(tmpVec, a_bitangent);
	lightVec.z = dot(tmpVec, a_normal);
	lightVec = normalize(lightVec);
	
/*
	tmpVec = -vVertex;
	eyeVec.x = dot(tmpVec, a_tangent);
	eyeVec.y = dot(tmpVec, a_bitangent);
	eyeVec.z = dot(tmpVec, a_normal);
*/

   // Pass through texture coordinate 
   v_texcoord = a_texcoord0.xy; 
      
      // Transform output position 
   gl_Position =  modelViewProjectionMatrix *    vec4(a_vertex,1.0) ;

}