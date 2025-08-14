attribute vec3 pos;
attribute vec2 texcoord;
attribute vec3 normal;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main()
{
	gl_PointSize=35.0; //设置点的大小
    gl_Position=P*V*M*vec4(pos,1.0);
}