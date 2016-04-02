#version 400

in vec4 fragment_color;
out vec4 final_color;

void main()
{
  final_color = fragment_color;
}