Shader "Unity Shaders Book/Chapter 5/False Color" {
	SubShader {
		Pass {
			CGPROGRAM
			
			#pragma vertex vert
			#pragma fragment frag
			

			//内置的shader库文件在Editor/Data/CGIncludes下
			#include "UnityCG.cginc"  //包含了内置的已经写好的shader库文件，比如下面使用的appdata_full就将这个文件里面
			
			struct v2f {
				float4 pos : SV_POSITION;
				fixed4 color : COLOR0;
			};
			
			/*
			struct appdata_full {
				float4 vertex : POSITION;
				float4 tangent : TANGENT;
				float3 normal : NORMAL;
				float4 texcoord : TEXCOORD0;
				float4 texcoord1 : TEXCOORD1;
				float4 texcoord2 : TEXCOORD2;
				float4 texcoord3 : TEXCOORD3;
				fixed4 color : COLOR;
				UNITY_VERTEX_INPUT_INSTANCE_ID
			};
			*/


//appdata_full  UnityCG.cginc中已经提我们定义好了的结构体，直接拿来主义
//appdata_base 
			v2f vert(appdata_full v) {
				v2f o;
				//将顶点从本地坐标转换到裁剪坐标  MVP矩阵，知道就行
				o.pos = UnityObjectToClipPos(v.vertex);
				
				// Visualize normal
				o.color = fixed4(v.normal * 0.5 + fixed3(0.5, 0.5, 0.5), 1.0);
				
				// Visualize tangent
				o.color = fixed4(v.tangent.xyz * 0.5 + fixed3(0.5, 0.5, 0.5), 1.0);
				
				// Visualize binormal
				fixed3 binormal = cross(v.normal, v.tangent.xyz) * v.tangent.w;
				o.color = fixed4(binormal * 0.5 + fixed3(0.5, 0.5, 0.5), 1.0);
				
				// Visualize the first set texcoord
				o.color = fixed4(v.texcoord.xy, 0.0, 1.0);
				
				// Visualize the second set texcoord
				o.color = fixed4(v.texcoord1.xy, 0.0, 1.0);
				
				// Visualize fractional part of the first set texcoord
				o.color = frac(v.texcoord);
				if (any(saturate(v.texcoord) - v.texcoord)) {
					o.color.b = 0.5;
				}
				o.color.a = 1.0;
				
				// Visualize fractional part of the second set texcoord
				o.color = frac(v.texcoord1);
				if (any(saturate(v.texcoord1) - v.texcoord1)) {
					o.color.b = 0.5;
				}
				o.color.a = 1.0;
				
				// Visualize vertex color
//				o.color = v.color;
				
				return o;
			}
			
			fixed4 frag(v2f i) : SV_Target {
				return i.color;
			}
			
			ENDCG
		}
	}


	FallBack "Diffuse"
}
