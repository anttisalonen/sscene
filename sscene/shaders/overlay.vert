attribute vec3 a_Position;
attribute vec2 a_texCoord;

varying vec2 v_texCoord;

uniform mat4 u_MVP;

void main()
{
    gl_Position = u_MVP * vec4(a_Position, 1.0);
    v_texCoord = a_texCoord;
}
