#version 330 core

in vec3 vColor;
in vec3 vNormal;
in vec3 vLight;
in vec3 vView;
in vec3 v3Pos;
in vec2 vTexCoord;

// Tangent and Bitangent from Vertex Shader
in vec3 vTangent;
in vec3 vBitangent;

uniform int  uShadingMode;
uniform vec4 ui4Color;

#define MAX_LIGHTS 8

struct LightSource {
    vec3 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float constant;
    float linear;
    float quadratic;
    // Spot Light
    vec3 direction;
    float cutOff;
    float outerCutOff;
    float exponent;
    int type; // 0 = POINT, 1 = SPOT, 2 = DIRECTIONAL
    bool enabled;
};

uniform LightSource uLights[MAX_LIGHTS];
uniform int uNumLights;

struct Material {
    vec4 ambient;   // ka
    vec4 diffuse;   // kd
    vec4 specular;  // ks
    float shininess;
    float alpha;
    
    sampler2D diffuseTexture;
    sampler2D normalTexture;
    sampler2D specularTexture;
    sampler2D alphaTexture;
    sampler2D lightMapTexture;
    
    bool hasDiffuseTexture;
    bool hasNormalTexture;
    bool hasSpecularTexture;
    bool hasAlphaTexture;
    bool hasLightMap;
    
    float lightMapIntensity;
};
uniform Material uMaterial;

uniform float uLightMapGamma = 2.2;      // Light Map 的 Gamma 值
uniform int uLightMapBlendMode = 0;      // 混合模式：0=Multiply, 1=Add, 2=Screen, 3=Overlay
uniform bool uUseLightMapAO = false;     // 是否將 Light Map 用作環境光遮蔽

uniform float uNormalStrength = 2.0;
uniform float uSpecularStrength = 3.0;
uniform float uSpecularPower = 1.5;

out vec4 FragColor;

vec3 blendLightMap(vec3 baseColor, vec3 lightMap, int blendMode) {
    switch(blendMode) {
        case 0:
            return baseColor * lightMap;
        case 1:
            return baseColor + lightMap;
        case 2:
            return 1.0 - (1.0 - baseColor) * (1.0 - lightMap);
        case 3: { // Overlay
            vec3 result;
            // Apply overlay blend mode component-wise
            if (baseColor.r < 0.5) {
                result.r = 2.0 * baseColor.r * lightMap.r;
            } else {
                result.r = 1.0 - 2.0 * (1.0 - baseColor.r) * (1.0 - lightMap.r);
            }
            if (baseColor.g < 0.5) {
                result.g = 2.0 * baseColor.g * lightMap.g;
            } else {
                result.g = 1.0 - 2.0 * (1.0 - baseColor.g) * (1.0 - lightMap.g);
            }
            if (baseColor.b < 0.5) {
                result.b = 2.0 * baseColor.b * lightMap.b;
            } else {
                result.b = 1.0 - 2.0 * (1.0 - baseColor.b) * (1.0 - lightMap.b);
            }
            return result;
        }
//
        default:
            return baseColor * lightMap;
    }
}

