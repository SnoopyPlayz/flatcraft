#version 330

// Input attributes (From the mesh/batch)
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

// Outputs to Fragment Shader
out vec2 fragTexCoord;
out vec4 fragColor;

// Uniforms (Sent by Raylib automatically)
uniform mat4 mvp; // Model-View-Projection Matrix

void main()
{
    // Pass coordinates and color to the fragment shader
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    
    // Calculate final vertex position on screen
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
