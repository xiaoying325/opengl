uniform sampler2D U_MainTexture; //注纹理
varying vec3 V_Normal;
varying vec4 V_WorldPos;
varying vec2 V_Texcoord;




// 高光反射的公式
// 什么是高光反射？就是物体表明上特别亮的那个点，你怎么用着色器代码表示出来
// 我们说过有公式
// 材质的高光反射系数 * 光源强度 * （max(r·v,0)） n(幂运算)
// 观察方向v向量怎么求得？
// 一般是从当前片元点指向摄像机方向的位置向量，注意都是在是世界空间下计算的，计算公式为：V =normalize(camearPos - fragmentPos)

//反射向量 r 怎么求？
// 一般是，光照方向L 打到法线N上之后，做镜面对称射出去得方向，就是R



void main()
{

	vec3 lightPos = vec3(10.0,10.0,0.0);
	vec3 L = lightPos;
	L = normalize(L); // 对世界空间下的光源向量做归一化运算
	vec3 n = normalize(V_Normal); // 对世界空间下的法线做归一化运算

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

	// 计算一下高光
	vec4 SpecularLightColor=vec4(1.0,1.0,1.0,1.0); //光源强度
	vec4 SpecularMaterial=vec4(0.8,0.8,0.8,1.0); //高光的反射系数

	// 需要用到GLSL内置的函数 reflect （入射光方向向量，法线），返回给你一个反射向量
	vec3 reflectDir = normalize( reflect(-L,n)); //为什么是负数  L其实是从点指向光源， 
	vec3 viewDir=normalize(vec3(0.0)-V_WorldPos.xyz); // 和向量的运算规则有关，在向量减法运算中，你要求一个向量A指向向量B的方向，不是A-B,然后B-A,这样算出来的结果才是你想要的
	vec4 specularColor=SpecularLightColor*SpecularMaterial*pow(max(0.0,dot(viewDir,reflectDir)),400.0);


    //gl_FragColor=ambientColor + diffuseColor + specularColor;
	vec4 texColor = texture2D(U_MainTexture, V_Texcoord);
	// 把所有光照加在一起
	vec4 lighting = ambientColor + diffuseColor + specularColor;
	// 将纹理颜色乘以光照颜色得到最终片元颜色
	gl_FragColor = texColor * lighting;
}