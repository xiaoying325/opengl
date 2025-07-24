attribute vec3 pos;
attribute vec4 color;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

varying vec4 V_Color;


void main(){
    V_Color = color;
    // 我们一个3d世界的点，转化到屏幕设备上的坐标经理了哪些流程呢？
    // 模型矩阵M 模型空间----》世界空间
    // 视图矩阵 V 世界空间----》视图空间，摄像机空间
    // 投影矩阵P 视图空间---》裁剪空间

    // 透视除法  裁剪空间 / w = NDC
    // 视口变化， 把NDC的坐标通过缩放以及其他手段 ---》屏幕像素坐标
    gl_Position=P*V*M*vec4(pos,1.0);
}