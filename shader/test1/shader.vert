#version 450
#extension GL_ARB_separate_shader_objects : enable

layout( location = 0 ) in vec2 inPos;
layout( location = 1 ) in vec3 inCol;

layout( binding = 0 ) uniform mvp_matrix {
	mat4 model;
	mat4 view;
	mat4 proj;
} mvp;

layout( location = 0 ) out vec3 fragCol;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	gl_Position = mvp.proj * mvp.view * mvp.model * vec4( inPos, 0.0, 1.0 );
	fragCol = inCol;
}
