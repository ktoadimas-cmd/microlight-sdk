#version 330 core

in vec3 vNormal;
in vec2 vUV;
in vec3 vWorldPos;

uniform sampler2D uTex;
uniform vec4      uTint;
uniform int       uUseTex;

uniform vec3 uLightDir;
uniform vec3 uLightCol;
uniform vec3 uAmbient;

#define MAX_LIGHTS 16
uniform int   uNumLights;
uniform vec3  uLightPos[MAX_LIGHTS];
uniform vec3  uLightColor[MAX_LIGHTS];
uniform float uLightRange[MAX_LIGHTS];

out vec4 FragColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uLightDir);

    float diff = max(dot(N, L), 0.0);
    vec3 light = uAmbient + uLightCol * diff;

    for (int i = 0; i < uNumLights; ++i) {
        vec3  toLight = uLightPos[i] - vWorldPos;
        float dist    = length(toLight);
        float atten   = max(0.0, 1.0 - dist / uLightRange[i]);
        atten = atten * atten;
        vec3  Lp  = toLight / max(dist, 0.001);
        float ndl = max(dot(N, Lp), 0.0);
        light += uLightColor[i] * atten * ndl;
    }

    vec4 base = vec4(1.0);
    if (uUseTex == 1) {
        base = texture(uTex, vUV);
    }
    base *= uTint;

    FragColor = vec4(base.rgb * light, base.a);
}
