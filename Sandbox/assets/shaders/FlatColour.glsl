#type vertex
#version 330 core

layout(location = 0) in vec3 a_Pos;

uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_Transform;

void main() {
	gl_Position = u_ViewProjectionMatrix * u_Transform * vec4(a_Pos, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 color;

uniform vec4 f_col;

void main() {
	color = f_col;
}
