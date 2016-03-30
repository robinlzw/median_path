#version 440
uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

in vec4 atom ;
in vec4 color;

out vec3 camera_vertex;
out vec4 camera_color;

void main()
{
  camera_vertex = vec3(view * model * vec4( atom.xyz, 1.0 ));
  gl_Position = projection * vec4(camera_vertex,1.0);
  camera_color = color;
}