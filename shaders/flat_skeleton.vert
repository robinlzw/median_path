#version 400

uniform mat4 mvp;

in vec4 atom;
in vec4 color;

out vec4 fragment_color;

void main()
{
  gl_Position = mvp * vec4( atom.xyz, 1.0 );
  fragment_color = color;
}