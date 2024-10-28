/* clang-format off */
#[modes]

mode_default =

#[specializations]

#[vertex]

#version 450

layout(location = 0) in vec2 vertex;

layout(set = 0, binding = 0, std140) uniform StencilWriteData {
    uniform highp mat4 canvas_transform;
    uniform highp mat4 screen_transform;
    uniform highp mat4 view_matrix;
	uniform highp mat4 projection_matrix;
}
write_data;

layout(push_constant, std430) uniform Params {
	vec2 rect_position;
	vec2 rect_size;
}
params;

void main() {
	// crash on Adreno 320/330
	//vec2 vertex_base_arr[6] = vec2[](vec2(0.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0), vec2(1.0, 0.0), vec2(0.0, 0.0), vec2(1.0, 1.0));
	//vec2 vertex_base = vertex_base_arr[gl_VertexID % 6];
	//-----------------------------------------
	// ID |  0  |  1  |  2  |  3  |  4  |  5  |
	//-----------------------------------------
	// X  | 0.0 | 0.0 | 1.0 | 1.0 | 0.0 | 1.0 |
	// Y  | 0.0 | 1.0 | 1.0 | 0.0 | 0.0 | 1.0 |
	//-----------------------------------------
	// no crash or freeze on all Adreno 3xx	with 'if / else if' and slightly faster!
	int vertex_id = gl_VertexIndex % 6;
	vec2 vertex_base;
	if (vertex_id == 0)
		vertex_base = vec2(0.0, 0.0);
	else if (vertex_id == 1)
		vertex_base = vec2(0.0, 1.0);
	else if (vertex_id == 2)
		vertex_base = vec2(1.0, 1.0);
	else if (vertex_id == 3)
		vertex_base = vec2(1.0, 0.0);
	else if (vertex_id == 4)
		vertex_base = vec2(0.0, 0.0);
	else if (vertex_id == 5)
		vertex_base = vec2(1.0, 1.0);
	vec2 vertex_2d = vertex_base;

	vertex_2d.xy *= params.rect_size;
	vertex_2d.xy += params.rect_position;
	gl_Position = write_data.projection_matrix * write_data.view_matrix * write_data.canvas_transform * write_data.screen_transform * vec4(vertex_2d, 0.0, 1.0);
}

#[fragment]
#version 450

layout(location = 0) out vec4 frag_color;

void main() {
	frag_color = vec4(1.0, 0.0, 0.0, 1.0);
}
