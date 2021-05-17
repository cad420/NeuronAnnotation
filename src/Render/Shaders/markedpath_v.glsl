#version 430
layout(location=0) in  vec3 vertexPos;
uniform mat4 MVPMatrix;
out vec3 worldPos;
void main()
{
    gl_Position=MVPMatrix*vec4(vertexPos,1.0);
    worldPos=vertexPos;
}
