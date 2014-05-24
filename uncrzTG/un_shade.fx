// lush lush lush

struct VS_Input
{
	float4 pos : POSITION0;
	float4 nrm : NORMAL0;
	float4 col : COLOR0;
	float tti : TEXCOORD0;
};

struct VS_Output
{
	float4 pos : POSITION0;
	float4 nrm : TEXCOORD2;
	float4 altPos : TEXCOORD1;
	float4 col : COLOR0;
};

struct VS_Input_Tex
{
	float4 pos : POSITION0;
	float4 nrm : NORMAL0;
	float4 col : COLOR0;
	float2 txc : TEXCOORD0;
	float tti : TEXCOORD1;
};

struct VS_Output_Tex
{
	float4 pos : POSITION0;
	float lit : TEXCOORD3;
	float4 altPos : TEXCOORD1;
	float4 col : COLOR0;
	float2 txc : TEXCOORD0;
	float4 lmc: TEXCOORD2;
};

struct VS_Input_Decal
{
	float4 pos : POSITION0;
	float4 nrm : NORMAL0;
	float col : COLOR0;
	float4 txc : TEXCOORD0;
	float tti : TEXCOORD1;
};

struct VS_Output_Decal
{
	float4 pos : POSITION0;
	float lit : TEXCOORD3;
	float4 altPos : TEXCOORD1;
	float col : COLOR0;
	float4 txc : TEXCOORD0;
	float4 lmc: TEXCOORD2;
};

struct VS_Output_Dyn
{
	float4 pos : POSITION0;
	float4 altPos : TEXCOORD0;
	float4 lmc: TEXCOORD1;
};

struct VS_Input_Terrain
{
	float4 pos : POSITION0;
	float4 nrm : NORMAL0;
	float2 txc : TEXCOORD0;
	float4 w : TEXCOORD1;
};

struct VS_Output_Terrain
{
	float4 pos : POSITION0;
	float lit : TEXCOORD5;
	float2 txc : TEXCOORD0;
	float4 w : TEXCOORD1;
	float4 altPos : TEXCOORD2;
	float orgY : TEXCOORD3;
	float4 lmc: TEXCOORD4;
};

struct VS_Output_Light
{
	float4 pos : POSITION0;
	float4 altPos : TEXCOORD0;
};

struct VS_Input_Over
{
	float4 pos : POSITION0;
	float2 txc : TEXCOORD0;
};

struct VS_Output_Over
{
	float4 pos : POSITION0;
	float2 txc : TEXCOORD0;
	float4 altPos : TEXCOORD1;
};

struct VS_Output_Water
{
	float4 pos : POSITION0;
	float2 txc : TEXCOORD0;
	float4 waterReflectPos : TEXCOORD1;
	float4 waterRefractPos : TEXCOORD2;
	float4 altPos : TEXCOORD3;
};

struct PS_Output
{
	float4 col : COLOR0;
};

float4x4 viewMat;
float4x4 projMat;
float4x4 viewProj; // viewMat * viewProj... or the other way round, I don't really know
float4x4 lightViewProj;
float4 eyePos;
float4 eyeDir;

Texture tex;
sampler texSampler = sampler_state { texture = <tex>;magfilter = NONE; minfilter = NONE; mipfilter = NONE; AddressU = mirror; AddressV = mirror;}; // no linear 
sampler texBorderSampler = sampler_state { texture = <tex>;magfilter = NONE; minfilter = NONE; mipfilter = NONE; AddressU = border; AddressV = border; BorderColor = 0x00000000;}; // border
sampler texLinearSampler = sampler_state { texture = <tex>;magfilter = LINEAR; minfilter = LINEAR; mipfilter = LINEAR; AddressU = mirror; AddressV = mirror;}; // linear 
sampler texLinearBorderSampler = sampler_state { texture = <tex>;magfilter = LINEAR; minfilter = LINEAR; mipfilter = LINEAR; AddressU = border; AddressV = border; BorderColor = 0x00000000;}; // linear 
sampler texNonMipLinearSampler = sampler_state { texture = <tex>;magfilter = LINEAR; minfilter = LINEAR; mipfilter = NONE; AddressU = mirror; AddressV = mirror;}; // linear 
sampler texNonMipLinearBorderSampler = sampler_state { texture = <tex>;magfilter = LINEAR; minfilter = LINEAR; mipfilter = NONE; AddressU = border; AddressV = border; BorderColor = 0x00000000;}; // linear 
float4 texData; // (offX, offY,) mulX, mulY

float4x4 vpMat;
float4 targTexData; // offX, offY, mulX, mulY
Texture targTex;
sampler targTexSampler = sampler_state { texture = <targTex>;magfilter = NONE; minfilter = NONE; mipfilter = NONE; AddressU = mirror; AddressV = mirror;};
sampler targTexLinearSampler = sampler_state { texture = <targTex>;magfilter = LINEAR; minfilter = LINEAR; mipfilter = LINEAR; AddressU = mirror; AddressV = mirror;}; // linear 
sampler targTexLinearBorderSampler = sampler_state { texture = <targTex>;magfilter = LINEAR; minfilter = LINEAR; mipfilter = LINEAR; AddressU = border; AddressV = border; BorderColor = 0x00000000;}; // linear 
sampler targTexNonMipLinearSampler = sampler_state { texture = <targTex>;magfilter = LINEAR; minfilter = LINEAR; mipfilter = NONE; AddressU = mirror; AddressV = mirror;}; // linear 
sampler targTexNonMipLinearBorderSampler = sampler_state { texture = <targTex>;magfilter = LINEAR; minfilter = LINEAR; mipfilter = NONE; AddressU = border; AddressV = border; BorderColor = 0x00000000;}; // linear 

float lightCoof;
float lightConeness;
float lightDepth = 100;
float4 lightPos;
float4 lightDir;
float lightDodge = 0.001;
float lightType;
float4 lightAmbient;
float4 lightColMod;

Texture lightTex;
sampler lightTexSampler = sampler_state { texture = <lightTex>;magfilter = NONE; minfilter = NONE; mipfilter = NONE; AddressU = border; AddressV = border; BorderColor = 0xFFFFFFFF;};
sampler lightTexLinearSampler = sampler_state { texture = <lightTex>;magfilter = LINEAR; minfilter = LINEAR; mipfilter = LINEAR; AddressU = border; AddressV = border; BorderColor = 0xFFFFFFFF;};
Texture lightPatternTex;
sampler lightPatternTexSampler = sampler_state { texture = <lightPatternTex>;magfilter = NONE; minfilter = NONE; mipfilter = NONE; AddressU = border; AddressV = border; BorderColor = 0x00000000;};
sampler lightPatternTexLinearSampler = sampler_state { texture = <lightPatternTex>;magfilter = LINEAR; minfilter = LINEAR; mipfilter = LINEAR; AddressU = border; AddressV = border; BorderColor = 0x00000000;};

Texture tex0;
sampler tex0Sampler = sampler_state { texture = <tex0>;magfilter = NONE; minfilter = NONE; mipfilter = NONE; AddressU = mirror; AddressV = mirror;};
sampler tex0LinearSampler = sampler_state { texture = <tex0>;magfilter = LINEAR; minfilter = LINEAR; mipfilter = LINEAR; AddressU = mirror; AddressV = mirror;};
sampler tex0NonMipLinearSampler = sampler_state { texture = <tex0>;magfilter = LINEAR; minfilter = LINEAR; mipfilter = NONE; AddressU = mirror; AddressV = mirror;}; // linear 
Texture tex1;
sampler tex1Sampler = sampler_state { texture = <tex1>;magfilter = NONE; minfilter = NONE; mipfilter = NONE; AddressU = mirror; AddressV = mirror;};
sampler tex1LinearSampler = sampler_state { texture = <tex1>;magfilter = LINEAR; minfilter = LINEAR; mipfilter = LINEAR; AddressU = mirror; AddressV = mirror;};
sampler tex1NonMipLinearSampler = sampler_state { texture = <tex1>;magfilter = LINEAR; minfilter = LINEAR; mipfilter = NONE; AddressU = mirror; AddressV = mirror;}; // linear 
Texture tex2;
sampler tex2Sampler = sampler_state { texture = <tex2>;magfilter = NONE; minfilter = NONE; mipfilter = NONE; AddressU = mirror; AddressV = mirror;};
sampler tex2LinearSampler = sampler_state { texture = <tex2>;magfilter = LINEAR; minfilter = LINEAR; mipfilter = LINEAR; AddressU = mirror; AddressV = mirror;};
sampler tex2NonMipLinearSampler = sampler_state { texture = <tex2>;magfilter = LINEAR; minfilter = LINEAR; mipfilter = NONE; AddressU = mirror; AddressV = mirror;}; // linear 
Texture tex3;
sampler tex3Sampler = sampler_state { texture = <tex3>;magfilter = NONE; minfilter = NONE; mipfilter = NONE; AddressU = mirror; AddressV = mirror;};
sampler tex3LinearSampler = sampler_state { texture = <tex3>;magfilter = LINEAR; minfilter = LINEAR; mipfilter = LINEAR; AddressU = mirror; AddressV = mirror;};
sampler tex3NonMipLinearSampler = sampler_state { texture = <tex3>;magfilter = LINEAR; minfilter = LINEAR; mipfilter = NONE; AddressU = mirror; AddressV = mirror;}; // linear 

float4x4 transarr[30]; // need to be same as number of segs
float4 spriteLoc[100]; // sprite buffer size must be no more than (this len / size of sprite data)
float4 spriteDim;
float4 colMod;
float farDepth = 1000;
float invFarDepth = 0.001;

float4x4 mats[8]; // generic mats, use and abuse, my friends

float ticker = 0;

float clampPositive(float num)
{
	if (num < 0)
		return 0;
	return num;
}

float4 normaliseXYZ(float4 vec)
{
	float mod = rsqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
	return vec * mod;
}

