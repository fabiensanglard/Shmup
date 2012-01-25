precision highp float; 

attribute vec3 a_vertex; 
varying vec4 position;

uniform mat4 modelViewProjectionMatrix ;

void main(void) 
{   
   gl_Position =  modelViewProjectionMatrix *    vec4(a_vertex,1.0) ;
   position = gl_Position;
   
}