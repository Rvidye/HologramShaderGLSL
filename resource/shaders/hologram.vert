#version 460 core

in vec3 a_position;
in vec3 a_normal;
in vec2 a_texcoord;
in vec3 a_tangent;
in vec3 a_bitangent;
in ivec4 a_boneIds;
in vec4 a_weights;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform float g_Time;
uniform float m_GlitchSpeed; 
uniform float m_GlitchItensity;

out vec2 out_texcoord;
out vec3 FragPosition;
out vec3 viewVector;
out vec3 Normals;

void main(void)
{
    vec4 eye = u_View * u_Model * vec4(a_position,1.0f);
    mat3 normalMatrix = mat3(u_View * u_Model);
    Normals = normalize(normalMatrix * a_normal);
    viewVector = normalize(-eye.xyz);

    FragPosition = vec3(u_Model * vec4(a_position,1.0f));
    FragPosition.x += m_GlitchItensity * step(0.5,sin(g_Time * 2.0 + a_position.y * 1.0)) * step(0.99 , sin(g_Time * m_GlitchSpeed * 0.5));
    out_texcoord = a_texcoord;
	gl_Position = u_Projection * u_View * u_Model * vec4(a_position,1.0f);
}