float4 reflect(float4 dir, float4 nrm)
{
	return dir + (nrm * -dot(nrm, dir)) * 2.0;
}

float4 lightTransOrtho(float4 pos)
{
	float4 res = mul(pos, lightViewProj);
	return res;
}

float4 lightTransPersp(float4 pos)
{
	float4 res = mul(pos, lightViewProj);
	res.z = res.z * res.w / lightDepth;
	return res;
}

float4 lightTransPoint(float4 pos)
{
	pos.w = 1;
	return pos;
}

float4 lightTrans(float4 pos)
{
	if (lightType == 0)
		return lightTransOrtho(pos);
	else if (lightType == 1)
		return lightTransPersp(pos);
	else if (lightType == 2)
		return lightTransPoint(pos);
	return (float4)0;
}

float4 lightTransOrthoVP(float4 pos)
{
	float4 res = mul(pos, viewProj);
	return res;
}

float4 lightTransPerspVP(float4 pos)
{
	float4 res = mul(pos, viewProj);
	res.z = res.z * res.w / lightDepth;
	return res;
}

float4 lightTransPointVP(float4 pos)
{
	pos.w = 1;
	return pos;
}

float4 lightTransVP(float4 pos)
{
	if (lightType == 0)
		return lightTransOrthoVP(pos);
	else if (lightType == 1)
		return lightTransPerspVP(pos);
	else if (lightType == 2)
		return lightTransPointVP(pos);
	return (float4)0;
}

float4 lightUnTransOrtho(float4 pos)
{
	float4 res = pos;
	return res;
}

float4 lightUnTransPersp(float4 pos)
{
	float4 res = pos;
	res.x = res.x / res.w;
	res.y = res.y / res.w;
	res.z = res.z / res.w;
	return res;
}

float4 lightUnTransPoint(float4 pos)
{
	return pos;
}

float4 lightUnTrans(float4 pos)
{
	if (lightType == 0)
		return lightUnTransOrtho(pos);
	else if (lightType == 1)
		return lightUnTransPersp(pos);
	else if (lightType == 2)
		return lightUnTransPoint(pos);
	return (float4)0;
}

float lightLitnessOrtho(float4 pos, float4 nrm)
{
	float res = -dot(nrm, lightDir);
	return res;
}

float lightLitnessPersp(float4 pos, float4 nrm)
{
	float4 plDir = pos - lightPos;
	plDir = normaliseXYZ(plDir);

	float res = -dot(nrm, plDir);
	return res;
}

/*float lightLitnessPoint(float4 pos, float4 nrm)
{
	float4 plDir = pos - lightPos;
	plDir = normaliseXYZ(plDir);

	float res = -dot(nrm, plDir);
	return res;
}*/

// reflectiveness (not quite specular)
float lightLitnessPoint(float4 pos, float4 nrm)
{
	float4 plDir = pos - lightPos;
	plDir = normaliseXYZ(plDir);
	float4 eDir = pos - eyePos; // eyeDir just doesn't cut it
	eDir = normaliseXYZ(eDir);

	plDir.w = 0; // need a zero w for the dot
	float res = -dot(nrm, plDir); // dullness
	//res += -dot(plDir, reflect(eDir, nrm)); // reflectivness
	return res;
}

float lightLitness(float4 pos, float4 nrm)
{
	if (lightType == 0)
		return lightLitnessOrtho(pos, nrm);
	else if (lightType == 1)
		return lightLitnessPersp(pos, nrm);
	else if (lightType == 2)
		return lightLitnessPoint(pos, nrm);
	return 0;
}

float4 calcLightModOrtho(float4 lmc)
{
	lmc = lightUnTransOrtho(lmc);

	float4 lightMod = 0.0;

	float2 lightCoords;
	lightCoords.x = lmc.x;
	lightCoords.y = lmc.y;

	float targDist = lmc.z;

	float4 lightCol = tex2D(lightTexLinearSampler, lightCoords);
	float lightDist = lightCol.x;

	if (lightDodge + lightDist > targDist)
	{
		lightMod = tex2D(lightPatternTexLinearSampler, lightCoords) * lightColMod;
		return lightMod;
	}
	lightMod = 0;
	return lightMod;
}
// these two (calcLightModOrtho and calcLightModPersp) are the SAME ATM
float4 calcLightModPersp(float4 lmc)
{
	lmc = lightUnTransPersp(lmc);

	float4 lightMod = 0.0;

	float2 lightCoords;
	lightCoords.x = lmc.x;
	lightCoords.y = lmc.y;

	float targDist = lmc.z;

	float4 lightCol = tex2D(lightTexLinearSampler, lightCoords);
	float lightDist = lightCol.x;

	if (lightDodge + lightDist > targDist)
	{
		lightMod = tex2D(lightPatternTexLinearSampler, lightCoords) * lightColMod;
		lightMod *= (1 - targDist * targDist);
		return lightMod;
	}
	lightMod = 0;
	return lightMod;
}

float4 calcLightModPoint(float4 lmc)
{
	// lmc is infact orignal pos in disguise
	lmc = lightUnTransPoint(lmc);

	float4 lightMod = 0.0;

	float x = lmc.x - lightPos.x;
	float y = lmc.y - lightPos.y;
	float z = lmc.z - lightPos.z;

	float targDist = (x * x + y * y + z * z);

	targDist = 1.0 - targDist / (lightDepth * lightDepth);
	targDist = max(targDist, 0.0); // clamp down

	lightMod = lightColMod * targDist;
	return lightMod;
}

float4 calcLightMod_Switch(float4 lmc)
{
	if (lightType == 0)
		return calcLightModOrtho(lmc);
	else if (lightType == 1)
		return calcLightModPersp(lmc);
	else if (lightType == 2)
		return calcLightModPoint(lmc);
	return 0;
}

float4 calcDynModOrtho(float4 lmc)
{
	lmc = lightUnTransOrtho(lmc);

	float4 lightMod = 0.0;

	float2 lightCoords;
	lightCoords.x = lmc.x;
	lightCoords.y = lmc.y;

	lightMod = tex2D(lightPatternTexSampler, lightCoords) * lightColMod;

	return lightMod;
}

float4 calcDynModPersp(float4 lmc)
{
	lmc = lightUnTransPersp(lmc);

	float4 lightMod = 0.0;

	float2 lightCoords;
	lightCoords.x = lmc.x;
	lightCoords.y = lmc.y;

	lightMod = tex2D(lightPatternTexSampler, lightCoords) * lightColMod;

	return lightMod;
}

float4 calcDynMod(float4 lmc)
{
	if (lightType == 0)
		return calcDynModOrtho(lmc);
	else if (lightType == 1)
		return calcDynModPersp(lmc);
	return 0;
}




VS_Output VShade_Dull(VS_Input inp)
{
	VS_Output outp = (VS_Output)0;
	outp.pos = mul(inp.pos, viewProj);
	outp.col = inp.col;
	return outp;
}

PS_Output PShade_Dull(VS_Output inp)
{
	PS_Output outp = (PS_Output)0;
	outp.col = inp.col;
	return outp;
}



VS_Output_Tex VShade_Ui(VS_Input_Tex inp)
{
	VS_Output_Tex outp = (VS_Output_Tex)0;
	
	outp.pos = inp.pos;
	outp.pos.w = 1;
	outp.col = inp.col;
	outp.txc = inp.txc;

	return outp;
}

PS_Output PShade_Ui(VS_Output_Tex inp)
{
	PS_Output outp = (PS_Output)0;
	outp.col = inp.col * tex2D(texLinearSampler, inp.txc) * colMod;

	return outp;
}

PS_Output PShade_Ui_Border(VS_Output_Tex inp)
{
	PS_Output outp = (PS_Output)0;
	outp.col = inp.col * tex2D(texLinearBorderSampler, inp.txc) * colMod;

	return outp;
}



VS_Output VShade(VS_Input inp)
{
	VS_Output outp = (VS_Output)0;
	if (inp.tti >= 0)
	{
		outp.pos = mul(mul(inp.pos, transarr[inp.tti]), viewProj);
	}
	else
	{
		outp.pos = mul(inp.pos, viewProj);
	}
	outp.col = inp.col;
	return outp;
}

PS_Output PShade(VS_Output inp)
{
	PS_Output outp = (PS_Output)0;
	outp.col = inp.col * colMod;

	return outp;
}


// Tex
VS_Output_Tex VShade_Tex(VS_Input_Tex inp)
{
	VS_Output_Tex outp = (VS_Output_Tex)0;
	inp.pos = mul(inp.pos, transarr[inp.tti]);
	inp.nrm = mul(inp.nrm, transarr[inp.tti]);
	outp.pos = mul(inp.pos, viewProj);
	outp.altPos = outp.pos;
	outp.altPos.z = outp.altPos.z * outp.altPos.w * invFarDepth;
	outp.col = inp.col;
	outp.txc = inp.txc;

	return outp;
}

/// TEST CODE
VS_Output_Tex VShade_Tex_Test(VS_Input_Tex inp)
{
	VS_Output_Tex outp = (VS_Output_Tex)0;
	inp.pos = mul(inp.pos, transarr[inp.tti]);
	inp.nrm = mul(inp.nrm, transarr[inp.tti]);
	outp.pos = mul(inp.pos, viewProj);
	outp.altPos = outp.pos;
	outp.col = inp.col;
	outp.txc = inp.txc;

	return outp;
}
/// END TEST CODE

/// TEST CODE
VS_Output_Water VShade_Tex_Test2(VS_Input_Tex inp)
{
	VS_Output_Water outp = (VS_Output_Water)0;
	inp.pos = mul(inp.pos, transarr[inp.tti]);
	inp.nrm = mul(inp.nrm, transarr[inp.tti]);

	outp.pos = mul(inp.pos, viewProj);

	outp.altPos = outp.pos;
	outp.altPos.z = outp.altPos.z * outp.altPos.w / farDepth;
	outp.waterReflectPos = mul(inp.pos, mats[0]);
	outp.waterReflectPos.z = outp.waterReflectPos.z * outp.waterReflectPos.w / farDepth;
	outp.waterRefractPos = mul(inp.pos, mats[1]);
	outp.waterRefractPos.z = outp.waterRefractPos.z * outp.waterRefractPos.w / farDepth;
	outp.txc = inp.txc;

	return outp;
}
/// END TEST CODE

