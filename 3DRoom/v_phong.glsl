// v_phong.glsl
#version 330 core
layout(location=0) in vec3 aPos;
//layout(location=1) in vec3 aColor;
layout(location=2) in vec3 aNormal;
layout(location=3) in vec2 aTex;    // Texture Coordinates

uniform mat4 mxModel;
uniform mat4 mxView;
uniform mat4 mxProj;

uniform vec3 viewPos;
uniform vec3 lightPos;

out vec3 vNormal;
out vec3 vLight;
out vec3 vView;
out vec3 vColor;
out vec3 v3Pos;
out vec2 vTexCoord;

// Tangent and Bitangent (out variables)
out vec3 vTangent;
out vec3 vBitangent;

void main() {
    vec4 worldPos = mxModel * vec4(aPos, 1.0);
    v3Pos   = worldPos.xyz;
    vNormal = normalize((mat3(mxModel) * aNormal));
    vLight  = normalize(lightPos - v3Pos);
    vView   = normalize(viewPos - v3Pos);
//    vColor   = aColor;
    vColor = vec3(1.0, 1.0, 1.0);
    vTexCoord = aTex;
    gl_Position = mxProj * mxView * worldPos;
    
    // Calculate Tangent and Bitangent (Simple Method - Requires UVs)
    vec3 edge1 = vec3(mxModel * vec4(aPos, 1.0) - mxModel * vec4(aPos - vec3(0.1, 0.0, 0.0), 1.0)); // Approximate
    vec3 edge2 = vec3(mxModel * vec4(aPos, 1.0) - mxModel * vec4(aPos - vec3(0.0, 0.1, 0.0), 1.0)); // Approximate
    vec2 deltaUV1 = vec2(aTex.x - (aTex.x - 0.1), aTex.y - aTex.y); // Approximate
    vec2 deltaUV2 = vec2(aTex.x - aTex.x, aTex.y - (aTex.y - 0.1)); // Approximate

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    vTangent = normalize(vec3(mxModel * vec4(f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x), 0.0, 0.0, 0.0)));
    vBitangent = normalize(vec3(mxModel * vec4(f * (deltaUV1.x * edge2.x - deltaUV2.x * edge1.x), 0.0, 0.0, 0.0)));

    vNormal = normalize(mat3(transpose(inverse(mxModel))) * aNormal); // Correct Normal Transformation
}


