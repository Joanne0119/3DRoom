#version 330 core
layout (location = 0) in vec3 aPos;    // Pos
layout (location = 1) in vec3 aColor;  // Color
layout (location = 2) in vec2 aTex;    // Texture Coordinates
out vec4 vColor;
uniform mat4 mxModel;
uniform mat4 mxView;
uniform mat4 mxProj;
uniform vec4 ui4Color;
uniform int  iColorType;
void main()
{
    vColor = ui4Color;
    gl_Position = mxProj*mxView*mxModel*vec4(aPos, 1.0);
}