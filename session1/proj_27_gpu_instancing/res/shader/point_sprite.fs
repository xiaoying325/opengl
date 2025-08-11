uniform sampler2D U_MainTexture;

void main()
{
    gl_FragColor=texture2D(U_MainTexture,gl_PointCoord.xy); // gl_PointCoord 直接返回点精灵这个图元的纹理采样坐标，都不需要我们算了
}