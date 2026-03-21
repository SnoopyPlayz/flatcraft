#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;
out vec2 fragTexCoord;
out vec4 fragColor;
uniform mat4 mvp;
uniform vec2 worldOffset;

void main(){
    fragTexCoord = vertexTexCoord; 
    fragColor = vertexColor;       
    vec3 shiftedPosition = vec3(vertexPosition.xy - worldOffset, vertexPosition.z);
    gl_Position = mvp * vec4(shiftedPosition, 1.0);
};
