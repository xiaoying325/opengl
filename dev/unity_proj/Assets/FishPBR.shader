Shader "Fishing3D/FishPBR"
{
	Properties
	{
		_MainColor  ("MainColor", Color) = (1,1,1,1)

        [ToggleOff] _CustomDirLight ("*使用自定义环境光参数", Float) = 0
		[HDR]_MainColor2         ("*全局光颜色RGB", Color) = (1,1,1,1)
		_DirectionalLightDirX    ("*全局光方向X", Range( -1, 1)) = 0
		_DirectionalLightDirY    ("*全局光方向Y", Range( -1, 1)) = 0
        _DirectionLightIntensity ("*全局光强度 Intensity", Float) = 1.0

		[HDR]_MainColor4         ("*环境光颜色RGB", Color) = (0,0,0,0)
        _Attenuation             ("*光照衰减", Range( 0, 5)) = 1

		[HDR]_GlowColor("后处理颜色HDR", Color) = (0,0,0,0)
        _GlowStrength        ("后处理强度", Range (0, 5)) = 0

        _GrayValue        ("---[HIT]---GrayValue", Range (0, 1)) = 0
        _HitColorValue    ("---[HIT]---击中颜色显示", Range (0, 1)) = 0
        _HitSaturation    ("---[HIT]---击中颜色饱和", Range (-10, 10)) = 0.2
        _HitColor         ("---[HIT]---击中最终颜色", Color)  = (1, 0, 0, 1)
        _PreHitColor      ("---[HIT]---pre hit color", Color) = (1, 0, 0, 1)
        _TestGrayFlag     ("---[HIT]---gray flag", Range (0, 1)) = 1
        _TestGrayColor    ("---[HIT]---gray color", Color) = (0.1, 0.1, 0.1, 0.1)
        _HitAniValue      ("---[HIT]---击中动画渐变", Range (0, 1)) = 0

		_Alpha              ("*透明度",  Range (0, 1)) = 1.0
		_AlbedoMap          ("MainTex", 2D) = "white" {}
		[Normal]_NormalMap  ("NormalTex", 2D) = "bump" {}

		_MaskMap    ("金属_遮蔽_自发光_粗糙", 2D) = "white" {}
		_MaskMap2   ("透贴", 2D) = "white" {}
        [ToggleOff] _MaskValue        ("使用透贴通道显示HDR", Float) = 0


        _TexA        ("Cubemap反射球", Cube) = "Black" {}
    	_CubeMapDirX ("Cubemap反射球 方向X", Range (-1.00, 1)) = 0
    	_CubeMapDirY ("Cubemap反射球 方向Y", Range (-1.00, 1)) = 0
        _Reflection  ("Cubemap反射强度", Range (0, 10)) = 1



		_Smooth          ("粗糙", Range( 0 , 2))            = 1
		[Gamma]_Metallic ("金属", Range( 0 , 2))   = 1
		_Occlusion       ("*环境光遮蔽 AO", Float)     = 1.0
		_BumpScale       ("*法线深度", Float)           = 1.0  



    	_RimOffsetX     ("[1]菲涅尔单色偏移X", Range (-1.00, 1)) = 0
    	_RimOffsetY     ("[1]菲涅尔单色偏移Y", Range (-1.00, 1)) = 0
		[HDR]_SideLightColor ("[1]菲涅尔单色RGB", Color) = (1,1,1,1)
		_SideLightScale ("[1]菲涅尔单色强度", Range( 0 , 5)) = 0

		[HideInInspector] _texcoord( "", 2D ) = "white" {}

        _InteriorColor  ("[2]菲涅尔双色内层RGB", Color) = (1,1,1,1)
        _InteriorColor2 ("[2]菲涅尔双色外层RGB", Color) = (1,1,1,1)
        _RimIntensity   ("[2]菲涅尔双色 Intensity", Range(0,10)) = 0
        _RimPower       ("[2]菲涅尔双色强度 Power", Range(0.0,3)) = 0

		_Brightness ("全局亮度", Range( 0, 2)) = 1
        _Contrast   ("全局对比度", Range( 0 , 3)) = 1
		_Saturation("全局饱和度", Float) = 1

	}

	SubShader {   

		pass {

		}

		//第一个pass
        Pass {
            ZWrite On //开启深度写入
            ColorMask 0
        }

		//第二个pass, 有人会问，为什么会有多个pass
        Pass {
            ZWrite Off //关闭深度写入
            Cull Back//剔除背面，就是告诉GPU不要渲染背面
            ColorMask RGB
            Blend SrcAlpha  OneMinusSrcAlpha
          
		  //模板测试，一般在输出合并阶段需要用到
            Stencil {
                Ref 0           
                Comp Equal          
                WriteMask 255       
                ReadMask 255
                Pass Invert
                Fail Keep
                ZFail Keep
            }
        

			CGPROGRAM

				#pragma vertex vert //固定写法，定义顶点着色器
				#pragma fragment frag //固定写法，定义片段着色i其

				float4 _ShadowPlane; //阴影平面
				float4 _ShadowProjDir;//阴影投影方向
				float4 _WorldPos; //世界方向
				float _ShadowInvLen;
				float _Scale;
				float4 _ShadowFadeParams;
				float _ShadowFalloff;
				float _Alpha;


				//应用程序阶段的数据，就是你从应用程序阶段要传什么数据给顶点着色器进行处理
				struct appdata {
					float4 vertex : POSITION; //顶点坐标信息
				};


				//顶点着色器程序阶段到片段着色器阶段数据结构
				struct v2f {
					float4 vertex : SV_POSITION;
					float4 xlv_TEXCOORD0 : TEXCOORD0;
				};


				//顶点着色器
				v2f vert(appdata v){
					v2f o;
					float3 lightdir = normalize(_ShadowProjDir);//归一化 阴影投影方向
					v.vertex.xyz = v.vertex.xyz * _Scale;
					float3 worldpos = mul(unity_ObjectToWorld, v.vertex).xyz; //本地转世界空间的矩阵变化
					float distance = (_ShadowPlane.w - dot(_ShadowPlane.xyz, worldpos)) / dot(_ShadowPlane.xyz, lightdir.xyz);
					worldpos = worldpos + distance * lightdir.xyz;
					o.vertex = mul(unity_MatrixVP, float4(worldpos, 1.0));
					o.xlv_TEXCOORD0.xyz = float3(0.0,0.0,0.0);
					float3 posToPlane_2 = (_WorldPos.xyz - worldpos);
					o.xlv_TEXCOORD0.w = (pow((1.0 - clamp(((sqrt(dot(posToPlane_2, posToPlane_2)) * _ShadowInvLen) - _ShadowFadeParams.x), 0.0, 1.0)), _ShadowFadeParams.y) * (_ShadowFadeParams.z * _Alpha));

					return o;
				}


				//片段着色器
				float4 frag(v2f i) : SV_Target {
					return i.xlv_TEXCOORD0;
				}

			ENDCG
    	}


		//标签，具体意思：表示当前shader是用来渲染透明队列的，不接受投影影响 渲染队列是透明队列
		Tags {"Queue" = "Transparent" "IgnoreProjector" = "True" "RenderType" = "Transparent"}
		ZWrite Off
		Cull Back
		ColorMask RGB
		Blend SrcAlpha Oneminussrcalpha



		//用了一个表面着色器，另外表面着色器代码一般定义在SubShader中
		CGPROGRAM

			//引入几个需要的内置的CG文件
			#include "UnityPBSLighting.cginc"
			#include "Lighting.cginc"
			#include "AutoLight.cginc"


			#pragma surface surf StandardCustomLighting alpha 
			#pragma fullforwardshadows
			#pragma target 3.0

			#ifdef UNITY_PASS_SHADOWCASTER
				#undef INTERNAL_DATA
				#undef WorldReflectionVector
				#undef WorldNormalVector
				#define INTERNAL_DATA half3 internalSurfaceTtoW0; half3 internalSurfaceTtoW1; half3 internalSurfaceTtoW2;
				#define WorldReflectionVector(data,normal) reflect (data.worldRefl, half3(dot(data.internalSurfaceTtoW0,normal), dot(data.internalSurfaceTtoW1,normal), dot(data.internalSurfaceTtoW2,normal)))
				#define WorldNormalVector(data,normal) half3(dot(data.internalSurfaceTtoW0,normal), dot(data.internalSurfaceTtoW1,normal), dot(data.internalSurfaceTtoW2,normal))
			#endif


			
			struct Input {
				float3 worldPos;
				float3 worldNormal;
				float3 worldRefl;
				float3 viewDir;
				INTERNAL_DATA
				float2 uv_texcoord;
			};

			struct SurfaceOutputCustomLightingCustom {
				half3 Albedo;
				half3 Normal;
				half3 Emission;
				half Metallic;
				half Smoothness;
				half Occlusion;
				half Alpha;
				Input SurfInput;
				UnityGIInput GIData;
			};

			uniform float _SideLightScale;
			uniform float4 _SideLightColor;
			uniform sampler2D _MaskMap;
			uniform float4 _MaskMap_ST;
			uniform sampler2D _MaskMap2;
			uniform float4 _MaskMap2_ST;
			uniform float4 _GlowColor;
			uniform sampler2D _AlbedoMap;
			uniform float4 _AlbedoMap_ST;
			uniform sampler2D _NormalMap;
			uniform float4 _NormalMap_ST;
			uniform sampler2D _NormalMap2;
			uniform float4 _NormalMap2_ST;
			uniform float _Metallic;
			uniform float _Smooth;
			uniform float4 _MaxColor;
			uniform float4 _MainColor;
			uniform float4 _MainColor2;

			float _BumpScale;
			float _BumpOffsetX;
			float _BumpOffsetY;
			float _Occlusion;
			float _Alpha;
			float _Contrast;

			samplerCUBE _CubeMap;
			fixed _CubeMapDirX;
			fixed _CubeMapDirY;
			float _AlphaA_Strength;
			float _AlphaB_Strength;

			float4 _Tint, _InteriorColor,_InteriorColor2,_SSSColor;
			float _FrontSubsurfaceDistortion, _BackSubsurfaceDistortion, _FrontSSSIntensity, _InteriorColorPower;
			float _PointLitRadius;
			float _Specular, _Gloss, _RimPower, _RimIntensity;

			sampler2D _RampTex;
			float _Gloss2;
			fixed4 _PreHitColor;
			float  _HitAniValue;
			fixed  _HitColorValue;
			fixed4 _HitColor;
			float  _HitSaturation;
			float  _TestGrayFlag;
			fixed4 _TestGrayColor;
			float _GlowStrength;
			float _MaskValue;
			float3 _viewDir;
			float _LightZ;
			float _SpecularHighlights;
			float _RimOffsetX;
			float _RimOffsetY;
			float _Brightness;
			half _Saturation;
			float _DirectionalLightDirX;
			float _DirectionalLightDirY;
			float _DirectionLightIntensity;
			float _CustomDirLight;
			float _Reflection;
			fixed4 _MainColor4;
			float _Attenuation;
			UNITY_DECLARE_TEXCUBE(_TexA);


			inline half3 UnityGI_IndirectSpecularEx(UnityGIInput data, half occlusion, Unity_GlossyEnvironmentData glossIn, float cubeMask) {
				half3 specular;
			
				#ifdef UNITY_SPECCUBE_BOX_PROJECTION
					// we will tweak reflUVW in glossIn directly (as we pass it to Unity_GlossyEnvironment twice for probe0 and probe1), so keep original to pass into BoxProjectedCubemapDirection
					half3 originalReflUVW = glossIn.reflUVW;
					glossIn.reflUVW = BoxProjectedCubemapDirection(originalReflUVW, data.worldPos, data.probePosition[0], data.boxMin[0], data.boxMax[0]);
				#endif
			
				#ifdef _GLOSSYREFLECTIONS_OFF
					specular = unity_IndirectSpecColor.rgb;
				#else
					half3 env0 = Unity_GlossyEnvironment(UNITY_PASS_TEXCUBE(unity_SpecCube0), data.probeHDR[0], glossIn);

					#ifdef UNITY_SPECCUBE_BLENDING
						const float kBlendFactor = 0.99999;
						float blendLerp = data.boxMin[0].w;
						UNITY_BRANCH
						if (blendLerp < kBlendFactor)
						{
							#ifdef UNITY_SPECCUBE_BOX_PROJECTION
								glossIn.reflUVW = BoxProjectedCubemapDirection(originalReflUVW, data.worldPos, data.probePosition[1], data.boxMin[1], data.boxMax[1]);
							#endif
				
							half3 env1 = Unity_GlossyEnvironment(UNITY_PASS_TEXCUBE_SAMPLER(_TexA,unity_SpecCube0), data.probeHDR[1], glossIn);
							specular = lerp(env1, env0, blendLerp);
						}
						else
						{
							specular = env0;
						}
					#else
						specular = env0;
					#endif
				#endif
			
				return specular * _Reflection * occlusion;
			}



			inline UnityGI UnityGlobalIlluminationEx(UnityGIInput data, half occlusion, half3 normalWorld, Unity_GlossyEnvironmentData glossIn, float cubeMask) {
				UnityGI o_gi = UnityGI_Base(data, occlusion, normalWorld);
				o_gi.indirect.specular = UnityGI_IndirectSpecularEx(data, occlusion, glossIn, cubeMask);
				return o_gi;
			}


			inline half4 LightingStandardCustomLighting(inout SurfaceOutputCustomLightingCustom s, half3 viewDir, UnityGI gi) {
				UnityGIInput data = s.GIData;
				Input i = s.SurfInput;
				half4 c = 0;

				float3 ase_worldNormal = s.Normal;
				
				float2 uv_MaskMap = i.uv_texcoord * _MaskMap_ST.xy + _MaskMap_ST.zw;
				float4 tex2DNode1 = tex2D(_MaskMap, uv_MaskMap);


				float2 uv_MaskMap2 = i.uv_texcoord * _MaskMap2_ST.xy + _MaskMap2_ST.zw;
				float4 tex2DNode1_2 = tex2D(_MaskMap2, uv_MaskMap2);

			

				SurfaceOutputStandard s6 = (SurfaceOutputStandard)0;
				float2 uv_AlbedoMap = i.uv_texcoord * _AlbedoMap_ST.xy + _AlbedoMap_ST.zw;

				float _occlusionTex = tex2DNode1.g;
				half2 metallicGloss = tex2DNode1.ra;
				half metallic = metallicGloss.x;
				half smoothness = metallicGloss.y;

				float3 ase_worldPos = i.worldPos;
				float3 worldLightDir = UnityWorldSpaceLightDir(i.worldPos);
				float3 worldViewDir = normalize(UnityWorldSpaceViewDir(i.worldPos));
				float originalLambert = dot(worldLightDir, ase_worldNormal);
				float halfLambert = originalLambert * 0.5 + 0.5;
				float4 mcol = tex2D(_AlbedoMap, uv_AlbedoMap);
				float3 diff = mcol.rgb * _LightColor0.rgb * 0.75 * tex2D(_RampTex, float2(halfLambert, halfLambert));

				float3 halfDir = normalize(worldLightDir + worldViewDir);
				float3 spec = mcol.rgb * _LightColor0.rgb * 0.25 * pow(saturate(dot(halfDir, ase_worldNormal)), _Gloss2);

				float3 col = spec + diff;

				if (_HitColorValue > 0 && _TestGrayFlag > 0) {
					float grey = dot(_MainColor.rgb, float3(_TestGrayColor.r,_TestGrayColor.g,_TestGrayColor.b));
					s6.Albedo = float3(grey, grey, grey);
				}else {
					s6.Albedo = (tex2D(_AlbedoMap, uv_AlbedoMap).rgb) * _MainColor.rgb;
				}


				fixed3 avgColor = fixed3(0.5, 0.5, 0.5);
				s6.Albedo = lerp(avgColor, s6.Albedo, _Contrast);
				fixed gray = 0.2125 * tex2D(_AlbedoMap, uv_AlbedoMap).r + 0.7154 * tex2D(_AlbedoMap, uv_AlbedoMap).g + 0.0721 * tex2D(_AlbedoMap, uv_AlbedoMap).b;
				fixed3 grayColor = fixed3(gray, gray, gray);
				s6.Albedo = lerp(grayColor, s6.Albedo, _Saturation);
				float2 uv_NormalMap = i.uv_texcoord * _NormalMap_ST.xy + _NormalMap_ST.zw;
				fixed3 tangentNormal = UnpackNormal(tex2D(_NormalMap, uv_NormalMap));
				tangentNormal.xy = tangentNormal.xy * _BumpScale;
				tangentNormal.z = sqrt(1.0 - saturate(dot(tangentNormal.xy, tangentNormal.xy)));
				tangentNormal = normalize(tangentNormal);
				s6.Normal = normalize(WorldNormalVector(i, tangentNormal));
				float3 _viewDir = float3(0,0,0);
				_viewDir += worldViewDir;
				_viewDir.x += _RimOffsetX;
				_viewDir.y += _RimOffsetY;

				float fresnelNdotV11 = dot(WorldNormalVector(i , max(0, tangentNormal)), _viewDir);
				float fresnelNode11 = (_SideLightScale * pow(1 - fresnelNdotV11, 3.0));			
				if (_HitColorValue > 0) {
					s6.Emission = s6.Emission * _HitSaturation;
						
					s6.Emission.r += _PreHitColor.r + (_HitColor.r - _PreHitColor.r) * _HitAniValue;
					s6.Emission.g += _PreHitColor.g + (_HitColor.g - _PreHitColor.g) * _HitAniValue;
					s6.Emission.b += _PreHitColor.b + (_HitColor.b - _PreHitColor.b) * _HitAniValue;

					s6.Emission = s6.Emission * _HitColorValue;
				}else {
					s6.Emission = (tex2DNode1_2.b * (_MaskValue)) * _GlowColor * _GlowStrength + (tex2DNode1.b * (1 - _MaskValue)) * _GlowColor * _GlowStrength;//test	
				}
					
				s6.Metallic = (metallic * _Metallic);//正式
				s6.Smoothness = (smoothness * (_Smooth))* halfLambert;//正式		
				s6.Occlusion = LerpOneTo(_occlusionTex, _Occlusion);//正式
				data.light = gi.light;
				data.atten = _Attenuation* originalLambert;
				data.ambient = _MainColor4;

				if (_CustomDirLight == 1.0)
				{
					data.light.color = _MainColor2.rgb * _DirectionLightIntensity;
					data.light.dir = float3(_DirectionalLightDirX, _DirectionalLightDirY, -1);
				}
					
				float rim;
				if (_RimPower == 0)
				{
					rim = (1.0 - dot(float3(s6.Normal.x,s6.Normal.y,max(0, s6.Normal.z)), worldViewDir));
				}
				else
				{
					rim = (1.0 - dot(float3(s6.Normal.x,s6.Normal.y,s6.Normal.z), worldViewDir));
				}
				float3 rimCol = lerp(_InteriorColor, _InteriorColor2, rim) * pow(rim, _RimPower) * _RimIntensity;
	
				_viewDir += half3(1, 1, -1);
				_viewDir.x *= _CubeMapDirX * 5;
				_viewDir.y *= _CubeMapDirY * 5;

				UnityGI gi6 = gi;
																							
				Unity_GlossyEnvironmentData g6 = UnityGlossyEnvironmentSetup(s6.Smoothness, normalize(_viewDir), s6.Normal, lerp(unity_ColorSpaceDielectricSpec.rgb, s.Albedo, s6.Metallic));
				gi6 = UnityGlobalIlluminationEx(data, s6.Occlusion, s6.Normal, g6, tex2DNode1.b);
				float3 surfResult6 = LightingStandard(s6, worldViewDir, gi6).rgb;
				surfResult6 += (s6.Emission);
				float3 clampResult9 = surfResult6;
				

				c.rgb = clampResult9 * _Brightness + saturate(fresnelNode11 * _SideLightColor)* halfLambert + rimCol;
				c.a = _MainColor.a* mcol.a* _Alpha* tex2DNode1_2.r;
						

				return c;
			}

			inline void LightingStandardCustomLighting_GI(inout SurfaceOutputCustomLightingCustom s, UnityGIInput data, inout UnityGI gi) {
				s.GIData = data;
			}

			//表面着色器代码
			void surf(Input i , inout SurfaceOutputCustomLightingCustom o) {
				o.SurfInput = i;
				o.Normal = float3(0,0,1);
			}

			ENDCG
    
	}



	Fallback "Diffuse" 
}