/// TEST CODE
VS_Output_Tex VShade_Tex_Test3(VS_Input_Tex inp)
{
	VS_Output_Tex outp = (VS_Output_Tex)0;
	inp.pos = mul(inp.pos, transarr[inp.tti]);
	inp.nrm = mul(inp.nrm, transarr[inp.tti]);
	outp.pos = mul(inp.pos, viewProj);
	outp.altPos = outp.pos;
	outp.col = inp.col;
	outp.txc = inp.txc;

	return outp;
}
/// END TEST CODE

// VShade_Tex_Lit
#define STD_MCR_VShade_Tex_Lit(lightName) \
VS_Output_Tex VShade_Tex_Lit##lightName##(VS_Input_Tex inp) \
{ \
	VS_Output_Tex outp = (VS_Output_Tex)0; \
	inp.pos = mul(inp.pos, transarr[inp.tti]); \
	inp.nrm = mul(inp.nrm, transarr[inp.tti]); \
	outp.pos = mul(inp.pos, viewProj); \
	outp.altPos = outp.pos; \
	outp.col = inp.col; \
	outp.txc = inp.txc; \
 \
	outp.lit = lightLitness##lightName##(inp.pos, inp.nrm); \
	outp.lmc = lightTrans##lightName##(inp.pos); \
 \
	return outp; \
}

STD_MCR_VShade_Tex_Lit(Ortho)
STD_MCR_VShade_Tex_Lit(Persp)
STD_MCR_VShade_Tex_Lit(Point)

// VShade_Tex_Light
#define STD_MCR_VShade_Tex_Light(lightName) \
VS_Output_Light VShade_Tex_Light##lightName##(VS_Input_Tex inp) \
{ \
	VS_Output_Light outp = (VS_Output_Light)0; \
	outp.pos = lightTrans##lightName##VP(mul(inp.pos, transarr[inp.tti])); \
	outp.altPos = outp.pos; \
	return outp; \
}

STD_MCR_VShade_Tex_Light(Ortho)
STD_MCR_VShade_Tex_Light(Persp)
STD_MCR_VShade_Tex_Light(Point)



// TexNoTti
VS_Output_Tex VShade_TexNoTti(VS_Input_Tex inp)
{
	VS_Output_Tex outp = (VS_Output_Tex)0;
	outp.pos = mul(inp.pos, viewProj);
	outp.altPos = outp.pos;
	outp.altPos.z = outp.altPos.z * outp.altPos.w * invFarDepth;
	outp.col = inp.col;
	outp.txc = inp.txc;

	return outp;
}

// VShade_TexNoTti_Lit
#define STD_MCR_VShade_TexNoTti_Lit(lightName) \
VS_Output_Tex VShade_TexNoTti_Lit##lightName##(VS_Input_Tex inp) \
{ \
	VS_Output_Tex outp = (VS_Output_Tex)0; \
	outp.pos = mul(inp.pos, viewProj); \
	outp.altPos = outp.pos; \
	outp.col = inp.col; \
	outp.txc = inp.txc; \
 \
	outp.lit = lightLitness##lightName##(inp.pos, inp.nrm); \
	outp.lmc = lightTrans##lightName##(inp.pos); \
 \
	return outp; \
}


// VShade_Tex_DynamicDecal
VS_Output_Dyn VShade_Tex_DynOrtho(VS_Input_Tex inp)
{
	VS_Output_Dyn outp = (VS_Output_Dyn)0;
	outp.pos = mul(inp.pos, viewProj);
	outp.altPos = outp.pos;
	outp.altPos.z = outp.altPos.z * outp.altPos.w * invFarDepth;
	outp.lmc = lightTransOrtho(inp.pos);

	return outp;
}

VS_Output_Dyn VShade_Tex_DynPersp(VS_Input_Tex inp)
{
	VS_Output_Dyn outp = (VS_Output_Dyn)0;
	outp.pos = mul(inp.pos, viewProj);
	outp.altPos = outp.pos;
	outp.altPos.z = outp.altPos.z * outp.altPos.w * invFarDepth;
	outp.lmc = lightTransPersp(inp.pos);

	return outp;
}


STD_MCR_VShade_TexNoTti_Lit(Ortho)
STD_MCR_VShade_TexNoTti_Lit(Persp)
STD_MCR_VShade_TexNoTti_Lit(Point)

// VShade_Tex_Light
#define STD_MCR_VShade_TexNoTti_Light(lightName) \
VS_Output_Light VShade_TexNoTti_Light##lightName##(VS_Input_Tex inp) \
{ \
	VS_Output_Light outp = (VS_Output_Light)0; \
	outp.pos = lightTrans##lightName##VP(inp.pos); \
	outp.altPos = outp.pos; \
	return outp; \
}

STD_MCR_VShade_TexNoTti_Light(Ortho)
STD_MCR_VShade_TexNoTti_Light(Persp)
STD_MCR_VShade_TexNoTti_Light(Point)



// VShade_Sprite_Light
VS_Output_Light VShade_Sprite_LightOrtho(VS_Input_Tex inp)
{
	return (VS_Output_Light)0;
}


// VShade_Sprite_Light
VS_Output_Light VShade_Sprite_LightPersp(VS_Input_Tex inp)
{
	return (VS_Output_Light)0;
}


// VShade_Sprite_Light
VS_Output_Light VShade_Sprite_LightPoint(VS_Input_Tex inp)
{
	return (VS_Output_Light)0;
}



// VShade_Sprite
VS_Output_Tex VShade_Sprite(VS_Input_Tex inp)
{
	VS_Output_Tex outp = (VS_Output_Tex)0;
	float4 centre = (float4)0;
	if (inp.tti >= 0)
	{
		centre = spriteLoc[inp.tti];
	}
	inp.pos *= spriteDim;
	centre = mul(centre, viewMat);
	outp.pos = mul(centre + inp.pos, projMat);
	outp.altPos = outp.pos;
	outp.altPos.z = outp.altPos.z * outp.altPos.w * invFarDepth;
	outp.col = inp.col;
	outp.txc = inp.txc;

	return outp;
}

// VShade_Sprite_Lit
#define STD_MCR_VShade_Sprite_Lit(lightName) \
VS_Output_Tex VShade_Sprite_Lit##lightName##(VS_Input_Tex inp) \
{ \
	VS_Output_Tex outp = (VS_Output_Tex)0; \
	float4 centre = (float4)0; \
	if (inp.tti >= 0) \
	{ \
		centre = spriteLoc[inp.tti]; \
	} \
	inp.pos *= spriteDim; \
	centre = mul(centre, viewMat); \
	outp.pos = mul(centre + inp.pos, projMat); \
	outp.altPos = outp.pos; \
	outp.altPos.z = outp.altPos.z * outp.altPos.w * invFarDepth; \
	outp.col = inp.col; \
	outp.txc = inp.txc; \
 \
	/*(not implemented, this is only here because I can't be bothered to remove it, I'd much rather type this, I make sense)*/ \
	outp.lit = lightLitness##lightName##(inp.pos, inp.nrm); \
	outp.lmc = lightTrans##lightName##(inp.pos); \
 \
	return outp; \
}

STD_MCR_VShade_Sprite_Lit(Ortho)
STD_MCR_VShade_Sprite_Lit(Persp)
STD_MCR_VShade_Sprite_Lit(Point)


VS_Output_Tex VShade_Sprite_Flat(VS_Input_Tex inp)
{
	VS_Output_Tex outp = (VS_Output_Tex)0;
	float4 centre = (float4)0;
	if (inp.tti >= 0)
	{
		centre = spriteLoc[inp.tti];
	}
	float4 oth = spriteLoc[inp.tti + 1];
	inp.pos *= spriteDim;
	centre = mul(centre, viewMat);
	outp.pos = mul(centre + inp.pos, projMat);
	outp.altPos = outp.pos;
	outp.altPos.z = outp.altPos.z * outp.altPos.w * invFarDepth;
	outp.col = inp.col;
	outp.col.w *= oth.w;
	outp.txc = inp.txc;

	return outp;
}



// VShade_Sprite_Fire
VS_Output_Tex VShade_Sprite_Fire(VS_Input_Tex inp)
{
	VS_Output_Tex outp = (VS_Output_Tex)0;
	float4 centre = (float4)0;
	if (inp.tti >= 0)
	{
		centre = spriteLoc[inp.tti];
	}
	float4 oth = spriteLoc[inp.tti + 1];
	inp.pos *= spriteDim;
	if (oth.x > oth.z)
	{
		inp.pos *= ((oth.y - oth.x) / (oth.y - oth.z));
	}
	centre = mul(centre, viewMat);
	outp.pos = mul(centre + inp.pos, projMat);
	outp.altPos = outp.pos;
	outp.altPos.z = outp.altPos.z * outp.altPos.w * invFarDepth;
	outp.col = inp.col;
	outp.col.w *= 1.0 - (oth.x / oth.w);
	outp.txc = inp.txc;

	return outp;
}

VS_Output_Tex VShade_Sprite_Fire_Radial_Offset(VS_Input_Tex inp)
{
	VS_Output_Tex outp = (VS_Output_Tex)0;
	float4 centre = (float4)0;
	if (inp.tti >= 0)
	{
		centre = spriteLoc[inp.tti];
	}
	float4 oth = spriteLoc[inp.tti + 1];
	inp.pos *= spriteDim;
	if (oth.x > oth.z)
	{
		inp.pos *= ((oth.y - oth.x) / (oth.y - oth.z));
	}
	centre = mul(centre, viewMat);
	outp.pos = mul(centre + inp.pos, projMat);
	outp.altPos = outp.pos;
	outp.col = inp.col;
	outp.txc = inp.txc;

	return outp;
}

