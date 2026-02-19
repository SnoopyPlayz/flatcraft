#version 330
in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform float brightness; // 1.0 is normal, 2.0 is double, etc.

out vec4 finalColor;

void main() {
    vec4 texelColor = texture(texture0, fragTexCoord);
    // Multiply the RGB channels, but leave Alpha alone
    finalColor = vec4(texelColor.rgb * brightness, texelColor.a) * fragColor;
}
