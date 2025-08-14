#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D depthMap;


float LinerlizeDepth(float depth)
{

    return depth;
    // float near = 0.1;
    // float far = 1000.0;
    // float z = depth * 2.0 - 1.0; // 从[-1, 1]转换到[0, 1]
    // return (2.0 * near * far) / (far + near - z * (far - near));
}



void main()
{
    // 深度纹理，有效内容存储在r中
    float depathValue = texture(depthMap, TexCoord).r; //采样深度纹理
    float linearDepth = LinerlizeDepth(depathValue); //线性化以下
    //linearDepth = (linearDepth - 0.1) / (1000.0 - 0.1); //归一化

    linearDepth = pow(linearDepth, 16); // gamma校正



    FragColor =  vec4(linearDepth, linearDepth, linearDepth, 1.0); //转为灰度图输出
}