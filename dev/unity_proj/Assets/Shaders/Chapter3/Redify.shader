Shader "Unity Shaders Book/Chapter 3/Redify" {
    Properties {
        _MainTex ("Base (RGB)", 2D) = "white" {}
    }

    //可以包含多个subshader，但是Unity会骚猫所有的subshader，然后选择一个合适的来使用
    SubShader {  
        Tags { "RenderType"="Opaque" }
        LOD    //如果在subShader内部设置了tags标签等，会运用于所有的pass
        
        CGPROGRAM //CG代码段

        #pragma surface surf Lambert addshadow
        #pragma shader_feature REDIFY_ON

        sampler2D _MainTex; //如果在CG代码段内，你要使用Properties块里的变量，必须先声明

        struct Input {
            float2 uv_MainTex;
        };
        
        void surf (Input IN, inout SurfaceOutput o) {



            half4 c = tex2D (_MainTex, IN.uv_MainTex);
            o.Albedo = c.rgb;
            o.Alpha = c.a;

            #if REDIFY_ON
            o.Albedo.gb *= 0.5;
            #endif
        }
        ENDCG
    } 
    CustomEditor "CustomShaderGUI"
}