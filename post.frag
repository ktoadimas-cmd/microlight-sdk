#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uScene;
uniform float uTime;
uniform float uResolutionX;
uniform float uResolutionY;

float rand(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec2 uv = vUV;

    // Джиттер по Y
    uv.y += sin(uv.y * 100.0 + uTime * 8.0) * 0.0008;

    // Хроматическая аберрация
    float offset = 0.003;
    float r = texture(uScene, uv + vec2( offset, 0.0)).r;
    float g = texture(uScene, uv).g;
    float b = texture(uScene, uv + vec2(-offset, 0.0)).b;
    vec3 col = vec3(r, g, b);

    // Зерно
    float n = rand(uv + uTime) - 0.5;
    col += n * 0.08;

    // Scanlines
    float scan = sin(uv.y * uResolutionY * 1.5) * 0.04;
    col -= scan;

    // Vignette
    vec2 v = uv - 0.5;
    float vig = 1.0 - dot(v, v) * 1.2;
    col *= vig;

    col *= 0.95;
    FragColor = vec4(col, 1.0);
}