// VShade_Sprite_Smoke
VS_Output_Tex VShade_Sprite_Smoke(VS_Input_Tex inp)
{
	VS_Output_Tex outp = (VS_Output_Tex)0;
	float4 centre = (float4)0;
	if (inp.tti >= 0)
	{
		centre = spriteLoc[inp.tti];
	}
	float4 oth = spriteLoc[inp.tti + 1];
	inp.pos *= spriteDim;
	outp.col = inp.col;
	if (oth.x > oth.w)
	{
		float smod = (oth.x - oth.w) / (1 - oth.w);
		inp.pos *= (1 + smod * oth.z);
		outp.col.w *= (1 - smod);
	}
	centre = mul(centre, viewMat);
	outp.pos = mul(centre + inp.pos, projMat);
	outp.altPos = outp.pos;
	outp.altPos.z = outp.altPos.z * outp.altPos.w * invFarDepth;
	outp.txc = inp.txc;

	return outp;
}





VS_Output_Decal VShade_Decal(VS_Input_Decal inp)
{
	VS_Output_Decal outp = (VS_Output_Decal)0;
	if (inp.tti >= 0)
	{
		inp.pos = mul(inp.pos, transarr[inp.tti]);
		inp.nrm = mul(inp.nrm, transarr[inp.tti]);
	}
	outp.pos = mul(inp.pos, viewProj);
	//outp.pos.z = outp.pos.z - 0.001;
	outp.altPos = outp.pos;
	outp.altPos.z = outp.altPos.z * outp.altPos.w * invFarDepth;
	outp.col = inp.col;
	outp.txc = inp.txc;

	return outp;
}

// VShade_Decal_Lit
#define STD_MCR_VShade_Decal_Lit(lightName) \
VS_Output_Decal VShade_Decal_Lit##lightName##(VS_Input_Decal inp) \
{ \
	VS_Output_Decal outp = (VS_Output_Decal)0; \
	if (inp.tti >= 0) \
	{ \
		inp.pos = mul(inp.pos, transarr[inp.tti]); \
		inp.nrm = mul(inp.nrm, transarr[inp.tti]); \
	} \
	outp.pos = mul(inp.pos, viewProj); \
	outp.altPos = outp.pos; \
	outp.altPos.z = outp.altPos.z * outp.altPos.w * invFarDepth; \
	outp.col = inp.col; \
	outp.txc = inp.txc; \
 \
	outp.lit = lightLitness##lightName##(inp.pos, inp.nrm); \
	outp.lmc = lightTrans##lightName##(inp.pos); \
 \
	return outp; \
}

STD_MCR_VShade_Decal_Lit(Ortho)
STD_MCR_VShade_Decal_Lit(Persp)
STD_MCR_VShade_Decal_Lit(Point)


// legacy
/*PS_Output PShade_Tex(VS_Output_Tex inp)
{
	//float num = (inp.altPos.z / inp.altPos.w);
	PS_Output outp = (PS_Output)0;
	//outp.dep = num;
	//num = 1.0 - num;
	outp.col = inp.col * tex2D(texSampler, inp.txc);

	outp.col = outp.col * colMod;
	float alphaPreserve = outp.col.w;

	outp.col = outp.col * (1.0 - lightCoof);

	outp.col *= alphaPreserve;
	outp.col.w = alphaPreserve;

	return outp;
}*/

PS_Output PShade_Tex(VS_Output_Tex inp)
{
	PS_Output outp = (PS_Output)0;
	outp.col = inp.col * tex2D(texSampler, inp.txc);

	outp.col = outp.col * colMod;
	float alphaPreserve = outp.col.w;

	outp.col = outp.col * (1.0 - lightCoof);

	outp.col *= alphaPreserve;
	outp.col.w = alphaPreserve;

	return outp;
}

/// TEST CODE
PS_Output PShade_Tex_Test(VS_Output_Tex inp)
{
	// altPos is NOT w depth!

	PS_Output outp = (PS_Output)0;
	float wNum = inp.altPos.w;

	inp.altPos = mul(inp.altPos, vpMat);
	float2 tcoords = float2(inp.altPos.x, inp.altPos.y);

	tcoords.x /= wNum;
	tcoords.y /= wNum;

	tcoords.x *= targTexData.z;
	tcoords.y *= targTexData.w;

	tcoords.x += targTexData.x;
	tcoords.y += targTexData.y;

	// get front colour
	outp.col = PShade_Tex(inp).col;
	float alphaPreserve = outp.col.w;
	//outp.col *= alphaPreserve; // done in PShade_Tex

	// pour in the back colour
	//float off = sin(ticker) * 3.0;

	float prop = 1.0 - alphaPreserve;
	float qprop = prop * alphaPreserve * 0.2;

	float off = 3.0;
	outp.col = outp.col + tex2D(targTexNonMipLinearSampler, tcoords) * (prop - qprop * 4.0);
	tcoords.x += targTexData.x * 1.0 * off;
	outp.col = outp.col + tex2D(targTexNonMipLinearSampler, tcoords) * qprop;
	tcoords.x -= targTexData.x * 2.0 * off;
	outp.col = outp.col + tex2D(targTexNonMipLinearSampler, tcoords) * qprop;
	tcoords.x += targTexData.x * 1.0 * off;
	tcoords.y += targTexData.y * 1.0 * off;
	outp.col = outp.col + tex2D(targTexNonMipLinearSampler, tcoords) * qprop;
	tcoords.y -= targTexData.y * 2.0 * off;
	outp.col = outp.col + tex2D(targTexNonMipLinearSampler, tcoords) * qprop;

	//outp.col = outp.col + tex2D(targTexNonMipLinearSampler, tcoords) * alphaPreserve;

	// flat alpha
	outp.col.w = 1;

	return outp;
}
/// END TEST CODE

/// TEST CODE
PS_Output PShade_Tex_Test2(VS_Output_Water inp)
{
	PS_Output outp = (PS_Output)0;
	float wNum = inp.altPos.w;

	inp.txc.x += ticker * 0.1;
	inp.txc.y += sin(ticker * 0.1);
	float4 rippleDat = tex2D(texSampler, inp.txc);

	//inp.altPos = mul(inp.altPos, vpMat);
	/*float2 tcoords = float2(inp.altPos.x + rippleDat.x * 500 - 250, inp.altPos.y + rippleDat.y * 500 - 250);

	tcoords.x /= wNum;
	tcoords.y /= wNum;

	tcoords.x *= targTexData.z;
	tcoords.y *= targTexData.w;

	tcoords.x += targTexData.x;
	tcoords.y += targTexData.y;

	tcoords.x = 1 - tcoords.x;
	// get front colour
	outp.col = tex2D(texSampler, tcoords);*/

	float2 waterReflectTexCoords;
	waterReflectTexCoords.x = (inp.waterReflectPos.x / inp.waterReflectPos.w) / 2.0 + 0.5;
	waterReflectTexCoords.y = -(inp.waterReflectPos.y / inp.waterReflectPos.w) / 2.0 + 0.5;
	waterReflectTexCoords.x += (rippleDat.x - 0.5) * 2.0 * 0.02;
	waterReflectTexCoords.y += (rippleDat.y - 0.5) * 2.0 * 0.02;
	outp.col = tex2D(tex0NonMipLinearSampler, waterReflectTexCoords) * 0.4;

	float2 waterRefractTexCoords;
	waterRefractTexCoords.x = (inp.waterRefractPos.x / inp.waterRefractPos.w) / 2.0 + 0.5;
	waterRefractTexCoords.y = -(inp.waterRefractPos.y / inp.waterRefractPos.w) / 2.0 + 0.5;
	waterRefractTexCoords.x -= (rippleDat.x - 0.5) * 2.0 * 0.02;
	waterRefractTexCoords.y -= (rippleDat.y - 0.5) * 2.0 * 0.02;
	outp.col += tex2D(tex1NonMipLinearSampler, waterRefractTexCoords) * 0.8;

	// flat alpha
	outp.col.w = 1;

	return outp;
}
/// END TEST CODE

/// TEST CODE
PS_Output PShade_Tex_Test3(VS_Output_Tex inp)
{
	// altPos is NOT w depth!

	PS_Output outp = (PS_Output)0;
	float wNum = inp.altPos.w;

	inp.txc.x += ticker * 0.1;
	inp.txc.y += sin(ticker * 0.1);
	float4 rippleTex = tex2D(tex0Sampler, inp.txc);
	
	inp.altPos = mul(inp.altPos, vpMat);
	float2 tcoords = float2(inp.altPos.x + rippleTex.x * 500 - 250, inp.altPos.y + rippleTex.y * 500 - 250);

	tcoords.x /= wNum;
	tcoords.y /= wNum;

	tcoords.x *= targTexData.z;
	tcoords.y *= targTexData.w;

	tcoords.x += targTexData.x;
	tcoords.y += targTexData.y;

	tcoords.x = 1 - tcoords.x;
	// get front colour
	outp.col = tex2D(texSampler, tcoords);

	// flat alpha
	outp.col.w = 1;

	return outp;
}
/// END TEST CODE

// PShade_Tex_Lit
#define STD_MCR_PShade_Tex_Lit(lightName) \
PS_Output PShade_Tex_Lit##lightName##(VS_Output_Tex inp) \
{ \
	PS_Output outp = (PS_Output)0; \
	outp.col = inp.col * tex2D(texSampler, inp.txc); \
 \
	float4 lightMod = calcLightMod##lightName##(inp.lmc); \
 \
	outp.col = outp.col * colMod; \
	float alphaPreserve = outp.col.w; \
 \
	outp.col = outp.col * (lightMod * inp.lit + lightAmbient) * lightCoof; \
 \
	outp.col *= alphaPreserve; \
	outp.col.w = 0; \
	return outp; \
}

