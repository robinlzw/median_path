# version 440
uniform vec2 window_dimensions;
uniform bool wireframe = true;
uniform bool use_atom_color = true;
uniform vec4 global_color = vec4(0, 0.2, 0.5, 1.0);


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
  vec3 distances[3] = vec3[3]( vec3(0,0,0), vec3(0,0,0), vec3(0,0,0) );
  vec4 colors[3] = vec4[3]( global_color, global_color, global_color );
  if( wireframe )
    {
      vec2 p0 = window_dimensions * gl_in[0].gl_Position.xy / gl_in[0].gl_Position.w;
	  vec2 p1 = window_dimensions * gl_in[1].gl_Position.xy / gl_in[1].gl_Position.w;
	  vec2 p2 = window_dimensions * gl_in[2].gl_Position.xy / gl_in[2].gl_Position.w;
	
	  vec2 v0 = p2 - p1;
	  vec2 v1 = p2 - p0;
	  vec2 v2 = p1 - p0;
	  float area = abs(v1.x*v2.y - v1.y * v2.x);
	  
	  distances[0] = vec3( area / length(v0), 0, 0 );
	  distances[1] = vec3( 0, area / length(v1), 0 );
	  distances[2] = vec3( 0, 0, area / length(v1) );
    }
    
  if( use_atom_color )
    {
  	  colors[0] = camera_color[0];
  	  colors[1] = camera_color[1];
  	  colors[2] = camera_color[2];
    }
    
    dist = distances[0];
	fragment_vertex = camera_vertex[0];
    fragment_color = colors[0];
	fragment_normal = normalize(cross(camera_vertex[1] - camera_vertex[0], camera_vertex[2] - camera_vertex[0] ));
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();  
	
	dist = distances[1];
	fragment_vertex = camera_vertex[1];
	fragment_color = colors[1];
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();
	  
	dist = distances[2];
	fragment_vertex = camera_vertex[2];
    fragment_color = colors[2];
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();
	  
	EndPrimitive();
}