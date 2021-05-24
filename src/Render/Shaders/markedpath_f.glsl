#version 430
out vec4 fragColor;
in vec3 worldPos;
in vec3 color;
void main()
{
    fragColor=vec4(color,1.0f);
    // vec4(1.0,0.0,0.0,1.0);//vec4(worldPos,0,1.0);
}
