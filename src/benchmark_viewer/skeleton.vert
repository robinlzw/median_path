#version 440
uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;
uniform bool grayscale;
uniform bool use_atom_color = true;
uniform vec4 global_color = vec4(0, 0.2, 0.5, 1.0);

in vec4 atom ;
in vec4 color;

out vec3 camera_vertex;
out vec4 camera_color;

void main()
{
  camera_vertex = vec3(view * model * vec4( atom.xyz, 1.0 ));
  gl_Position = projection * vec4(camera_vertex,1.0);
  
  if( use_atom_color )
  {
	  camera_color = color;
	  if( grayscale )
	  {
	    float luminance = 0.2126 * camera_color.r + 0.7152 * camera_color.g + 0.0722 * camera_color.b;
	    camera_color.r = luminance;
	    camera_color.g = luminance;
	    camera_color.b = luminance;
	  }
  }
  else
    camera_color = global_color;
}