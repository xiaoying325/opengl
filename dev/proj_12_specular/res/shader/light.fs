varying vec3 V_Normal;


void main()
{

	vec3 lightPos = vec3(10.0,10.0,0.0);
	vec3 L = lightPos;
	L = normalize(L);

	vec3 n = normalize(V_Normal);

	// opengl中当前片段 的最终颜色输出，输出到屏幕上的颜色，就是你看到的颜色
    // PHONG着色，因为是逐像素计算光照颜色的
    // 环境光如何实现呢？
	vec4 AmbientLightColor=vec4(0.2,0.2,0.2,1.0);
	vec4 AmbientMaterial=vec4(0.2,0.2,0.2,1.0);
	vec4 ambientColor=AmbientLightColor*AmbientMaterial;


	//计算下漫反射光
	vec4 DiffuseLightColor=vec4(1.0,1.0,1.0,1.0);
	vec4 DiffuseMaterial=vec4(0.4,0.4,0.4,1.0);
	vec4 diffuseColor=DiffuseLightColor*DiffuseMaterial*max(0.0,dot(L,n));


    gl_FragColor=ambientColor + diffuseColor;
}