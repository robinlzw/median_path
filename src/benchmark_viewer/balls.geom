#version 440
uniform mat4 projection;
uniform mat4 mv;

//**********
//* INPUTS *
//******************************************************************************
layout( points ) in;
in vec4 ball [];
in vec4 color[];


//***********
//* OUTPUTS *
//******************************************************************************
layout( triangle_strip, max_vertices = 4 ) out;
smooth out vec3 fragment_position;
out vec4 diffuse_color;
// contains:
//   in xyz: the ball center in camera coordinates
//   in w: 4 times ( squared distance to camera - squared radius ) 
out vec4 data; 

//todo: build formulae to have a tight fitting quad and check if it is faster
const float box_correction = 1.5;

void main()
{
  data.xyz = (mv * vec4( ball[0].xyz, 1.0 )).xyz;
  data.w = 4.0 * ( dot( data.xyz, data.xyz ) - ball[0].w * ball[0].w );
  diffuse_color = color[0];

  float scale = ball[0].w * box_correction;
  
  fragment_position = data.xyz + vec3( -scale, -scale, 0 );
  gl_Position = projection * vec4( fragment_position, 1.0 );
  EmitVertex();
  
  fragment_position = data.xyz + vec3(  scale, -scale, 0 );
  gl_Position = projection * vec4( fragment_position, 1.0 );
  EmitVertex();  
  
  fragment_position = data.xyz + vec3( -scale,  scale, 0 );
  gl_Position = projection * vec4( fragment_position, 1.0 );
  EmitVertex();
  
  fragment_position = data.xyz + vec3(  scale,  scale, 0 );
  gl_Position = projection * vec4( fragment_position, 1.0 );
  EmitVertex();

  EndPrimitive();
  
    // discard fragment with .z < 0 ?
}