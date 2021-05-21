#version 430
layout(location=0) in  vec3 vertexPos;
layout(location=1) in  vec3 inColor;
uniform mat4 MVPMatrix;
out vec3 worldPos;
out vec3 color;
void main()
{
    gl_Position=MVPMatrix*vec4(vertexPos,1.0);
    worldPos=vertexPos;
    color=inColor;
}
