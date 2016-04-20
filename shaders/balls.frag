#version 440
uniform mat4 projection;
smooth in vec3 fragment_position;
in vec4 diffuse_color;
// contains:
//   in xyz: the ball center in camera coordinates
//   in w: 4 times ( squared distance to camera - squared radius ) 
in vec4 data;

out vec4 final_color;


// the light is on the camera for now 
const float shininess = 22.0;
const vec3 light_ambient  = vec3(0.1, 0.1, 0.1 );
const vec3 light_specular = vec3(1.0, 1.0, 1.0 );
const vec3 light_diffuse  = vec3(1.0, 1.0, 1.0 );

vec3 phong( vec3 surfel_position, vec3 surfel_normal, vec3 surfel_color )
{
  vec3 surfel_to_camera  = normalize( -surfel_position);
  vec3 surfel_to_light   = surfel_to_camera;
  vec3 reflect_direction = reflect( -surfel_to_light, surfel_normal ); 
  
  float diffuse_factor  = max( dot( surfel_normal, surfel_to_light ), 0.0 );
  float specular_factor = pow( max( 0.0, dot( surfel_to_camera, reflect_direction )), shininess );
  
  return (light_ambient + light_diffuse * diffuse_factor + light_specular * specular_factor)*surfel_color;
}

void main()
{
  vec3 ray_direction = normalize( fragment_position );
  
  float B = - 2.0 * dot( ray_direction, data.xyz );
  float det = B * B - data.w;
  if( det < 0.0 )
  	discard;
  det = sqrt( det );
  
  // the first intersection if for the minimum t solution
  // such minimum is min( -B + det, -B - det ) * 0.5
  vec3 sphere_position = ray_direction * min( -B + det, -B - det ) * 0.5;
  vec3 fragment_normal = normalize( sphere_position - data.xyz );
  
  // the depth of the fragment has been updated. Indeed, it does not lie on the
  // quad anymore, but on the ball's surface
  vec4 point_clip = projection * vec4( sphere_position, 1.0);
  float ndcDepth = point_clip.z / point_clip.w;
  gl_FragDepth = ((gl_DepthRange.diff * ndcDepth) + gl_DepthRange.near + gl_DepthRange.far) * 0.5;
  
  final_color = vec4( phong( sphere_position, fragment_normal, diffuse_color.xyz ), 1.0 );
}