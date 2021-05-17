#version 430
out vec4 fragColor;
in vec3 worldPos;
void main()
{
    fragColor=vec4(1.0,0,0,0.0,1.0)//vec4(worldPos,0,1.0);
}