void main() {

    if( uShadingMode == 1) { FragColor = vec4(vColor, 1.0);  return; }
    if( uShadingMode == 2 ){ FragColor = ui4Color; return; }

    mat3 TBN = mat3(normalize(vTangent), normalize(vBitangent), normalize(vNormal));
    vec3 normalMap = texture(uMaterial.normalTexture, vTexCoord).rgb;
    
//    vec3 N = normalize(TBN * (2.0 * normalMap - 1.0));
    vec3 N;
    if (uMaterial.hasNormalTexture) {
        vec3 T = normalize(vTangent);
        vec3 B = normalize(vBitangent);
        vec3 vertexNormal = normalize(vNormal);
        
        T = normalize(T - dot(T, vertexNormal) * vertexNormal);
        B = normalize(cross(vertexNormal, T));
        
        mat3 TBN = mat3(T, B, vertexNormal);
        
        vec3 normalMap = texture(uMaterial.normalTexture, vTexCoord).rgb;
        normalMap = normalize(normalMap * 2.0 - 1.0);
        
        normalMap.xy *= uNormalStrength;
        normalMap = normalize(normalMap);
        
        N = normalize(TBN * normalMap);
    } else {
        N = normalize(vNormal);
    }

//    vec3 L = normalize(vLight);
    vec3 V = normalize(vView);
//    vec3 H = normalize(L + V);  // Halfway vector (Blinn-Phong)
//    vec3 R = reflect(-L, N); // Phong

    vec4 texDiffuse = vec4(1.0);
    vec4 texSpecular = vec4(1.0);
    float finalAlpha = uMaterial.alpha;

    if(uMaterial.hasDiffuseTexture) {
        texDiffuse = texture(uMaterial.diffuseTexture, vTexCoord);
//        finalAlpha *= texDiffuse.a;
    }
    
    if(uMaterial.hasAlphaTexture) {
        vec4 alphaTexture = texture(uMaterial.alphaTexture, vTexCoord);
//        float alphaFromTexture = max(max(alphaTexture.r, alphaTexture.g),max(alphaTexture.b, alphaTexture.a));
         float alphaFromTexture = alphaTexture.a;  // Alpha
        // float alphaFromTexture = alphaTexture.r;  // Red
        // float alphaFromTexture = alphaTexture.g;  // Green
        finalAlpha = alphaFromTexture;
    }
    vec3 lightMapColor = vec3(1.0);
    float aoFactor = 1.0;
    
    if(uMaterial.hasLightMap) {
        vec4 lightMapSample = texture(uMaterial.lightMapTexture, vTexCoord);
//        FragColor = vec4(lightMapSample.rgb, 1.0);
//        return;
        
        // 應用 Gamma 校正到 Light Map
        lightMapColor = pow(lightMapSample.rgb, vec3(uLightMapGamma));
        lightMapColor *= uMaterial.lightMapIntensity;
        
        // 如果使用 Light Map 作為 AO，計算遮蔽因子
        if(uUseLightMapAO) {
            aoFactor = dot(lightMapColor, vec3(0.299, 0.587, 0.114)); // 轉換為灰階作為 AO
            aoFactor = clamp(aoFactor, 0.1, 1.0); // 限制最小值避免過暗
        }
    }
//     finalAlpha = max(finalAlpha, 0.1);
    if(finalAlpha < 0.01) {
       discard;
   }

    if(uMaterial.hasSpecularTexture) {
        texSpecular = texture(uMaterial.specularTexture, vTexCoord);
        texSpecular.rgb = pow(texSpecular.rgb, vec3(0.8));
    }
    
    vec4 finalColor = vec4(0.0);
        
    // add ambient, diffuse, specular
    vec4 totalAmbient = vec4(0.0);
    vec4 totalDiffuse = vec4(0.0);
    vec4 totalSpecular = vec4(0.0);
    
    for (int i = 0; i < min(uNumLights, MAX_LIGHTS); i++) {
        if (!uLights[i].enabled) continue;
        
        vec3 L;
        float attenuation = 1.0;
        
        if (uLights[i].type == 2) { // DIRECTIONAL
            L = normalize(-uLights[i].direction);
        } else { // POINT or SPOT
            L = normalize(uLights[i].position - v3Pos);
            
            float dist = length(uLights[i].position - v3Pos);
            attenuation = 1.0 / (uLights[i].constant + uLights[i].linear * dist + uLights[i].quadratic * dist * dist);
        }
        
        // Spot light
        if (uLights[i].type == 1 && uLights[i].cutOff > 0.0) { // SPOT
            float theta = dot(L, normalize(-uLights[i].direction));
            float intensity = clamp(
                (theta - uLights[i].outerCutOff) / (uLights[i].cutOff - uLights[i].outerCutOff),
                0.0, 1.0
            );
            
            if (uLights[i].exponent == 1.0) {
                attenuation *= intensity;
            } else {
                attenuation *= pow(intensity, uLights[i].exponent);
            }
        }
        
        vec3 H = normalize(L + V);
        
        // first Ambient
//        if (i == 0) {
//            totalAmbient += uLights[i].ambient * uMaterial.ambient * texDiffuse * attenuation * 1.0;
//        }
        if (i == 0) {
            vec4 ambient = uLights[i].ambient * uMaterial.ambient * texDiffuse * attenuation;
            // 應用 AO 到環境光
            if(uUseLightMapAO) {
                ambient.rgb *= aoFactor;
            }
            totalAmbient += ambient;
        }
        
        // Diffuse
        float diff = max(dot(N, L), 0.0);
        totalDiffuse += uLights[i].diffuse * diff * uMaterial.diffuse * texDiffuse * attenuation;
        
        // Specular
        float spec = pow(max(dot(N, H), 0.0), uMaterial.shininess * uSpecularPower);
        vec4 specularColor = uLights[i].specular * spec * uMaterial.specular * texSpecular * uSpecularStrength;
        float fresnel = pow(1.0 - max(dot(N, V), 0.0), 2.0);
        specularColor *= (1.0 + fresnel * 0.5);
        totalSpecular += specularColor * attenuation;
    }
    
    finalColor = totalAmbient + totalDiffuse + totalSpecular;
    
    if(uMaterial.hasLightMap) {
        finalColor.rgb = blendLightMap(finalColor.rgb, lightMapColor, uLightMapBlendMode);
    }
    
//    finalColor.rgb = finalColor.rgb / (finalColor.rgb + vec3(1.0));
//    finalColor.rgb = pow(finalColor.rgb, vec3(1.0/2.2)); // Gamma correction
    finalColor = clamp(finalColor, 0.0, 1.0);
    finalColor.a = finalAlpha;
    FragColor = finalColor;
    
}
