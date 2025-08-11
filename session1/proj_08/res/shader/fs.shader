varying vec4 V_Color;

void main()
{
    // opengl中当前片段 的最终颜色输出，输出到屏幕上的颜色，就是你看到的颜色
    gl_FragColor = V_Color;
}