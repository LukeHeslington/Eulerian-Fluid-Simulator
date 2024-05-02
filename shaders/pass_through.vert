#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 view;
uniform mat4 view2;
uniform mat4 viewSquare;
uniform mat4 projection;
out vec2 TexCoord;
uniform int objectType;

void main()
{
    if (objectType == 0) // Circle
    {
        gl_Position = projection * view * vec4(aPos, 1.0);
        TexCoord = vec2(aTexCoord.x, aTexCoord.y);
    }
    else if (objectType == 1) // Grid
    {
        gl_Position = projection * view2 * vec4(aPos, 1.0);
        TexCoord = vec2(aTexCoord.x, aTexCoord.y);
    }
    else if (objectType == 2) //
    {
        gl_Position = projection * viewSquare * vec4(aPos, 1.0);
        TexCoord = vec2(aTexCoord.x, aTexCoord.y);
    }
}