STD_MCR_PShade_Tex_Lit(Ortho)
STD_MCR_PShade_Tex_Lit(Persp)
STD_MCR_PShade_Tex_Lit(Point)


PS_Output PShade_Sprite(VS_Output_Tex inp)
{ // no light coof
	//float num = (inp.altPos.z / inp.altPos.w);
	PS_Output outp = (PS_Output)0;
	//outp.dep = num;
	//num = 1.0 - num;
	outp.col = inp.col * tex2D(texSampler, inp.txc);

	outp.col = outp.col * colMod;
	float alphaPreserve = outp.col.w;

	outp.col *= alphaPreserve;
	outp.col.w = alphaPreserve;

	return outp;
}




PS_Output PShade_Cloud(VS_Output_Tex inp)
{
//	float num = (inp.altPos.z / inp.altPos.w);
	PS_Output outp = (PS_Output)0;
	//outp.dep = num;
//	num = 1.0 - num;
	outp.col = inp.col * tex2D(texSampler, inp.txc);

	float alphaPreserve = outp.col.w;
	//clip (alphaPreserve <= 0.0 ? -1 : 1); // freindly alpha clip

	outp.col = outp.col * colMod;

	outp.col.w = alphaPreserve;

	outp.col.x = 0;

	return outp;
}

PS_Output PShade_Decal(VS_Output_Decal inp)
{
	/*clip(inp.txc.x);
	clip(1.0 - inp.txc.x);
	clip(inp.txc.y);
	clip(1.0 - inp.txc.y);
	clip(inp.txc.z);
	clip(1.0 - inp.txc.z);*/

	PS_Output outp = (PS_Output)0;
	
	float2 txc;
	txc.x = inp.txc.x;
	txc.y = inp.txc.y;
	outp.col = tex2D(texBorderSampler, txc) * colMod * inp.col;

	float alphaPreserve = outp.col.w;
	//clip (alphaPreserve <= 0.0 ? -1 : 1); // freindly alpha clip

	outp.col = outp.col * (1.0 - lightCoof);

	outp.col *= alphaPreserve;
	outp.col.w = alphaPreserve;

	return outp;
}

// PShade_Decal_Lit
PS_Output PShade_Decal_LitOrtho(VS_Output_Decal inp)
{
	//clip(inp.lit);

	/*clip(inp.txc.x);
	clip(1.0 - inp.txc.x);
	clip(inp.txc.y);
	clip(1.0 - inp.txc.y);
	clip(inp.txc.z);
	clip(1.0 - inp.txc.z);*/

	PS_Output outp = (PS_Output)0;
	
	float2 txc;
	txc.x = inp.txc.x;
	txc.y = inp.txc.y;
	outp.col = tex2D(texBorderSampler, txc) * colMod * inp.col;

	float alphaPreserve = outp.col.w;
	//clip (alphaPreserve <= 0.0 ? -1 : 1); // freindly alpha clip

	float4 lightMod = calcLightModOrtho(inp.lmc);

	outp.col = outp.col * (lightMod * inp.lit + lightAmbient) * lightCoof;

	outp.col *= alphaPreserve;
	outp.col.w = 0;//alphaPreserve;

	return outp;
}

PS_Output PShade_Decal_LitPersp(VS_Output_Decal inp)
{
	//clip(inp.lit);

	/*clip(inp.txc.x);
	clip(1.0 - inp.txc.x);
	clip(inp.txc.y);
	clip(1.0 - inp.txc.y);
	clip(inp.txc.z);
	clip(1.0 - inp.txc.z);*/

	PS_Output outp = (PS_Output)0;
	
	float2 txc;
	txc.x = inp.txc.x;
	txc.y = inp.txc.y;
	outp.col = tex2D(texBorderSampler, txc) * colMod * inp.col;

	float alphaPreserve = outp.col.w;
	//clip (alphaPreserve <= 0.0 ? -1 : 1); // freindly alpha clip

	float4 lightMod = calcLightModPersp(inp.lmc);

	outp.col = outp.col * (lightMod * inp.lit + lightAmbient) * lightCoof;

	outp.col *= alphaPreserve;
	outp.col.w = 0;//alphaPreserve;

	return outp;
}

PS_Output PShade_Decal_LitPoint(VS_Output_Decal inp)
{
	//clip(inp.lit);

	/*clip(inp.txc.x);
	clip(1.0 - inp.txc.x);
	clip(inp.txc.y);
	clip(1.0 - inp.txc.y);
	clip(inp.txc.z);
	clip(1.0 - inp.txc.z);*/

	PS_Output outp = (PS_Output)0;
	
	float2 txc;
	txc.x = inp.txc.x;
	txc.y = inp.txc.y;
	outp.col = tex2D(texBorderSampler, txc) * colMod * inp.col;

	float alphaPreserve = outp.col.w;
	//clip (alphaPreserve <= 0.0 ? -1 : 1); // freindly alpha clip

	float4 lightMod = calcLightModPoint(inp.lmc);

	outp.col = outp.col * (lightMod * inp.lit + lightAmbient) * lightCoof;

	outp.col *= alphaPreserve;
	outp.col.w = 0;//alphaPreserve;

	return outp;
}



// this is used for opaque stuff - if it contains alphaness, then it needs it's own Light PShade
// PShade_Light
PS_Output PShade_LightOrtho(VS_Output_Light inp)
{
	inp.altPos = lightUnTransOrtho(inp.altPos);

	float num = inp.altPos.z;
	PS_Output outp = (PS_Output)0;
	outp.col.x = num;

	return outp;
}

PS_Output PShade_LightPersp(VS_Output_Light inp)
{
	inp.altPos = lightUnTransPersp(inp.altPos);

	float num = inp.altPos.z;
	PS_Output outp = (PS_Output)0;
	outp.col.x = num;

	return outp;
}

PS_Output PShade_LightPoint(VS_Output_Light inp)
{
	inp.altPos = lightUnTransPoint(inp.altPos);

	float num = inp.altPos.z;
	PS_Output outp = (PS_Output)0;
	outp.col.x = num;

	return outp;
}




// clips translucent stuff (these don't work, need to ADAPTED for individual use, with a new set of VShades)
// PShade_Tex_Light
#define STD_MCR_PShade_Tex_Light(lightName) \
PS_Output PShade_Tex_Light##lightName##(VS_Output_Tex inp) \
{ \
	float4 testCol = inp.col * tex2D(texSampler, inp.txc); \
 \
	float alphaPreserve = testCol.w; \
	clip (alphaPreserve <= 0.9 ? -1 : 1); \
 \
	inp.altPos = lightUnTrans##lightName##(inp.altPos); \
 \
	float num = inp.altPos.z; \
	PS_Output outp = (PS_Output)0; \
	outp.col.x = num; \
 \
	return outp; \
}

STD_MCR_PShade_Tex_Light(Ortho)
STD_MCR_PShade_Tex_Light(Persp)
STD_MCR_PShade_Tex_Light(Point)


VS_Output_Terrain VShade_Terrain(VS_Input_Terrain inp)
{
	VS_Output_Terrain outp = (VS_Output_Terrain)0;
	outp.pos = mul(inp.pos, viewProj);
	outp.altPos = outp.pos;
	outp.altPos.z = outp.altPos.z * outp.altPos.w * invFarDepth;
	outp.orgY = inp.pos.y;
	outp.txc = inp.txc;
	outp.w = inp.w;

	return outp;
}

// VShade_Terrain_Lit
#define STD_MCR_VShade_Terrain_Lit(lightName) \
VS_Output_Terrain VShade_Terrain_Lit##lightName##(VS_Input_Terrain inp) \
{ \
	VS_Output_Terrain outp = (VS_Output_Terrain)0; \
	outp.pos = mul(inp.pos, viewProj); \
	outp.altPos = outp.pos; \
	outp.altPos.z = outp.altPos.z * outp.altPos.w * invFarDepth; \
	outp.orgY = inp.pos.y; \
	outp.txc = inp.txc; \
	outp.w = inp.w; \
 \
	outp.lit = lightLitness##lightName##(inp.pos, inp.nrm); \
	outp.lmc = lightTrans##lightName##(inp.pos); \
 \
	return outp; \
}

STD_MCR_VShade_Terrain_Lit(Ortho)
STD_MCR_VShade_Terrain_Lit(Persp)
STD_MCR_VShade_Terrain_Lit(Point)


// VShade_Terrain_DynamicDecal
VS_Output_Dyn VShade_Terrain_DynOrtho(VS_Input_Terrain inp)
{
	VS_Output_Dyn outp = (VS_Output_Dyn)0;
	outp.pos = mul(inp.pos, viewProj);
	outp.altPos = outp.pos;
	outp.altPos.z = outp.altPos.z * outp.altPos.w * invFarDepth;
	outp.lmc = lightTransOrtho(inp.pos);

	return outp;
}

VS_Output_Dyn VShade_Terrain_DynPersp(VS_Input_Terrain inp)
{
	VS_Output_Dyn outp = (VS_Output_Dyn)0;
	outp.pos = mul(inp.pos, viewProj);
	outp.altPos = outp.pos;
	outp.altPos.z = outp.altPos.z * outp.altPos.w * invFarDepth;
	outp.lmc = lightTransPersp(inp.pos);

	return outp;
}


// VShade_Terrain_Light
#define STD_MCR_VShade_Terrain_Light(lightName) \
VS_Output_Light VShade_Terrain_Light##lightName##(VS_Input_Terrain inp) \
{ \
	VS_Output_Light outp = (VS_Output_Light)0; \
	outp.pos = lightTrans##lightName##VP(inp.pos); \
	outp.altPos = outp.pos; \
 \
	return outp; \
}

STD_MCR_VShade_Terrain_Light(Ortho)
STD_MCR_VShade_Terrain_Light(Persp)
STD_MCR_VShade_Terrain_Light(Point)






