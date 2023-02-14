#version 460 core 

uniform vec4 m_MainColor; 
uniform float g_Time; 
uniform float m_BarSpeed; 
uniform float m_BarDistance; 
uniform float m_alpha; 
uniform float m_FlickerSpeed; 
uniform vec4 m_RimColor; 
uniform float m_RimPower; 
uniform float m_GlowSpeed; 
uniform float m_GlowDistance; 

in vec2 out_texcoord;
in vec3 FragPosition;
in vec3 viewVector;
in vec3 Normals; 

out vec4 FragColor;

float rand(float n){ 
       return fract(sin(n) * 43758.5453123); 
} 

float noise(float p){ 
    float fl = floor(p); 
    float fc = fract(p); 
    return mix(rand(fl),rand(fl+1.0),fc); 
} 

void main(void) 
    {
       float val = g_Time * m_BarSpeed + FragPosition.y * m_BarDistance; 
       float bars = step(val - floor(val),0.5) * 0.65; 
       float flicker = clamp(noise(g_Time * m_FlickerSpeed),0.65,1.0); 
       float rim = 1.0 - clamp(dot(viewVector,Normals),0.0,1.0); 
       vec4 rimColor = m_RimColor * pow(rim,m_RimPower); 
       float glow = FragPosition.y * m_GlowDistance - g_Time * m_GlowSpeed; 
        
       FragColor = m_MainColor + rimColor + ( 0.35 * m_MainColor);
       FragColor.a = m_alpha * (bars + rim ) * flicker;

        if(FragColor.a <= 0.01f)
            discard;
    };