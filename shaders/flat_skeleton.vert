#version 400
uniform mat4 mvp;
uniform bool use_atom_color = true;
uniform vec4 global_color = vec4(0, 0.2, 0.5, 1.0);

in vec4 atom;
in vec4 color;

out vec4 fragment_color;

void main()
{
  gl_Position = mvp * vec4( atom.xyz, 1.0 );
  fragment_color = use_atom_color ? color : global_color;
}