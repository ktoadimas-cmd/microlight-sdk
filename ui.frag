#version 330 core

in vec2 vUV;
in vec4 vColor;

uniform sampler2D uTex;
uniform int       uUseTex;
uniform int       uIsFont;   // 1 = шрифт (R = alpha), 0 = обычная RGBA-текстура

out vec4 FragColor;

void main() {
    vec4 c = vColor;
    if (uUseTex == 1) {
        if (uIsFont == 1) {
            float a = texture(uTex, vUV).r;
            c.a *= a;
        } else {
            c *= texture(uTex, vUV);
        }
    }
    FragColor = c;
}
