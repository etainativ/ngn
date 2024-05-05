#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_debug_printf : enable

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;

struct Vertex {
	vec3 position;
	float pad1;
	vec3 normal;
	float pad2;
	vec3 color;
	float pad3;
};

layout(std430, buffer_reference, buffer_reference_align = 1) readonly buffer VertexBuffer{
	Vertex vertices[];
};

//push constants block
layout( push_constant ) uniform constants
{
	mat4 render_matrix;
	VertexBuffer vertexBuffer;
} PushConstants;

void main()
{
	//load vertex data from device adress
	Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

	//output data
	//gl_Position = vec4(v.position, 1.0f);
	gl_Position = vec4(v.position, 1.0f) * PushConstants.render_matrix;
	debugPrintfEXT("%d: %f, %f, %f", gl_VertexIndex, gl_Position.x, gl_Position.y, gl_Position.z);
	outColor = vec3(1.0f, 0.0f, 0.0f);
}
