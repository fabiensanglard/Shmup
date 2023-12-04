precision highp float; 

uniform sampler2D s_baseMap;

varying  vec2 v_texcoord;

void main(void) 
{
   vec4  color = texture2D(s_baseMap, v_texcoord);
   gl_FragColor =  color  ;
}