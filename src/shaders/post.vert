#version 450

layout(location = 0) out vec2 fragTexCoord;

void main() {
    // Generate fullscreen triangle vertices
    vec2 positions[3] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );
    
    vec2 pos = positions[gl_VertexIndex];
    gl_Position = vec4(pos, 0.0, 1.0);
    fragTexCoord = pos * 0.5 + 0.5;
} 