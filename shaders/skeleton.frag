# version 440
out vec4 final_color;

in 
vec3 fragment_vertex;
in 
vec4 fragment_color ;
flat in 
vec3 fragment_normal;
noperspective in vec3 dist;

uniform vec3 light_position;// = vec4(0,0,0,1);
const vec4 light_ambient  = vec4(0.3, 0.3, 0.3, 1);
const vec4 light_specular = vec4(1.0, 1.0, 1.0, 1);
const vec4 light_diffuse  = vec4(1.0, 1.0, 1.0, 1);

void main()
{
  vec3 L = normalize( light_position - fragment_vertex );
  vec3 E = normalize( -fragment_vertex );
  vec3 R = -reflect( L, fragment_normal );

  vec3 i_ambient  = light_ambient.xyz;
  vec3 i_diffuse  = light_diffuse.xyz * abs( dot( fragment_normal, L ) );
  vec3 i_specular = light_specular.xyz * pow( abs( dot(R, E ) ), 5 );

  float nearD = min(min(dist[0],dist[1]),dist[2]);
  float edge_intensity = exp2( -1.0 * nearD * nearD );
  
  final_color =  edge_intensity * vec4(0.1,0.1,0.1,1.0) 
    + (1.0 - edge_intensity) * ( vec4(i_ambient + clamp( i_diffuse, 0, 1 ) + clamp( i_specular, 0, 1 ),1)*fragment_color);
}
