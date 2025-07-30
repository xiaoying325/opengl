attribute vec3 pos;
attribute vec2 texcoord;
attribute vec3 normal;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform mat4 NM;

varying vec4 V_Color;
varying vec3 V_Normal;
varying vec4 V_WorldPos;

void main()
{
    //把本地局部世界坐标中的法线坐标 也转换到世界空间中
    V_Normal = mat3(NM) *normal; 
    V_WorldPos =M *vec4(pos,1.0); //当前点的世界空间下的位置坐标
    gl_Position=P*V*M*vec4(pos,1.0);;
}