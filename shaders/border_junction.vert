#version 400

uniform mat4 mvp;
uniform vec4 color;

in vec4 atom;
out vec4 fragment_color;

void main()
{
  gl_Position = mvp * vec4( atom.xyz, 1.0 );
  fragment_color = color;
}