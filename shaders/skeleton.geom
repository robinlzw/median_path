# version 440
uniform vec2 window_dimensions;

layout( triangles ) in;
in vec3 camera_vertex[];
in vec4 camera_color[];

layout( triangle_strip, max_vertices = 3 ) out;
out 
vec3 fragment_vertex;
out 
vec4 fragment_color;
flat out 
vec3 fragment_normal;
noperspective out vec3 dist;

void main(void)
{
  vec2 p0 = window_dimensions * gl_in[0].gl_Position.xy / gl_in[0].gl_Position.w;
  vec2 p1 = window_dimensions * gl_in[1].gl_Position.xy / gl_in[1].gl_Position.w;
  vec2 p2 = window_dimensions * gl_in[2].gl_Position.xy / gl_in[2].gl_Position.w;

  vec2 v0 = p2 - p1;
  vec2 v1 = p2 - p0;
  vec2 v2 = p1 - p0;
  float area = abs(v1.x*v2.y - v1.y * v2.x);
  
  dist = vec3( area / length(v0), 0, 0 );
  fragment_vertex = camera_vertex[0];
	fragment_color = camera_color[0];
	fragment_normal = normalize(cross(camera_vertex[1] - camera_vertex[0], camera_vertex[2] - camera_vertex[0] ));
  gl_Position = gl_in[0].gl_Position;
  EmitVertex();  

  dist = vec3( 0, area / length(v1), 0 );
  fragment_vertex = camera_vertex[1];
	fragment_color = camera_color[1];
  gl_Position = gl_in[1].gl_Position;
  EmitVertex();
  
  dist = vec3( 0, 0, area / length(v2) );
  fragment_vertex = camera_vertex[2];
	fragment_color = camera_color[2];
  gl_Position = gl_in[2].gl_Position;
  EmitVertex();
  
  EndPrimitive();  
}