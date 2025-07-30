
void main()
{
	// opengl中当前片段 的最终颜色输出，输出到屏幕上的颜色，就是你看到的颜色
    // PHONG着色，因为是逐像素计算光照颜色的
    // 环境光如何实现呢？
	vec4 AmbientLightColor=vec4(0.2,0.2,0.2,1.0);
	vec4 AmbientMaterial=vec4(0.2,0.2,0.2,1.0);
	vec4 ambientColor=AmbientLightColor*AmbientMaterial;
    gl_FragColor=ambientColor;
}



