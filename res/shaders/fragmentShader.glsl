#version 330

// Inputs from Vertex Shader
in vec2 fragTexCoord;
in vec4 fragColor;

// Output to the Screen
out vec4 finalColor;

// Uniforms
uniform sampler2D texture0;
uniform vec4 colDiffuse;

void main()
{
    // 1. Sample the texture color first
    vec4 texelColor = texture(texture0, fragTexCoord);

    // --- ADDED: Alpha Discard Logic ---
    // If the alpha of the pixel is below 0.1 (or 0.5), throw it away entirely.
    // We use 0.1 usually to catch "mostly transparent" pixels but keep antialiasing if possible,
    // though 0.5 creates the sharpest "pixel art" style cutout.
    if (texelColor.a < 0.5) discard;
    // ----------------------------------

    // 2. Calculate lighting/tinting only for pixels that survived
    finalColor = texelColor * fragColor * colDiffuse;
}
