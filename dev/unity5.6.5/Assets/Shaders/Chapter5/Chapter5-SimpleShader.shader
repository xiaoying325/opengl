Shader "Unity Shaders Book/Chapter 5/Simple Shader" {
	Properties {
		_Color ("Color Tint", Color) = (1, 1, 1, 1)
	}

    
	SubShader {

        //ZWrite off  //禁止了写入深度，就不会和缓冲区中的之前计算好的深度值进行比较，这样可以用来实现透明效果
       cull Front //剔除了正面 ，只渲染背面
        //Cull Back

        //如果cull Front 或者 Cull Back 都写上，那就是啥也不剔除


//"PreviewType" = "Plane"  当前shader效果下材质球的预览类型，不过设置成plane 或者skyBox貌似不生效
       //Tags {"RenderType" = "TransparentCutout"}

        Pass {


            Name "Fuck"  //定义pass名称

            CGPROGRAM

			#include "UnityCG.cginc"

            #pragma vertex vert
            #pragma fragment frag


            
            //记住在CGPROGRAME代码中如果要使用Properties定义的属性，必须要在这里重新生命
             fixed4 _Color;


// a2v 定义了从应用程序阶段，要传递给顶点着色器的数据结构体 application-2-vertex
			struct a2v {
                float4 vertex : POSITION; //顶点信息
				float3 normal : NORMAL; //发现贴图
				float4 texcoord : TEXCOORD0;//第一套纹理坐标，因为可能有号机组
                float4 texcoord1:TEXCOORD1; //第二套纹理坐标
            };
            

            //定义了从顶点着色器要传递给片段着色器的数据结构体
            struct v2f {
                float4 pos : SV_POSITION;
                fixed3 color : COLOR0;
            };
            
            v2f vert(a2v v) {
            	v2f o;
            	o.pos = UnityObjectToClipPos(v.vertex); //必须要做的，转换到裁剪坐标系下，本地模型+世界+投影+裁剪空间，归一化的
            	o.color = v.normal * 0.8+ fixed3(0.5, 0.5, 0.5);  //本质上也是经验模型，就是要去不停的调整参数，以其达到想要的效果
                return o;
            }

            fixed4 frag(v2f i) : SV_Target {
            	fixed3 c = i.color; //在顶点着色阶段，计算好了的色值数据
            	c *= _Color.rgb *1; //不乘以面板上定义的颜色也是可以的
                return fixed4(c, 0.5);
            }

            ENDCG
        }
    }
}