// based off VShade_Tex_Decal
VS_Output_Decal VShade_Terrain_Decal(VS_Input_Decal inp)
{
	VS_Output_Decal outp = (VS_Output_Decal)0;
	outp.pos = mul(inp.pos, viewProj);
	//outp.pos.z = outp.pos.z - 0.001;
	outp.altPos = outp.pos;
	outp.altPos.z = outp.altPos.z * outp.altPos.w * invFarDepth;
	outp.col = inp.col;
	outp.txc = inp.txc;

	return outp;
}

// VShade_Terrain_Decal_Lit
#define STD_MCR_VShade_Terrain_Decal_Lit(lightName) \
VS_Output_Decal VShade_Terrain_Decal_Lit##lightName##(VS_Input_Decal inp) \
{ \
	VS_Output_Decal outp = (VS_Output_Decal)0; \
	outp.pos = mul(inp.pos, viewProj); \
	outp.altPos = outp.pos; \
	outp.altPos.z = outp.altPos.z * outp.altPos.w * invFarDepth; \
	outp.col = inp.col; \
	outp.txc = inp.txc; \
 \
	outp.lit = lightLitness##lightName##(inp.pos, inp.nrm); \
	outp.lmc = lightTrans##lightName##(inp.pos); \
 \
	return outp; \
}

STD_MCR_VShade_Terrain_Decal_Lit(Ortho)
STD_MCR_VShade_Terrain_Decal_Lit(Persp)
STD_MCR_VShade_Terrain_Decal_Lit(Point)


PS_Output PShade_Terrain(VS_Output_Terrain inp)
{
	//float num = (inp.altPos.z / inp.altPos.w);
	PS_Output outp = (PS_Output)0;
	//outp.dep = num;
	//num = 1.0 - num;
	outp.col = (inp.w.x * tex2D(tex0Sampler, inp.txc) + inp.w.y * tex2D(tex1Sampler, inp.txc) + inp.w.z * tex2D(tex2Sampler, inp.txc) + inp.w.w * tex2D(tex3Sampler, inp.txc));
	
	float alphaPreserve = outp.col.w;

	outp.col = outp.col * (1.0 - lightCoof);

	outp.col.w = alphaPreserve;

	return outp;
}

// PShade_Terrain_Lit
#define STD_MCR_PShade_Terrain_Lit(lightName) \
PS_Output PShade_Terrain_Lit##lightName##(VS_Output_Terrain inp) \
{ \
	PS_Output outp = (PS_Output)0; \
	outp.col = (inp.w.x * tex2D(tex0Sampler, inp.txc) + inp.w.y * tex2D(tex1Sampler, inp.txc) + inp.w.z * tex2D(tex2Sampler, inp.txc) + inp.w.w * tex2D(tex3Sampler, inp.txc)); \
 \
	float4 lightMod = calcLightMod##lightName##(inp.lmc); \
 \
	outp.col = outp.col * (lightMod * inp.lit + lightAmbient) * lightCoof; \
 \
	return outp; \
}

STD_MCR_PShade_Terrain_Lit(Ortho)
STD_MCR_PShade_Terrain_Lit(Persp)
STD_MCR_PShade_Terrain_Lit(Point)




// PShade_Dyn (amicDecal)
//  - note that the DynamicDecal and Dyn shaders and techniques depend on the lit shaders
//    and are very, very similiar, and use the same shader variables/handles
PS_Output PShade_DynOrtho(VS_Output_Dyn inp)
{
	PS_Output outp = (PS_Output)0;
	
	float4 lightMod = calcDynModOrtho(inp.lmc);

	outp.col = lightMod;

	return outp;
}

PS_Output PShade_DynPersp(VS_Output_Dyn inp)
{
	PS_Output outp = (PS_Output)0;

	float4 lightMod = calcDynModPersp(inp.lmc);

	outp.col = lightMod;

	return outp;
}





// side/over shaders

VS_Output_Over VShade_Over(VS_Input_Over inp)
{
	VS_Output_Over outp = (VS_Output_Over)0;
	outp.pos = mul(inp.pos, viewProj);
	outp.altPos = outp.pos;
	outp.txc = inp.txc;
	return outp;
}

PS_Output PShade_Over(VS_Output_Over inp)
{
	PS_Output outp = (PS_Output)0;

	outp.col = tex2D(texSampler, inp.txc);

	return outp;
}

VS_Output_Over VShade_Over_Final(VS_Input_Over inp)
{
	VS_Output_Over outp = (VS_Output_Over)0;
	outp.pos = mul(inp.pos, viewProj);
	outp.altPos = outp.pos;
	outp.txc = inp.txc;
	return outp;
}

PS_Output PShade_Over_Final(VS_Output_Over inp)
{
	PS_Output outp = (PS_Output)0;

	outp.col = tex2D(texSampler, inp.txc);

	return outp;
}

PS_Output PShade_Over_Final_Fun(VS_Output_Over inp)
{
	PS_Output outp = (PS_Output)0;

	float4 coldat = tex2D(tex0LinearSampler, inp.txc);

	float prop = coldat.x * coldat.w;
	float qprop = prop * 0.2;

	outp.col = float4(qprop, qprop, qprop, 1.0);

	float2 tcoords = inp.txc;

	float off = 3.0;
	outp.col = outp.col + tex2D(texNonMipLinearSampler, tcoords) * (1.0 - prop + qprop);

	tcoords.x += texData.x * 1.0 * off;
	outp.col = outp.col + tex2D(texNonMipLinearSampler, tcoords) * qprop;
	tcoords.x -= texData.x * 2.0 * off;
	outp.col = outp.col + tex2D(texNonMipLinearSampler, tcoords) * qprop;
	tcoords.x += texData.x * 1.0 * off;
	tcoords.y += texData.y * 1.0 * off;
	outp.col = outp.col + tex2D(texNonMipLinearSampler, tcoords) * qprop;
	tcoords.y -= texData.y * 2.0 * off;
	outp.col = outp.col + tex2D(texNonMipLinearSampler, tcoords) * qprop;

	outp.col.w = 1;
	return outp;
}

PS_Output PShade_Over_Final_Fun1(VS_Output_Over inp)
{
	PS_Output outp = (PS_Output)0;

	float4 coldat;
	float4 coldat2;
	float2 coords;
	float2 endcoords;
	float xc = inp.txc.x / texData.z;
	float yc = inp.txc.y / texData.w;

	float mod;
	float modx;
	float mody;

	float xt = ticker * 1.0;// + 0.01 * xc;
	float yt = ticker * 1.0;// + 0.01 * yc;

	// first
	mod = 20;
	modx = mod * texData.z;
	mody = mod * texData.w;

	endcoords = inp.txc;
	coords = inp.txc + float2(-xt * 1.5, sin(yt * 1.3 + 1.2));
	coldat = tex2D(tex0LinearSampler, coords);
	coords = inp.txc + float2(sin(xt * 1.1 + 0.4), yt);
	coldat2 = tex2D(tex1LinearSampler, coords);
	endcoords.x += (coldat.r - 0.5) * modx * coldat2.r;
	endcoords.y += (coldat.g - 0.5) * mody * coldat2.g;

	outp.col = outp.col + tex2D(texNonMipLinearSampler, endcoords) * 0.8;

	// second
	mod = 50;
	modx = mod * texData.z;
	mody = mod * texData.w;

	endcoords = inp.txc;
	coords = inp.txc + float2(-cos(xt + 0.9), yt * 1.2);
	coldat = tex2D(tex2LinearSampler, coords);
	coords = inp.txc + float2(xt * 1.3, cos(yt * 0.9 + 0.2));
	coldat2 = tex2D(tex3LinearSampler, coords);
	endcoords.x += (coldat.r - 0.5) * modx * coldat2.r;
	endcoords.y += (coldat.g - 0.5) * mody * coldat2.g;

	outp.col = outp.col + tex2D(texNonMipLinearSampler, endcoords) * 0.2;

	// alpha preserve
	outp.col.w = 0.6;
	return outp;
}

PS_Output PShade_Over_Final_Fun2(VS_Output_Over inp)
{
	PS_Output outp = (PS_Output)0;

	float n = 3;
	float g = 5;
	float coof = 1.0 / (2.0 * n + 1);
	float2 coords = inp.txc;
	coords.y -= texData.w * n * g;

	// should be 2n + 1 of these
	outp.col = outp.col + tex2D(texNonMipLinearSampler, coords) * coof;
	coords.y += texData.w * g;
	outp.col = outp.col + tex2D(texNonMipLinearSampler, coords) * coof;
	coords.y += texData.w * g;
	outp.col = outp.col + tex2D(texNonMipLinearSampler, coords) * coof;
	coords.y += texData.w * g;
	outp.col = outp.col + tex2D(texNonMipLinearSampler, coords) * coof;
	coords.y += texData.w * g;
	outp.col = outp.col + tex2D(texNonMipLinearSampler, coords) * coof;
	coords.y += texData.w * g;
	outp.col = outp.col + tex2D(texNonMipLinearSampler, coords) * coof;
	coords.y += texData.w * g;
	outp.col = outp.col + tex2D(texNonMipLinearSampler, coords) * coof;
	coords.y += texData.w * g;

	// alpha preserve
	outp.col.w = 0.5;
	return outp;
}

PS_Output PShade_Over_Final_Wobble(VS_Output_Over inp)
{
	PS_Output outp = (PS_Output)0;

	float4 coldat = tex2D(tex0LinearSampler, inp.txc);
	float rmod = coldat.x * coldat.w;
	float xc = inp.txc.x / texData.z;
	float yc = inp.txc.y / texData.w;

	float c = ticker * 10.0 + 0.1 * yc + sin(0.02 * xc);
	float t = sin(c) * (cos(c) * 0.4 + 0.6);
	
	//inp.txc.x += t * texData.z * 5.0 * rmod;

	outp.col = tex2D(texNonMipLinearSampler, inp.txc);

	outp.col *= (1.0 + t * 0.2 * rmod);

	outp.col.w = 1;
	return outp;
}

