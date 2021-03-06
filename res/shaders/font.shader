[common]

#version 330 core

//------------------------------------------------------------------------------//

[vertex]

layout (location = 0) in vec4 vertex;

out vec2 fUVTexture;

uniform mat4 projectionMatrix;

void main() {
    gl_Position = projectionMatrix * vec4(vertex.xy, 0, 1);
    fUVTexture = vertex.zw;
}

//------------------------------------------------------------------------------//

[fragment]

in vec2 fUVTexture;
out vec4 outColor;

uniform sampler2D text;
uniform vec4 color;

void main() {    
    outColor = vec4(color.xyz, color.w * texture(text, fUVTexture).r);
} 