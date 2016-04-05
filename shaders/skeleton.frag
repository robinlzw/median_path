# version 440
uniform bool wireframe = true;

out vec4 final_color;

in vec3 fragment_vertex;
in vec4 fragment_color ;
flat in vec3 fragment_normal;
noperspective in vec3 dist;

// the light is on the camera for now 
const float shininess = 22.0;
const vec4 light_ambient  = vec4(0.1, 0.1, 0.1, 1);
const vec4 light_specular = vec4(1.0, 1.0, 1.0, 1);
const vec4 light_diffuse  = vec4(1.0, 1.0, 1.0, 1);

void main()
{
  vec3 fragment_to_camera = normalize( - fragment_vertex ); // in the camera frame, camera is at (0,0,0)
  vec3 fragment_to_light = fragment_to_camera; // light on the camera
  
  float diffuse_factor = abs( dot(fragment_normal, fragment_to_light));
  
  vec3 reflect_direction = reflect( -fragment_to_light, fragment_normal );
  
  
  float specular_factor = pow(abs(dot(fragment_to_camera,reflect_direction)), shininess);

  vec3 phong = 
        light_ambient.xyz 
     +  light_diffuse.xyz *  diffuse_factor
     + light_specular.xyz * specular_factor;
    
  if( wireframe )
  {
    float nearD = min(min(dist[0],dist[1]),dist[2]);
    float edge_intensity = exp2( -1.0 * nearD * nearD );
  
    final_color =  edge_intensity * vec4(0.1,0.1,0.1,1.0) 
    + (1.0 - edge_intensity) * vec4( phong, 1 )*fragment_color;
  }
  else
  {
    final_color = vec4(phong, 1)*fragment_color;
  }

  
}
