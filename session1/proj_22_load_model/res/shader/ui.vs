attribute vec3 pos;
attribute vec2 texcoord;
attribute vec3 normal;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

varying vec2 V_Texcoord;

void main()
{
	V_Texcoord=vec2(texcoord.x,texcoord.y);
    gl_Position=P*V*M*vec4(pos,1.0);
}