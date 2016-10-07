#version 440
uniform bool use_atom_color = true;
uniform vec4 global_color = vec4(0, 0.2, 0.5, 1.0);

in vec4  ball_attribute;
in vec4 color_attribute;

out vec4 ball ;
out vec4 color;

void main()
{
  ball  =  ball_attribute;
  color = use_atom_color ? color_attribute : global_color;
}