PS_Output PShade_Over_Final_FlatAlpha(VS_Output_Over inp)
{
	PS_Output outp = (PS_Output)0;

	outp.col = tex2D(texSampler, inp.txc);
	outp.col.w = 1;

	return outp;
}

PS_Output PShade_Over_Linear(VS_Output_Over inp)
{
	PS_Output outp = (PS_Output)0;

	outp.col = tex2D(texNonMipLinearSampler, inp.txc);

	return outp;
}

PS_Output PShade_Over_Linear_FlatAlpha(VS_Output_Over inp)
{
	PS_Output outp = (PS_Output)0;

	outp.col = tex2D(texNonMipLinearSampler, inp.txc);

	return outp;
}

PS_Output PShade_Over_Greys(VS_Output_Over inp)
{
	PS_Output outp = (PS_Output)0;

	outp.col = tex2D(texSampler, inp.txc);
	
	//float roottwo = 1.4142135623730950488016887242097;
	float two = 2.00000000000000000000000000000000;
	float nx = (inp.txc.x - 0.5) * two;
	float ny = (inp.txc.y - 0.5) * two;
	float num = sqrt(nx * nx + ny * ny);

	num *= num;

	num *= 1.2;
	num -= 0.2;
	if (num < 0)
	{
		num = 0;
	}
	if (num > 1)
	{
		num = 1;
	}

	float grey = (outp.col.x + outp.col.y + outp.col.z) / 3;

	
	outp.col.x = outp.col.x * (1.0 - num) + grey * num;
	outp.col.y = outp.col.y * (1.0 - num) + grey * num;
	outp.col.z = outp.col.z * (1.0 - num) + grey * num;

	return outp;
}

PS_Output PShade_Over_Final_Fuzzy(VS_Output_Over inp)
{
	PS_Output outp = (PS_Output)0;

	outp.col = tex2D(texSampler, inp.txc) * 0.6;

	inp.txc.x -= texData.z;
	inp.txc.y -= texData.w;
	outp.col += tex2D(texSampler, inp.txc) * 0.1;

	inp.txc.x += texData.z * 2;
	outp.col += tex2D(texSampler, inp.txc) * 0.1;

	inp.txc.y += texData.w * 2;
	outp.col += tex2D(texSampler, inp.txc) * 0.1;

	inp.txc.x -= texData.z * 2;
	outp.col += tex2D(texSampler, inp.txc) * 0.1;

	return outp;
}

PS_Output PShade_Over_Fuzz(VS_Output_Over inp)
{ // uses tex0
	PS_Output outp = (PS_Output)0;

	float4 off = tex2D(tex0Sampler, inp.txc);

	float mod = 0.005 - 0.01 * ((ticker * 10) % 1);
	inp.txc.x += off.x;
	inp.txc.y += off.y;

	outp.col = tex2D(texSampler, inp.txc);

	return outp;
}


PS_Output PShade_Over_Offset(VS_Output_Over inp)
{
	PS_Output outp = (PS_Output)0;

	float4 offCol = tex2D(tex1Sampler, inp.txc);
	float2 offset;

	offset.x = offCol.x;
	offset.y = offCol.y;

	offset.x -= 0.5 * offCol.z;
	offset.y -= 0.5 * offCol.z;

	//outp.col = tex2D(tex0Sampler, inp.txc);

	offset.x *= 0.2;
	offset.y *= 0.2;

	//offset *= ticker;

	inp.txc += offset;

	outp.col = tex2D(tex0Sampler, inp.txc);

/*
	outp.col.x = inp.txc.x;
	outp.col.y = inp.txc.y;
	outp.col.z = 0;
	outp.col.w = 1;
*/

	return outp;
}

PS_Output PShade_Tex_Radials(VS_Output_Tex inp)
{
	PS_Output outp = (PS_Output)0;

	float4 offCol = tex2D(texSampler, inp.txc);

	outp.col.x = (inp.txc.x - 0.5);
	outp.col.y = -(inp.txc.y - 0.5);

	outp.col *= (offCol.x - offCol.y) * colMod.x;

	outp.col.x += 0.5;
	outp.col.y += 0.5;

	outp.col.z = 1;
	outp.col.w = 1;

	return outp;
}

PS_Output PShade_Tex_Radial_Offset(VS_Output_Tex inp)
{
	PS_Output outp = (PS_Output)0;

	float4 offCol = tex2D(texSampler, inp.txc);

	float2 offTxc = float2(offCol.x, offCol.y);
	offTxc *= colMod.x;


	// altPos is NOT w depth!
	float wNum = inp.altPos.w;

	inp.altPos = mul(inp.altPos, vpMat);
	float2 tcoords = float2(inp.altPos.x, inp.altPos.y);

	tcoords.x /= wNum;
	tcoords.y /= wNum;

	tcoords.x *= targTexData.z;
	tcoords.y *= targTexData.w;

	tcoords.x += targTexData.x;
	tcoords.y += targTexData.y;

	tcoords.x += offTxc.x;
	tcoords.y += offTxc.y;

	outp.col = tex2D(targTexSampler, tcoords);


	return outp;
}





// all lit techniques must have:
// P0 - no lighting
// P1 - Ortho Lit
// P2 - Persp Lit
// P3 - Point Lit


// all light techniques must have:
// P0 - Ortho Light
// P1 - Persp Light
// P2 - Point Light



technique dull
{
	pass unlit
	{
		VertexShader = compile vs_2_0 VShade_Dull();
		PixelShader = compile ps_2_0 PShade_Dull();
	}
}

technique simpleUi
{
	pass unlit
	{
		VertexShader = compile vs_2_0 VShade_Ui();
		PixelShader = compile ps_2_0 PShade_Ui();
	}
}

technique simpleUiBorder
{
	pass unlit
	{
		VertexShader = compile vs_2_0 VShade_Ui();
		PixelShader = compile ps_2_0 PShade_Ui_Border();
	}
}

technique simple
{
	pass unlit
	{
		VertexShader = compile vs_2_0 VShade();
		PixelShader = compile ps_2_0 PShade();
	}
}

technique simpleTex
{
	pass unlit
	{
		VertexShader = compile vs_2_0 VShade_Tex();
		PixelShader = compile ps_2_0 PShade_Tex();
	}

	pass litortho
	{ // LitOrtho
		VertexShader = compile vs_2_0 VShade_Tex_LitOrtho();
		PixelShader = compile ps_2_0 PShade_Tex_LitOrtho();
	}

	pass litpersp
	{ // LitPersp
		VertexShader = compile vs_2_0 VShade_Tex_LitPersp();
		PixelShader = compile ps_2_0 PShade_Tex_LitPersp();
	}

	pass litpoint
	{ // LitPoint
		VertexShader = compile vs_2_0 VShade_Tex_LitPoint();
		PixelShader = compile ps_2_0 PShade_Tex_LitPoint();
	}
}

/// TEST CODE
technique simpleTex_test
{
	pass unlit
	{
		VertexShader = compile vs_2_0 VShade_Tex_Test();
		PixelShader = compile ps_2_0 PShade_Tex_Test();
	}

	pass litortho
	{ // LitOrtho
		VertexShader = compile vs_2_0 VShade_Tex_LitOrtho();
		PixelShader = compile ps_2_0 PShade_Tex_LitOrtho();
	}

	pass litpersp
	{ // LitPersp
		VertexShader = compile vs_2_0 VShade_Tex_LitPersp();
		PixelShader = compile ps_2_0 PShade_Tex_LitPersp();
	}

	pass litpoint
	{ // LitPoint
		VertexShader = compile vs_2_0 VShade_Tex_LitPoint();
		PixelShader = compile ps_2_0 PShade_Tex_LitPoint();
	}
}
/// END TEST CODE

/// TEST CODE
technique simpleTex_test2
{
	pass unlit
	{
		VertexShader = compile vs_2_0 VShade_Tex_Test2();
		PixelShader = compile ps_2_0 PShade_Tex_Test2();
	}

	pass litortho
	{ // LitOrtho
		VertexShader = compile vs_2_0 VShade_Tex_LitOrtho();
		PixelShader = compile ps_2_0 PShade_Tex_LitOrtho();
	}

	pass litpersp
	{ // LitPersp
		VertexShader = compile vs_2_0 VShade_Tex_LitPersp();
		PixelShader = compile ps_2_0 PShade_Tex_LitPersp();
	}

	pass litpoint
	{ // LitPoint
		VertexShader = compile vs_2_0 VShade_Tex_LitPoint();
		PixelShader = compile ps_2_0 PShade_Tex_LitPoint();
	}
}
/// END TEST CODE

technique simpleTex_light
{
	pass lightortho
	{ // LightOrtho
		VertexShader = compile vs_2_0 VShade_Tex_LightOrtho();
		PixelShader = compile ps_2_0 PShade_LightOrtho(); // no trans
	}

	pass lightpersp
	{ // LightPersp
		VertexShader = compile vs_2_0 VShade_Tex_LightPersp();
		PixelShader = compile ps_2_0 PShade_LightPersp();
	}

	pass lightpoint
	{ // LightPoint - this shoudln't do anthing.... it should never be called!
		VertexShader = compile vs_2_0 VShade_Tex_LightPoint();
		PixelShader = compile ps_2_0 PShade_LightPoint();
	}
}


technique simpleTexNoTti
{
	pass unlit
	{
		VertexShader = compile vs_2_0 VShade_TexNoTti();
		PixelShader = compile ps_2_0 PShade_Tex();
	}

	pass litortho
	{ // LitOrtho
		VertexShader = compile vs_2_0 VShade_TexNoTti_LitOrtho();
		PixelShader = compile ps_2_0 PShade_Tex_LitOrtho();
	}

	pass litpersp
	{ // LitPersp
		VertexShader = compile vs_2_0 VShade_TexNoTti_LitPersp();
		PixelShader = compile ps_2_0 PShade_Tex_LitPersp();
	}

	pass litpoint
	{ // LitPoint
		VertexShader = compile vs_2_0 VShade_TexNoTti_LitPoint();
		PixelShader = compile ps_2_0 PShade_Tex_LitPoint();
	}
}

