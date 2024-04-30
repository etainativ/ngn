#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_debug_printf : enable

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;

struct Vertex {
	vec3 position;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer{
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
	debugPrintfEXT("My float is %d", gl_VertexIndex);

	//output data
	//gl_Position = vec4(v.position, 1.0f);
	gl_Position = vec4(v.position, 1.0f);
	outColor = vec3(1.0f, 0.0f, 0.0f);
}