attribute vec3 pos;
attribute vec2 texcoord;
attribute vec3 normal;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

varying vec2 V_Texcoord;

void main()
{
	V_Texcoord=vec2(texcoord.x,texcoord.y);//必须要翻转下，不然图象是倒的
    gl_Position=P*V*M*vec4(pos,1.0);
	//gl_Position=vec4(pos,1.0);
}