technique simpleTexNoTti_light
{
	pass lightortho
	{ // LightOrtho
		VertexShader = compile vs_2_0 VShade_TexNoTti_LightOrtho();
		PixelShader = compile ps_2_0 PShade_LightOrtho(); // no trans
	}

	pass lightpersp
	{ // LightPersp
		VertexShader = compile vs_2_0 VShade_TexNoTti_LightPersp();
		PixelShader = compile ps_2_0 PShade_LightPersp();
	}

	pass lightpoint
	{ // LightPoint - this shoudln't do anthing.... it should never be called!
		VertexShader = compile vs_2_0 VShade_TexNoTti_LightPoint();
		PixelShader = compile ps_2_0 PShade_LightPoint();
	}
}

technique simpleTex_dynamicdecal
{
	pass dynortho
	{ // DynOrtho
		VertexShader = compile vs_2_0 VShade_Tex_DynOrtho();
		PixelShader = compile ps_2_0 PShade_DynOrtho();
	}

	pass dynpersp
	{ // DynPersp
		VertexShader = compile vs_2_0 VShade_Tex_DynPersp();
		PixelShader = compile ps_2_0 PShade_DynPersp();
	}
}

technique sprite
{
	pass unlit
	{
		VertexShader = compile vs_2_0 VShade_Sprite();
		PixelShader = compile ps_2_0 PShade_Tex();
	}

	pass litortho
	{ // LitOrtho
		VertexShader = compile vs_2_0 VShade_Sprite_LitOrtho();
		PixelShader = compile ps_2_0 PShade_Tex_LitOrtho();
	}

	pass litpersp
	{ // LitPersp
		VertexShader = compile vs_2_0 VShade_Sprite_LitPersp();
		PixelShader = compile ps_2_0 PShade_Tex_LitPersp();
	}

	pass litpoint
	{ // LitPoint
		VertexShader = compile vs_2_0 VShade_Sprite_LitPoint();
		PixelShader = compile ps_2_0 PShade_Tex_LitPoint();
	}
}

technique sprite_flat
{
	pass unlit
	{
		VertexShader = compile vs_2_0 VShade_Sprite_Flat();
		PixelShader = compile ps_2_0 PShade_Sprite();
	}
}

technique sprite_fire
{
	pass unlit
	{
		VertexShader = compile vs_2_0 VShade_Sprite_Fire();
		PixelShader = compile ps_2_0 PShade_Sprite();
	}
}

technique sprite_smoke
{
	pass unlit
	{
		VertexShader = compile vs_2_0 VShade_Sprite_Smoke();
		PixelShader = compile ps_2_0 PShade_Sprite();
	}
}

technique sprite_fire_radial_offset
{
	pass unlit
	{
		VertexShader = compile vs_2_0 VShade_Sprite_Fire_Radial_Offset();
		PixelShader = compile ps_2_0 PShade_Tex_Radial_Offset();
	}
}

technique sprite_light
{
	pass lightortho
	{ // LightOrtho
		VertexShader = compile vs_2_0 VShade_Sprite_LightOrtho();
		PixelShader = compile ps_2_0 PShade_LightOrtho(); // no trans
	}

	pass lightpersp
	{ // LightPersp
		VertexShader = compile vs_2_0 VShade_Sprite_LightPersp();
		PixelShader = compile ps_2_0 PShade_LightPersp();
	}

	pass lightpoint
	{ // LightPoint
		VertexShader = compile vs_2_0 VShade_Sprite_LightPoint();
		PixelShader = compile ps_2_0 PShade_LightPoint();
	}
}

// custom for clouds
technique cloud
{
	pass unlit
	{
		VertexShader = compile vs_2_0 VShade_Tex();
		PixelShader = compile ps_2_0 PShade_Cloud();
	}
}

technique simple_decal
{
	pass unlit
	{
		VertexShader = compile vs_2_0 VShade_Decal();
		PixelShader = compile ps_2_0 PShade_Decal();
	}

	pass litortho
	{ // LitOrtho
		VertexShader = compile vs_2_0 VShade_Decal_LitOrtho();
		PixelShader = compile ps_2_0 PShade_Decal_LitOrtho();
	}

	pass litpersp
	{ // LitPersp
		VertexShader = compile vs_2_0 VShade_Decal_LitPersp();
		PixelShader = compile ps_2_0 PShade_Decal_LitPersp();
	}

	pass litpoint
	{ // LitPoint
		VertexShader = compile vs_2_0 VShade_Decal_LitPoint();
		PixelShader = compile ps_2_0 PShade_Decal_LitPoint();
	}
}

technique terrain
{
	pass unlit
	{
		VertexShader = compile vs_2_0 VShade_Terrain();
		PixelShader = compile ps_2_0 PShade_Terrain();
	}

	pass litortho
	{ // LitOrtho
		VertexShader = compile vs_2_0 VShade_Terrain_LitOrtho();
		PixelShader = compile ps_2_0 PShade_Terrain_LitOrtho();
	}

	pass litpersp
	{ // LitPersp
		VertexShader = compile vs_2_0 VShade_Terrain_LitPersp();
		PixelShader = compile ps_2_0 PShade_Terrain_LitPersp();
	}

	pass litpoint
	{ // LitPoint
		VertexShader = compile vs_2_0 VShade_Terrain_LitPoint();
		PixelShader = compile ps_2_0 PShade_Terrain_LitPoint();
	}
}

technique terrain_decal
{
	pass unlit
	{
		VertexShader = compile vs_2_0 VShade_Terrain_Decal();
		PixelShader = compile ps_2_0 PShade_Decal();
	}

	pass litortho
	{ // LitOrtho
		VertexShader = compile vs_2_0 VShade_Terrain_Decal_LitOrtho();
		PixelShader = compile ps_2_0 PShade_Decal_LitOrtho();
	}

	pass litpersp
	{ // LitPersp
		VertexShader = compile vs_2_0 VShade_Terrain_Decal_LitPersp();
		PixelShader = compile ps_2_0 PShade_Decal_LitPersp();
	}

	pass litpoint
	{ // LitPoint
		VertexShader = compile vs_2_0 VShade_Terrain_Decal_LitPoint();
		PixelShader = compile ps_2_0 PShade_Decal_LitPoint();
	}
}

technique terrain_dynamicdecal
{
	pass dynortho
	{ // DynOrtho
		VertexShader = compile vs_2_0 VShade_Terrain_DynOrtho();
		PixelShader = compile ps_2_0 PShade_DynOrtho();
	}

	pass dynpersp
	{ // DynPersp
		VertexShader = compile vs_2_0 VShade_Terrain_DynPersp();
		PixelShader = compile ps_2_0 PShade_DynPersp();
	}
}

technique terrain_light
{
	pass lightortho
	{ // LightOrtho
		VertexShader = compile vs_2_0 VShade_Terrain_LightOrtho();
		PixelShader = compile ps_2_0 PShade_LightOrtho(); // no transparencies, keep it simples
	}

	pass lightpersp
	{ // LightPersp
		VertexShader = compile vs_2_0 VShade_Terrain_LightPersp();
		PixelShader = compile ps_2_0 PShade_LightPersp();
	}
	
	pass lightpoint
	{ // LightPoint
		VertexShader = compile vs_2_0 VShade_Terrain_LightPoint();
		PixelShader = compile ps_2_0 PShade_LightPoint();
	}
}

technique radialTex
{
	pass unlit
	{
		VertexShader = compile vs_2_0 VShade_Tex();
		PixelShader = compile ps_2_0 PShade_Tex_Radials();
	}
}

technique over
{
	pass over
	{
		VertexShader = compile vs_2_0 VShade_Over();
		PixelShader = compile ps_2_0 PShade_Over();
	}
}

technique over_final
{
	pass over
	{
		VertexShader = compile vs_2_0 VShade_Over_Final();
		PixelShader = compile ps_2_0 PShade_Over_Final();
		//VertexShader = compile vs_2_0 VShade_Over_Final();
		//PixelShader = compile ps_2_0 PShade_Over_Final_Fuzzy();
	}
}

technique over_final_fun
{
	pass over0
	{
		VertexShader = compile vs_2_0 VShade_Over_Final();
		PixelShader = compile ps_2_0 PShade_Over_Final_Fun();
	}
	/*pass over1
	{
		VertexShader = compile vs_2_0 VShade_Over_Final();
		PixelShader = compile ps_2_0 PShade_Over_Final_Fun2();
	}*/
}

technique over_final_flatalpha
{
	pass over
	{
		VertexShader = compile vs_2_0 VShade_Over_Final();
		PixelShader = compile ps_2_0 PShade_Over_Final_FlatAlpha();
	}
}

technique over_linear
{
	pass over
	{
		VertexShader = compile vs_2_0 VShade_Over_Final();
		PixelShader = compile ps_2_0 PShade_Over_Linear();
	}
}

technique over_linear_flatalpha
{
	pass over
	{
		VertexShader = compile vs_2_0 VShade_Over_Final();
		PixelShader = compile ps_2_0 PShade_Over_Linear_FlatAlpha();
	}
}

technique over_greys
{
	pass over
	{
		VertexShader = compile vs_2_0 VShade_Over_Final();
		PixelShader = compile ps_2_0 PShade_Over_Greys();
	}
}

technique over_fuzzy
{
	pass over
	{
		VertexShader = compile vs_2_0 VShade_Over_Final();
		PixelShader = compile ps_2_0 PShade_Over_Final_Fuzzy();
	}
}

technique over_offset
{
	pass over
	{
		VertexShader = compile vs_2_0 VShade_Over_Final();
		PixelShader = compile ps_2_0 PShade_Over_Offset();
	}
}