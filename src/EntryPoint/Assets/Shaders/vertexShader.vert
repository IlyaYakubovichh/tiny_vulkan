#version 460 core
#extension GL_EXT_buffer_reference2 : require

layout(location = 0) out vec4 vertexColor;

struct Vertex
{
	vec3 position;
	float uv_x;
	vec3 normal;
	float uv_y;
	vec4 color;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer
{
	Vertex vertices[];
};

layout( push_constant ) uniform PushConstants
{
	mat4 worldMatrix;
	VertexBuffer vertexBuffer;
} push_constants;

void main()
{
	Vertex v = push_constants.vertexBuffer.vertices[gl_VertexIndex];
	gl_Position = push_constants.worldMatrix * vec4(v.position, 1.0f);
	vertexColor = v.color;
}