#include <windows.h>
#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <tchar.h>
#include <cmath>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

// dx
#include <d3d9.h>
#include <d3dx9.h>

const float halfPi = D3DX_PI / 2.0f;
const float FOV = D3DX_PI / 2.5f;

// :?
// :?
// START OF UNCRZ STUFF
// :D
// :D

#define UNCRZ_STDSAMPLE D3DMULTISAMPLE_NONE

// debug/hr (performance) defines
#define DEBUG_HR_START(x) if (debugData) hr_start(x)
#define DEBUG_HR_ZERO(x) if (debugData) hr_zero(x)
#define DEBUG_HR_END(x, y, z) if (debugData) hr_end(x, y, z)
#define DEBUG_HR_ACCEND(x, y, z) if (debugData) hr_accend(x, y, z)
#define DEBUG_DX_FLUSH() if (debugData && debugFlushing) { flushQuery->Issue(D3DISSUE_END); while(S_FALSE == flushQuery->GetData( NULL, 0, D3DGETDATA_FLUSH )) ; }

const DWORD VX_PC = 0x00000001;
const DWORD VX_PCT = 0x00000002;
const DWORD VX_PTW4 = 0x00000004;
const DWORD VX_PAT4 = 0x00000008;
const DWORD VX_Over = 0x00000010;

const DWORD DF_light = 0x00000001;
const DWORD DF_default = 0x00000000;

const DWORD SF_notimed = 0x00000001;
const DWORD SF_noclouds = 0x00000002;

const DWORD SF_default = 0x00000000;

LPDIRECT3DVERTEXDECLARATION9 vertexDecPC;
LPDIRECT3DVERTEXDECLARATION9 vertexDecPCT;
LPDIRECT3DVERTEXDECLARATION9 vertexDecPTW4;
LPDIRECT3DVERTEXDECLARATION9 vertexDecPAT4;
LPDIRECT3DVERTEXDECLARATION9 vertexDecOver;

float stof(std::string str)
{
	return atof(str.c_str());
}

int stoi(std::string str)
{
	return atoi(str.c_str());
}

std::string trim(std::string str)
{
	int ni;
	while ((ni = str.find("  ", 0)) != std::string::npos)
	{
		str = str.erase(ni, 1);
	}
	while ((ni = str.find("\r", 0)) != std::string::npos)
	{
		str = str.erase(ni, 1);
	}
	return str;
}

std::vector<std::string> split(std::string str, std::string s)
{
	std::vector<std::string> output;
	int ni;
	while ((ni = str.find(s)) != std::string::npos)
	{
		if (ni > 0)
			output.push_back(str.substr(0, ni));
		str = str.erase(0, ni + s.length());
	}
	output.push_back(str);
	return output;
}

std::string replace(std::string str, std::string gin, std::string rpl)
{
	int off = 0;
	int ni;
	while ((ni = str.find(gin, off)) != std::string::npos)
	{
		str = str.replace(ni, gin.length(), rpl);
		off = ni + rpl.length();
	}
	return str;
}

const float LT_ortho = 0;
const float LT_persp = 1;
const float LT_point = 2;

// tex IDs
const DWORD TID_tex = -1;
const DWORD TID_tex0 = 0;
const DWORD TID_tex1 = 1;
const DWORD TID_tex2 = 2;
const DWORD TID_tex3 = 3;

struct UNCRZ_obj;

const int maxMats = 8; // heh
const int maxClips = 4; // heh
struct UNCRZ_matrix
{
public:
	std::string name;

	LPD3DXMATRIX mat;

	UNCRZ_matrix();
	UNCRZ_matrix(std::string, LPD3DXMATRIX);
};

UNCRZ_matrix::UNCRZ_matrix()
{

}

UNCRZ_matrix::UNCRZ_matrix(std::string nameN, LPD3DXMATRIX matN)
{
	name = nameN;
	mat = matN;
}

UNCRZ_matrix* getMatrix(char* name, std::vector<UNCRZ_matrix*>* matrixList)
{
	for (int i = matrixList->size() - 1; i >= 0; i--)
	{
		if (matrixList->at(i)->name == name)
		{
			return matrixList->at(i);
		}
	}

	return NULL;
}

void setMatrix(std::string nameN, LPD3DXMATRIX matN, std::vector<UNCRZ_matrix*>* matrixList)
{
	for (int i = matrixList->size() - 1; i >= 0; i--)
	{
		if (matrixList->at(i)->name == nameN)
		{
			matrixList->at(i)->mat = matN;
			return;
		}
	}

	matrixList->push_back(new UNCRZ_matrix(nameN, matN));
}

struct UNCRZ_texture
{
public:
	std::string name;

	LPDIRECT3DTEXTURE9 tex;

	UNCRZ_texture();
	UNCRZ_texture(std::string, LPDIRECT3DTEXTURE9);
};

UNCRZ_texture::UNCRZ_texture()
{

}

UNCRZ_texture::UNCRZ_texture(std::string nameN, LPDIRECT3DTEXTURE9 texN)
{
	name = nameN;
	tex = texN;
}

void createTexture(LPDIRECT3DDEVICE9 dxDevice, char* fileName, LPDIRECT3DTEXTURE9* dest, std::vector<UNCRZ_texture*>* textureList)
{
	for (int i = textureList->size() - 1; i >= 0; i--)
	{
		if (textureList->at(i)->name == fileName)
		{
			*dest = textureList->at(i)->tex;
			return;
		}
	}

	D3DXCreateTextureFromFile(dxDevice, fileName, dest);
	UNCRZ_texture* tex = new UNCRZ_texture(fileName, *dest);
	textureList->push_back(tex);
}

const int bboxIndices[36] = {
	0, 1, 2, // base
	0, 2, 3,

	4, 7, 6, // top
	4, 6, 5,

	0, 4, 5, // the others...
	0, 5, 1,

	1, 5, 6,
	1, 6, 2,

	2, 6, 7,
	2, 7, 3,

	3, 7, 4,
	3, 4, 0,
	};

struct UNCRZ_bbox
{
public:
	bool empty;
	float minX, minY, minZ;
	float maxX, maxY, maxZ;
	D3DXVECTOR3 vecArr[8];

	UNCRZ_bbox()
	{
		empty = true;
	}

	UNCRZ_bbox(D3DXVECTOR3 center, float xd, float yd, float zd)
	{
		minX = center.x - xd;
		minY = center.y - yd;
		minZ = center.z - zd;

		maxX = center.x + xd;
		maxY = center.y + yd;
		maxZ = center.z + zd;

		empty = false;
	}

	bool inside(D3DXVECTOR3 point)
	{
		if (point.x >= minX && point.x <= maxX && point.y >= minY && point.y <= maxY && point.z >= minZ && point.z <= maxZ)
			return true;
		return false;
	}
	
	bool overlap(UNCRZ_bbox* bbox)
	{
		if (bbox->minX < maxX && bbox->maxX > minX && bbox->minY < maxY && bbox->maxY > minY && bbox->minZ < maxZ && bbox->maxZ > minZ)
			return true;
		return false;
	}

	bool dothSurviveClipTransformed(D3DXMATRIX* mat)
	{
		D3DXVECTOR4 vecsA[8];

		// transform bbox
		transformVectors(mat, &(vecsA[0]));

		float w;
		D3DXVECTOR4* curVec;

		// check everything
		for (int i = 0; i < 8; i++)
		{
			curVec = &(vecsA[i]);

			w = curVec->w;
			curVec->x /= w;
			curVec->y /= w;
			curVec->z /= w;
			if (-1 <= curVec->x && curVec->x <= 1 && -1 <= curVec->y && curVec->y <= 1 && 0 <= curVec->z && curVec->z <= 1)
				return true;
		}

		// check everything
		for (int i = 0; i < 8; i++)
		{
			curVec = &(vecsA[i]);

			w = curVec->w;
		}

		// try some other thing
		float aminX, aminY, aminZ, amaxX, amaxY, amaxZ;

		// get 2D box
		aminX = vecsA[0].x;
		amaxX = aminX;
		aminY = vecsA[0].y;
		amaxY = aminY;
		aminZ = vecsA[0].z;
		amaxZ = aminZ;
		for (int i = 1; i < 8; i++)
		{
			if (vecsA[i].x < aminX)
				aminX = vecsA[i].x;
			if (vecsA[i].x > amaxX)
				amaxX = vecsA[i].x;
			if (vecsA[i].y < aminY)
				aminY = vecsA[i].y;
			if (vecsA[i].y > amaxY)
				amaxY = vecsA[i].y;
			if (vecsA[i].z < aminZ)
				aminZ = vecsA[i].z;
			if (vecsA[i].z > amaxZ)
				amaxZ = vecsA[i].z;
		}

		if (-1 <= amaxX && aminX <= 1 && -1 <= amaxY && aminY <= 1 && 0 <= amaxZ && aminZ <= 1)
			return true;
		return false;
	}

	bool overlapTransformed(UNCRZ_bbox* bbox, D3DXMATRIX* mat)
	{
		D3DXVECTOR4 vecsA[8];

		// transform bbox
		transformVectors(mat, &(vecsA[0]));

		float aminX, aminY, aminZ, amaxX, amaxY, amaxZ;

		// get 2D box
		aminX = vecsA[0].x;
		amaxX = aminX;
		aminY = vecsA[0].y;
		amaxY = aminY;
		aminZ = vecsA[0].z;
		amaxZ = aminZ;
		for (int i = 1; i < 8; i++)
		{
			if (vecsA[i].x < aminX)
				aminX = vecsA[i].x;
			if (vecsA[i].x > amaxX)
				amaxX = vecsA[i].x;
			if (vecsA[i].y < aminY)
				aminY = vecsA[i].y;
			if (vecsA[i].y > amaxY)
				amaxY = vecsA[i].y;
			if (vecsA[i].z < aminZ)
				aminZ = vecsA[i].z;
			if (vecsA[i].z > amaxZ)
				amaxZ = vecsA[i].z;
		}

		if (bbox->minX < amaxX && bbox->maxX > aminX && bbox->minY < amaxY && bbox->maxY > aminY && bbox->minZ < amaxZ && bbox->maxZ > aminZ)
			return true;
		return false;
	}

	// overlap in x.z plane - assumes projected result is a 2D bounding box, not for precision (no surprises there)
	bool projectedBoundsOverlap(UNCRZ_bbox* bbox, D3DXMATRIX* mat)
	{
		D3DXVECTOR4 vecsA[8];
		D3DXVECTOR4 vecsB[8];

		// transform bboxes
		transformVectors(mat, &(vecsA[0]));
		bbox->transformVectors(mat, &(vecsB[0]));

		float aminX, aminZ, amaxX, amaxZ;
		float bminX, bminZ, bmaxX, bmaxZ;

		// get 2D boxes
		aminX = vecsA[0].x;
		amaxX = aminX;
		aminZ = vecsA[0].z;
		amaxZ = aminZ;
		for (int i = 1; i < 8; i++)
		{
			if (vecsA[i].x < aminX)
				aminX = vecsA[i].x;
			if (vecsA[i].x > amaxX)
				amaxX = vecsA[i].x;
			if (vecsA[i].z < aminZ)
				aminZ = vecsA[i].z;
			if (vecsA[i].z > amaxZ)
				amaxZ = vecsA[i].z;
		}

		bminX = vecsB[0].x;
		bmaxX = bminX;
		bminZ = vecsB[0].z;
		bmaxZ = bminZ;
		for (int i = 1; i < 8; i++)
		{
			if (vecsB[i].x < bminX)
				bminX = vecsB[i].x;
			if (vecsB[i].x > bmaxX)
				bmaxX = vecsB[i].x;
			if (vecsB[i].z < bminZ)
				bminZ = vecsB[i].z;
			if (vecsB[i].z > bmaxZ)
				bmaxZ = vecsB[i].z;
		}

		// see if boxes overlap
		if (bminX < amaxX && bmaxX > aminX && bminX < amaxX && bmaxX > aminX)
			return true;
		return false;
	}

	bool collides(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir)
	{
		float uRes, vRes, distRes; // not returned

		D3DXVECTOR3* a;
		D3DXVECTOR3* b;
		D3DXVECTOR3* c;

		for (int i = 0; i < 36; i += 3)
		{
			a = &(vecArr[bboxIndices[i]]);
			b = &(vecArr[bboxIndices[i + 1]]);
			c = &(vecArr[bboxIndices[i + 2]]);

			if (D3DXIntersectTri(a, b, c, rayPos, rayDir, &uRes, &vRes, &distRes))
			{
				return true;
			}
		}

		return false;
	}
	
	void transformVectors(D3DXMATRIX* mat, D3DXVECTOR4* vecs)
	{
		D3DXVec3TransformArray(vecs, sizeof(D3DXVECTOR4), &(vecArr[0]), sizeof(D3DXVECTOR3), mat, 8);
		//for (int i = 0; i < 8; i++)
		//{
		//	D3DXVec3Transform(&(vecs[i]), &(vecArr[i]), mat);
		//}
	}

	void fillVectors()
	{
		// boottom
		vecArr[0].x = minX;
		vecArr[0].y = minY;
		vecArr[0].z = minZ;

		vecArr[1].x = maxX;
		vecArr[1].y = minY;
		vecArr[1].z = minZ;
		
		vecArr[2].x = maxX;
		vecArr[2].y = maxY;
		vecArr[2].z = minZ;

		vecArr[3].x = minX;
		vecArr[3].y = maxY;
		vecArr[3].z = minZ;
		
		// top
		vecArr[4].x = minX;
		vecArr[4].y = minY;
		vecArr[4].z = maxZ;

		vecArr[5].x = maxX;
		vecArr[5].y = minY;
		vecArr[5].z = maxZ;
		
		vecArr[6].x = maxX;
		vecArr[6].y = maxY;
		vecArr[6].z = maxZ;

		vecArr[7].x = minX;
		vecArr[7].y = maxY;
		vecArr[7].z = maxZ;
	}

	void include(UNCRZ_bbox* bbox, D3DXMATRIX* mat)
	{
		D3DXVECTOR4 vecs[8];
		D3DXVECTOR4 curVec;
		
		if (bbox->empty)
		{
			return;
		}

		bbox->transformVectors(mat, &(vecs[0]));

		for (int i = 0; i < 8; i++)
		{
			curVec = vecs[i];
			include(D3DXVECTOR3(curVec.x, curVec.y, curVec.z));
		}
	}

	void include(UNCRZ_bbox* bbox)
	{
		if (empty)
		{
			minX = bbox->minX;
			minY = bbox->minY;
			minZ = bbox->minZ;

			maxX = bbox->maxX;
			maxY = bbox->maxY;
			maxZ = bbox->maxZ;

			empty = false;
		}
		else
		{
			if (bbox->minX < minX)
				minX = bbox->minX;
			if (bbox->minY < minY)
				minY = bbox->minY;
			if (bbox->minZ < minZ)
				minZ = bbox->minZ;

			if (bbox->maxX > maxX)
				maxX = bbox->maxX;
			if (bbox->maxY > maxY)
				maxY = bbox->maxY;
			if (bbox->maxZ > maxZ)
				maxZ = bbox->maxZ;
		}
	}

	void include(D3DXVECTOR3 vec)
	{
		if (empty)
		{
			minX = vec.x;
			minY = vec.y;
			minZ = vec.z;

			maxX = vec.x;
			maxY = vec.y;
			maxZ = vec.z;

			empty = false;
		}
		else
		{
			if (vec.x < minX)
				minX = vec.x;
			if (vec.y < minY)
				minY = vec.y;
			if (vec.z < minZ)
				minZ = vec.z;

			if (vec.x > maxX)
				maxX = vec.x;
			if (vec.y > maxY)
				maxY = vec.y;
			if (vec.z > maxZ)
				maxZ = vec.z;
		}
	}
};

struct lightData
{
public:
	std::string name;

	bool lightEnabled;

	float dimX;
	float dimY;
	float lightDepth;
	float coneness;

	float lightType;

	D3DXVECTOR4 lightAmbient;
	D3DXVECTOR4 lightColMod;

	D3DXVECTOR4 lightPos;
	D3DXVECTOR4 lightDir;
	D3DXVECTOR3 lightUp;

	LPDIRECT3DTEXTURE9 lightTex;
	LPDIRECT3DTEXTURE9 lightPatternTex;
	LPDIRECT3DSURFACE9 lightSurface;
	UINT texWidth;
	UINT texHeight;

	D3DXMATRIX lightViewProj; // (texAligned, x and y are 0..1)
	D3DXMATRIX lightViewProjVP; // (x and y are -1..1)

	std::vector<int> zsoLocalIndexes;

	bool useLightMap; // should be disabled for LT_point!

	UNCRZ_bbox lightBox;
	bool allowSkip; // allow stuff to skip this light if the light thinks it won't shine on them
	bool curDrawSkip;

	lightData(std::string nameN)
	{
		name = nameN;
	}

	void init(LPDIRECT3DDEVICE9 dxDevice, UINT w, UINT h, char* lightPatternFile, std::vector<UNCRZ_texture*>* textureList)
	{	
		D3DXCreateTexture(dxDevice, w, h, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &lightTex); // I don't know what implications there are of using this, but it seems to work
		//D3DXCreateTexture(dxDevice, w, h, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &lightTex);
		lightTex->GetSurfaceLevel(0, &lightSurface);
		
		createTexture(dxDevice, lightPatternFile, &lightPatternTex, textureList);
		//HRESULT res = D3DXCreateTextureFromFile(dxDevice, lightPatternFile, &lightPatternTex);

		float ldMod = sqrtf(lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z);
		lightDir.x /= ldMod;
		lightDir.y /= ldMod;
		lightDir.z /= ldMod;

		texWidth = w;
		texHeight = h;
	}

	void updateBox()
	{
		if (lightType == LT_point)
		{
			lightBox = UNCRZ_bbox((D3DXVECTOR3)lightPos, lightDepth, lightDepth, lightDepth);
		}
		else
		{
			lightBox = UNCRZ_bbox(); // empty
		}
	}

	bool canSkip(UNCRZ_bbox* bbox)
	{
		if (!allowSkip)
			return false;

		if (lightBox.empty == false && !lightBox.overlap(bbox))
			return true;

		return false;
	}

	void release()
	{
		// implement
	}
};

const float DDT_ortho = 0;
const float DDT_persp = 1;

// this looks /alot/ like light data, because it is a simplified light data
//  - this projects pure colour (source colour etc. ignored, like a decal) without a shadow map
//  - these have NO respect for lighting and should be used for highlighting effects and such
//    if you want lighting build proper decals
struct dynamicDecalData
{
public:
	bool decalEnabled;

	float dimX;
	float dimY;
	float lightDepth;
	float lightType;

	D3DXVECTOR4 lightColMod;

	D3DXVECTOR4 lightPos;
	D3DXVECTOR4 lightDir;
	D3DXVECTOR3 lightUp;

	LPDIRECT3DTEXTURE9 lightPatternTex;

	D3DXMATRIX lightViewProj;

	void init(LPDIRECT3DDEVICE9 dxDevice, char* lightPatternFile, std::vector<UNCRZ_texture*>* textureList)
	{
		createTexture(dxDevice, lightPatternFile, &lightPatternTex, textureList);
		//HRESULT res = D3DXCreateTextureFromFile(dxDevice, lightPatternFile, &lightPatternTex);

		float ldMod = sqrtf(lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z);
		lightDir.x /= ldMod;
		lightDir.y /= ldMod;
		lightDir.z /= ldMod;
	}

	void release()
	{
		// implement
	}
};

struct uiItem;
struct uiTexItem;
struct uiTextItem;

const DWORD GDOF_ms = 0;
const DWORD GDOF_dms = 1;
const DWORD GDOF_prop100 = 2;

struct drawData
{
public:
	LPD3DXMATRIX viewProj;
	LPD3DXMATRIX viewMat;
	LPD3DXMATRIX projMat;
	D3DXVECTOR4 eyePos;
	D3DXVECTOR4 eyeDir;
	std::vector<lightData*> lightDatas;
	std::vector<dynamicDecalData*> dynamicDecalDatas;
	bool performPlainPass; // does this make a shred of sense? (something has to sort out the alpha channel)
	float farDepth;
	float lightCoof;
	float lightType;
	float lightDepth;
	float lightConeness;

	DWORD clipEnable;

	float ticker;

	int cullCount;
	int drawCount;

	D3DVIEWPORT9* targetVp;
	D3DVIEWPORT9* sideVp;
	LPDIRECT3DTEXTURE9 targetTex;
	LPDIRECT3DSURFACE9 targetSurface;
	LPDIRECT3DTEXTURE9 sideTex;
	LPDIRECT3DSURFACE9 sideSurface;
	
	LPDIRECT3DSURFACE9 zSideSurface;
	LPDIRECT3DSURFACE9 zTargetSurface;


	// these are for things passes draw datas without global access
	char buff[100];
	IDirect3DQuery9* flushQuery;
	bool debugData;
	bool debugFlushing;
	uiTexItem* genericDebugView;
	uiTextItem** genericLabel;
	uiTexItem** genericBar;
	LARGE_INTEGER hrsec;

	LARGE_INTEGER hridstart;
	LARGE_INTEGER hridend;

	void disableClip(LPDIRECT3DDEVICE9 dxDevice)
	{
		dxDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
		dxDevice->SetRenderState(D3DRS_CLIPPING, false);
	}

	void enableClip(LPDIRECT3DDEVICE9 dxDevice)
	{
		dxDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, clipEnable);
		dxDevice->SetRenderState(D3DRS_CLIPPING, true);
	}

	drawData(LPD3DXMATRIX viewProjN, LPD3DXMATRIX viewMatN, LPD3DXMATRIX projMatN, std::vector<lightData*> lightDatasN, std::vector<dynamicDecalData*> dynamicDecalDatasN, float lightCoofN)
	{
		viewProj = viewProjN;
		viewMat = viewMatN;
		projMat = projMatN;
		lightDatas = lightDatasN;
		dynamicDecalDatas = dynamicDecalDatasN;
		lightCoof = lightCoofN;

		cullCount = 0;
		drawCount = 0;
	}

private:
	void hr_start(LARGE_INTEGER* start)
	{
		QueryPerformanceCounter(start);
	}

	void hr_end(LARGE_INTEGER* start, LARGE_INTEGER* end, float* outTime)
	{
		QueryPerformanceCounter(end);
		*outTime = ((double)(end->QuadPart - start->QuadPart) / (double)(hrsec.QuadPart));
	}

public:
	void startTimer(bool preFlushDx);
	void stopTimer(int genericIndex, bool flushDx, char* label, DWORD);
	void nontimed(int genericIndex, char* label, float value, DWORD);
};

struct vertexPC
{
public:
	float x;
	float y;
	float z;
	float w;

	float nx;
	float ny;
	float nz;
	float nw;

	float r;
	float g;
	float b;
	float a;

	float tti;

	vertexPC() { }

	vertexPC(D3DXVECTOR3 posN, D3DXVECTOR3 colN, float ttiN)
	{
		x = posN.x;
		y = posN.y;
		z = posN.z;
		w = 1.0;

		nx = 0.0;
		ny = 0.0;
		nz = 0.0;
		nw = 0.0;

		a = 1.0;
		r = colN.x;
		g = colN.y;
		b = colN.z;

		tti = ttiN;
	}

	vertexPC(D3DXVECTOR3 posN, float rN, float gN, float bN, float ttiN)
	{
		x = posN.x;
		y = posN.y;
		z = posN.z;
		w = 1.0;

		nx = 0.0;
		ny = 0.0;
		nz = 0.0;
		nw = 0.0;

		a = 1.0;
		r = rN;
		g = gN;
		b = bN;

		tti = ttiN;
	}

	vertexPC(float xN, float yN, float zN, D3DXVECTOR3 colN, float ttiN)
	{
		x = xN;
		y = yN;
		z = zN;
		w = 1.0;

		nx = 0.0;
		ny = 0.0;
		nz = 0.0;
		nw = 0.0;

		a = 1.0;
		r = colN.x;
		g = colN.y;
		b = colN.z;

		tti = ttiN;
	}

	vertexPC(float xN, float yN, float zN, float rN, float gN, float bN, float ttiN)
	{
		x = xN;
		y = yN;
		z = zN;
		w = 1.0;

		nx = 0.0;
		ny = 0.0;
		nz = 0.0;
		nw = 0.0;

		a = 1.0;
		r = rN;
		g = gN;
		b = bN;

		tti = ttiN;
	}

	vertexPC(float xN, float yN, float zN, float nxN, float nyN, float nzN, float rN, float gN, float bN, float ttiN)
	{
		x = xN;
		y = yN;
		z = zN;
		w = 1.0;

		nx = nxN;
		ny = nyN;
		nz = nzN;
		nw = 0.0;

		a = 1.0;
		r = rN;
		g = gN;
		b = bN;

		tti = ttiN;
	}

	D3DXVECTOR3 getPosVec3()
	{
		return D3DXVECTOR3(x, y, z);
	}
};

struct vertexPCT
{
public:
	float x;
	float y;
	float z;
	float w;

	float nx;
	float ny;
	float nz;
	float nw;

	float r;
	float g;
	float b;
	float a;

	float tu;
	float tv;

	float tti;

	vertexPCT() { }

	vertexPCT(vertexPC posCol, float tuN, float tvN)
	{
		x = posCol.x;
		y = posCol.y;
		z = posCol.z;
		w = posCol.w;

		nx = posCol.nx;
		ny = posCol.ny;
		nz = posCol.nz;
		nw = 0.0;

		a = posCol.a;
		r = posCol.r;
		g = posCol.g;
		b = posCol.b;
	
		tu = tuN;
		tv = tvN;

		tti = posCol.tti;
	}

	D3DXVECTOR3 getPosVec3()
	{
		return D3DXVECTOR3(x, y, z);
	}
};

struct vertexPTW4
{
public:
	float x;
	float y;
	float z;
	float w;

	float nx;
	float ny;
	float nz;
	float nw;

	float tu;
	float tv;

	float w0;
	float w1;
	float w2;
	float w3;

	vertexPTW4() { }

	vertexPTW4(float xN, float yN, float zN, float uN, float vN, float w0N, float w1N, float w2N, float w3N)
	{
		x = xN;
		y = yN;
		z = zN;
		w = 1.0f;

		nx = 0.0;
		ny = 1.0;
		nz = 0.0;
		nw = 0.0;
		
		tu = uN;
		tv = vN;
		
		w0 = w0N;
		w1 = w1N;
		w2 = w2N;
		w3 = w3N;
	}

	vertexPCT quickPCT()
	{
		vertexPCT res(vertexPC(x, y, z, 1.0f, 1.0f, 1.0f, 0), tu, tv);
		res.nx = nx;
		res.ny = ny;
		res.nz = nz;
		res.nw = nw;
		return res;
	}

	vertexPCT quickPCT(float a, float r, float g, float b)
	{
		vertexPCT res(vertexPC(x, y, z, r, g, b, 0), tu, tv);
		res.a = a;
		res.nx = nx;
		res.ny = ny;
		res.nz = nz;
		res.nw = nw;
		return res;
	}

	D3DXVECTOR3 quickPos()
	{
		D3DXVECTOR3 res(x, y, z);
		return res;
	}
};

struct vertexPSW4
{
public:
	float x;
	float y;
	float z;
	float w;

	float sx;
	float sy;

	float w0;
	float w1;
	float w2;
	float w3;
};

struct vertexPAT4
{
public:
	float x;
	float y;
	float z;
	float w;

	float nx;
	float ny;
	float nz;
	float nw;

	float a;

	float tx;
	float ty;
	float tz;
	float tw;

	float tti;

	vertexPAT4() { }

	vertexPAT4(vertexPC posCol, float txN, float tyN, float tzN, float twN)
	{
		x = posCol.x;
		y = posCol.y;
		z = posCol.z;
		w = posCol.w;

		nx = posCol.nx;
		ny = posCol.ny;
		nz = posCol.nz;
		nw = posCol.nw;

		a = posCol.a;
	
		tx = txN;
		ty = tyN;
		tz = tzN;
		tw = twN;

		tti = posCol.tti;
	}

	vertexPAT4(vertexPC pos, float aN, float txN, float tyN, float tzN, float twN)
	{
		x = pos.x;
		y = pos.y;
		z = pos.z;
		w = pos.w;

		nx = pos.nx;
		ny = pos.ny;
		nz = pos.nz;
		nw = pos.nw;

		a = aN;
	
		tx = txN;
		ty = tyN;
		tz = tzN;
		tw = twN;

		tti = pos.tti;
	}

	vertexPAT4(vertexPCT pos, float tzN, float twN)
	{
		x = pos.x;
		y = pos.y;
		z = pos.z;
		w = pos.w;

		nx = pos.nx;
		ny = pos.ny;
		nz = pos.nz;
		nw = pos.nw;

		a = pos.a;
	
		tx = pos.tu;
		ty = pos.tv;
		tz = tzN;
		tw = twN;

		tti = pos.tti;
	}

	vertexPAT4(vertexPTW4 pos, float aN, float tzN, float twN)
	{
		x = pos.x;
		y = pos.y;
		z = pos.z;
		w = pos.w;

		nx = pos.nx;
		ny = pos.ny;
		nz = pos.nz;
		nw = pos.nw;

		a = aN;
	
		tx = pos.tu;
		ty = pos.tv;
		tz = tzN;
		tw = twN;

		tti = 0.0;
	}

	vertexPAT4(vertexPTW4 pos, float aN, float txN, float tyN, float tzN, float twN)
	{
		x = pos.x;
		y = pos.y;
		z = pos.z;
		w = pos.w;

		nx = pos.nx;
		ny = pos.ny;
		nz = pos.nz;
		nw = pos.nw;

		a = aN;
	
		tx = txN;
		ty = tyN;
		tz = tzN;
		tw = twN;

		tti = 0.0;
	}
};

struct vertexOver
{
public:
	float x;
	float y;
	float z;
	float w;

	float tu;
	float tv;

	vertexOver() { }

	vertexOver(float xN, float yN, float zN, float tuN, float tvN)
	{
		x = xN;
		y = yN;
		z = zN;
		w = 1.0;
	
		tu = tuN;
		tv = tvN;
	}
};

struct UNCRZ_FBF_anim;
struct UNCRZ_FBF_anim_inst;
struct UNCRZ_FBF_anim_motion;
struct UNCRZ_FBF_anim_instr;
struct UNCRZ_section;
struct UNCRZ_segment;
struct UNCRZ_trans_arr;

struct UNCRZ_decal
{
public:
	int age;
	int remAge;
	int numFaces;
	vertexPAT4* vertexArray;
	int* indexArray; // for re-retrieving values (should they change) (NOT for rendering)
	LPDIRECT3DTEXTURE9 tex;
	void* vertexSource;
	DWORD vertexType;

	UNCRZ_decal(int, vertexPAT4*, int*, void*, DWORD, int);
	void updateVerts();
	void updatePTW4();
	void updatePCT();
	void destroy();
};

UNCRZ_decal::UNCRZ_decal(int remAgeN, vertexPAT4* vertexArraySrc, int* indexArraySrc, void* vertexSourceN, DWORD vertexTypeN, int numFacesN)
{
	remAge = remAgeN;
	numFaces = numFacesN;

	int stride = sizeof(vertexPAT4);
	int iStride = sizeof(int);
	vertexArray = (vertexPAT4*)malloc(numFaces * 3 * stride);
	memcpy(vertexArray, vertexArraySrc, numFaces * 3 * stride);
	indexArray = (int*)malloc(numFaces * 3 * iStride);
	memcpy(indexArray, indexArraySrc, numFaces * 3 * iStride);
	vertexSource = vertexSourceN;
	vertexType = vertexTypeN;
}

void UNCRZ_decal::updateVerts()
{
	if (vertexType == VX_PCT)
		updatePCT();
	else if (vertexType == VX_PTW4)
		updatePTW4();
}

void UNCRZ_decal::updatePTW4()
{
	vertexPTW4* vPTW4s = (vertexPTW4*)vertexSource;

	for (int i = 0; i < numFaces * 3; i++)
	{
		int si = indexArray[i];
		vertexArray[i].x = vPTW4s[si].x;
		vertexArray[i].y = vPTW4s[si].y;
		vertexArray[i].z = vPTW4s[si].z;
		vertexArray[i].w = vPTW4s[si].w;
		vertexArray[i].nx = vPTW4s[si].nx;
		vertexArray[i].ny = vPTW4s[si].ny;
		vertexArray[i].nz = vPTW4s[si].nz;
		vertexArray[i].nw = vPTW4s[si].nw;
	}
}

void UNCRZ_decal::updatePCT()
{
	vertexPCT* vPCTs = (vertexPCT*)vertexSource;

	for (int i = 0; i < numFaces * 3; i++)
	{
		int si = indexArray[i];
		vertexArray[i].x = vPCTs[si].x;
		vertexArray[i].y = vPCTs[si].y;
		vertexArray[i].z = vPCTs[si].z;
		vertexArray[i].w = vPCTs[si].w;
		vertexArray[i].nx = vPCTs[si].nx;
		vertexArray[i].ny = vPCTs[si].ny;
		vertexArray[i].nz = vPCTs[si].nz;
		vertexArray[i].nw = vPCTs[si].nw;
	}
}

void UNCRZ_decal::destroy()
{
	delete[numFaces * 3 * sizeof(vertexPCT)] vertexArray;
}

const DWORD LF_lit = 0x00000001; // uses multi-pass lighting to be pretty
const DWORD LF_shadows = 0x00000002; // rendered onto light maps

const DWORD LF_default = 0x00000003; // lit | shadows

struct UNCRZ_lodData
{
public:
	DWORD lightingFlags;

	/* draw dists (concept)
	float decalDrawDist; //  dist at which decals stop being drawn (from center of model/obj (not sure which yet))
	float drawDist; //  dist at which the model stops being drawn (from center of model/obj (not sure which yet))
	*/

};

struct UNCRZ_model
{
public:
	std::string name;

	LPDIRECT3DVERTEXBUFFER9 vBuff;
	LPDIRECT3DVERTEXDECLARATION9 vertexDec;
	LPDIRECT3DINDEXBUFFER9 iBuff;
	DWORD vertexType;
	UINT stride;
	int numVertices;
	int numIndices;
	UNCRZ_trans_arr* transArr;

	std::vector<UNCRZ_segment*> segments;
	std::vector<UNCRZ_segment*> allSegs;
	std::vector<UNCRZ_section*> sections;

	UNCRZ_FBF_anim_inst* animInst;

	void* vertexArray; // array of vertices
	short* indexArray; // array of indicies

	UNCRZ_bbox modelBox;
	bool noCull;
	
	void allSegsRequireUpdate();
	void update(LPD3DXMATRIX trans, bool forceUpdate = false);
	void drawMany(LPDIRECT3DDEVICE9 dxDevice, drawData* ddat, UNCRZ_model** arr, int count, DWORD drawArgs);
	void draw(LPDIRECT3DDEVICE9 dxDevice, drawData* ddat, DWORD drawArgs);
	void drawBBoxDebug(LPDIRECT3DDEVICE9 dxDevice, drawData* ddat);
	UNCRZ_model();
	UNCRZ_model(UNCRZ_model* gin);
	UNCRZ_model(std::string nameN);
	void createSegmentBoxes();
	void changeAnim(UNCRZ_FBF_anim* newAnim);
	void release();
	void clearDecals();
	void fillVBuff();
	void createVBuff(LPDIRECT3DDEVICE9 dxDevice, void* vds);
	void fillIBuff();
	void createIBuff(LPDIRECT3DDEVICE9 dxDevice, short* ids);
	bool collides(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes);
	bool collidesVX_PC(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes);
	bool collidesVX_PCT(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes);
	int collidesVertex(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes);
	int collidesVertexVX_PC(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes);
	int collidesVertexVX_PCT(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes);
};

struct UNCRZ_trans_arr
{
	std::vector<D3DXMATRIX> arr;
	int len;
	
	void setValue(int index, D3DXMATRIX* m)
	{
		arr[index] = *m;
	}
	
	D3DXMATRIX* getValue(int index)
	{
		return &arr[index];
	}

	void create(int lenN)
	{
		len = lenN;
		for (int i = 0; i < len; i++)
			arr.push_back(D3DXMATRIX());
	}

	UNCRZ_trans_arr() { }
};

struct UNCRZ_effect
{
public:
	std::string name;

	LPD3DXEFFECT effect;
	D3DXHANDLE colMod;
	D3DXHANDLE texture;
	D3DXHANDLE textureData;
	D3DXHANDLE targetVPMat;
	D3DXHANDLE targetTexture;
	D3DXHANDLE targetTextureData;
	D3DXHANDLE farDepth;
	D3DXHANDLE farDepthInv;
	D3DXHANDLE lightType;
	D3DXHANDLE lightCoof;
	D3DXHANDLE lightDepth;
	D3DXHANDLE lightConeness;
	D3DXHANDLE lightPos;
	D3DXHANDLE lightDir;
	D3DXHANDLE lightAmbient;
	D3DXHANDLE lightColMod;
	D3DXHANDLE lightTexture;
	D3DXHANDLE lightPatternTexture;
	D3DXHANDLE texture0;
	D3DXHANDLE texture1;
	D3DXHANDLE texture2;
	D3DXHANDLE texture3;
	D3DXHANDLE transArr;
	D3DXHANDLE spriteLoc;
	D3DXHANDLE spriteDim;
	D3DXHANDLE viewMat;
	D3DXHANDLE projMat;
	D3DXHANDLE viewProj;
	D3DXHANDLE eyePos;
	D3DXHANDLE eyeDir;
	D3DXHANDLE lightViewProj;
	D3DXHANDLE mats;
	D3DXHANDLE matsElems[maxMats];
	D3DXHANDLE ticker;

	void setLightData(lightData* ld)
	{
		setLightType(ld->lightType);
		setLightDepth(ld->lightDepth);
		//setLightConeness(ld->coneness);
		setLightPos(&ld->lightPos);
		setLightDir(&ld->lightDir);
		setLightAmbient((float*)&ld->lightAmbient);
		setLightColMod((float*)&ld->lightColMod);

		if (ld->useLightMap)
		{
			setLightTexture(ld->lightTex);
			setLightViewProj(&ld->lightViewProj);
			setLightPatternTexture(ld->lightPatternTex);
		}
	}

	void setDynamicDecalData(dynamicDecalData* ddd)
	{
		setLightType(ddd->lightType);
		setLightDepth(ddd->lightDepth);
		setLightPos(&ddd->lightPos);
		setLightDir(&ddd->lightDir);
		setLightColMod((float*)&ddd->lightColMod);

		setLightViewProj(&ddd->lightViewProj);
		setLightPatternTexture(ddd->lightPatternTex);
	}

	void setFarDepth(float dep)
	{
		effect->SetFloat(farDepth, dep);
		effect->SetFloat(farDepthInv, 1.0 / dep);
	}

	void setcolMod(float* ptr)
	{
		effect->SetFloatArray(colMod, ptr, 4);
	}

	void setLightAmbient(float* ptr)
	{
		effect->SetFloatArray(lightAmbient, ptr, 4);
	}

	void setLightColMod(float* ptr)
	{
		effect->SetFloatArray(lightColMod, ptr, 4);
	}

	void setTexture(LPDIRECT3DTEXTURE9 tex)
	{
		effect->SetTexture(texture, tex);
	}

	void setTextureData(float* dat)
	{
		effect->SetFloatArray(textureData, dat, 4);
	}

	void setTargetTexture(LPDIRECT3DTEXTURE9 tex)
	{
		effect->SetTexture(targetTexture, tex);
	}

	void setTargetTextureData(float* dat)
	{
		effect->SetFloatArray(targetTextureData, dat, 4);
	}

	void setTargetVPMat(float* mat)
	{
		effect->SetFloatArray(targetVPMat, mat, 16);
	}

	void setLightCoof(float coof)
	{
		effect->SetFloatArray(lightCoof, &coof, 1);
	}

	void setLightDepth(float coof)
	{
		effect->SetFloatArray(lightDepth, &coof, 1);
	}

	void setLightConeness(float coneness)
	{
		effect->SetFloatArray(lightConeness, &coneness, 1);
	}

	void setLightType(float type)
	{
		effect->SetFloatArray(lightType, &type, 1);
	}

	void setLightPos(D3DXVECTOR4* pos)
	{
		effect->SetFloatArray(lightPos, (float*)pos, 4);
	}

	void setLightDir(D3DXVECTOR4* dir)
	{
		effect->SetFloatArray(lightDir, (float*)dir, 4);
	}

	void setLightTexture(LPDIRECT3DTEXTURE9 tex)
	{
		effect->SetTexture(lightTexture, tex);
	}

	void setLightPatternTexture(LPDIRECT3DTEXTURE9 tex)
	{
		effect->SetTexture(lightPatternTexture, tex);
	}

	void setTexture0(LPDIRECT3DTEXTURE9 tex)
	{
		effect->SetTexture(texture0, tex);
	}

	void setTexture1(LPDIRECT3DTEXTURE9 tex)
	{
		effect->SetTexture(texture1, tex);
	}

	void setTexture2(LPDIRECT3DTEXTURE9 tex)
	{
		effect->SetTexture(texture2, tex);
	}

	void setTexture3(LPDIRECT3DTEXTURE9 tex)
	{
		effect->SetTexture(texture3, tex);
	}

	void setTransArr(UNCRZ_trans_arr* tarr)
	{
		HRESULT res = effect->SetFloatArray(transArr, (float*)tarr->arr.front(), tarr->len * 16);
	}

	void setSpriteLoc(float* larr, int offset, int num, int dataSizeInFloats)
	{
		HRESULT res = effect->SetFloatArray(spriteLoc, (float*)(larr + offset * dataSizeInFloats), num * dataSizeInFloats);
		if (res != D3D_OK)
			std::cout << "Help!";
	}

	void setSpriteDim(D3DXVECTOR4* dim)
	{
		HRESULT res = effect->SetFloatArray(spriteDim, (float*)dim, 4);
	}

	void setViewMat(LPD3DXMATRIX viewMatrix)
	{
		effect->SetMatrix(viewMat, viewMatrix);
	}

	void setProjMat(LPD3DXMATRIX projMatrix)
	{
		effect->SetMatrix(projMat, projMatrix);
	}

	void setViewProj(LPD3DXMATRIX viewProjMatrix)
	{
		effect->SetMatrix(viewProj, viewProjMatrix);
	}

	void setEyePos(D3DXVECTOR4* pos)
	{
		effect->SetFloatArray(eyePos, (float*)pos, 4);
	}

	void setEyeDir(D3DXVECTOR4* dir)
	{
		effect->SetFloatArray(eyeDir, (float*)dir, 4);
	}

	void setLightViewProj(LPD3DXMATRIX lightViewProjMatrix)
	{
		effect->SetMatrix(lightViewProj, lightViewProjMatrix);
	}

	void setTechnique(D3DXHANDLE tech)
	{
		effect->SetTechnique(tech);
	}

	void setMats(LPD3DXMATRIX marr, int count)
	{
		effect->SetMatrixArray(mats, (const D3DXMATRIX*)marr, count);
	}

	void setMat(int idx, LPD3DXMATRIX mat)
	{
		effect->SetMatrix(matsElems[idx], (const D3DXMATRIX*)mat);
	}

	void setTicker(float val)
	{
		effect->SetFloat(ticker, val);
	}

	UNCRZ_effect() { }
	
	static UNCRZ_effect UNCRZ_effectFromFile(char* fileName, LPDIRECT3DDEVICE9 dxDevice, DWORD vertexType, std::string nameN)
	{
		LPD3DXEFFECT tempEffect;
		LPD3DXBUFFER errs = NULL;
		HRESULT res = D3DXCreateEffectFromFile(dxDevice, fileName, NULL, NULL, 0, NULL, &tempEffect, &errs);

		return UNCRZ_effect(tempEffect, vertexType, nameN);
	}

	UNCRZ_effect(LPD3DXEFFECT effectN, DWORD vertexType, std::string nameN)
	{
		effect = effectN;

		// vPC, vPCT
		colMod = effect->GetParameterByName(NULL, "colMod");
		// vPCT
		texture = effect->GetParameterByName(NULL, "tex");
		textureData = effect->GetParameterByName(NULL, "texData");
		
		// vPTW4
		texture0 = effect->GetParameterByName(NULL, "tex0");
		texture1 = effect->GetParameterByName(NULL, "tex1");
		texture2 = effect->GetParameterByName(NULL, "tex2");
		texture3 = effect->GetParameterByName(NULL, "tex3");

		targetVPMat = effect->GetParameterByName(NULL, "vpMat");
		targetTexture = effect->GetParameterByName(NULL, "targTex");
		targetTextureData = effect->GetParameterByName(NULL, "targTexData");
		farDepth = effect->GetParameterByName(NULL, "farDepth");
		farDepthInv = effect->GetParameterByName(NULL, "farDepthInv");
		lightType = effect->GetParameterByName(NULL, "lightType");
		lightCoof = effect->GetParameterByName(NULL, "lightCoof");
		lightDepth = effect->GetParameterByName(NULL, "lightDepth");
		lightConeness = effect->GetParameterByName(NULL, "lightConeness");
		lightPos = effect->GetParameterByName(NULL, "lightPos");
		lightDir = effect->GetParameterByName(NULL, "lightDir");
		lightAmbient = effect->GetParameterByName(NULL, "lightAmbient");
		lightColMod = effect->GetParameterByName(NULL, "lightColMod");
		lightTexture = effect->GetParameterByName(NULL, "lightTex");
		lightPatternTexture = effect->GetParameterByName(NULL, "lightPatternTex");
		transArr = effect->GetParameterByName(NULL, "transarr");
		spriteLoc = effect->GetParameterByName(NULL, "spriteLoc");
		spriteDim = effect->GetParameterByName(NULL, "spriteDim");
		viewMat = effect->GetParameterByName(NULL, "viewMat");
		projMat = effect->GetParameterByName(NULL, "projMat");
		viewProj = effect->GetParameterByName(NULL, "viewProj");
		eyePos = effect->GetParameterByName(NULL, "eyePos");
		eyeDir = effect->GetParameterByName(NULL, "eyeDir");
		lightViewProj = effect->GetParameterByName(NULL, "lightViewProj");
		mats = effect->GetParameterByName(NULL, "mats");
		for (int i = 0; i < maxMats; i++)
		{
			matsElems[i] = effect->GetParameterElement(mats, i);
		}
		ticker = effect->GetParameterByName(NULL, "ticker");
		name = nameN;
	}

	void release()
	{
		effect->Release();
	}
};

struct UNCRZ_FBF_anim_flow_start
{
	int motion;
	int motionDuration;

	UNCRZ_FBF_anim_flow_start()
	{
		motion = 0;
		motionDuration = 0;
	}

	UNCRZ_FBF_anim_flow_start(int motionN, int motionDurationN)
	{
		motion = motionN;
		motionDuration = motionDurationN;
	}
};

struct UNCRZ_FBF_anim_flow_state
{
	UNCRZ_FBF_anim_flow_start* start;
	int curMotion;
	int curMotionDuration;

	void reset()
	{
		curMotion = start->motion;
		curMotionDuration = start->motionDuration;
	}

	UNCRZ_FBF_anim_flow_state(UNCRZ_FBF_anim_flow_start* startN)
	{
		start = startN;
		reset();
	}
};

struct UNCRZ_FBF_anim_flow
{
public:
	UNCRZ_FBF_anim_flow_start start;
	std::string name;
	std::vector<UNCRZ_FBF_anim_instr> instrs;
	std::vector<UNCRZ_FBF_anim_motion> motions;

	void run(UNCRZ_FBF_anim_inst* inst, UNCRZ_FBF_anim_flow_state* state);
	UNCRZ_FBF_anim_flow() { }
	UNCRZ_FBF_anim_flow(std::string nameN);
};

struct UNCRZ_FBF_anim
{
public:
	std::vector<UNCRZ_FBF_anim_flow> flows;
	UNCRZ_model* model; // used for create, and NOTHING ELSE
	UNCRZ_FBF_anim_flow* lastFlow;

	std::string name;
	std::string modelName; // used for checking if a model can use it

	UNCRZ_FBF_anim::UNCRZ_FBF_anim() { }

	UNCRZ_FBF_anim::UNCRZ_FBF_anim(std::string nameN)
	{
		name = nameN;
	}

	void run(UNCRZ_FBF_anim_inst* inst);
	void addFlow(std::string name);
	void setFlowStart(int sm, int smd);
	void setFlowStart(std::string sms, int smd);
	void endMotion();
	void addMotion(std::string name);
	void setMotionDuration(int duration);
	void setMotionCausesUpdate(bool mg);
	void addLine(std::string line);
	UNCRZ_FBF_anim(UNCRZ_model* modelN);
};

struct UNCRZ_FBF_anim_inst
{
	UNCRZ_model* model;
	std::vector<UNCRZ_FBF_anim_flow_state> states;
	UNCRZ_FBF_anim* anim;

	void reset(UNCRZ_FBF_anim* animN)
	{
		anim = animN;
		int olen = states.size() - 1;
		for (int i = anim->flows.size() - 1; i >= olen; i--)
		{
			states.push_back(UNCRZ_FBF_anim_flow_state(&anim->flows[i].start));
		}
		for (int i = states.size() - 1; i >= 0; i--)
		{
			states[i].reset();
		}
	}

	UNCRZ_FBF_anim_inst(UNCRZ_model* modelN, UNCRZ_FBF_anim* animN)
	{
		anim = animN;
		model = modelN;
		for (int i = anim->flows.size() - 1; i >= 0; i--)
		{
			states.push_back(UNCRZ_FBF_anim_flow_state(&anim->flows[i].start));
		}
	}
};

const DWORD AM_none = 0x00000000;
const DWORD AM_nice = 0x00000001;
const DWORD AM_add = 0x00000002;

const DWORD LM_none = 0x00000000;
const DWORD LM_full = 0x00000001;

const DWORD SD_none = 0x00000000; // don't use this
const DWORD SD_colour = 0x00000001;
const DWORD SD_depth = 0x00000002;
const DWORD SD_default = 0x00000003;

// this is for internal use only
const DWORD SD_itrl_sidewise = 0x00000004; // see drawSideWise

struct UNCRZ_sprite_data
{
public:
	D3DXVECTOR4 pos;
	D3DXVECTOR4 other;

	UNCRZ_sprite_data(D3DXVECTOR4 posN, D3DXVECTOR4 otherN)
	{
		pos = posN;
		other = otherN;
	}

	UNCRZ_sprite_data(float x, float y, float z, float w, float ox, float oy, float oz, float ow)
	{
		pos = D3DXVECTOR4(x, y, z, w);
		other = D3DXVECTOR4(ox, oy, oz, ow);
	}
};

struct UNCRZ_sprite_buffer
{
public:
	int size;

	void* vertexArray; // array of vertices
	short* indexArray; // array of indicies

	LPDIRECT3DVERTEXBUFFER9 vBuff;
	LPDIRECT3DVERTEXDECLARATION9 vertexDec; // vertexDecPCT (always)
	LPDIRECT3DINDEXBUFFER9 iBuff;
	DWORD vertexType;
	UINT stride;
	int numVertices;
	int numIndices;

	UNCRZ_sprite_buffer()
	{
	}

	void create(LPDIRECT3DDEVICE9 dxDevice, LPDIRECT3DVERTEXDECLARATION9 vertexDecN, int sizeN)
	{
		size = sizeN;
		stride = sizeof(vertexPCT);
		vertexType = VX_PCT;
		vertexDec = vertexDecN;
	
		std::vector<vertexPCT> vPCTs;
		std::vector<short> indicies;

		int ci = 0;
		for (int i = 0; i < size; i++)
		{
			int tti = i * sizeof(UNCRZ_sprite_data) / (sizeof(float) * 4);
			vPCTs.push_back(vertexPCT(vertexPC(-1, -1, 1, 1, 1, 1, tti), 0, 0));
			vPCTs.push_back(vertexPCT(vertexPC(1, -1, 1, 1, 1, 1, tti), 1, 0));
			vPCTs.push_back(vertexPCT(vertexPC(1, 1, 1, 1, 1, 1, tti), 1, 1));
			vPCTs.push_back(vertexPCT(vertexPC(-1, 1, 1, 1, 1, 1, tti), 0, 1));

			indicies.push_back((short)ci + 0);
			indicies.push_back((short)ci + 1);
			indicies.push_back((short)ci + 2);

			indicies.push_back((short)ci + 0);
			indicies.push_back((short)ci + 2);
			indicies.push_back((short)ci + 3);

			ci += 4;
		}

		numVertices = (int)vPCTs.size();
		createVBuff(dxDevice, (void*)&vPCTs.front());
		numIndices = (int)indicies.size();
		createIBuff(dxDevice, (short*)&indicies.front());
	}

	// lineqs
	void createLinesq(LPDIRECT3DDEVICE9 dxDevice, LPDIRECT3DVERTEXDECLARATION9 vertexDecN, int sizeN)
	{
		size = sizeN + 3;
		stride = sizeof(vertexPCT);
		vertexType = VX_PCT;
		vertexDec = vertexDecN;
	
		std::vector<vertexPCT> vPCTs;
		std::vector<short> indicies;

		// need space for 2 things at either end
		int ci = 0;
		for (int i = 1; i < size - 2; i++)
		{
			int tti = i * sizeof(UNCRZ_sprite_data) / (sizeof(float) * 4);
			vPCTs.push_back(vertexPCT(vertexPC(-1, -1, 1, 1, 1, 1, tti), 0, 0));
			vPCTs.push_back(vertexPCT(vertexPC(1, -1, 1, 1, 1, 1, tti), 1, 0));
			vPCTs.push_back(vertexPCT(vertexPC(1, 1, 1, 1, 1, 1, tti + 1), 1, 1));
			vPCTs.push_back(vertexPCT(vertexPC(-1, 1, 1, 1, 1, 1, tti + 1), 0, 1));

			indicies.push_back((short)ci + 0);
			indicies.push_back((short)ci + 1);
			indicies.push_back((short)ci + 2);

			indicies.push_back((short)ci + 0);
			indicies.push_back((short)ci + 2);
			indicies.push_back((short)ci + 3);

			ci += 4;
		}

		numVertices = (int)vPCTs.size();
		createVBuff(dxDevice, (void*)&vPCTs.front());
		numIndices = (int)indicies.size();
		createIBuff(dxDevice, (short*)&indicies.front());
	}

	void fillVBuff()
	{
		VOID* buffPtr;
		vBuff->Lock(0, numVertices * stride, (VOID**)&buffPtr, D3DLOCK_DISCARD);
		memcpy(buffPtr, vertexArray, numVertices * stride);
		vBuff->Unlock();
	}

	void createVBuff(LPDIRECT3DDEVICE9 dxDevice, void* vds)
	{
		dxDevice->CreateVertexBuffer(numVertices * stride, D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &vBuff, NULL);

		vertexArray = malloc(stride * numVertices);
		memcpy(vertexArray, vds, numVertices * stride);

		fillVBuff();
	}

	void fillIBuff()
	{
		VOID* buffPtr;
		iBuff->Lock(0, numIndices * sizeof (short), (VOID**)&buffPtr, D3DLOCK_DISCARD);
		memcpy(buffPtr, indexArray, numIndices * sizeof (short));
		iBuff->Unlock();
	}

	void createIBuff(LPDIRECT3DDEVICE9 dxDevice, short* ids)
	{
		dxDevice->CreateIndexBuffer(numIndices * sizeof (short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &iBuff, NULL);

		indexArray = (short*)malloc(sizeof (short) * numIndices);
		memcpy(indexArray, ids, numIndices * sizeof (short));

		fillIBuff();
	}
};

// sprite re-implements most of UNCRZ_section
struct UNCRZ_sprite
{
public:
	std::string name;

	UNCRZ_effect effect;

	bool useTex;
	LPDIRECT3DTEXTURE9 tex;
	bool useTex0;
	LPDIRECT3DTEXTURE9 tex0;
	bool useTex1;
	LPDIRECT3DTEXTURE9 tex1;
	bool useTex2;
	LPDIRECT3DTEXTURE9 tex2;
	bool useTex3;
	LPDIRECT3DTEXTURE9 tex3;
	D3DXHANDLE tech;
	D3DXHANDLE lightTech;
	D3DXHANDLE overTech;
	D3DXVECTOR4 colMod;
	DWORD alphaMode;
	DWORD sideWiseAlphaMode;
	DWORD lightingMode;

	float dimX, dimY, dimZ;

	UNCRZ_matrix* mats[maxMats];

	void zeroIsh();
	void setAlpha(LPDIRECT3DDEVICE9);
	void setTextures();
	void setMats();
	void setSideWiseAlpha(LPDIRECT3DDEVICE9);
	void draw(LPDIRECT3DDEVICE9, drawData*, UNCRZ_sprite_buffer*, UNCRZ_sprite_data*, int, int, DWORD, DWORD);
	void drawSideWise(LPDIRECT3DDEVICE9, drawData*, UNCRZ_sprite_buffer*, UNCRZ_sprite_data*, int, int, DWORD, DWORD);
	void drawSideOver(LPDIRECT3DDEVICE9, drawData*);
	void drawToSide(LPDIRECT3DDEVICE9, drawData*, UNCRZ_sprite_buffer*, UNCRZ_sprite_data*, int, int, DWORD, DWORD);

	UNCRZ_sprite();
	UNCRZ_sprite(std::string);
};

struct UNCRZ_section
{
public:
	std::string name;

	UNCRZ_effect effect;
	bool useTex;
	LPDIRECT3DTEXTURE9 tex;
	bool useTex0;
	LPDIRECT3DTEXTURE9 tex0;
	bool useTex1;
	LPDIRECT3DTEXTURE9 tex1;
	bool useTex2;
	LPDIRECT3DTEXTURE9 tex2;
	bool useTex3;
	LPDIRECT3DTEXTURE9 tex3;
	D3DXVECTOR4 colMod;
	D3DXHANDLE tech;
	D3DXHANDLE lightTech;
	D3DXHANDLE decalTech;
	D3DXHANDLE dynamicDecalTech;
	D3DXHANDLE overTech;
	DWORD vertexType;
	DWORD alphaMode;
	DWORD lightingMode;

	int vOffset; // in Indices
	int vLen; // in Triangles

	int numVertices; // needed for DrawIndexedPrim call

	// decals?
	std::vector<UNCRZ_decal*> decals;
	bool drawDecals;
	bool acceptDecals;
	bool drawDynamicDecals;
	
	bool sectionEnabled; // whether it should draw or not

	// fun fun fun
	UNCRZ_matrix* mats[maxMats];

	// indicates whether the section should not be rendered for the rest of this draw sequence (for multi-draw sequences)
	bool curDrawCull;

	void setAlpha(LPDIRECT3DDEVICE9 dxDevice)
	{ // for drawing
		switch (alphaMode)
		{
		case AM_none:
			dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			break;
		case AM_nice:
			dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			/*dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);*/
			break;
		case AM_add:
			dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE); // formerly D3DBLEND_SRCALPHA
			dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			break;
		}
	}

	void setAlphaForOver(LPDIRECT3DDEVICE9 dxDevice)
	{ // for over
		dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE); // stuff writing to side should be doing it's own bleninding
		dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}

	void setTextures()
	{
		if (useTex)
			effect.setTexture(tex);
		if (useTex0)
			effect.setTexture0(tex0);
		if (useTex1)
			effect.setTexture1(tex1);
		if (useTex2)
			effect.setTexture2(tex2);
		if (useTex3)
			effect.setTexture3(tex3);
	}

	void setMats()
	{
		for (int i = 0; i < maxMats; i++)
		{
			if (mats[i] != NULL)
			{
				effect.setMat(i, mats[i]->mat);
			}
		}
	}

	// doesn't support any alphaMode except AM_none
	void drawMany(LPDIRECT3DDEVICE9 dxDevice, drawData* ddat, UNCRZ_model** arr, int count, int secIndex, DWORD drawArgs)
	{
		HRESULT res;

		ddat->enableClip(dxDevice);

		// model
		if (drawArgs & DF_light)
		{
			effect.setTechnique(lightTech);
			effect.setLightType(ddat->lightType);
			effect.setLightDepth(ddat->lightDepth);
			effect.setLightConeness(ddat->lightConeness);
		}
		else
		{
			effect.setTechnique(tech);
			effect.setLightCoof(ddat->lightCoof);
			effect.setFarDepth(ddat->farDepth);
		}

		setTextures();
		setMats();

		effect.setEyePos(&ddat->eyePos);
		effect.setEyeDir(&ddat->eyeDir);
		effect.setTicker(ddat->ticker);
		effect.setViewProj(ddat->viewProj);

		effect.effect->CommitChanges();

		setAlpha(dxDevice);

		UINT numPasses, pass;
		effect.effect->Begin(&numPasses, 0);

		// pass 0
		if (drawArgs & DF_light)
			effect.effect->BeginPass((int)ddat->lightType);
		else if (ddat->performPlainPass)
			effect.effect->BeginPass(0);
		else
			goto skipPlainPass;

		for (int i = count - 1; i >= 0; i--)
		{
			if (arr[i]->sections[secIndex]->curDrawCull)
				continue;
			if (vertexType == VX_PC || vertexType == VX_PCT)
				effect.setcolMod(&arr[i]->sections[secIndex]->colMod.x);
			effect.setTransArr(arr[i]->transArr);
			effect.effect->CommitChanges();
			res = dxDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, numVertices, vOffset, vLen);
			if (res != D3D_OK)
				res = res;
		}
		effect.effect->EndPass();
skipPlainPass:

		if (lightingMode == LM_full && numPasses > 1 && (drawArgs & DF_light) == false)
		{
			// enable blend
			dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

			// light pass
			for (int ldi = ddat->lightDatas.size() - 1; ldi >= 0; ldi--)
			{
				lightData* ld = ddat->lightDatas[ldi];
				
				if (!ld->lightEnabled)
					continue;
				
				effect.setLightData(ld);
				effect.effect->CommitChanges();

				effect.effect->BeginPass((int)ld->lightType + 1);
				for (int i = count - 1; i >= 0; i--)
				{
					if (arr[i]->sections[secIndex]->curDrawCull || ld->canSkip(&arr[i]->modelBox))
						continue;
					if (vertexType == VX_PC || vertexType == VX_PCT)
						effect.setcolMod(&arr[i]->sections[secIndex]->colMod.x);
					effect.setTransArr(arr[i]->transArr);
					effect.effect->CommitChanges();
					res = dxDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, numVertices, vOffset, vLen);
					if (res != D3D_OK)
						res = res;
				}
				effect.effect->EndPass();
			}

		}
		// disable blend
		dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

		effect.effect->End();

		/*UINT numPasses, pass;
		effect.effect->Begin(&numPasses, 0);

		for (pass = 0; pass < numPasses; pass++)
		{
			effect.effect->BeginPass(pass);

			for (int i = count - 1; i >= 0; i--)
			{
				if (vertexType == VX_PC || vertexType == VX_PCT)
					effect.setcolMod(&arr[i]->sections[secIndex]->colMod.x);
				effect.setTransArr(arr[i]->transArr);
				effect.effect->CommitChanges();
				res = dxDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, numVertices, vOffset, vLen);
				if (res != D3D_OK)
					res = res;
			}

			effect.effect->EndPass();
		}

		effect.effect->End();*/

		if (drawDecals == false || drawArgs & DF_light)
			return; // no decals for light

		// decals
		dxDevice->SetVertexDeclaration(vertexDecPAT4);
		effect.setTechnique(decalTech);

		effect.effect->CommitChanges();

		dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		effect.effect->Begin(&numPasses, 0);

		// pass 0
		if (drawArgs & DF_light)
			effect.effect->BeginPass((int)ddat->lightType);
		else if (ddat->performPlainPass)
			effect.effect->BeginPass(0);
		else
			goto skipPlainDecalPass;

		for (int i = count - 1; i >= 0; i--)
		{
			UNCRZ_section* curSec = arr[i]->sections[secIndex];
			if (curSec->curDrawCull)
				continue;
			if (curSec->decals.size() != 0)
			{
				effect.setcolMod(&arr[i]->sections[secIndex]->colMod.x);
				effect.setTransArr(arr[i]->transArr);

				//for (int i = curSec->decals.size() - 1; i >= 0; i--)
				for (int i = 0; i < curSec->decals.size(); i++)
				{
					effect.setTexture(curSec->decals[i]->tex);
					effect.effect->CommitChanges();
					res = dxDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, curSec->decals[i]->numFaces, curSec->decals[i]->vertexArray, sizeof(vertexPAT4));
					if (res != D3D_OK)
						res = res;
				}
			}
		}
		effect.effect->EndPass();
skipPlainDecalPass:

		if (lightingMode == LM_full && numPasses > 1 && (drawArgs & DF_light) == false)
		{
			// enable blend
			dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

			// light pass
			for (int ldi = ddat->lightDatas.size() - 1; ldi >= 0; ldi--)
			{
				lightData* ld = ddat->lightDatas[ldi];
				
				if (!ld->lightEnabled)
					continue;

				effect.setLightData(ld);
				effect.effect->CommitChanges();

				effect.effect->BeginPass((int)ld->lightType + 1);
				for (int i = count - 1; i >= 0; i--)
				{
					UNCRZ_section* curSec = arr[i]->sections[secIndex];

					if (curSec->decals.size() != 0)
					{
						effect.setcolMod(&arr[i]->sections[secIndex]->colMod.x);
						effect.setTransArr(arr[i]->transArr);

						//for (int i = curSec->decals.size() - 1; i >= 0; i--)
						for (int i = 0; i < curSec->decals.size(); i++)
						{
							effect.setTexture(curSec->decals[i]->tex);
							effect.effect->CommitChanges();
							res = dxDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, curSec->decals[i]->numFaces, curSec->decals[i]->vertexArray, sizeof(vertexPAT4));
							if (res != D3D_OK)
								res = res;
						}
					}
				}
				effect.effect->EndPass();
			}

			// disable blend
			dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		}

		effect.effect->End();

		/*effect.effect->Begin(&numPasses, 0);

		for (pass = 0; pass < numPasses; pass++)
		{
			effect.effect->BeginPass(pass);

			for (int i = count - 1; i >= 0; i--)
			{
				UNCRZ_section* curSec = arr[i]->sections[secIndex];
				if (curSec->decals.size() != 0)
				{
					effect.setcolMod(&arr[i]->sections[secIndex]->colMod.x);
					effect.setTransArr(arr[i]->transArr);

					//for (int i = curSec->decals.size() - 1; i >= 0; i--)
					for (int i = 0; i < curSec->decals.size(); i++)
					{
						effect.setTexture(curSec->decals[i]->tex);
						effect.effect->CommitChanges();
						res = dxDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, curSec->decals[i]->numFaces, curSec->decals[i]->vertexArray, sizeof(vertexPAT4));
						if (res != D3D_OK)
							res = res;
					}
				}
			}

			effect.effect->EndPass();
		}

		effect.effect->End();*/

		dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	}

	void draw(LPDIRECT3DDEVICE9 dxDevice, UNCRZ_trans_arr* transArr, drawData* ddat, DWORD drawArgs)
	{
		effect.setEyePos(&ddat->eyePos);
		effect.setEyeDir(&ddat->eyeDir);
		effect.setTicker(ddat->ticker);

		if (sectionEnabled == false)
			return;

		if (drawArgs & DF_light || alphaMode == AM_none)
		{
			drawDraw(dxDevice, transArr, ddat, drawArgs);
		}
		else
		{
			DWORD oldFillMode;
			dxDevice->GetRenderState(D3DRS_FILLMODE, &oldFillMode);

			drawToSide(dxDevice, transArr, ddat, drawArgs);
			
			dxDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
			drawSideOver(dxDevice, ddat);

			dxDevice->SetRenderState(D3DRS_FILLMODE, oldFillMode);
		}
	}

	void drawSideOver(LPDIRECT3DDEVICE9 dxDevice, drawData* ddat)
	{
		ddat->disableClip(dxDevice);

		D3DXMATRIX idMat;
		D3DXMatrixIdentity(&idMat);
		dxDevice->SetRenderTarget(0, ddat->targetSurface);
		//dxDevice->SetViewport(ddat->targetVp);

		dxDevice->SetRenderState(D3DRS_ZENABLE, false);
		dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);

		vertexOver overVerts[4]; // make this const / VBuff
		overVerts[0] = vertexOver(-1, -1, 0, 0, 1);
		overVerts[1] = vertexOver(-1, 1, 0, 0, 0);
		overVerts[2] = vertexOver(1, -1, 0, 1, 1);
		overVerts[3] = vertexOver(1, 1, 0, 1, 0);

		D3DXVECTOR4 texData = D3DXVECTOR4(0.5 / (float)ddat->targetVp->Width, 0.5 / (float)ddat->targetVp->Height, 1.0 / (float)ddat->targetVp->Width, 1.0 / (float)ddat->targetVp->Height);
		effect.setTextureData((float*)&texData.x);
		
		for (int i = 0; i < 4; i++) // do ahead of shader
		{
			overVerts[i].tu += texData.x;
			overVerts[i].tv += texData.y;
		}

		effect.setTexture(ddat->sideTex);
		effect.setTechnique(overTech);
		effect.setViewProj(&idMat);
		effect.effect->CommitChanges();

		setAlphaForOver(dxDevice);

		UINT numPasses, pass;
		effect.effect->Begin(&numPasses, 0);
		for (int i = 0; i < numPasses; i++)
		{
			effect.effect->BeginPass(i);

			dxDevice->SetVertexDeclaration(vertexDecOver);
			dxDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, overVerts, sizeof(vertexOver));

			effect.effect->EndPass();
		}
		effect.effect->End();
		
		dxDevice->SetRenderState(D3DRS_ZENABLE, true);
		dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	}

	void drawToSide(LPDIRECT3DDEVICE9 dxDevice, UNCRZ_trans_arr* transArr, drawData* ddat, DWORD drawArgs)
	{
		dxDevice->SetRenderTarget(0, ddat->sideSurface);
		dxDevice->SetViewport(ddat->sideVp);
		//dxDevice->SetDepthStencilSurface(ddat->zSideSurface);

		dxDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 0, 0);
		dxDevice->SetRenderState(D3DRS_ZENABLE, true);
		dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);

		effect.setTargetTexture(ddat->targetTex);
		D3DXVECTOR4 targTexData = D3DXVECTOR4(0.5 / (float)ddat->targetVp->Width, 0.5 / (float)ddat->targetVp->Height, 1.0 / (float)ddat->targetVp->Width, 1.0 / (float)ddat->targetVp->Height);
		effect.setTargetTextureData((float*)&targTexData.x);
		
		// targetVP allows the shader to sample the target texture
		float matDat[16] = {
			(float)ddat->targetVp->Width / 2.0, 0.0, 0.0, 0.0,
			0.0, -(float)ddat->targetVp->Height / 2.0, 0.0, 0.0,
			0.0, 0.0, ddat->targetVp->MaxZ - ddat->targetVp->MinZ, 0.0,
			ddat->targetVp->X + (float)ddat->targetVp->Width / 2.0, ddat->targetVp->Y + (float)ddat->targetVp->Height / 2.0, ddat->targetVp->MinZ, 1.0
		};
		//D3DXMATRIX vpMat(&matDat);

		effect.setTargetVPMat((float*)&matDat);

		drawDraw(dxDevice, transArr, ddat, drawArgs);
	}

	void drawDraw(LPDIRECT3DDEVICE9 dxDevice, UNCRZ_trans_arr* transArr, drawData* ddat, DWORD drawArgs)
	{
		HRESULT res;

		ddat->enableClip(dxDevice);

		if (drawArgs & DF_light)
		{
			effect.setTechnique(lightTech);
			effect.setLightType(ddat->lightType);
			effect.setLightDepth(ddat->lightDepth);
			effect.setLightConeness(ddat->lightConeness);
		}
		else
		{
			effect.setTechnique(tech);
			effect.setLightCoof(ddat->lightCoof);
			effect.setFarDepth(ddat->farDepth);
		}

		effect.setTransArr(transArr);
		if (vertexType == VX_PC || vertexType == VX_PCT)
			effect.setcolMod(&colMod.x);
		
		setTextures();
		setMats();

		// disable blend (either side render  or  alphaMode is set to AM_none  or  DF_light)
		dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

		effect.setViewProj(ddat->viewProj);

		effect.effect->CommitChanges();

		UINT numPasses, pass;
		effect.effect->Begin(&numPasses, 0);

		// pass 0
		if (drawArgs & DF_light)
			effect.effect->BeginPass((int)ddat->lightType);
		else if (ddat->performPlainPass || lightingMode == LM_none)
			effect.effect->BeginPass(0);
		else
			goto skipPlainPass;

		res = dxDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, numVertices, vOffset, vLen);
		if (res != D3D_OK)
			res = res;
		effect.effect->EndPass();
skipPlainPass:

		if (lightingMode == LM_full && numPasses > 1 && (drawArgs & DF_light) == false)
		{
			// enable blend
			dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

			// light pass
			for (int ldi = ddat->lightDatas.size() - 1; ldi >= 0; ldi--)
			{
				lightData* ld = ddat->lightDatas[ldi];
				
				if (!ld->lightEnabled || ld->curDrawSkip)
					continue;

				effect.setLightData(ld);
				effect.effect->CommitChanges();

				effect.effect->BeginPass((int)ld->lightType + 1);
				res = dxDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, numVertices, vOffset, vLen);
				if (res != D3D_OK)
					res = res;
				effect.effect->EndPass();
			}

		}
		// disable blend
		dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

		effect.effect->End();

		/*UINT numPasses, pass;
		effect.effect->Begin(&numPasses, 0);

		for (pass = 0; pass < numPasses; pass++)
		{
			effect.effect->BeginPass(pass);
			// draw model
			res = dxDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, numVertices, vOffset, vLen);
			if (res != D3D_OK)
				res = res;
			effect.effect->EndPass();
		}

		effect.effect->End();*/

		if (drawArgs & DF_light)
			return; // no decals for light
		if (decals.size() == 0 || drawDecals == false)
			goto skipToDynamicDecals;

		// decals
		LPDIRECT3DVERTEXDECLARATION9 oldDec;

		dxDevice->GetVertexDeclaration(&oldDec);
		dxDevice->SetVertexDeclaration(vertexDecPAT4);

		effect.setTechnique(decalTech);

		effect.effect->CommitChanges();

		dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		effect.effect->Begin(&numPasses, 0);

		// pass 0
		if (drawArgs & DF_light)
			effect.effect->BeginPass((int)ddat->lightType);
		else if (ddat->performPlainPass || lightingMode == LM_none)
			effect.effect->BeginPass(0);
		else
			goto skipPlainDecalPass;

		for (int i = 0; i < decals.size(); i++)
		{
			effect.setTexture(decals[i]->tex);
			effect.effect->CommitChanges();
			res = dxDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, decals[i]->numFaces, decals[i]->vertexArray, sizeof(vertexPAT4));
			if (res != D3D_OK)
				res = res;
		}
		effect.effect->EndPass();
skipPlainDecalPass:

		if (lightingMode == LM_full && numPasses > 1 && (drawArgs & DF_light) == false)
		{
			// enable blend
			dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

			// light pass
			for (int ldi = ddat->lightDatas.size() - 1; ldi >= 0; ldi--)
			{
				lightData* ld = ddat->lightDatas[ldi];
				
				if (!ld->lightEnabled)
					continue;

				effect.setLightData(ld);
				effect.effect->CommitChanges();

				effect.effect->BeginPass((int)ld->lightType + 1);
				for (int i = 0; i < decals.size(); i++)
				{
					effect.setTexture(decals[i]->tex);
					effect.effect->CommitChanges();
					res = dxDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, decals[i]->numFaces, decals[i]->vertexArray, sizeof(vertexPAT4));
					if (res != D3D_OK)
						res = res;
				}
				effect.effect->EndPass();
			}

		}

		// what the hell are these?
		dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		
		dxDevice->SetVertexDeclaration(oldDec);

		effect.effect->End();

		/*effect.effect->Begin(&numPasses, 0);

		for (pass = 0; pass < numPasses; pass++)
		{
			effect.effect->BeginPass(pass);

			for (int i = 0; i < decals.size(); i++)
			{
				effect.setTexture(decals[i]->tex);
				effect.effect->CommitChanges();
				res = dxDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, decals[i]->numFaces, decals[i]->vertexArray, sizeof(vertexPAT4));
				if (res != D3D_OK)
					res = res;
			}

			effect.effect->EndPass();
		}

		effect.effect->End();*/

		// dynamic decals (DO NOT GET CONFUSED WITH LIGHTS)
skipToDynamicDecals:
		if (ddat->dynamicDecalDatas.size() == 0 || drawDynamicDecals == false)
			return;

		//ddat->startTimer(true);

		// disable z write
		dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);

		effect.setTechnique(dynamicDecalTech);
		
		effect.effect->Begin(&numPasses, 0);

		if (numPasses > 0 && (drawArgs & DF_light) == false)
		{
			// enable blend
			dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

			// dynamic decal pass
			for (int dddi = ddat->dynamicDecalDatas.size() - 1; dddi >= 0; dddi--)
			{
				dynamicDecalData* ddd = ddat->dynamicDecalDatas[dddi];
				
				if (!ddd->decalEnabled)
					continue;

				effect.setDynamicDecalData(ddd);
				effect.effect->CommitChanges();

				effect.effect->BeginPass((int)ddd->lightType);
				res = dxDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, numVertices, vOffset, vLen);
				if (res != D3D_OK)
					res = res;
				effect.effect->EndPass();
			}

		}
		// disable blend
		dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

		// re-enable z write
		dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
		
		//ddat->stopTimer(1, true, "DynDecals: ", GDOF_dms);

		effect.effect->End();
	}

	void zeroIsh()
	{
		useTex = false;
		useTex0 = false;
		useTex1 = false;
		useTex2 = false;
		useTex3 = false;

		for (int i = 0; i < maxMats; i++)
		{
			mats[i] = NULL;
		}
	}

	UNCRZ_section()
	{
		zeroIsh();
	}
	
	UNCRZ_section(UNCRZ_section* gin)
	{
		zeroIsh();
		name = gin->name;
		tech = gin->tech;
		lightTech = gin->lightTech;
		decalTech = gin->decalTech;
		dynamicDecalTech = gin->dynamicDecalTech;
		overTech = gin->overTech;
		lightingMode = gin->lightingMode;
		alphaMode = gin->alphaMode;
		colMod = gin->colMod;
		effect = gin->effect;
		vOffset = gin->vOffset;
		vLen = gin->vLen;
		numVertices = gin->numVertices;
		vertexType = gin->vertexType;
		drawDecals = gin->drawDecals;
		drawDynamicDecals = gin->drawDynamicDecals;
		acceptDecals = gin->acceptDecals;
		sectionEnabled = gin->sectionEnabled;
		useTex = gin->useTex;
		tex = gin->tex;
		useTex0 = gin->useTex0;
		tex0 = gin->tex0;
		useTex1 = gin->useTex1;
		tex1 = gin->tex1;
		useTex2 = gin->useTex2;
		tex2 = gin->tex2;
		useTex3 = gin->useTex3;
		tex3 = gin->tex3;
	}

	UNCRZ_section(std::string nameN)
	{
		zeroIsh();
		name = nameN;
	}

	void release()
	{
		effect.release();
		tex->Release();
	}
};

struct UNCRZ_blend
{
public:
	std::string name;

	UNCRZ_segment* parent;

	D3DXVECTOR3 offset;
	D3DXVECTOR3 rotation;
	D3DXMATRIX offsetMatrix; // call updateMatrices whenever the offset are modified
	D3DXMATRIX rotationMatrix; // call updateMatrices whenever the offset/rotation are modified
	D3DXMATRIX offsetMatrixinv; // call updateMatrices whenever the offse are modified
	D3DXMATRIX rotationMatrixinv; // call updateMatrices whenever the offset/rotation are modified
	D3DXMATRIX localTrans; // call updateMatrices whenever the offset/rotation are modified
	D3DXMATRIX localTransinv; // call updateMatrices whenever the offset/rotation are modified

	int transIndex;
	float prop;

	UNCRZ_blend(std::string nameN, int tti, float propN, UNCRZ_segment* parentN)
	{
		name = nameN;
		transIndex = tti;
		prop = propN;
		parent = parentN;
	}

	void update(D3DXMATRIX trans, UNCRZ_trans_arr* transArr)
	{
		updateMatrices();

		D3DXMatrixMultiply(&trans, &offsetMatrix, &trans);
		D3DXMatrixMultiply(&trans, &rotationMatrix, &trans);

		transArr->setValue(transIndex, &trans);
	}

	void updateMatrices()
	{
		D3DXMatrixTranslation(&offsetMatrix, offset.x, offset.y, offset.z);
		D3DXMatrixRotationYawPitchRoll(&rotationMatrix, rotation.y, rotation.x, rotation.z);
		D3DXMatrixMultiply(&localTrans, &offsetMatrix, &rotationMatrix);
		
		D3DXMatrixInverse(&offsetMatrixinv, NULL, &offsetMatrix);
		D3DXMatrixInverse(&rotationMatrixinv, NULL, &rotationMatrix);
		D3DXMatrixInverse(&localTransinv, NULL, &localTrans);
	}
};

struct UNCRZ_segment
{
public:
	std::string name;

	D3DXVECTOR3 offset;
	D3DXVECTOR3 rotation;
	D3DXMATRIX offsetMatrix; // call updateMatrices whenever the offset are modified
	D3DXMATRIX rotationMatrix; // call updateMatrices whenever the offset/rotation are modified
	D3DXMATRIX offsetMatrixinv; // call updateMatrices whenever the offse are modified
	D3DXMATRIX rotationMatrixinv; // call updateMatrices whenever the offset/rotation are modified
	D3DXMATRIX localTrans; // call updateMatrices whenever the offset/rotation are modified
	D3DXMATRIX localTransinv; // call updateMatrices whenever the offset/rotation are modified
	
	UNCRZ_segment* parent;
	UNCRZ_model* model;
	std::vector<UNCRZ_segment*> segments;
	std::vector<UNCRZ_blend*> blends;

	UNCRZ_bbox segBox; // bounding box for vertices of segment

	int transIndex;

	bool requiresUpdate;

	void addBlend(std::string name, int tti, float prop)
	{
		blends.push_back(new UNCRZ_blend(name, tti, prop, this));
	}

	// need to transform
	void update(LPD3DXMATRIX trans, UNCRZ_trans_arr* transArr)
	{
		if (requiresUpdate)
		{
			for each (UNCRZ_blend* b in blends)
			{
				b->offset = offset * b->prop;
				b->rotation = rotation * b->prop;
				b->update(*trans, transArr);
			}

			updateMatrices();

			D3DXMatrixMultiply(trans, &offsetMatrix, trans);
			D3DXMatrixMultiply(trans, &rotationMatrix, trans);

			transArr->setValue(transIndex, trans);

			int len = segments.size();
			for (int i = len - 1; i >= 0; i--)
			{
				segments[i]->requiresUpdate = true;
				segments[i]->update(trans, transArr);
			}

			requiresUpdate = false;
		}
		else
		{
			D3DXMatrixMultiply(trans, &offsetMatrix, trans);
			D3DXMatrixMultiply(trans, &rotationMatrix, trans);

			int len = segments.size();
			for (int i = len - 1; i >= 0; i--)
			{
				segments[i]->update(trans, transArr);
			}
		}

		D3DXMatrixMultiply(trans, &rotationMatrixinv, trans);
		D3DXMatrixMultiply(trans, &offsetMatrixinv, trans);
	}

	UNCRZ_segment()
	{
		requiresUpdate = true;
	}

	UNCRZ_segment(UNCRZ_segment* gin, UNCRZ_segment* parentN, UNCRZ_model* modelN, std::vector<UNCRZ_segment*>* allSegs)
	{
		requiresUpdate = true;

		name = gin->name;
		for each(UNCRZ_segment* s in gin->segments)
		{
			segments.push_back(new UNCRZ_segment(s, this, modelN, allSegs));
		}
		parent = parentN;
		model = modelN;

		offset = gin->offset;
		rotation = gin->rotation;
		segBox = gin->segBox;

		transIndex = gin->transIndex;

		for (int i = allSegs->size() - 1; i >= 0; i--)
		{
			if (allSegs->at(i) == gin)
				allSegs->at(i) = this;
		}

		for each (UNCRZ_blend* b in gin->blends)
		{
			addBlend(b->name, b->transIndex, b->prop);
		}
	}

	UNCRZ_segment(std::string nameN)
	{
		requiresUpdate = true;
		name = nameN;
	}

	void updateMatrices()
	{
		D3DXMatrixTranslation(&offsetMatrix, offset.x, offset.y, offset.z);
		D3DXMatrixRotationYawPitchRoll(&rotationMatrix, rotation.y, rotation.x, rotation.z);
		D3DXMatrixMultiply(&localTrans, &offsetMatrix, &rotationMatrix);
		
		D3DXMatrixInverse(&offsetMatrixinv, NULL, &offsetMatrix);
		D3DXMatrixInverse(&rotationMatrixinv, NULL, &rotationMatrix);
		D3DXMatrixInverse(&localTransinv, NULL, &localTrans);
	}

	void createSegBox()
	{
		if (model->vertexType == VX_PC)
		{
			createSegBoxVX_PC();
		}
		else if (model->vertexType == VX_PCT)
		{
			createSegBoxVX_PCT();
		}
	}

	void createSegBoxVX_PC()
	{
		segBox = UNCRZ_bbox();
		vertexPC curVX_PC;
		D3DXVECTOR3 curVec;
		for (int i = model->numVertices - 1; i >= 0; i--)
		{
			curVX_PC = ((vertexPC*)model->vertexArray)[i];

			if (curVX_PC.tti == transIndex)
			{
				curVec.x = curVX_PC.x;
				curVec.y = curVX_PC.y;
				curVec.z = curVX_PC.z;
				segBox.include(curVec);
			}
		}
		segBox.fillVectors();
	}

	void createSegBoxVX_PCT()
	{
		segBox = UNCRZ_bbox();
		vertexPCT curVX_PCT;
		D3DXVECTOR3 curVec;
		for (int i = model->numVertices - 1; i >= 0; i--)
		{
			curVX_PCT = ((vertexPCT*)model->vertexArray)[i];

			if (curVX_PCT.tti == transIndex)
			{
				curVec.x = curVX_PCT.x;
				curVec.y = curVX_PCT.y;
				curVec.z = curVX_PCT.z;
				segBox.include(curVec);
			}
		}
		segBox.fillVectors();
	}

	void release()
	{
		int len = segments.size();
		for (int i = len - 1; i >= 0; i--)
		{
			segments[i]->release();
		}
	}
};

struct UNCRZ_FBF_anim_inst;

// uncrz_sprite (don't ask why it's here... it just is)
UNCRZ_sprite::UNCRZ_sprite()
	{
		zeroIsh();
	}

UNCRZ_sprite::UNCRZ_sprite(std::string nameN)
	{
		zeroIsh();
		name = nameN;
	}

void UNCRZ_sprite::zeroIsh()
	{
		useTex = false;
		useTex0 = false;
		useTex1 = false;
		useTex2 = false;
		useTex3 = false;

		for (int i = 0; i < maxMats; i++)
		{
			mats[i] = NULL;
		}
	}

void UNCRZ_sprite::setAlpha(LPDIRECT3DDEVICE9 dxDevice)
	{
		switch (alphaMode)
		{
		case AM_none:
			dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			break;
		case AM_nice:
			dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			break;
		case AM_add:
			dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE); // formerly D3DBLEND_SRCALPHA
			dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			break;
		}
	}

void UNCRZ_sprite::setTextures()
	{
		if (useTex)
			effect.setTexture(tex);
		if (useTex0)
			effect.setTexture0(tex0);
		if (useTex1)
			effect.setTexture1(tex1);
		if (useTex2)
			effect.setTexture2(tex2);
		if (useTex3)
			effect.setTexture3(tex3);
	}

void UNCRZ_sprite::setMats()
	{
		for (int i = 0; i < maxMats; i++)
		{
			if (mats[i] != NULL)
			{
				effect.setMat(i, mats[i]->mat);
			}
		}
	}

void UNCRZ_sprite::setSideWiseAlpha(LPDIRECT3DDEVICE9 dxDevice)
	{
		switch (sideWiseAlphaMode)
		{
		case AM_none:
			dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			break;
		case AM_nice:
			dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			break;
		case AM_add:
			dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE); // formerly D3DBLEND_SRCALPHA
			dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			break;
		}
	}

void UNCRZ_sprite::draw(LPDIRECT3DDEVICE9 dxDevice, drawData* ddat, UNCRZ_sprite_buffer* sbuff, UNCRZ_sprite_data* larr, int offset, int count, DWORD drawArgs, DWORD spriteDrawArgs)
	{
		HRESULT res;

		ddat->enableClip(dxDevice);

		if (spriteDrawArgs & SD_depth)
			dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		else
			dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

		if (spriteDrawArgs & SD_colour)
		{
			if (spriteDrawArgs & SD_itrl_sidewise)
				setSideWiseAlpha(dxDevice);
			else
				setAlpha(dxDevice);
		}
		else
		{
			// colour OFF (ish)
			dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
			dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		}

		dxDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		dxDevice->SetVertexDeclaration(sbuff->vertexDec);
		dxDevice->SetStreamSource(0, sbuff->vBuff, 0, sbuff->stride);
		dxDevice->SetIndices(sbuff->iBuff);

		if (drawArgs & DF_light)
		{
			effect.setTechnique(lightTech);
			effect.setLightType(ddat->lightType);
			effect.setLightDepth(ddat->lightDepth);
			effect.setLightConeness(ddat->lightConeness);
		}
		else
		{
			effect.setTechnique(tech);
			effect.setLightCoof(ddat->lightCoof);
			effect.setFarDepth(ddat->farDepth);
		}

		D3DXVECTOR4 dim(dimX, dimY, dimZ, 0);
		effect.setSpriteDim(&dim);

		effect.setEyePos(&ddat->eyePos);
		effect.setEyeDir(&ddat->eyeDir);
		effect.setTicker(ddat->ticker);

		setTextures();
		setMats();

		effect.setViewMat(ddat->viewMat);
		effect.setProjMat(ddat->projMat);
		effect.setViewProj(ddat->viewProj);
		effect.setcolMod(colMod);
		
		effect.effect->CommitChanges();

		UINT numPasses, pass;
		effect.effect->Begin(&numPasses, 0);

		// pass 0
		if (drawArgs & DF_light)
			effect.effect->BeginPass((int)ddat->lightType);
		else if (ddat->performPlainPass)
			effect.effect->BeginPass(0);
		else
			goto skipPlainPass;

		for (int i = 0; i < count; i += sbuff->size)
		{
			int ccount = count - i; // ccount is min(sbuff->size, count - i)
			if (ccount > sbuff->size)
				ccount = sbuff->size;

			effect.setSpriteLoc((float*)larr, i, ccount, sizeof(UNCRZ_sprite_data) / sizeof(float));
			effect.effect->CommitChanges();
			
			res = dxDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, ccount * 4, 0, ccount * 2);
		}
		effect.effect->EndPass();
skipPlainPass:

		if (lightingMode == LM_full && numPasses > 1 && (drawArgs & DF_light) == false)
		{
			// enable blend
			dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

			// light pass
			for (int ldi = ddat->lightDatas.size() - 1; ldi >= 0; ldi--)
			{
				lightData* ld = ddat->lightDatas[ldi];
				
				if (!ld->lightEnabled)
					continue;
				
				effect.setLightData(ld);

				effect.effect->BeginPass((int)ld->lightType + 1);

				for (int i = offset; i < count; i += sbuff->size)
				{
					int ccount = count - i; // ccount is min(sbuff->size, count - i)
					if (ccount > sbuff->size)
						ccount = sbuff->size;

					effect.setSpriteLoc((float*)larr, i, ccount, sizeof(UNCRZ_sprite_data) / sizeof(float));
					effect.effect->CommitChanges();
					
					res = dxDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, ccount * 4, 0, ccount * 2);
				}

				effect.effect->EndPass();
			}

		}
		// disable blend
		dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

		effect.effect->End();

		/*UINT numPasses, pass;
		effect.effect->Begin(&numPasses, 0);

		for (pass = 0; pass < numPasses; pass++)
		{
			effect.effect->BeginPass(pass);

			for (int i = count - 1; i >= 0; i--)
			{
				if (vertexType == VX_PC || vertexType == VX_PCT)
					effect.setcolMod(&arr[i]->sections[secIndex]->colMod.x);
				effect.setTransArr(arr[i]->transArr);
				effect.effect->CommitChanges();
				res = dxDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, numVertices, vOffset, vLen);
				if (res != D3D_OK)
					res = res;
			}

			effect.effect->EndPass();
		}

		effect.effect->End();*/
	}



void UNCRZ_sprite::drawSideWise(LPDIRECT3DDEVICE9 dxDevice, drawData* ddat, UNCRZ_sprite_buffer* sbuff, UNCRZ_sprite_data* larr, int offset, int count, DWORD drawArgs, DWORD spriteDrawArgs)
	{
		effect.setEyePos(&ddat->eyePos);
		effect.setEyeDir(&ddat->eyeDir);
		effect.setTicker(ddat->ticker);

		if (drawArgs & DF_light || alphaMode == AM_none)
		{
			draw(dxDevice, ddat, sbuff, larr, offset, count, drawArgs, spriteDrawArgs);
		}
		else
		{
			drawToSide(dxDevice, ddat, sbuff, larr, offset, count, drawArgs, spriteDrawArgs | SD_itrl_sidewise);
			drawSideOver(dxDevice, ddat);
		}
	}

void UNCRZ_sprite::drawSideOver(LPDIRECT3DDEVICE9 dxDevice, drawData* ddat)
	{
		ddat->disableClip(dxDevice);

		D3DXMATRIX idMat;
		D3DXMatrixIdentity(&idMat);
		dxDevice->SetRenderTarget(0, ddat->targetSurface);
		//dxDevice->SetViewport(ddat->targetVp);

		dxDevice->SetRenderState(D3DRS_ZENABLE, false);
		dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);

		vertexOver overVerts[4]; // make this const / VBuff
		overVerts[0] = vertexOver(-1, -1, 0, 0, 1);
		overVerts[1] = vertexOver(-1, 1, 0, 0, 0);
		overVerts[2] = vertexOver(1, -1, 0, 1, 1);
		overVerts[3] = vertexOver(1, 1, 0, 1, 0);

		D3DXVECTOR4 texData = D3DXVECTOR4(0.5 / (float)ddat->targetVp->Width, 0.5 / (float)ddat->targetVp->Height, 1.0 / (float)ddat->targetVp->Width, 1.0 / (float)ddat->targetVp->Height);
		effect.setTextureData((float*)&texData.x);
		
		for (int i = 0; i < 4; i++) // do ahead of shader
		{
			overVerts[i].tu += texData.x;
			overVerts[i].tv += texData.y;
		}

		effect.setTexture(ddat->sideTex);
		effect.setTechnique(overTech);
		effect.setViewProj(&idMat);
		effect.effect->CommitChanges();

		setAlpha(dxDevice);

		UINT numPasses, pass;
		effect.effect->Begin(&numPasses, 0);
		for (int i = 0; i < numPasses; i++)
		{
			effect.effect->BeginPass(i);

			dxDevice->SetVertexDeclaration(vertexDecOver);
			dxDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, overVerts, sizeof(vertexOver));

			effect.effect->EndPass();
		}
		effect.effect->End();
		
		dxDevice->SetRenderState(D3DRS_ZENABLE, true);
		dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	}

void UNCRZ_sprite::drawToSide(LPDIRECT3DDEVICE9 dxDevice, drawData* ddat, UNCRZ_sprite_buffer* sbuff, UNCRZ_sprite_data* larr, int offset, int count, DWORD drawArgs, DWORD spriteDrawArgs)
	{
		ddat->enableClip(dxDevice);

		dxDevice->SetRenderTarget(0, ddat->sideSurface);
		dxDevice->SetViewport(ddat->sideVp);
		//dxDevice->SetDepthStencilSurface(ddat->zSideSurface);

		dxDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 0, 0);
		dxDevice->SetRenderState(D3DRS_ZENABLE, true);
		dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);

		effect.setTargetTexture(ddat->targetTex);
		D3DXVECTOR4 targTexData = D3DXVECTOR4(0.5 / (float)ddat->targetVp->Width, 0.5 / (float)ddat->targetVp->Height, 1.0 / (float)ddat->targetVp->Width, 1.0 / (float)ddat->targetVp->Height);
		effect.setTargetTextureData((float*)&targTexData.x);
		
		float matDat[16] = {
			(float)ddat->targetVp->Width / 2.0, 0.0, 0.0, 0.0,
			0.0, -(float)ddat->targetVp->Height / 2.0, 0.0, 0.0,
			0.0, 0.0, ddat->targetVp->MaxZ - ddat->targetVp->MinZ, 0.0,
			ddat->targetVp->X + (float)ddat->targetVp->Width / 2.0, ddat->targetVp->Y + (float)ddat->targetVp->Height / 2.0, ddat->targetVp->MinZ, 1.0
		};
		//D3DXMATRIX vpMat(&matDat);

		effect.setTargetVPMat((float*)&matDat);

		draw(dxDevice, ddat, sbuff, larr, offset, count, drawArgs, spriteDrawArgs);
	}

// uncrz_model
void UNCRZ_model::allSegsRequireUpdate()
	{
		int len = allSegs.size();
		for (int i = len - 1; i >= 0; i--)
		{
			allSegs[i]->requiresUpdate = true;
		}
	}

void UNCRZ_model::update(LPD3DXMATRIX trans, bool forceUpdate)
{
	int len = segments.size();
	for (int i = len - 1; i >= 0; i--)
	{
		if (forceUpdate)
			segments[i]->requiresUpdate = true;
		segments[i]->update(trans, transArr);
	}

	// assemble bbox
	modelBox = UNCRZ_bbox();

	len = allSegs.size();
	for (int i = len - 1; i >= 0; i--)
	{
		modelBox.include(&(allSegs[i]->segBox), transArr->getValue(allSegs[i]->transIndex));
	}

	modelBox.fillVectors();
}

void UNCRZ_model::drawBBoxDebug(LPDIRECT3DDEVICE9 dxDevice, drawData* ddat)
{
	vertexPC vPCs[36];
	D3DXVECTOR3 curVec;

	for (int i = 0; i < 36; i += 6)
	{
		curVec = modelBox.vecArr[bboxIndices[i]];
		vPCs[i] = vertexPC(curVec.x, curVec.y, curVec.z, 1.0, 0.0, 0.0, -1);

		curVec = modelBox.vecArr[bboxIndices[i + 1]];
		vPCs[i + 1] = vertexPC(curVec.x, curVec.y, curVec.z, 1.0, 1.0, 0.0, -1);

		curVec = modelBox.vecArr[bboxIndices[i + 2]];
		vPCs[i + 2] = vertexPC(curVec.x, curVec.y, curVec.z, 0.0, 1.0, 0.0, -1);

		curVec = modelBox.vecArr[bboxIndices[i + 3]];
		vPCs[i + 3] = vertexPC(curVec.x, curVec.y, curVec.z, 1.0, 0.0, 0.0, -1);

		curVec = modelBox.vecArr[bboxIndices[i + 4]];
		vPCs[i + 4] = vertexPC(curVec.x, curVec.y, curVec.z, 0.0, 1.0, 0.0, -1);

		curVec = modelBox.vecArr[bboxIndices[i + 5]];
		vPCs[i + 5] = vertexPC(curVec.x, curVec.y, curVec.z, 0.0, 0.0, 1.0, -1);
	}

	UNCRZ_effect effect = sections[0]->effect;

	effect.setcolMod(sections[0]->colMod);
	effect.effect->SetTechnique(effect.effect->GetTechniqueByName("simple"));
	effect.setViewProj(ddat->viewProj);
	effect.effect->CommitChanges();
	dxDevice->SetVertexDeclaration(vertexDecPC);

	UINT numPasses, pass;
	effect.effect->Begin(&numPasses, 0);

	HRESULT res;

	for (pass = 0; pass < numPasses; pass++)
	{
		effect.effect->BeginPass(pass);
		res = dxDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 12, &vPCs, sizeof(vertexPC));
		if (res != D3D_OK)
			res = res;
		effect.effect->EndPass();
	}

	effect.effect->End();
}

void UNCRZ_model::drawMany(LPDIRECT3DDEVICE9 dxDevice, drawData* ddat, UNCRZ_model** arr, int count, DWORD drawArgs)
{
	bool res; // true means it will be culled
	for (int i = count - 1; i >= 0; i--)
	{
		res = true;
		if (noCull || arr[i]->modelBox.dothSurviveClipTransformed(ddat->viewProj))
		{
			res = false;
			goto set;
		}
set:
		if (res == true)
			ddat->cullCount++;
		else
			ddat->drawCount++;
		for (int j = sections.size() - 1; j >= 0; j--)
		{
			arr[i]->sections[j]->curDrawCull = res;
		}
	}

	dxDevice->SetVertexDeclaration(vertexDec);
	dxDevice->SetStreamSource(0, vBuff, 0, stride);
	dxDevice->SetIndices(iBuff);

	int len = sections.size();
	for (int i = len - 1; i >= 0; i--)
	{
		sections[i]->drawMany(dxDevice, ddat, arr, count, i, drawArgs);
	}
}

void UNCRZ_model::draw(LPDIRECT3DDEVICE9 dxDevice, drawData* ddat, DWORD drawArgs)
{
	if (noCull || modelBox.dothSurviveClipTransformed(ddat->viewProj))
		goto notOcced;
	
	ddat->cullCount++;
	return;
notOcced:
	ddat->drawCount++;

	//dxDevice->SetVertexDeclaration(vertexDec);
	//dxDevice->SetStreamSource(0, vBuff, 0, stride);
	//dxDevice->SetIndices(iBuff);

	for (int i = ddat->lightDatas.size() - 1; i >= 0; i--)
	{
		ddat->lightDatas[i]->curDrawSkip = ddat->lightDatas[i]->canSkip(&modelBox);
	}

	int len = sections.size();
	for (int i = len - 1; i >= 0; i--)
	{
		dxDevice->SetVertexDeclaration(vertexDec);
		dxDevice->SetStreamSource(0, vBuff, 0, stride);
		dxDevice->SetIndices(iBuff);

		sections[i]->draw(dxDevice, transArr, ddat, drawArgs);
	}
}

UNCRZ_model::UNCRZ_model()
{
	animInst = NULL;
}

UNCRZ_model::UNCRZ_model(UNCRZ_model* gin)
{
	name = gin->name;

	for each(UNCRZ_segment* s in gin->allSegs)
	{
		allSegs.push_back(s);
	}
	for each(UNCRZ_segment* s in gin->segments)
	{
		segments.push_back(new UNCRZ_segment(s, s->parent, this, &allSegs));
	}
	for each(UNCRZ_section* ss in gin->sections)
	{
		sections.push_back(new UNCRZ_section(ss));
	}

	vBuff = gin->vBuff;
	iBuff = gin->iBuff;
	vertexArray = gin->vertexArray;
	indexArray = gin->indexArray;
	stride = gin->stride;
	vertexDec = gin->vertexDec;
	vertexType = gin->vertexType;
	numVertices = gin->numVertices;
	numIndices = gin->numIndices;
	noCull = gin->noCull;

	transArr = new UNCRZ_trans_arr();
	transArr->create((int)allSegs.size() * 2);
	
	animInst = new UNCRZ_FBF_anim_inst(this, gin->animInst->anim);
}

UNCRZ_model::UNCRZ_model(std::string nameN)
{
	name = nameN;
	animInst = NULL;
	noCull = false;
}

void UNCRZ_model::createSegmentBoxes()
{
	int len = allSegs.size();
	for (int i = len - 1; i >= 0; i--)
	{
		allSegs[i]->createSegBox();
	}
}

void UNCRZ_model::changeAnim(UNCRZ_FBF_anim* newAnim)
{
	if (animInst == NULL)
		animInst = new UNCRZ_FBF_anim_inst(this, newAnim);
	else
		animInst->reset(newAnim);
}

void UNCRZ_model::release()
{
	vBuff->Release();
	iBuff->Release();
	delete[stride * numVertices] vertexArray;
	delete[sizeof(short) * numIndices] indexArray;
	int len = segments.size();
	for (int i = len - 1; i >= 0; i--)
	{
		segments[i]->release();
	}
}

void UNCRZ_model::clearDecals()
{
	for (int i = sections.size() - 1; i >= 0; i--)
	{
		sections[i]->decals.clear();
	}
}

void UNCRZ_model::fillVBuff()
{
	VOID* buffPtr;
	vBuff->Lock(0, numVertices * stride, (VOID**)&buffPtr, D3DLOCK_DISCARD);
	memcpy(buffPtr, vertexArray, numVertices * stride);
	vBuff->Unlock();
}

void UNCRZ_model::createVBuff(LPDIRECT3DDEVICE9 dxDevice, void* vds)
{
	dxDevice->CreateVertexBuffer(numVertices * stride, D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &vBuff, NULL);

	vertexArray = malloc(stride * numVertices);
	memcpy(vertexArray, vds, numVertices * stride);

	fillVBuff();
}

void UNCRZ_model::fillIBuff()
{
	VOID* buffPtr;
	iBuff->Lock(0, numIndices * sizeof (short), (VOID**)&buffPtr, D3DLOCK_DISCARD);
	memcpy(buffPtr, indexArray, numIndices * sizeof (short));
	iBuff->Unlock();
}

void UNCRZ_model::createIBuff(LPDIRECT3DDEVICE9 dxDevice, short* ids)
{
	dxDevice->CreateIndexBuffer(numIndices * sizeof (short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &iBuff, NULL);

	indexArray = (short*)malloc(sizeof (short) * numIndices);
	memcpy(indexArray, ids, numIndices * sizeof (short));

	fillIBuff();
}

bool UNCRZ_model::collides(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes)
{
	// bbox collision
	if (modelBox.collides(rayPos, rayDir) == false)
		return false;

	// vertex collision
	if (vertexType == VX_PC)
	{
		return collidesVX_PC(rayPos, rayDir, distRes);
	}
	else if (vertexType == VX_PCT)
	{
		return collidesVX_PCT(rayPos, rayDir, distRes);
	}
	// ERR... :S
	return false;
}

D3DXVECTOR3 baryToVec(D3DXVECTOR3 va, D3DXVECTOR3 vb, D3DXVECTOR3 vc, float u, float v)
{
	D3DXVECTOR3 res = (vb * u) + (vc * v) + (va * (1 - (u + v)));
	return res;
}

bool UNCRZ_model::collidesVX_PC(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes)
{
	float uRes, vRes;
	vertexPC a;
	vertexPC b;
	vertexPC c;
	D3DXVECTOR4 va;
	D3DXVECTOR4 vb;
	D3DXVECTOR4 vc;

	bool victory = false;
	float bestRes;
	float tempRes;

	for (int i = 0; i < numIndices; i += 3)
	{
		a = (((vertexPC*)vertexArray)[(int)indexArray[i]]);
		b = (((vertexPC*)vertexArray)[(int)indexArray[i + 1]]);
		c = (((vertexPC*)vertexArray)[(int)indexArray[i + 2]]);

		va = D3DXVECTOR4(a.x, a.y, a.z, 1.0f);
		D3DXVec4Transform(&va, &va, transArr->getValue((int)a.tti));
		vb = D3DXVECTOR4(b.x, b.y, b.z, 1.0f);
		D3DXVec4Transform(&vb, &vb, transArr->getValue((int)b.tti));
		vc = D3DXVECTOR4(c.x, c.y, c.z, 1.0f);
		D3DXVec4Transform(&vc, &vc, transArr->getValue((int)c.tti));

		if (D3DXIntersectTri((D3DXVECTOR3*)&va, (D3DXVECTOR3*)&vb, (D3DXVECTOR3*)&vc, rayPos, rayDir, &uRes, &vRes, &tempRes))
		{
			if (victory == false)
			{
				victory = true;
				bestRes = tempRes;
			}
			else if (tempRes < bestRes)
			{
				bestRes = tempRes;
			}
		}
		//if (D3DXIntersectTri((D3DXVECTOR3*)&va, (D3DXVECTOR3*)&vb, (D3DXVECTOR3*)&vc, rayPos, rayDir, &uRes, &vRes, distRes))
		//	return true;
	}

	if (victory)
	{
		*distRes = bestRes;
		return true;
	}
	return false;
}

bool UNCRZ_model::collidesVX_PCT(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes)
{
	float uRes, vRes;
	vertexPCT a;
	vertexPCT b;
	vertexPCT c;
	D3DXVECTOR4 va;
	D3DXVECTOR4 vb;
	D3DXVECTOR4 vc;

	bool victory = false;
	float bestRes;
	float tempRes;

	for (int i = 0; i < numIndices; i += 3)
	{
		a = (((vertexPCT*)vertexArray)[(int)indexArray[i]]);
		b = (((vertexPCT*)vertexArray)[(int)indexArray[i + 1]]);
		c = (((vertexPCT*)vertexArray)[(int)indexArray[i + 2]]);

		va = D3DXVECTOR4(a.x, a.y, a.z, 1.0f);
		D3DXVec4Transform(&va, &va, transArr->getValue((int)a.tti));
		vb = D3DXVECTOR4(b.x, b.y, b.z, 1.0f);
		D3DXVec4Transform(&vb, &vb, transArr->getValue((int)b.tti));
		vc = D3DXVECTOR4(c.x, c.y, c.z, 1.0f);
		D3DXVec4Transform(&vc, &vc, transArr->getValue((int)c.tti));

		if (D3DXIntersectTri((D3DXVECTOR3*)&va, (D3DXVECTOR3*)&vb, (D3DXVECTOR3*)&vc, rayPos, rayDir, &uRes, &vRes, &tempRes))
		{
			if (victory == false)
			{
				victory = true;
				bestRes = tempRes;
			}
			else if (tempRes < bestRes)
			{
				bestRes = tempRes;
			}
		}
		//if (D3DXIntersectTri((D3DXVECTOR3*)&va, (D3DXVECTOR3*)&vb, (D3DXVECTOR3*)&vc, rayPos, rayDir, &uRes, &vRes, distRes))
		//	return true;
	}

	if (victory)
	{
		*distRes = bestRes;
		return true;
	}
	return false;
}



int UNCRZ_model::collidesVertex(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes)
{
	// bbox collision
	if (modelBox.collides(rayPos, rayDir) == false)
		return -1;

	// vertex collision
	if (vertexType == VX_PC)
	{
		return collidesVertexVX_PC(rayPos, rayDir, distRes);
	}
	else if (vertexType == VX_PCT)
	{
		return collidesVertexVX_PCT(rayPos, rayDir, distRes);
	}
	// ERR... :S
	return -1;
}

int UNCRZ_model::collidesVertexVX_PC(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes)
{
	float uRes, vRes;
	vertexPC a;
	vertexPC b;
	vertexPC c;
	D3DXVECTOR4 va;
	D3DXVECTOR4 vb;
	D3DXVECTOR4 vc;

	bool victory = false;
	float bestRes;
	float tempRes;
	int indexRes;

	for (int i = 0; i < numIndices; i += 3)
	{
		a = (((vertexPC*)vertexArray)[(int)indexArray[i]]);
		b = (((vertexPC*)vertexArray)[(int)indexArray[i + 1]]);
		c = (((vertexPC*)vertexArray)[(int)indexArray[i + 2]]);

		va = D3DXVECTOR4(a.x, a.y, a.z, 1.0f);
		D3DXVec4Transform(&va, &va, transArr->getValue((int)a.tti));
		vb = D3DXVECTOR4(b.x, b.y, b.z, 1.0f);
		D3DXVec4Transform(&vb, &vb, transArr->getValue((int)b.tti));
		vc = D3DXVECTOR4(c.x, c.y, c.z, 1.0f);
		D3DXVec4Transform(&vc, &vc, transArr->getValue((int)c.tti));

		if (D3DXIntersectTri((D3DXVECTOR3*)&va, (D3DXVECTOR3*)&vb, (D3DXVECTOR3*)&vc, rayPos, rayDir, &uRes, &vRes, &tempRes))
		{
			//if (victory == false)
			//{
			//	victory = true;
			//	bestRes = tempRes;
			//}
			//else if (tempRes < bestRes)
			//{
			//	bestRes = tempRes;
			//}
			
			if (victory == false || (victory && tempRes < bestRes))
			{
				victory = true;
				bestRes = tempRes;
				
				if (uRes < 0.5f && vRes < 0.5f)
					indexRes = (int)indexArray[i];
				else if (uRes > vRes)
					indexRes = indexArray[i + 1];
				else
					indexRes = indexArray[i + 2];
			}
		}
		//if (D3DXIntersectTri((D3DXVECTOR3*)&va, (D3DXVECTOR3*)&vb, (D3DXVECTOR3*)&vc, rayPos, rayDir, &uRes, &vRes, distRes))
		//	return true;
	}

	if (victory)
	{
		*distRes = bestRes;
		return indexRes;
	}
	return -1;
}

int UNCRZ_model::collidesVertexVX_PCT(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes)
{
	float uRes, vRes;
	vertexPCT a;
	vertexPCT b;
	vertexPCT c;
	D3DXVECTOR4 va;
	D3DXVECTOR4 vb;
	D3DXVECTOR4 vc;

	bool victory = false;
	float bestRes;
	float tempRes;
	int indexRes;

	for (int i = 0; i < numIndices; i += 3)
	{
		a = (((vertexPCT*)vertexArray)[(int)indexArray[i]]);
		b = (((vertexPCT*)vertexArray)[(int)indexArray[i + 1]]);
		c = (((vertexPCT*)vertexArray)[(int)indexArray[i + 2]]);

		va = D3DXVECTOR4(a.x, a.y, a.z, 1.0f);
		D3DXVec4Transform(&va, &va, transArr->getValue((int)a.tti));
		vb = D3DXVECTOR4(b.x, b.y, b.z, 1.0f);
		D3DXVec4Transform(&vb, &vb, transArr->getValue((int)b.tti));
		vc = D3DXVECTOR4(c.x, c.y, c.z, 1.0f);
		D3DXVec4Transform(&vc, &vc, transArr->getValue((int)c.tti));

		if (D3DXIntersectTri((D3DXVECTOR3*)&va, (D3DXVECTOR3*)&vb, (D3DXVECTOR3*)&vc, rayPos, rayDir, &uRes, &vRes, &tempRes))
		{
			//if (victory == false)
			//{
			//	victory = true;
			//	bestRes = tempRes;
			//}
			//else if (tempRes < bestRes)
			//{
			//	bestRes = tempRes;
			//}

			if (victory == false || (victory && tempRes < bestRes))
			{
				victory = true;
				bestRes = tempRes;
				
				if (uRes < 0.5f && vRes < 0.5f)
					indexRes = (int)indexArray[i];
				else if (uRes > vRes)
					indexRes = indexArray[i + 1];
				else
					indexRes = indexArray[i + 2];
			}
		}
		//if (D3DXIntersectTri((D3DXVECTOR3*)&va, (D3DXVECTOR3*)&vb, (D3DXVECTOR3*)&vc, rayPos, rayDir, &uRes, &vRes, distRes))
		//	return true;
	}

	if (victory)
	{
		*distRes = bestRes;
		return indexRes;
	}
	return -1;
}
// end uncrz_model

const DWORD UNCRZ_fai_shift = 0x00000000;
const DWORD UNCRZ_fai_rotate = 0x00000001;
const DWORD UNCRZ_fai_modcol = 0x00000002;
const DWORD UNCRZ_fai_rotate_around = 0x00000003;

const DWORD UNCRZ_fai_offset = 0x00000010;
const DWORD UNCRZ_fai_rotation = 0x00000011;
const DWORD UNCRZ_fai_colmod = 0x00000012;

const DWORD UNCRZ_fai_offset_smth0 = 0x00000020;
const DWORD UNCRZ_fai_rotation_smth0 = 0x00000021;
const DWORD UNCRZ_fai_colmod_smth0 = 0x00000021;

const DWORD UNCRZ_fai_offset_smth1 = 0x00000030;
const DWORD UNCRZ_fai_rotation_smth1 = 0x00000031;
const DWORD UNCRZ_fai_colmod_smth1 = 0x00000031;

const DWORD AI_proc_0 = 0x00000001;
const DWORD AI_proc_1 = 0x00000002;
const DWORD AI_proc_2 = 0x00000004;
const DWORD AI_proc_3 = 0x00000008;

const DWORD AI_proc_all4 = 0x000000000f;

struct UNCRZ_FBF_anim_instr
{
	DWORD instrType;
	int segIndex;
	int secIndex;
	D3DXVECTOR4 val;
	DWORD aiFlags; 

	std::string dbgLine;

	// proportions (out of 1.0) of duration consumed
	static float smth0_func(float num)
	{
		num = D3DX_PI * num;
		return num * 0.5f - 0.25f * sin(num * 2.0f);
	}

	// proportions (out of 1.0) of duration consumed
	static float smth1_func(float num)
	{
		num = D3DX_PI * num;
		return -cos(num);
	}

	void run(UNCRZ_model* model, int curDuration, int totalDuration)
	{
		UNCRZ_segment* seg;
		if (segIndex != -1)
		{
			seg = model->allSegs[segIndex];
			seg->requiresUpdate = true;
		}

		UNCRZ_section* sec;
		if (secIndex != -1)
			sec = model->sections[secIndex];

		float gone; // what is gone
		float left; // what is left
		float yum; // what will be used
		float ratio; // what will be used, over what is left to be used

		D3DXMATRIX tempMatrix; // general use tempory array of 16 values (feel free to use it NOT as a matrix ;) )

		if (instrType == UNCRZ_fai_shift)
		{
			if (aiFlags & AI_proc_0)
				seg->offset.x += val.x;
			if (aiFlags & AI_proc_1)
				seg->offset.y += val.y;
			if (aiFlags & AI_proc_2)
				seg->offset.z += val.z;
		}
		else if (instrType == UNCRZ_fai_rotate)
		{
			if (aiFlags & AI_proc_0)
				seg->rotation.x += val.x;
			if (aiFlags & AI_proc_1)
				seg->rotation.y += val.y;
			if (aiFlags & AI_proc_2)
				seg->rotation.z += val.z;
		}
		else if (instrType == UNCRZ_fai_rotate_around)
		{
			D3DXMatrixRotationAxis(&tempMatrix, (D3DXVECTOR3*)(&val.x), val.w);

			D3DXVec3TransformNormal(&seg->rotation, &seg->rotation, &tempMatrix);

			//if (aiFlags & AI_proc_0)
			//	seg->rotation.x += val.x;
			//if (aiFlags & AI_proc_1)
			//	seg->rotation.y += val.y;
			//if (aiFlags & AI_proc_2)
			//	seg->rotation.z += val.z;
		}
		else if (instrType == UNCRZ_fai_modcol)
		{
			if (aiFlags & AI_proc_0)
			{
				sec->colMod.x += val.x;
				while (sec->colMod.x > 1.0)
					sec->colMod.x -= 1.0;
				while (sec->colMod.x < 0.0)
					sec->colMod.x += 1.0;
			}
			
			if (aiFlags & AI_proc_1)
			{
				sec->colMod.y += val.y;
				while (sec->colMod.y > 1.0)
					sec->colMod.y -= 1.0;
				while (sec->colMod.y < 0.0)
					sec->colMod.y += 1.0;
			}
			if (aiFlags & AI_proc_2)
			{
				sec->colMod.z += val.z;
				while (sec->colMod.z > 1.0)
					sec->colMod.z -= 1.0;
				while (sec->colMod.z < 0.0)
					sec->colMod.z += 1.0;
			}
			if (aiFlags & AI_proc_3)
			{
				sec->colMod.w += val.w;
				while (sec->colMod.w > 1.0)
					sec->colMod.w -= 1.0;
				while (sec->colMod.w < 0.0)
					sec->colMod.w += 1.0;
			}
		}
		else if (instrType == UNCRZ_fai_offset)
		{
			left = totalDuration - curDuration;
			ratio = 1.0 / left;
			if (aiFlags & AI_proc_0)
				seg->offset.x += (val.x - seg->offset.x) * ratio;
			if (aiFlags & AI_proc_1)
				seg->offset.y += (val.y - seg->offset.y) * ratio;
			if (aiFlags & AI_proc_2)
				seg->offset.z += (val.z - seg->offset.z) * ratio;
		}
		else if (instrType == UNCRZ_fai_rotation) // dodgy!!!
		{
			left = totalDuration - curDuration;
			ratio = 1.0 / left;
			if (aiFlags & AI_proc_0)
				seg->rotation.x += (val.x - seg->rotation.x) * ratio;
			if (aiFlags & AI_proc_1)
				seg->rotation.y += (val.y - seg->rotation.y) * ratio;
			if (aiFlags & AI_proc_2)
				seg->rotation.z += (val.z - seg->rotation.z) * ratio;
		}
		else if (instrType == UNCRZ_fai_colmod) // dodgy!!!
		{
			left = totalDuration - curDuration;
			ratio = 1.0 / left;
			if (aiFlags & AI_proc_0)
				sec->colMod.x += (val.x - sec->colMod.x) * ratio;
			if (aiFlags & AI_proc_1)
				sec->colMod.y += (val.y - sec->colMod.y) * ratio;
			if (aiFlags & AI_proc_2)
				sec->colMod.z += (val.z - sec->colMod.z) * ratio;
			if (aiFlags & AI_proc_3)
				sec->colMod.w += (val.w - sec->colMod.w) * ratio;
		}
		else if (instrType == UNCRZ_fai_offset_smth0) // dodgy
		{
			yum = 1.0f / (float)totalDuration;
			gone = (float)curDuration * yum;
			yum = smth0_func(yum + gone);
			gone = -smth0_func(gone);
			left = D3DX_PI * 0.5 + gone;
			yum += gone;
			ratio = yum / left;

			if (aiFlags & AI_proc_0)
				seg->offset.x += (val.x - seg->offset.x) * ratio;
			if (aiFlags & AI_proc_1)
				seg->offset.y += (val.y - seg->offset.y) * ratio;
			if (aiFlags & AI_proc_2)
				seg->offset.z += (val.z - seg->offset.z) * ratio;
		}
		else if (instrType == UNCRZ_fai_rotation_smth0) // dodgy!!!
		{
			//yum = 1.0f / (float)totalDuration;
			//gone = (float)curDuration / (float)totalDuration;
			//left = D3DX_PI * 0.5 - smth0_func(gone);
			//yum = smth0_func(yum + gone) - smth0_func(gone);
			//ratio = yum / left; // original, unabridged, ish

			yum = 1.0f / (float)totalDuration;
			gone = (float)curDuration * yum;
			yum = smth0_func(yum + gone);
			gone = -smth0_func(gone);
			left = D3DX_PI * 0.5 + gone;
			yum += gone;
			ratio = yum / left;

			if (aiFlags & AI_proc_0)
				seg->rotation.x += (val.x - seg->rotation.x) * ratio;
			if (aiFlags & AI_proc_1)
				seg->rotation.y += (val.y - seg->rotation.y) * ratio;
			if (aiFlags & AI_proc_2)
				seg->rotation.z += (val.z - seg->rotation.z) * ratio;
		}
		else if (instrType == UNCRZ_fai_colmod_smth0) // dodgy!!!
		{
			yum = 1.0f / (float)totalDuration;
			gone = (float)curDuration * yum;
			yum = smth0_func(yum + gone);
			gone = -smth0_func(gone);
			left = D3DX_PI * 0.5 + gone;
			yum += gone;
			ratio = yum / left;

			if (aiFlags & AI_proc_0)
				sec->colMod.x += (val.x - sec->colMod.x) * ratio;
			if (aiFlags & AI_proc_1)
				sec->colMod.y += (val.y - sec->colMod.y) * ratio;
			if (aiFlags & AI_proc_2)
				sec->colMod.z += (val.z - sec->colMod.z) * ratio;
			if (aiFlags & AI_proc_3)
				sec->colMod.w += (val.z - sec->colMod.w) * ratio;
		}
		else if (instrType == UNCRZ_fai_offset_smth1) // dodgy
		{
			yum = 1.0f / (float)totalDuration;
			gone = (float)curDuration * yum;
			yum = smth1_func(yum + gone);
			gone = -smth1_func(gone);
			left = 1 + gone;
			yum += gone;
			ratio = yum / left;

			if (aiFlags & AI_proc_0)
				seg->offset.x += (val.x - seg->offset.x) * ratio;
			if (aiFlags & AI_proc_1)
				seg->offset.y += (val.y - seg->offset.y) * ratio;
			if (aiFlags & AI_proc_2)
				seg->offset.z += (val.z - seg->offset.z) * ratio;
		}
		else if (instrType == UNCRZ_fai_rotation_smth1) // dodgy!!!
		{
			//yum = 1.0f / (float)totalDuration;
			//gone = (float)curDuration / (float)totalDuration;
			//left = D3DX_PI * 0.5 - smth1_func(gone);
			//yum = smth1_func(yum + gone) - smth1_func(gone);
			//ratio = yum / left; // original, unabridged, ish

			yum = 1.0f / (float)totalDuration;
			gone = (float)curDuration * yum;
			yum = smth1_func(yum + gone);
			gone = -smth1_func(gone);
			left = 1 + gone;
			yum += gone;
			ratio = yum / left;

			if (aiFlags & AI_proc_0)
				seg->rotation.x += (val.x - seg->rotation.x) * ratio;
			if (aiFlags & AI_proc_1)
				seg->rotation.y += (val.y - seg->rotation.y) * ratio;
			if (aiFlags & AI_proc_2)
				seg->rotation.z += (val.z - seg->rotation.z) * ratio;
		}
		else if (instrType == UNCRZ_fai_colmod_smth1) // dodgy!!!
		{
			yum = 1.0f / (float)totalDuration;
			gone = (float)curDuration * yum;
			yum = smth1_func(yum + gone);
			gone = -smth1_func(gone);
			left = 1 + gone;
			yum += gone;
			ratio = yum / left;

			if (aiFlags & AI_proc_0)
				sec->colMod.x += (val.x - sec->colMod.x) * ratio;
			if (aiFlags & AI_proc_1)
				sec->colMod.y += (val.y - sec->colMod.y) * ratio;
			if (aiFlags & AI_proc_2)
				sec->colMod.z += (val.z - sec->colMod.z) * ratio;
			if (aiFlags & AI_proc_3)
				sec->colMod.w += (val.z - sec->colMod.w) * ratio;
		}
	}

	void getSeg(std::string segName, UNCRZ_model* model)
	{
		for (int i = model->allSegs.size() - 1; i >= 0; i--)
		{
			if (model->allSegs[i]->name == segName)
			{
				segIndex = i;
			}
		}
	}

	void getSec(std::string secName, UNCRZ_model* model)
	{
		for (int i = model->sections.size() - 1; i >= 0; i--)
		{
			if (model->sections[i]->name == secName)
			{
				secIndex = i;
			}
		}
	}

	UNCRZ_FBF_anim_instr(std::string line, UNCRZ_model* model)
	{
		secIndex = -1;
		segIndex = -1;

		dbgLine = line;
		std::vector<std::string> data = split(line, " ");

		if (data[0] == "shift")
		{
			instrType = UNCRZ_fai_shift;
			getSeg(data[1], model);
		}
		else if (data[0] == "rotate")
		{
			instrType = UNCRZ_fai_rotate;
			getSeg(data[1], model);
		}
		else if (data[0] == "modcol")
		{
			instrType = UNCRZ_fai_modcol;
			getSec(data[1], model);
		}
		else if (data[0] == "offset")
		{
			instrType = UNCRZ_fai_offset;
			getSeg(data[1], model);
		}
		else if (data[0] == "rotation")
		{
			instrType = UNCRZ_fai_rotation;
			getSeg(data[1], model);
		}
		else if (data[0] == "colmod")
		{
			instrType = UNCRZ_fai_colmod;
			getSec(data[1], model);
		}
		else if (data[0] == "offset_smth0")
		{
			instrType = UNCRZ_fai_offset_smth0;
			getSeg(data[1], model);
		}
		else if (data[0] == "rotation_smth0")
		{
			instrType = UNCRZ_fai_rotation_smth0;
			getSeg(data[1], model);
		}
		else if (data[0] == "offset_smth1")
		{
			instrType = UNCRZ_fai_offset_smth1;
			getSeg(data[1], model);
		}
		else if (data[0] == "rotation_smth1")
		{
			instrType = UNCRZ_fai_rotation_smth1;
			getSeg(data[1], model);
		}

		if (data.size() == 7)
		{
			val = D3DXVECTOR4(stof(data[2]), stof(data[3]), stof(data[4]), stof(data[5]));
			aiFlags = (DWORD)stoi(data[6]);
		}
		else
		{
			aiFlags = AI_proc_all4;
			if (data.size() == 6)
				val = D3DXVECTOR4(stof(data[2]), stof(data[3]), stof(data[4]), stof(data[5]));
			else if (data.size() == 5)
				val = D3DXVECTOR4(stof(data[2]), stof(data[3]), stof(data[4]), 0.0f);
			else if (data.size() == 4)
				val = D3DXVECTOR4(stof(data[2]), stof(data[3]), 0.0f, 0.0f);
			else if (data.size() == 3)
				val = D3DXVECTOR4(stof(data[2]), 0.0, 0.0f, 0.0f);
		}
	}
};

struct UNCRZ_FBF_anim_motion
{
public:
	std::string name;
	int startInstr;
	int endInstr;
	int duration; // frames
	bool causesUpdate;

	UNCRZ_FBF_anim_motion()
	{
		causesUpdate = true;
	}

	UNCRZ_FBF_anim_motion(std::string nameN)
	{
		name = nameN;
		causesUpdate = true;
	}
};

//struct UNCRZ_FBF_anim_flow
//{
void UNCRZ_FBF_anim_flow::run(UNCRZ_FBF_anim_inst* inst, UNCRZ_FBF_anim_flow_state* state)
	{
		int d = motions[state->curMotion].duration;
		int end = motions[state->curMotion].endInstr;
		for (int i = motions[state->curMotion].startInstr; i <= end; i++)
		{
			instrs[i].run(inst->model, state->curMotionDuration, d);
		}

		state->curMotionDuration = state->curMotionDuration + 1;
		if (state->curMotionDuration >= d)
		{
			state->curMotionDuration = 0;
			state->curMotion = state->curMotion + 1;
			if (state->curMotion >= motions.size())
			{
				state->curMotion = 0;
			}
		}

		if (motions[state->curMotion].causesUpdate)
		{
			inst->model->allSegsRequireUpdate();
		}
	}

UNCRZ_FBF_anim_flow::UNCRZ_FBF_anim_flow(std::string nameN)
	{
		name = nameN;
	}

//};

//struct UNCRZ_FBF_anim
//{

void UNCRZ_FBF_anim::run(UNCRZ_FBF_anim_inst* inst)
	{
		for (int i = flows.size() - 1; i >= 0; i--)
		{
			flows[i].run(inst, &inst->states[i]);
		}
	}

void UNCRZ_FBF_anim::addFlow(std::string name)
	{
		flows.push_back(UNCRZ_FBF_anim_flow(name));
		lastFlow = &flows[flows.size() - 1];
	}

void UNCRZ_FBF_anim::setFlowStart(int sm, int smd)
	{
		lastFlow->start.motion = sm;
		lastFlow->start.motionDuration = smd;
	}

void UNCRZ_FBF_anim::setFlowStart(std::string sms, int smd)
	{
		for (int i = lastFlow->motions.size() - 1; i >= 0; i--)
		{
			if (lastFlow->motions[i].name == sms)
			{
				lastFlow->start.motion = i;
				lastFlow->start.motionDuration = smd;
				return;
			}
		}
	}
	
void UNCRZ_FBF_anim::endMotion()
	{
		lastFlow->motions[lastFlow->motions.size() - 1].endInstr = (int)lastFlow->instrs.size() - 1;
	}
	
void UNCRZ_FBF_anim::addMotion(std::string name)
	{
		lastFlow->motions.push_back(UNCRZ_FBF_anim_motion(name));
		lastFlow->motions[lastFlow->motions.size() - 1].startInstr = (int)lastFlow->instrs.size();
	}
	
void UNCRZ_FBF_anim::setMotionDuration(int duration)
	{
		lastFlow->motions[lastFlow->motions.size() - 1].duration = duration;
	}

void UNCRZ_FBF_anim::setMotionCausesUpdate(bool mg)
	{
		lastFlow->motions[lastFlow->motions.size() - 1].causesUpdate = mg;
	}

void UNCRZ_FBF_anim::addLine(std::string line)
	{
		lastFlow->instrs.push_back(UNCRZ_FBF_anim_instr(line, model));
	}

UNCRZ_FBF_anim::UNCRZ_FBF_anim(UNCRZ_model* modelN)
	{
		model = modelN;
		modelName = model->name;
	}
//};

/*
OMG OMG Concept Code, right?
// node flags
DWORD NF_none = 0x00000000;

// node game flags
DWORD NGF_none = 0x00000000;

struct UNCRZ_node
{
public:
	bool tight; // whether or not it's tight here
	DWORD nodeFlags;
	DWORD nodeGameFlags;
	D3DXVECTOR3 location;

};

struct UNCRZ_pathing
{
public:
	std::vector<UNCRZ_node> nodes;


};
*/

struct UNCRZ_obj
{
public:
	D3DXVECTOR3 offset;
	D3DXVECTOR3 rotation;
	D3DXMATRIX offsetMatrix; // call updateMatrices whenever the offset are modified
	D3DXMATRIX rotationMatrix; // call updateMatrices whenever the offset/rotation are modified
	D3DXMATRIX offsetMatrixinv; // call updateMatrices whenever the offset are modified
	D3DXMATRIX rotationMatrixinv; // call updateMatrices whenever the offset/rotation are modified
	D3DXMATRIX localTrans; // call updateMatrices whenever the offset are modified
	D3DXMATRIX localTransinv; // call updateMatrices whenever the offset/rotation are modified

	UNCRZ_model* model; // must be unique if you want unique transforms

	float zSortDepth; // used for sorting
	bool noDraw;

	// when it has a new dest, it calls 'walk', and then it gets a new targ - when it gets there, it calls 'walk' again, and so and so forth until it hits dest

	// Obj/Sprites (this is a very inconplete concept which will be used to /try/ and solve the transparency problem for sprites
	UNCRZ_sprite* sprite;
	int spriteOffset;
	int spriteCount;
	UNCRZ_sprite_buffer* spriteBuffer;

	UNCRZ_obj(UNCRZ_model* modelN)
	{
		model = modelN;
		offset = D3DXVECTOR3(0.0, 0.0, 0.0);
		rotation = D3DXVECTOR3(0.0, 0.0, 0.0);

		noDraw = false;
	}

	void changeAnim(UNCRZ_FBF_anim* newAnim)
	{
		if (model->animInst == NULL)
			model->animInst = new UNCRZ_FBF_anim_inst(model, newAnim);
		else
			model->animInst->reset(newAnim);
	}

	void run()
	{
		if (model->animInst != NULL)
			model->animInst->anim->run(model->animInst);
	}

	void update(bool forceUpdate = false)
	{
		updateMatrices();

		D3DXMATRIX trans;
		D3DXMatrixIdentity(&trans);

		D3DXMatrixMultiply(&trans, &offsetMatrix, &trans);
		D3DXMatrixMultiply(&trans, &rotationMatrix, &trans);

		model->update(&trans, forceUpdate);
	}

	void drawMany(LPDIRECT3DDEVICE9 dxDevice, drawData* ddat, UNCRZ_model** arr, int count, DWORD drawArgs)
	{
		// no noDraw check here
		model->drawMany(dxDevice, ddat, arr, count, drawArgs);
	}

	void draw(LPDIRECT3DDEVICE9 dxDevice, drawData* ddat, DWORD drawArgs)
	{
		if (noDraw)
			return;
		model->draw(dxDevice, ddat, drawArgs);
	}

	void updateMatrices()
	{
		D3DXMatrixTranslation(&offsetMatrix, offset.x, offset.y, offset.z);
		D3DXMatrixRotationYawPitchRoll(&rotationMatrix, rotation.y, rotation.x, rotation.z);
		D3DXMatrixMultiply(&localTrans, &offsetMatrix, &rotationMatrix);
		
		D3DXMatrixInverse(&offsetMatrixinv, NULL, &offsetMatrix);
		D3DXMatrixInverse(&rotationMatrixinv, NULL, &rotationMatrix);
		D3DXMatrixInverse(&localTransinv, NULL, &localTrans);
	}
};

// enum MD
const DWORD MD_access = 0x00000001;

// PTW4 etc. terrain
struct UNCRZ_terrain
{
	std::string name;

	UNCRZ_effect effect;

	bool useTex;
	LPDIRECT3DTEXTURE9 tex;
	bool useTex0;
	LPDIRECT3DTEXTURE9 tex0;
	bool useTex1;
	LPDIRECT3DTEXTURE9 tex1;
	bool useTex2;
	LPDIRECT3DTEXTURE9 tex2;
	bool useTex3;
	LPDIRECT3DTEXTURE9 tex3;

	D3DXHANDLE tech;
	D3DXHANDLE lightTech;
	D3DXHANDLE decalTech;
	D3DXHANDLE dynamicDecalTech;

	LPDIRECT3DVERTEXBUFFER9 vBuff;
	LPDIRECT3DVERTEXDECLARATION9 vertexDec;
	LPDIRECT3DINDEXBUFFER9 iBuff;
	DWORD vertexType;
	UINT stride;
	int numVertices;
	int numIndices;

	void* vertexArray; // array of vertices
	short* indexArray; // array of indicies

	std::vector<UNCRZ_decal*> decals;

	float xCorner;
	float zCorner;
	int width;
	int height;
	float dimension;

	UNCRZ_terrain()
	{
	}

	UNCRZ_terrain(std::string nameN)
	{
		name = nameN;
	}

	void destroy()
	{
		int len = width * height;
		vBuff->Release();
		iBuff->Release();
	}

	vertexPTW4* getPTW4(int x, int z)
	{
		return (((vertexPTW4*)vertexArray) + x + z * width);
	}

	vertexPTW4* getPTW4(int index)
	{
		int x = index % width;
		int z = (index - x) / width;
		if (x < 0 || z < 0 || x >= width || z >= height)
			return NULL;
		return (((vertexPTW4*)vertexArray) + x + z * width);
	}

	vertexPTW4* getPTW4(int index, int* x, int* z)
	{
		*x = index % width;
		*z = (index - *x) / width;
		return (((vertexPTW4*)vertexArray) + *x + *z * width);
	}

	int getX(int index)
	{
		return index % width;
	}

	int getZ(int index)
	{
		int x = index % width;
		return (index - x) / width;
	}

	void setTextures()
	{
		if (useTex)
			effect.setTexture(tex);
		if (useTex0)
			effect.setTexture0(tex0);
		if (useTex1)
			effect.setTexture1(tex1);
		if (useTex2)
			effect.setTexture2(tex2);
		if (useTex3)
			effect.setTexture3(tex3);
	}

	void draw(LPDIRECT3DDEVICE9 dxDevice, drawData* ddat, DWORD drawArgs)
	{
		HRESULT res;

		ddat->enableClip(dxDevice);

		dxDevice->SetVertexDeclaration(vertexDec);
		dxDevice->SetStreamSource(0, vBuff, 0, stride);
		dxDevice->SetIndices(iBuff);

		if (drawArgs & DF_light)
		{
			effect.setTechnique(lightTech);
			effect.setLightType(ddat->lightType);
			effect.setLightDepth(ddat->lightDepth);
			effect.setLightConeness(ddat->lightConeness);
		}
		else
		{
			effect.setTechnique(tech);
			effect.setLightCoof(ddat->lightCoof);
			effect.setFarDepth(ddat->farDepth);
		}

		setTextures();

		effect.setEyePos(&ddat->eyePos);
		effect.setEyeDir(&ddat->eyeDir);
		effect.setTicker(ddat->ticker);
		effect.setViewProj(ddat->viewProj);

		effect.effect->CommitChanges();

		UINT numPasses, pass;
		effect.effect->Begin(&numPasses, 0);

		// disable blend
		dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

		// pass 0
		if (drawArgs & DF_light)
			effect.effect->BeginPass((int)ddat->lightType);
		else if (ddat->performPlainPass)
			effect.effect->BeginPass(0);
		else
			goto skipPlainPass;

		res = dxDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, numVertices, 0, numIndices / 3);
		if (res != D3D_OK)
			res = res;
		effect.effect->EndPass();
skipPlainPass:

		if (numPasses > 1 && (drawArgs & DF_light) == false)
		{
			// enable blend
			dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

			// light pass
			for (int ldi = ddat->lightDatas.size() - 1; ldi >= 0; ldi--)
			{
				lightData* ld = ddat->lightDatas[ldi];
				
				if (!ld->lightEnabled)
					continue;

				effect.setLightData(ld);
				effect.effect->CommitChanges();

				effect.effect->BeginPass((int)ld->lightType + 1);
				res = dxDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, numVertices, 0, numIndices / 3);
				if (res != D3D_OK)
					res = res;
				effect.effect->EndPass();
			}

		}
		// disable blend
		dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

		effect.effect->End();

		if (drawArgs & DF_light)
			return; // no decals for light

		if (decals.size() == 0)
			goto skipToDynamicDecals;

		ddat->startTimer(true);

		// decals
		dxDevice->SetVertexDeclaration(vertexDecPAT4);
		effect.setTechnique(decalTech);
		
		float white[4];
		white[0] = 1.0;
		white[1] = 1.0;
		white[2] = 1.0;
		white[3] = 1.0;
		effect.setcolMod(&(white[0]));

		effect.effect->CommitChanges();

		dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		effect.effect->Begin(&numPasses, 0);

		// pass 0
		if (drawArgs & DF_light)
			effect.effect->BeginPass((int)ddat->lightType);
		else if (ddat->performPlainPass)
			effect.effect->BeginPass(0);
		else
			goto skipPlainDecalPass;

		for (int i = 0; i < decals.size(); i++)
		{
			effect.setTexture(decals[i]->tex);
			effect.effect->CommitChanges();

			res = dxDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, decals[i]->numFaces, decals[i]->vertexArray, sizeof(vertexPAT4));
			if (res != D3D_OK)
				res = res;
		}
		effect.effect->EndPass();
skipPlainDecalPass:

		if (numPasses > 1 && (drawArgs & DF_light) == false)
		{
			// enable blend
			dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

			// light pass
			for (int ldi = ddat->lightDatas.size() - 1; ldi >= 0; ldi--)
			{
				lightData* ld = ddat->lightDatas[ldi];
				
				if (!ld->lightEnabled)
					continue;

				effect.setLightData(ld);
				effect.effect->CommitChanges();

				effect.effect->BeginPass((int)ld->lightType + 1);
				for (int i = 0; i < decals.size(); i++)
				{
					effect.setTexture(decals[i]->tex);
					effect.effect->CommitChanges();

					res = dxDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, decals[i]->numFaces, decals[i]->vertexArray, sizeof(vertexPAT4));
					if (res != D3D_OK)
						res = res;
				}
				effect.effect->EndPass();
			}

		}
		// disable blend
		dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		
		ddat->stopTimer(0, true, "Decals: ", GDOF_dms);

		effect.effect->End();

		/*effect.effect->Begin(&numPasses, 0);

		for (pass = 0; pass < numPasses; pass++)
		{
			effect.effect->BeginPass(pass);

			//for (int i = decals.size() - 1; i >= 0; i--)
			for (int i = 0; i < decals.size(); i++)
			{
				effect.setTexture(decals[i]->tex);
				effect.effect->CommitChanges();

				res = dxDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, decals[i]->numFaces, decals[i]->vertexArray, sizeof(vertexPAT4));
				if (res != D3D_OK)
					res = res;
			}

			effect.effect->EndPass();
		}

		effect.effect->End();*/

		// dynamic decals (DO NOT GET CONFUSED WITH LIGHTS)
skipToDynamicDecals:
		if (ddat->dynamicDecalDatas.size() == 0)
			return;

		ddat->startTimer(true);

		// disable z write
		dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);

		dxDevice->SetVertexDeclaration(vertexDec);
		dxDevice->SetStreamSource(0, vBuff, 0, stride);
		dxDevice->SetIndices(iBuff);
		effect.setTechnique(dynamicDecalTech);
		
		effect.effect->Begin(&numPasses, 0);

		if (numPasses > 0 && (drawArgs & DF_light) == false)
		{
			// enable blend
			dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

			// dynamic decal pass
			for (int dddi = ddat->dynamicDecalDatas.size() - 1; dddi >= 0; dddi--)
			{
				dynamicDecalData* ddd = ddat->dynamicDecalDatas[dddi];
				
				if (!ddd->decalEnabled)
					continue;

				effect.setDynamicDecalData(ddd);
				effect.effect->CommitChanges();

				effect.effect->BeginPass((int)ddd->lightType);
				res = dxDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, numVertices, 0, numIndices / 3);
				if (res != D3D_OK)
					res = res;
				effect.effect->EndPass();
			}

		}
		// disable blend
		dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

		// re-enable z write
		dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
		
		ddat->stopTimer(1, true, "DynDecals: ", GDOF_dms);

		effect.effect->End();
	}

	void release()
	{
		vBuff->Release();
		iBuff->Release();
		delete[stride * numVertices] vertexArray;
		delete[sizeof(short) * numIndices] indexArray;
	}

	void fillVBuff()
	{
		VOID* buffPtr;
		vBuff->Lock(0, numVertices * stride, (VOID**)&buffPtr, D3DLOCK_DISCARD);
		memcpy(buffPtr, vertexArray, numVertices * stride);
		vBuff->Unlock();
	}

	void createVBuff(LPDIRECT3DDEVICE9 dxDevice, void* vds)
	{
		dxDevice->CreateVertexBuffer(numVertices * stride, D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &vBuff, NULL);

		vertexArray = malloc(stride * numVertices);
		memcpy(vertexArray, vds, numVertices * stride);

		fillVBuff();
	}

	void fillIBuff()
	{
		VOID* buffPtr;
		iBuff->Lock(0, numIndices * sizeof (short), (VOID**)&buffPtr, D3DLOCK_DISCARD);
		memcpy(buffPtr, indexArray, numIndices * sizeof (short));
		iBuff->Unlock();
	}

	void createIBuff(LPDIRECT3DDEVICE9 dxDevice, short* ids)
	{
		dxDevice->CreateIndexBuffer(numIndices * sizeof (short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &iBuff, NULL);

		indexArray = (short*)malloc(sizeof (short) * numIndices);
		memcpy(indexArray, ids, numIndices * sizeof (short));

		fillIBuff();
	}

	// just modifies the y prop, autoGenNormals and fillVBuff should be done externally
	void level(int sx, int sz, int ex, int ez, float height, float coof)
	{
		for (int i = sx; i <= ex; i++)
		{
			for (int j = sz; j <= ez; j++)
			{
				vertexPTW4* vPTW4 = getPTW4(i, j);
				vPTW4->y = height * coof + (1.0 - coof) * vPTW4->y;
				//(((vertexPTW4*)vertexArray)[i + j * width]).y = height + (1.0 - coof) * ((((vertexPTW4*)vertexArray)[i + j * width]).y - height);
			}
		}
	}

	bool collidesVert(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes)
	{
		if (vertexType == VX_PTW4)
		{
			return collidesVertVX_PTW4(rayPos, rayDir, distRes);
		}
		return false;
	}

	bool collidesVertVX_PTW4(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes)
	{
		float widthmo = width - 1; // this makes sense, don't doubt it

		// 2 indicies per sqr
		int rx = (rayPos->x - xCorner) / dimension; // roughX
		int rz = (rayPos->z - zCorner) / dimension; // roughZ
		int offset = (rx + widthmo * rz) * 6;

		float uRes, vRes;
		vertexPTW4 a;
		vertexPTW4 b;
		vertexPTW4 c;
		D3DXVECTOR4 va;
		D3DXVECTOR4 vb;
		D3DXVECTOR4 vc;

		bool victory = false;
		float bestRes;
		float tempRes;

		for (int j = offset - widthmo * 6 * 2; j < offset + widthmo * 6 * 2; j += widthmo * 6)
		{
			for (int i = j - 12; i < j + 12; i += 3)
			{
				if (i < 0)
					continue;
				if (i >= widthmo * height * 6)
					continue;
				a = (((vertexPTW4*)vertexArray)[(int)indexArray[i]]);
				b = (((vertexPTW4*)vertexArray)[(int)indexArray[i + 1]]);
				c = (((vertexPTW4*)vertexArray)[(int)indexArray[i + 2]]);

				va = D3DXVECTOR4(a.x, a.y, a.z, 1.0f);
				vb = D3DXVECTOR4(b.x, b.y, b.z, 1.0f);
				vc = D3DXVECTOR4(c.x, c.y, c.z, 1.0f);

				if (D3DXIntersectTri((D3DXVECTOR3*)&va, (D3DXVECTOR3*)&vb, (D3DXVECTOR3*)&vc, rayPos, rayDir, &uRes, &vRes, &tempRes))
				{
					if (victory == false)
					{
						victory = true;
						bestRes = tempRes;
					}
					else if (tempRes < bestRes)
					{
						bestRes = tempRes;
					}
				}
				//if (D3DXIntersectTri((D3DXVECTOR3*)&va, (D3DXVECTOR3*)&vb, (D3DXVECTOR3*)&vc, rayPos, rayDir, &uRes, &vRes, distRes))
				//	return true;
			}
		}

		if (victory)
		{
			*distRes = bestRes;
			return true;
		}
		return false;
	}

	int collidesVertex(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes)
	{
		if (vertexType == VX_PTW4)
		{
			return collidesVertexVX_PTW4(rayPos, rayDir, distRes);
		}

		return false;
	}

	bool collides(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes)
	{
		if (vertexType == VX_PTW4)
		{
			return collidesVX_PTW4(rayPos, rayDir, distRes);
		}

		return false;
	}

	int collidesVertexVX_PTW4(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes)
	{
		float uRes, vRes;
		vertexPTW4 a;
		vertexPTW4 b;
		vertexPTW4 c;
		D3DXVECTOR4 va;
		D3DXVECTOR4 vb;
		D3DXVECTOR4 vc;

		bool victory = false;
		int indexRes;
		float bestRes;
		float tempRes;

		for (int i = 0; i < numIndices; i += 3)
		{
			a = (((vertexPTW4*)vertexArray)[(int)indexArray[i]]);
			b = (((vertexPTW4*)vertexArray)[(int)indexArray[i + 1]]);
			c = (((vertexPTW4*)vertexArray)[(int)indexArray[i + 2]]);

			va = D3DXVECTOR4(a.x, a.y, a.z, 1.0f);
			vb = D3DXVECTOR4(b.x, b.y, b.z, 1.0f);
			vc = D3DXVECTOR4(c.x, c.y, c.z, 1.0f);

			if (D3DXIntersectTri((D3DXVECTOR3*)&va, (D3DXVECTOR3*)&vb, (D3DXVECTOR3*)&vc, rayPos, rayDir, &uRes, &vRes, &tempRes))
			{
				if (victory == false || (victory && tempRes < bestRes))
				{
					victory = true;
					bestRes = tempRes;
					
					if (uRes < 0.5f && vRes < 0.5f)
						indexRes = (int)indexArray[i];
					else if (uRes > vRes)
						indexRes = indexArray[i + 1];
					else
						indexRes = indexArray[i + 2];
				}
			}
			//if (D3DXIntersectTri((D3DXVECTOR3*)&va, (D3DXVECTOR3*)&vb, (D3DXVECTOR3*)&vc, rayPos, rayDir, &uRes, &vRes, distRes))
			//{
			//	if (uRes < 0.5f && vRes < 0.5f)
			//		return (int)indexArray[i];
			//	if (uRes > vRes)
			//		return (int)indexArray[i + 1];
			//	return (int)indexArray[i + 2];
			//}
		}

		if (victory)
		{
			*distRes = bestRes;
			return indexRes;
		}
		return -1;
	}

	bool collidesVX_PTW4(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes)
	{
		float uRes, vRes;
		vertexPTW4 a;
		vertexPTW4 b;
		vertexPTW4 c;
		D3DXVECTOR4 va;
		D3DXVECTOR4 vb;
		D3DXVECTOR4 vc;

		bool victory = false;
		float bestRes;
		float tempRes;

		for (int i = 0; i < numIndices; i += 3)
		{
			a = (((vertexPTW4*)vertexArray)[(int)indexArray[i]]);
			b = (((vertexPTW4*)vertexArray)[(int)indexArray[i + 1]]);
			c = (((vertexPTW4*)vertexArray)[(int)indexArray[i + 2]]);

			va = D3DXVECTOR4(a.x, a.y, a.z, 1.0f);
			vb = D3DXVECTOR4(b.x, b.y, b.z, 1.0f);
			vc = D3DXVECTOR4(c.x, c.y, c.z, 1.0f);

			if (D3DXIntersectTri((D3DXVECTOR3*)&va, (D3DXVECTOR3*)&vb, (D3DXVECTOR3*)&vc, rayPos, rayDir, &uRes, &vRes, &tempRes))
			{
				if (victory == false)
				{
					victory = true;
					bestRes = tempRes;
				}
				else if (tempRes < bestRes)
				{
					bestRes = tempRes;
				}
			}
			//if (D3DXIntersectTri((D3DXVECTOR3*)&va, (D3DXVECTOR3*)&vb, (D3DXVECTOR3*)&vc, rayPos, rayDir, &uRes, &vRes, distRes))
			//	return true;
		}

		if (victory)
		{
			*distRes = bestRes;
			return true;
		}
		return false;
	}
};

// flat rendered terrain

// like a mip level...
struct UNCRZ_frion
{
	UNCRZ_effect effect;

	LPDIRECT3DTEXTURE9 tex;

	D3DXHANDLE flatTech;
	D3DXHANDLE tech;
	D3DXHANDLE lightTech;
	D3DXHANDLE dynamicDecalTech;

	LPDIRECT3DVERTEXBUFFER9 vBuff;
	LPDIRECT3DVERTEXDECLARATION9 vertexDec;
	LPDIRECT3DINDEXBUFFER9 iBuff;
	DWORD vertexType;
	UINT stride;
	int numVertices;
	int numIndices;

	void* vertexArray; // array of vertices
	short* indexArray; // array of indicies

	float sx, sz, ex, ez; // start/end coords of flat rect
	int resX, resY;
	int startIndex;
	int drawLevel;

	void setTextures()
	{
		effect.setTexture(tex);
	}

	void draw(LPDIRECT3DDEVICE9 dxDevice, drawData* ddat, DWORD drawArgs)
	{
		HRESULT res;

		ddat->enableClip(dxDevice);

		dxDevice->SetVertexDeclaration(vertexDec);
		dxDevice->SetStreamSource(0, vBuff, 0, stride);
		dxDevice->SetIndices(iBuff);

		if (drawArgs & DF_light)
		{
			effect.setTechnique(lightTech);
			effect.setLightType(ddat->lightType);
			effect.setLightDepth(ddat->lightDepth);
			effect.setLightConeness(ddat->lightConeness);
		}
		else
		{
			effect.setTechnique(tech);
			effect.setLightCoof(ddat->lightCoof);
			effect.setFarDepth(ddat->farDepth);
		}

		setTextures();

		effect.setEyePos(&ddat->eyePos);
		effect.setEyeDir(&ddat->eyeDir);
		effect.setTicker(ddat->ticker);
		effect.setViewProj(ddat->viewProj);

		effect.effect->CommitChanges();

		UINT numPasses, pass;
		effect.effect->Begin(&numPasses, 0);

		// disable blend
		dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

		// pass 0
		if (drawArgs & DF_light)
			effect.effect->BeginPass((int)ddat->lightType);
		else if (ddat->performPlainPass)
			effect.effect->BeginPass(0);
		else
			goto skipPlainPass;

		res = dxDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, numVertices, 0, numIndices / 3);
		if (res != D3D_OK)
			res = res;
		effect.effect->EndPass();
skipPlainPass:

		if (numPasses > 1 && (drawArgs & DF_light) == false)
		{
			// enable blend
			dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

			// light pass
			for (int ldi = ddat->lightDatas.size() - 1; ldi >= 0; ldi--)
			{
				lightData* ld = ddat->lightDatas[ldi];
				
				if (!ld->lightEnabled)
					continue;

				effect.setLightData(ld);
				effect.effect->CommitChanges();

				effect.effect->BeginPass((int)ld->lightType + 1);
				res = dxDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, numVertices, 0, numIndices / 3);
				if (res != D3D_OK)
					res = res;
				effect.effect->EndPass();
			}

		}
		// disable blend
		dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

		effect.effect->End();

		if (drawArgs & DF_light)
			return; // no dyndecals for light

		// dynamic decals (DO NOT GET CONFUSED WITH LIGHTS)
skipToDynamicDecals:
		if (ddat->dynamicDecalDatas.size() == 0)
			return;

		ddat->startTimer(true);

		// disable z write
		dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);

		dxDevice->SetVertexDeclaration(vertexDec);
		dxDevice->SetStreamSource(0, vBuff, 0, stride);
		dxDevice->SetIndices(iBuff);
		effect.setTechnique(dynamicDecalTech);
		
		effect.effect->Begin(&numPasses, 0);

		if (numPasses > 0 && (drawArgs & DF_light) == false)
		{
			// enable blend
			dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

			// dynamic decal pass
			for (int dddi = ddat->dynamicDecalDatas.size() - 1; dddi >= 0; dddi--)
			{
				dynamicDecalData* ddd = ddat->dynamicDecalDatas[dddi];
				
				if (!ddd->decalEnabled)
					continue;

				effect.setDynamicDecalData(ddd);
				effect.effect->CommitChanges();

				effect.effect->BeginPass((int)ddd->lightType);
				res = dxDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, numVertices, 0, numIndices / 3);
				if (res != D3D_OK)
					res = res;
				effect.effect->EndPass();
			}

		}
		// disable blend
		dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

		// re-enable z write
		dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
		
		ddat->stopTimer(1, true, "DynDecals: ", GDOF_dms);

		effect.effect->End();
	}

	void release()
	{
		vBuff->Release();
		iBuff->Release();
		delete[stride * numVertices] vertexArray;
		delete[sizeof(short) * numIndices] indexArray;
	}

	void fillVBuff()
	{
		VOID* buffPtr;
		vBuff->Lock(0, numVertices * stride, (VOID**)&buffPtr, D3DLOCK_DISCARD);
		memcpy(buffPtr, vertexArray, numVertices * stride);
		vBuff->Unlock();
	}

	void createVBuff(LPDIRECT3DDEVICE9 dxDevice, void* vds)
	{
		dxDevice->CreateVertexBuffer(numVertices * stride, D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &vBuff, NULL);

		vertexArray = malloc(stride * numVertices);
		memcpy(vertexArray, vds, numVertices * stride);

		fillVBuff();
	}

	void fillIBuff()
	{
		VOID* buffPtr;
		iBuff->Lock(0, numIndices * sizeof (short), (VOID**)&buffPtr, D3DLOCK_DISCARD);
		memcpy(buffPtr, indexArray, numIndices * sizeof (short));
		iBuff->Unlock();
	}

	void createIBuff(LPDIRECT3DDEVICE9 dxDevice, short* ids)
	{
		dxDevice->CreateIndexBuffer(numIndices * sizeof (short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &iBuff, NULL);

		indexArray = (short*)malloc(sizeof (short) * numIndices);
		memcpy(indexArray, ids, numIndices * sizeof (short));

		fillIBuff();
	}
};

// flat rendered terrain
struct UNCRZ_frain
{
	std::string name;

	std::vector<UNCRZ_frion*> frions;

	UNCRZ_effect effect;

	D3DXHANDLE flatTech;
	D3DXHANDLE tech;
	D3DXHANDLE lightTech;
	D3DXHANDLE dynamicDecalTech;

	LPDIRECT3DVERTEXBUFFER9 vBuff;
	LPDIRECT3DVERTEXDECLARATION9 vertexDec;
	LPDIRECT3DINDEXBUFFER9 iBuff;
	DWORD vertexType;
	UINT stride;
	int numVertices;
	int numIndices;

	void* vertexArray; // array of vertices
	short* indexArray; // array of indicies

	//std::vector<UNCRZ_decal*> decals; TODO: write new decal management for frions

	float xCorner;
	float zCorner;
	int width;
	int height;
	float dimension;

	UNCRZ_frain()
	{
	}

	UNCRZ_frain(std::string nameN)
	{
		name = nameN;
	}

	void draw(LPDIRECT3DDEVICE9 dxDevice, drawData* ddat, DWORD drawArgs)
	{
		int drawLevel = 0;

		for (int i = frions.size() - 1; i >= 0; i--)
		{
			if (frions[i]->drawLevel == drawLevel)
			{
				frions[i]->draw(dxDevice, ddat, drawArgs);
			}
		}
	}

	void destroy()
	{
		int len = width * height;
		vBuff->Release();
		iBuff->Release();
	}

	vertexPTW4* getPTW4(int x, int z)
	{
		return (((vertexPTW4*)vertexArray) + x + z * width);
	}

	vertexPTW4* getPTW4(int index)
	{
		int x = index % width;
		int z = (index - x) / width;
		if (x < 0 || z < 0 || x >= width || z >= height)
			return NULL;
		return (((vertexPTW4*)vertexArray) + x + z * width);
	}

	vertexPTW4* getPTW4(int index, int* x, int* z)
	{
		*x = index % width;
		*z = (index - *x) / width;
		return (((vertexPTW4*)vertexArray) + *x + *z * width);
	}

	int getX(int index)
	{
		return index % width;
	}

	int getZ(int index)
	{
		int x = index % width;
		return (index - x) / width;
	}

	// just modifies the y prop, autoGenNormals and fillVBuff should be done externally
	void level(int sx, int sz, int ex, int ez, float height, float coof)
	{
		for (int i = sx; i <= ex; i++)
		{
			for (int j = sz; j <= ez; j++)
			{
				vertexPTW4* vPTW4 = getPTW4(i, j);
				vPTW4->y = height * coof + (1.0 - coof) * vPTW4->y;
				//(((vertexPTW4*)vertexArray)[i + j * width]).y = height + (1.0 - coof) * ((((vertexPTW4*)vertexArray)[i + j * width]).y - height);
			}
		}
	}

	bool collidesVert(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes)
	{
		if (vertexType == VX_PTW4)
		{
			return collidesVertVX_PTW4(rayPos, rayDir, distRes);
		}
		return false;
	}

	bool collidesVertVX_PTW4(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes)
	{
		float widthmo = width - 1; // this makes sense, don't doubt it

		// 2 indicies per sqr
		int rx = (rayPos->x - xCorner) / dimension; // roughX
		int rz = (rayPos->z - zCorner) / dimension; // roughZ
		int offset = (rx + widthmo * rz) * 6;

		float uRes, vRes;
		vertexPTW4 a;
		vertexPTW4 b;
		vertexPTW4 c;
		D3DXVECTOR4 va;
		D3DXVECTOR4 vb;
		D3DXVECTOR4 vc;

		bool victory = false;
		float bestRes;
		float tempRes;

		for (int j = offset - widthmo * 6 * 2; j < offset + widthmo * 6 * 2; j += widthmo * 6)
		{
			for (int i = j - 12; i < j + 12; i += 3)
			{
				if (i < 0)
					continue;
				if (i >= widthmo * height * 6)
					continue;
				a = (((vertexPTW4*)vertexArray)[(int)indexArray[i]]);
				b = (((vertexPTW4*)vertexArray)[(int)indexArray[i + 1]]);
				c = (((vertexPTW4*)vertexArray)[(int)indexArray[i + 2]]);

				va = D3DXVECTOR4(a.x, a.y, a.z, 1.0f);
				vb = D3DXVECTOR4(b.x, b.y, b.z, 1.0f);
				vc = D3DXVECTOR4(c.x, c.y, c.z, 1.0f);

				if (D3DXIntersectTri((D3DXVECTOR3*)&va, (D3DXVECTOR3*)&vb, (D3DXVECTOR3*)&vc, rayPos, rayDir, &uRes, &vRes, &tempRes))
				{
					if (victory == false)
					{
						victory = true;
						bestRes = tempRes;
					}
					else if (tempRes < bestRes)
					{
						bestRes = tempRes;
					}
				}
				//if (D3DXIntersectTri((D3DXVECTOR3*)&va, (D3DXVECTOR3*)&vb, (D3DXVECTOR3*)&vc, rayPos, rayDir, &uRes, &vRes, distRes))
				//	return true;
			}
		}

		if (victory)
		{
			*distRes = bestRes;
			return true;
		}
		return false;
	}

	int collidesVertex(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes)
	{
		if (vertexType == VX_PTW4)
		{
			return collidesVertexVX_PTW4(rayPos, rayDir, distRes);
		}

		return false;
	}

	bool collides(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes)
	{
		if (vertexType == VX_PTW4)
		{
			return collidesVX_PTW4(rayPos, rayDir, distRes);
		}

		return false;
	}

	int collidesVertexVX_PTW4(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes)
	{
		float uRes, vRes;
		vertexPTW4 a;
		vertexPTW4 b;
		vertexPTW4 c;
		D3DXVECTOR4 va;
		D3DXVECTOR4 vb;
		D3DXVECTOR4 vc;

		bool victory = false;
		int indexRes;
		float bestRes;
		float tempRes;

		for (int i = 0; i < numIndices; i += 3)
		{
			a = (((vertexPTW4*)vertexArray)[(int)indexArray[i]]);
			b = (((vertexPTW4*)vertexArray)[(int)indexArray[i + 1]]);
			c = (((vertexPTW4*)vertexArray)[(int)indexArray[i + 2]]);

			va = D3DXVECTOR4(a.x, a.y, a.z, 1.0f);
			vb = D3DXVECTOR4(b.x, b.y, b.z, 1.0f);
			vc = D3DXVECTOR4(c.x, c.y, c.z, 1.0f);

			if (D3DXIntersectTri((D3DXVECTOR3*)&va, (D3DXVECTOR3*)&vb, (D3DXVECTOR3*)&vc, rayPos, rayDir, &uRes, &vRes, &tempRes))
			{
				if (victory == false || (victory && tempRes < bestRes))
				{
					victory = true;
					bestRes = tempRes;
					
					if (uRes < 0.5f && vRes < 0.5f)
						indexRes = (int)indexArray[i];
					else if (uRes > vRes)
						indexRes = indexArray[i + 1];
					else
						indexRes = indexArray[i + 2];
				}
			}
			//if (D3DXIntersectTri((D3DXVECTOR3*)&va, (D3DXVECTOR3*)&vb, (D3DXVECTOR3*)&vc, rayPos, rayDir, &uRes, &vRes, distRes))
			//{
			//	if (uRes < 0.5f && vRes < 0.5f)
			//		return (int)indexArray[i];
			//	if (uRes > vRes)
			//		return (int)indexArray[i + 1];
			//	return (int)indexArray[i + 2];
			//}
		}

		if (victory)
		{
			*distRes = bestRes;
			return indexRes;
		}
		return -1;
	}

	bool collidesVX_PTW4(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes)
	{
		float uRes, vRes;
		vertexPTW4 a;
		vertexPTW4 b;
		vertexPTW4 c;
		D3DXVECTOR4 va;
		D3DXVECTOR4 vb;
		D3DXVECTOR4 vc;

		bool victory = false;
		float bestRes;
		float tempRes;

		for (int i = 0; i < numIndices; i += 3)
		{
			a = (((vertexPTW4*)vertexArray)[(int)indexArray[i]]);
			b = (((vertexPTW4*)vertexArray)[(int)indexArray[i + 1]]);
			c = (((vertexPTW4*)vertexArray)[(int)indexArray[i + 2]]);

			va = D3DXVECTOR4(a.x, a.y, a.z, 1.0f);
			vb = D3DXVECTOR4(b.x, b.y, b.z, 1.0f);
			vc = D3DXVECTOR4(c.x, c.y, c.z, 1.0f);

			if (D3DXIntersectTri((D3DXVECTOR3*)&va, (D3DXVECTOR3*)&vb, (D3DXVECTOR3*)&vc, rayPos, rayDir, &uRes, &vRes, &tempRes))
			{
				if (victory == false)
				{
					victory = true;
					bestRes = tempRes;
				}
				else if (tempRes < bestRes)
				{
					bestRes = tempRes;
				}
			}
			//if (D3DXIntersectTri((D3DXVECTOR3*)&va, (D3DXVECTOR3*)&vb, (D3DXVECTOR3*)&vc, rayPos, rayDir, &uRes, &vRes, distRes))
			//	return true;
		}

		if (victory)
		{
			*distRes = bestRes;
			return true;
		}
		return false;
	}
};

struct strPair
{
public:
	std::string gin;
	std::string rpl;

	strPair() { }

	strPair(std::string ginN, std::string rplN)
	{
		gin = ginN;
		rpl = rplN;
	}
};

struct iOff
{
public:
	std::string name;
	int i;

	iOff() { }

	iOff(std::string nameN, int iN)
	{
		name = nameN;
		i = iN;
	}
};

short stoIndex(std::string str, std::vector<iOff>* iOffs)
{
	std::vector<std::string> data = split(str, "+");
	if (data.size() > 1)
	{
		short f;
		for (int i = iOffs->size() - 1; i >= 0; i--)
		{
			if (data[0] == iOffs->at(i).name)
			{
				f = iOffs->at(i).i;
				break;
			}
		}
		f += atoi(data[1].c_str());
		return f;
	}
	return atoi(str.c_str());
}

void autoGenNormals(void*, short*, int, int, DWORD, std::vector<vertexPC>*);

struct UNCRZ_font
{
public:
	std::string name;

	LPD3DXFONT font;

	UNCRZ_font();
	UNCRZ_font(std::string, LPD3DXFONT);
};

UNCRZ_font::UNCRZ_font()
{

}

UNCRZ_font::UNCRZ_font(std::string nameN, LPD3DXFONT fontN)
{
	name = nameN;
	font = fontN;
}

void createFont(LPDIRECT3DDEVICE9 dxDevice, int size, std::string name, LPD3DXFONT* dest, std::vector<UNCRZ_font*>* fontList)
{
	for (int i = fontList->size() - 1; i >= 0; i--)
	{
		if (fontList->at(i)->name == name)
		{
			*dest = fontList->at(i)->font;
			return;
		}
	}

	D3DXCreateFont(dxDevice, size, 0, FW_NORMAL, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Tahoma", dest);
	UNCRZ_font* font = new UNCRZ_font(name, *dest);
	fontList->push_back(font);
}

UNCRZ_effect createEffect(LPDIRECT3DDEVICE9 dxDevice, char* fileName, DWORD vertexType, std::string name, std::vector<UNCRZ_effect>* effectList)
{
	for (int i = effectList->size() - 1; i >= 0; i--)
	{
		if (effectList->at(i).name == name)
		{
			return effectList->at(i);
		}
	}

	UNCRZ_effect res = UNCRZ_effect::UNCRZ_effectFromFile(fileName, dxDevice, vertexType, name);
	effectList->push_back(res);
	return res;
}

void loadSpritesFromFile(char* fileName, LPDIRECT3DDEVICE9 dxDevice, std::vector<UNCRZ_sprite*>* spriteList, std::vector<UNCRZ_effect>* effectList, std::vector<UNCRZ_texture*>* textureList, std::vector<UNCRZ_matrix*>* matrixList)
{
	UNCRZ_sprite* curSprite;

	std::ifstream file(fileName);
	if (file.is_open())
	{
		while (file.good())
		{
			std::string line;
			std::getline(file, line);
			int ci;
			if ((ci = line.find("//")) != -1)
			{
				line.erase(ci, line.length() - ci);
			}
			if (line == "")
				continue;

			line = trim(line);

			std::vector<std::string> data = split(line, " ");

			if (data.size() > 0)
			{
				if (data[0] == "end")
				{
					if (data[1] == "sprite")
					{
						spriteList->push_back(curSprite);
					}
				}
				else if (data[0] == "sprite")
				{
					curSprite = new UNCRZ_sprite(data[1]);
				}
				else if (data[0] == "mat")
				{
					int idx = stoi(data[1]);
					curSprite->mats[idx] = getMatrix((char*)data[2].c_str(), matrixList);
				}
				else if (data[0] == "texture")
				{
					createTexture(dxDevice, (char*)line.substr(8).c_str(), &curSprite->tex, textureList);
					curSprite->useTex = true;
				}
				else if (data[0] == "texture0")
				{
					createTexture(dxDevice, (char*)line.substr(9).c_str(), &curSprite->tex0, textureList);
					curSprite->useTex0 = true;
				}
				else if (data[0] == "texture1")
				{
					createTexture(dxDevice, (char*)line.substr(9).c_str(), &curSprite->tex1, textureList);
					curSprite->useTex1 = true;
				}
				else if (data[0] == "texture2")
				{
					createTexture(dxDevice, (char*)line.substr(9).c_str(), &curSprite->tex2, textureList);
					curSprite->useTex2 = true;
				}
				else if (data[0] == "texture3")
				{
					createTexture(dxDevice, (char*)line.substr(9).c_str(), &curSprite->tex3, textureList);
					curSprite->useTex3 = true;
				}
				else if (data[0] == "dim")
				{
					curSprite->dimX = stof(data[1]);
					curSprite->dimY = stof(data[2]);
					curSprite->dimZ = stof(data[3]);
				}
				else if (data[0] == "colmod")
				{
					curSprite->colMod = D3DXVECTOR4(stof(data[1]), stof(data[2]), stof(data[3]), stof(data[4])); 
				}
				else if (data[0] == "alpha")
				{
					if (data[1] == "none")
						curSprite->alphaMode = AM_none;
					else if (data[1] == "nice")
						curSprite->alphaMode = AM_nice;
					else if (data[1] == "add")
						curSprite->alphaMode = AM_add;
				}
				else if (data[0] == "sidewisealpha")
				{
					if (data[1] == "none")
						curSprite->sideWiseAlphaMode = AM_none;
					else if (data[1] == "nice")
						curSprite->sideWiseAlphaMode = AM_nice;
					else if (data[1] == "add")
						curSprite->sideWiseAlphaMode = AM_add;
				}
				else if (data[0] == "lighting")
				{
					if (data[1] == "none")
					{
						curSprite->lightingMode = LM_none;
					}
					else if (data[1] == "full")
					{
						curSprite->lightingMode = LM_full;
					}
				}
				else if (data[0] == "shader_dx9")
				{
					std::string effectName = line.substr(11);
					for (int i = effectList->size() - 1; i >= 0; i--)
					{
						if ((effectList->at(i).name == effectName))
						{
							curSprite->effect = effectList->at(i);
							continue;
						}
					}	
					//curSection->effect = UNCRZ_effect::UNCRZ_effectFromFile((char*)line.substr(11).c_str(), dxDevice, vertexType, line.substr(11)); 
					curSprite->effect = createEffect(dxDevice, (char*)line.substr(11).c_str(), VX_PCT, line.substr(11), effectList); 
				}
				else if (data[0] == "technique")
				{
					curSprite->tech = curSprite->effect.effect->GetTechniqueByName(data[1].c_str());
				}
				else if (data[0] == "technique_light")
				{
					curSprite->lightTech = curSprite->effect.effect->GetTechniqueByName(data[1].c_str());
				}
				else if (data[0] == "technique_over")
				{
					curSprite->overTech = curSprite->effect.effect->GetTechniqueByName(data[1].c_str());
				}
			}
		}
	}
}

struct lpTti
{
	std::string segName;
	int index;

	lpTti(std::string segNameN, int indexN)
	{
		segName = segNameN;
		index = indexN;
	}
};

int getSegTti(UNCRZ_model* model, std::string segOrBlendName)
{
	for (int i = model->allSegs.size() - 1; i >= 0; i--)
	{
		UNCRZ_segment* seg = model->allSegs[i];
		if (seg->name == segOrBlendName)
		{
			return seg->transIndex;
		}
		
		for (int i = seg->blends.size() - 1; i >= 0; i--)
		{
			if (seg->blends[i]->name == segOrBlendName)
			{
				return seg->blends[i]->transIndex;
			}
		}
	}
	return -1;
}

UNCRZ_segment* getSeg(UNCRZ_model* model, std::string segName)
{						
	for (int i = model->allSegs.size() - 1; i >= 0; i--)
	{
		if (model->allSegs[i]->name == segName)
		{
			return model->allSegs[i];
		}
	}
	return NULL;
}

UNCRZ_section* getSec(UNCRZ_model* model, std::string secName)
{						
	for (int i = model->sections.size() - 1; i >= 0; i--)
	{
		if (model->sections[i]->name == secName)
		{
			return model->sections[i];
		}
	}
	return NULL;
}

UNCRZ_model* getModel(std::vector<UNCRZ_model*>* modelList, std::string modelName)
{
	for (int i = modelList->size() - 1; i >= 0; i--)
	{
		if (modelList->at(i)->name == modelName)
		{
			return modelList->at(i);
		}
	}
	return NULL;
}

UNCRZ_FBF_anim* getFBF_anim(std::vector<UNCRZ_FBF_anim*>* animList, std::string animName)
{
	for (int i = animList->size() - 1; i >= 0; i--)
	{
		if (animList->at(i)->name == animName)
		{
			return animList->at(i);
		}
	}
	return NULL;
}

UNCRZ_sprite* getSprite(std::vector<UNCRZ_sprite*>* spriteList, std::string spriteName)
{
	for (int i = spriteList->size() - 1; i >= 0; i--)
	{
		if (spriteList->at(i)->name == spriteName)
		{
			return spriteList->at(i);
		}
	}
	return NULL;
}

void loadModelsFromFile(char* fileName, LPDIRECT3DDEVICE9 dxDevice, std::vector<UNCRZ_model*>* modelList, std::vector<UNCRZ_effect>* effectList, std::vector<UNCRZ_texture*>* textureList, std::vector<UNCRZ_matrix*>* matrixList, std::vector<vertexPC>* nrmVis)
{
	std::vector<strPair> reps;
	std::vector<iOff> iOffs;

	UNCRZ_model* curModel;
	UNCRZ_segment* lastSegment = NULL; // if null, means curSeg should be added to the model
	UNCRZ_segment* curSegment;
	UNCRZ_section* curSection;

	std::vector<vertexPC> vPCs;
	std::vector<vertexPCT> vPCTs;
	std::vector<short> indices;

	std::vector<lpTti> latePlaceTtis;

	int curTti = -1;
	int nextTti = 0;
	bool manualNormals = false;
	bool subOffset = false;

	DWORD vertexType = VX_PC;

	std::ifstream file(fileName);
	if (file.is_open())
	{
		while (file.good())
		{
			std::string line;
			std::getline(file, line);
			int ci;
			if ((ci = line.find("//")) != -1)
			{
				line.erase(ci, line.length() - ci);
			}
			if (line == "")
				continue;

			line = trim(line);

			if (line.length() > 4 && line.substr(0, 4) == "rep ")
			{
			}
			else
			{
				for each(strPair sp in reps)
				{
					line = replace(line, sp.gin, sp.rpl);
				}
			}

			std::vector<std::string> data = split(line, " ");

			if (data.size() > 0)
			{
				if (data[0] == "end")
				{
					if (data[1] == "mdl")
					{
						if (vertexType == VX_PC)
						{
							for (int i = latePlaceTtis.size() - 1; i >= 0; i--)
							{
								lpTti *lpt = &latePlaceTtis[i];
								vPCs[lpt->index].tti = getSegTti(curModel, lpt->segName);
							}

							if (!manualNormals)
								autoGenNormals((void*)&vPCs.front(), (short*)&indices.front(), vPCs.size(), indices.size(), VX_PC, nrmVis);

							curModel->numVertices = (int)vPCs.size();
							curModel->createVBuff(dxDevice, (void*)&vPCs.front());
						}
						else if (vertexType == VX_PCT)
						{
							for (int i = latePlaceTtis.size() - 1; i >= 0; i--)
							{
								lpTti *lpt = &latePlaceTtis[i];
								vPCTs[lpt->index].tti = getSegTti(curModel, lpt->segName);
							}

							if (!manualNormals)
								autoGenNormals((void*)&vPCTs.front(), (short*)&indices.front(), vPCTs.size(), indices.size(), VX_PCT, nrmVis);

							curModel->numVertices = (int)vPCTs.size();
							curModel->createVBuff(dxDevice, (void*)&vPCTs.front());
						}
						curModel->numIndices = (int)indices.size();
						curModel->createIBuff(dxDevice, (short*)&indices.front());
						vPCs.clear();
						vPCTs.clear();
						indices.clear();
						latePlaceTtis.clear();
						curModel->transArr = new UNCRZ_trans_arr();
						curModel->transArr->create((int)curModel->allSegs.size() * 2);
						curModel->createSegmentBoxes();
						modelList->push_back(curModel);

						for each(UNCRZ_section* ss in curModel->sections)
						{
							ss->numVertices = curModel->numVertices;
						}

						iOffs.clear();
						lastSegment = NULL;
						curSegment = NULL;
						manualNormals = false;
						subOffset = false;
					}
					else if (data[1] == "blend")
					{
						curTti = curSegment->transIndex;
					}
					else if (data[1] == "seg")
					{
						lastSegment = curSegment->parent;
						curSegment = lastSegment;
						if (curSegment != NULL)
							curTti = curSegment->transIndex;
						else
							curTti = -1; // useful to check for vertices which arn't actually latePlaced
					}
					else if (data[1] == "sec")
					{
						curSection->vLen = ((int)indices.size() - curSection->vOffset) / 3;
					}
				}
				else if (data[0] == "mdl")
				{
					curModel = new UNCRZ_model(data[1]);
					nextTti = 0;
				}
				else if (data[0] == "blend")
				{
					curTti = nextTti;
					nextTti++;
					curSegment->addBlend(data[1], curTti, stof(data[2]));
				}
				else if (data[0] == "seg")
				{
					for each (UNCRZ_segment* ts in curModel->allSegs)
					{
						if (ts->name == data[1])
						{
							curSegment = ts;
							lastSegment = curSegment;
							curTti = curSegment->transIndex;
							goto oldSeg;
						}
					}

					curSegment = new UNCRZ_segment(data[1]);
					curTti = nextTti;
					nextTti++;
					curSegment->transIndex = curTti;
					if (lastSegment == NULL)
					{
						curModel->segments.push_back(curSegment);
						curSegment->parent = NULL;
						curSegment->model = curModel;
					}
					else
					{
						lastSegment->segments.push_back(curSegment);
						curSegment->parent = lastSegment;
						curSegment->model = curModel;
					}
					lastSegment = curSegment;
					curModel->allSegs.push_back(curSegment);
oldSeg:
					continue;
				}
				else if (data[0] == "sec")
				{
					curSection = new UNCRZ_section(data[1]);

					curSection->sectionEnabled = true;
					curSection->lightingMode = LM_full;

					curSection->drawDecals = true;
					curSection->acceptDecals = true;

					curSection->alphaMode = AM_none;

					curSection->vertexType = vertexType;
					curSection->vOffset = (int)indices.size();

					curModel->sections.push_back(curSection);
				}
				else if (data[0] == "shader_dx9")
				{
					std::string effectName = line.substr(11);
					for (int i = effectList->size() - 1; i >= 0; i--)
					{
						if ((effectList->at(i).name == effectName))
						{
							curSection->effect = effectList->at(i);
							continue;
						}
					}
					//curSection->effect = UNCRZ_effect::UNCRZ_effectFromFile((char*)line.substr(11).c_str(), dxDevice, vertexType, line.substr(11)); 
					curSection->effect = createEffect(dxDevice, (char*)line.substr(11).c_str(), vertexType, line.substr(11), effectList);
					//effectList->push_back(curSection->effect); // pretty sure we don't want this (done by createEffect these days)
				}
				else if (data[0] == "mat")
				{
					int idx = stoi(data[1]);
					curSection->mats[idx] = getMatrix((char*)data[2].c_str(), matrixList);
				}
				else if (data[0] == "texture")
				{
					createTexture(dxDevice, (char*)line.substr(8).c_str(), &curSection->tex, textureList);
					curSection->useTex = true;
				}
				else if (data[0] == "texture0")
				{
					createTexture(dxDevice, (char*)line.substr(9).c_str(), &curSection->tex0, textureList);
					curSection->useTex0 = true;
				}
				else if (data[0] == "texture1")
				{
					createTexture(dxDevice, (char*)line.substr(9).c_str(), &curSection->tex1, textureList);
					curSection->useTex1 = true;
				}
				else if (data[0] == "texture2")
				{
					createTexture(dxDevice, (char*)line.substr(9).c_str(), &curSection->tex2, textureList);
					curSection->useTex2 = true;
				}
				else if (data[0] == "texture3")
				{
					createTexture(dxDevice, (char*)line.substr(9).c_str(), &curSection->tex3, textureList);
					curSection->useTex3 = true;
				}
				else if (data[0] == "colmod")
				{
					curSection->colMod = D3DXVECTOR4(stof(data[1]), stof(data[2]), stof(data[3]), stof(data[4])); 
				}
				else if (data[0] == "alpha")
				{
					if (data[1] == "none")
						curSection->alphaMode = AM_none;
					else if (data[1] == "nice")
						curSection->alphaMode = AM_nice;
					else if (data[1] == "add")
						curSection->alphaMode = AM_add;
				}
				else if (data[0] == "decals")
				{
					if (data[1] == "accept")
					{
						curSection->acceptDecals = true;
					}
					else if (data[1] == "noaccept")
					{
						curSection->acceptDecals = false;
					}
					else if (data[1] == "draw")
					{
						curSection->drawDecals = true;
					}
					else if (data[1] == "nodraw")
					{
						curSection->drawDecals = false;
					}
				}
				else if (data[0] == "dyndecals")
				{
					if (data[1] == "draw")
					{
						curSection->drawDynamicDecals = true;
					}
					else if (data[1] == "nodraw")
					{
						curSection->drawDynamicDecals = false;
					}
				}
				else if (data[0] == "lighting")
				{
					if (data[1] == "none")
					{
						curSection->lightingMode = LM_none;
					}
					else if (data[1] == "full")
					{
						curSection->lightingMode = LM_full;
					}
				}
				else if (data[0] == "manualnormals")
				{
					manualNormals = true;
				}
				else if (data[0] == "suboffset")
				{
					subOffset = true;
				}
				else if (data[0] == "v")
				{
					if (manualNormals)
					{
						if (vertexType == VX_PC)
							vPCs.push_back(vertexPC(stof(data[1]), stof(data[2]), stof(data[3]), stof(data[4]), stof(data[5]), stof(data[6]), stof(data[7]), stof(data[8]), stof(data[9]), curTti));
						else if (vertexType == VX_PCT)
							vPCTs.push_back(vertexPCT(vertexPC(stof(data[1]), stof(data[2]), stof(data[3]), stof(data[4]), stof(data[5]), stof(data[6]), stof(data[7]), stof(data[8]), stof(data[9]), curTti), stof(data[10]), stof(data[11])));
					}
					else
					{
						if (vertexType == VX_PC)
							vPCs.push_back(vertexPC(stof(data[1]), stof(data[2]), stof(data[3]), stof(data[4]), stof(data[5]), stof(data[6]), curTti));
						else if (vertexType == VX_PCT)
							vPCTs.push_back(vertexPCT(vertexPC(stof(data[1]), atof(data[2].c_str()), stof(data[3]), stof(data[4]), stof(data[5]), stof(data[6]), curTti), stof(data[7]), stof(data[8])));
					}
					if (subOffset)
					{
						if (vertexType == VX_PC)
						{
							vPCs[vPCs.size() - 1].x -= curSegment->offset.x;
							vPCs[vPCs.size() - 1].y -= curSegment->offset.y;
							vPCs[vPCs.size() - 1].z -= curSegment->offset.z;
						}
						else if (vertexType == VX_PCT)
						{
							vPCTs[vPCTs.size() - 1].x -= curSegment->offset.x;
							vPCTs[vPCTs.size() - 1].y -= curSegment->offset.y;
							vPCTs[vPCTs.size() - 1].z -= curSegment->offset.z;
						}
					}
				}
				else if (data[0] == "lpt")
				{
					int ttti = -1;
					if (vertexType == VX_PC)
					{
						if ((ttti = getSegTti(curModel, data[1])) != -1) // data[1] is segName
						{
							vPCs[vPCs.size() - 1].tti = ttti;
						}
						else
						{
							latePlaceTtis.push_back(lpTti(data[1], vPCs.size() - 1));
						}
					}
					else if (vertexType == VX_PCT)
					{
						if ((ttti = getSegTti(curModel, data[1])) != -1)
						{
							vPCTs[vPCTs.size() - 1].tti = ttti;
						}
						else
						{
							latePlaceTtis.push_back(lpTti(data[1], vPCTs.size() - 1));
						}
					}
				}
				else if (data[0] == "f")
				{
					indices.push_back(stoIndex(data[1], &iOffs));
					indices.push_back(stoIndex(data[2], &iOffs));
					indices.push_back(stoIndex(data[3], &iOffs));
				}
				else if (data[0] == "s")
				{
					int surfaceStart = stoi(data[1]);
					int colDim = stoi(data[2]);
					int numCols = stoi(data[3]);
					for (int i = 0; i < numCols - 1; i++)
					{
						for (int j = 0; j < colDim - 1; j++)
						{
							indices.push_back(surfaceStart + i * colDim + j);
							indices.push_back(surfaceStart + (i + 1) * colDim + j + 1);
							indices.push_back(surfaceStart + i * colDim + j + 1);
							
							indices.push_back(surfaceStart + i * colDim + j);
							indices.push_back(surfaceStart + (i + 1) * colDim + j);
							indices.push_back(surfaceStart + (i + 1) * colDim + j + 1);
						}
					}
				}
				else if (data[0] == "cs")
				{
					int surfaceStart = stoi(data[1]);
					int colDim = stoi(data[2]);
					int numCols = stoi(data[3]);
					for (int i = 0; i < numCols - 1; i++)
					{
						for (int j = 0; j < colDim - 1; j++)
						{
							indices.push_back(surfaceStart + i * colDim + j);
							indices.push_back(surfaceStart + (i + 1) * colDim + j + 1);
							indices.push_back(surfaceStart + i * colDim + j + 1);
							
							indices.push_back(surfaceStart + i * colDim + j);
							indices.push_back(surfaceStart + (i + 1) * colDim + j);
							indices.push_back(surfaceStart + (i + 1) * colDim + j + 1);
						}
						indices.push_back(surfaceStart + i * colDim + colDim - 1);
						indices.push_back(surfaceStart + (i + 1) * colDim);
						indices.push_back(surfaceStart + i * colDim);
					
						indices.push_back(surfaceStart + i * colDim + colDim - 1);
						indices.push_back(surfaceStart + (i + 1) * colDim + colDim - 1);
						indices.push_back(surfaceStart + (i + 1) * colDim);
					}
				}
				else if (data[0] == "offset")
				{
					curSegment->offset = D3DXVECTOR3(stof(data[1]), stof(data[2]), stof(data[3]));
				}
				else if (data[0] == "rotation")
				{
					curSegment->rotation = D3DXVECTOR3(stof(data[1]), stof(data[2]), stof(data[3]));
				}
				else if (data[0] == "technique")
				{
					curSection->tech = curSection->effect.effect->GetTechniqueByName(data[1].c_str());
				}
				else if (data[0] == "technique_light")
				{
					curSection->lightTech = curSection->effect.effect->GetTechniqueByName(data[1].c_str());
				}
				else if (data[0] == "technique_decal")
				{
					curSection->decalTech = curSection->effect.effect->GetTechniqueByName(data[1].c_str());
				}
				else if (data[0] == "technique_dyndecal")
				{
					curSection->dynamicDecalTech = curSection->effect.effect->GetTechniqueByName(data[1].c_str());
				}
				else if (data[0] == "technique_over")
				{
					curSection->overTech = curSection->effect.effect->GetTechniqueByName(data[1].c_str());
				}
				else if (data[0] == "vertex")
				{
					if (data[1] == "PC")
					{
						vertexType = VX_PC;
						curModel->vertexDec = vertexDecPC;
						curModel->stride = sizeof(vertexPC);
					}
					else if (data[1] == "PCT")
					{
						vertexType = VX_PCT;
						curModel->vertexDec = vertexDecPCT;
						curModel->stride = sizeof(vertexPCT);
					}
					curModel->vertexType = vertexType;
				}
				else if (data[0] == "rep")
				{
					// rem old
					for (int i = reps.size() - 1; i >= 0; i--)
					{
						if (reps[i].gin == data[1])
						{
							reps[i] = reps[reps.size() - 1];
							reps.pop_back();
						}
					}
					// add new
					reps.push_back(strPair(data[1], line.substr(5 + data[1].length())));
				}
				else if (data[0] == "ioff")
				{
					// rem old
					for (int i = iOffs.size() - 1; i >= 0; i--)
					{
						if (iOffs[i].name == data[1])
						{
							iOffs[i] = iOffs[iOffs.size() - 1];
							iOffs.pop_back();
						}
					}
					// add new
					if (vertexType == VX_PC)
						iOffs.push_back(iOff(data[1], (int)vPCs.size()));
					else if (vertexType == VX_PCT)
						iOffs.push_back(iOff(data[1], (int)vPCTs.size()));
				}
			}
		}
		file.close();
	}
}

void loadAnimsFromFile(char* fileName, std::vector<UNCRZ_FBF_anim*>* animList, std::vector<UNCRZ_model*>* modelList)
{
	std::vector<strPair> reps;

	UNCRZ_FBF_anim* curAnim;
	UNCRZ_model* curModel;

	std::string startMotion;
	int startDur;
	bool startSet = false;

	std::ifstream file(fileName);
	if (file.is_open())
	{
		while (file.good())
		{
			std::string line;
			std::getline(file, line);
			int ci;
			if ((ci = line.find("//")) != -1)
			{
				line.erase(ci, line.length() - ci);
			}
			if (line == "")
				continue;

			line = trim(line);

			for each(strPair sp in reps)
			{
				line = replace(line, sp.gin, sp.rpl);
			}

			std::vector<std::string> data = split(line, " ");

			if (data.size() > 0)
			{
				if (data[0] == "end")
				{
					if (data[1] == "anim")
					{
					}
					else if (data[1] == "flow")
					{
						if (startSet)
							curAnim->setFlowStart(startMotion, startDur);
						startSet = false;
					}
					else if (data[1] == "motion")
					{
						curAnim->endMotion();
					}
				}
				else if (data[0] == "anim")
				{
					curAnim = new UNCRZ_FBF_anim(data[1]);
					animList->push_back(curAnim);
				}
				else if (data[0] == "a")
				{
					curAnim->addLine(line.substr(2));
				}
				else if (data[0] == "noupdate")
				{
					curAnim->setMotionCausesUpdate(false);
				}
				else if (data[0] == "motion")
				{
					curAnim->addMotion(data[1]);
				}
				else if (data[0] == "flow")
				{
					curAnim->addFlow(data[1]);
				}
				else if (data[0] == "start")
				{
					startSet = true;
					startMotion = data[1];
					startDur = stoi(data[2]);
				}
				else if (data[0] == "duration")
				{
					curAnim->setMotionDuration(stoi(data[1]));
				}
				else if (data[0] == "model")
				{
					for (int i = modelList->size() - 1; i >= 0; i--)
					{
						if ((curModel = modelList->at(i))->name == data[1])
						{
							curAnim->model = curModel;
							curAnim->modelName = data[1];
							break;
						}
					}
				}
				else if (data[0] == "rep")
				{
					reps.push_back(strPair(data[1], line.substr(5 + data[1].length())));
				}
			}
		}
		file.close();
	}
}
void initVertexDec(LPDIRECT3DDEVICE9 dxDevice)
{
	D3DVERTEXELEMENT9 vecA[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 4 * sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 8 * sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		{ 0, 12 * sizeof(float), D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 }, // transArrIndex
		D3DDECL_END()
	};

	if (FAILED(dxDevice->CreateVertexDeclaration(vecA, &vertexDecPC)))
	{
		int i = 7;
		i++;
	}

	D3DVERTEXELEMENT9 vecB[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 4 * sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 8 * sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		{ 0, 12 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 14 * sizeof(float), D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 }, // transArrIndex
		D3DDECL_END()
	};

	if (FAILED(dxDevice->CreateVertexDeclaration(vecB, &vertexDecPCT)))
	{
		int i = 7;
		i++;
	}

	D3DVERTEXELEMENT9 vecC[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 4 * sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 8 * sizeof(float), D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		{ 0, 9 * sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 13 * sizeof(float), D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 }, // transArrIndex
		D3DDECL_END()
	};

	if (FAILED(dxDevice->CreateVertexDeclaration(vecC, &vertexDecPAT4)))
	{
		int i = 7;
		i++;
	}

	D3DVERTEXELEMENT9 vecD[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 4 * sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 8 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 10 * sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 }, // weights
		D3DDECL_END()
	};

	if (FAILED(dxDevice->CreateVertexDeclaration(vecD, &vertexDecPTW4)))
	{
		int i = 7;
		i++;
	}

	D3DVERTEXELEMENT9 vecE[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 4 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	if (FAILED(dxDevice->CreateVertexDeclaration(vecE, &vertexDecOver)))
	{
		int i = 7;
		i++;
	}
}

UNCRZ_decal* splatSquareDecalPCT(int remAge, vertexPCT* vertexArray, short* indexArray, int vOffset, int vLen, D3DXMATRIX* splatProj, D3DXVECTOR3* rayDir, float nrmCoof, UNCRZ_trans_arr* transArr)
{
	std::vector<vertexPAT4> resVertices;
	std::vector<int> resIndicies;

	vertexPCT a;
	vertexPCT b;
	vertexPCT c;
	D3DXVECTOR4 va;
	D3DXVECTOR4 vb;
	D3DXVECTOR4 vc;
	//D3DXVECTOR4 na;
	//D3DXVECTOR4 nb;
	//D3DXVECTOR4 nc;
	D3DXVECTOR3 nrm;
	D3DXVECTOR3 fnt;
	D3DXVECTOR3 bck;

	for (int i = vOffset; i < vOffset + vLen * 3; i += 3)
	{
		a = (((vertexPCT*)vertexArray)[(int)indexArray[i]]);
		b = (((vertexPCT*)vertexArray)[(int)indexArray[i + 1]]);
		c = (((vertexPCT*)vertexArray)[(int)indexArray[i + 2]]);

		//D3DXMATRIX trans;

		//va = D3DXVECTOR4(a.x, a.y, a.z, 1.0f);
		//D3DXMatrixMultiply(&trans, transArr->getValue((int)a.tti), splatProj);
		//D3DXVec4Transform(&va, &va, &trans);
		//vb = D3DXVECTOR4(b.x, b.y, b.z, 1.0f);
		//D3DXMatrixMultiply(&trans, transArr->getValue((int)b.tti), splatProj);
		//D3DXVec4Transform(&vb, &vb, &trans);
		//vc = D3DXVECTOR4(c.x, c.y, c.z, 1.0f);
		//D3DXMatrixMultiply(&trans, transArr->getValue((int)c.tti), splatProj);
		//D3DXVec4Transform(&vc, &vc, &trans);

		/*va = D3DXVECTOR4(a.x, a.y, a.z, 1.0f);
		na = D3DXVECTOR4(a.nx, a.ny, a.nz, 1.0f);
		//va += na;
		D3DXVec4Transform(&va, &va, transArr->getValue((int)a.tti));
		D3DXVec4Transform(&va, &va, splatProj);

		vb = D3DXVECTOR4(b.x, b.y, b.z, 1.0f);
		nb = D3DXVECTOR4(b.nx, b.ny, b.nz, 1.0f);
		//vb += nb;
		D3DXVec4Transform(&vb, &vb, transArr->getValue((int)b.tti));
		D3DXVec4Transform(&vb, &vb, splatProj);

		vc = D3DXVECTOR4(c.x, c.y, c.z, 1.0f);
		nc = D3DXVECTOR4(c.nx, c.ny, c.nz, 1.0f);
		//vc += nc;
		D3DXVec4Transform(&vc, &vc, transArr->getValue((int)c.tti));
		D3DXVec4Transform(&vc, &vc, splatProj);*/

		va = D3DXVECTOR4(a.x, a.y, a.z, 1.0f);
		vb = D3DXVECTOR4(b.x, b.y, b.z, 1.0f);
		vc = D3DXVECTOR4(c.x, c.y, c.z, 1.0f);

		D3DXVec4Transform(&vb, &vb, transArr->getValue((int)b.tti));
		D3DXVec4Transform(&va, &va, transArr->getValue((int)a.tti));
		D3DXVec4Transform(&vc, &vc, transArr->getValue((int)c.tti));

		// find normal
		fnt = D3DXVECTOR3(vb.x - va.x, vb.y - va.y, vb.z - va.z);
		bck = D3DXVECTOR3(vc.x - va.x, vc.y - va.y, vc.z - va.z);

		nrm.x = fnt.y * bck.z - bck.y * fnt.z;
		nrm.y = fnt.z * bck.x - bck.z * fnt.x;
		nrm.z = fnt.x * bck.y - bck.x * fnt.y;

		float mod = sqrtf(nrm.x * nrm.x + nrm.y * nrm.y + nrm.z * nrm.z);

		if (mod == 0)
			nrm = D3DXVECTOR3(0, 1, 0);
		else
		{
			nrm.x /= mod;
			nrm.y /= mod;
			nrm.z /= mod;
		}

		// dot/filter
		float aa = -D3DXVec3Dot(&nrm, rayDir);
		if (aa < 0.0)
			continue;
		aa = (1.0 - nrmCoof) + nrmCoof * aa;

		// splat proj
		D3DXVec4Transform(&va, &va, splatProj);
		D3DXVec4Transform(&vb, &vb, splatProj);
		D3DXVec4Transform(&vc, &vc, splatProj);

		// filter
		if (va.x > 1.0 && vb.x > 1.0 && vc.x > 1.0)
			continue;
		if (va.y > 1.0 && vb.y > 1.0 && vc.y > 1.0)
			continue;
		if (va.z > 1.0 && vb.z > 1.0 && vc.z > 1.0)
			continue;

		if (va.x < -1.0 && vb.x < -1.0 && vc.x < -1.0)
			continue;
		if (va.y < -1.0 && vb.y < -1.0 && vc.y < -1.0)
			continue;
		if (va.z < 0.0 && vb.z < 0.0 && vc.z < 0.0)
			continue;

		// put them all in! (x : -1, y : -1) => (u : 0, v : 0), (x : 1, y : 1) => (u : 1, v : 1)

		a.tu = va.x * 0.5 + 0.5;
		a.tv = va.y * 0.5 + 0.5;
		a.a = aa;

		b.tu = vb.x * 0.5 + 0.5;
		b.tv = vb.y * 0.5 + 0.5;
		b.a = aa;

		c.tu = vc.x * 0.5 + 0.5;
		c.tv = vc.y * 0.5 + 0.5;
		c.a = aa;

		resIndicies.push_back(indexArray[i]);
		resVertices.push_back(vertexPAT4(a, 0.0, 0.0));
		resIndicies.push_back(indexArray[i + 1]);
		resVertices.push_back(vertexPAT4(b, 0.0, 0.0));
		resIndicies.push_back(indexArray[i + 2]);
		resVertices.push_back(vertexPAT4(c, 0.0, 0.0));

		/*va = D3DXVECTOR4(a.x, a.y, a.z, 1.0f);
		na = D3DXVECTOR4(a.nx, a.ny, a.nz, 0.0f);
		na += va;
		D3DXVec4Transform(&va, &va, transArr->getValue((int)a.tti));
		D3DXVec4Transform(&na, &na, transArr->getValue((int)a.tti));
		na -= va;
		D3DXVec4Transform(&va, &va, splatProj);

		vb = D3DXVECTOR4(b.x, b.y, b.z, 1.0f);
		nb = D3DXVECTOR4(b.nx, b.ny, b.nz, 0.0f);
		nb += vb;
		D3DXVec4Transform(&vb, &vb, transArr->getValue((int)b.tti));
		D3DXVec4Transform(&nb, &nb, transArr->getValue((int)b.tti));
		nb -= vb;
		D3DXVec4Transform(&vb, &vb, splatProj);

		vc = D3DXVECTOR4(c.x, c.y, c.z, 1.0f);
		nc = D3DXVECTOR4(c.nx, c.ny, c.nz, 0.0f);
		nc += vc;
		D3DXVec4Transform(&vc, &vc, transArr->getValue((int)c.tti));
		D3DXVec4Transform(&nc, &nc, transArr->getValue((int)c.tti));
		nc -= vc;
		D3DXVec4Transform(&vc, &vc, splatProj);

		if (va.x > 1.0 && vb.x > 1.0 && vc.x > 1.0)
			continue;
		if (va.y > 1.0 && vb.y > 1.0 && vc.y > 1.0)
			continue;
		if (va.z > 1.0 && vb.z > 1.0 && vc.z > 1.0)
			continue;

		if (va.x < -1.0 && vb.x < -1.0 && vc.x < -1.0)
			continue;
		if (va.y < -1.0 && vb.y < -1.0 && vc.y < -1.0)
			continue;
		if (va.z < 0.0 && vb.z < 0.0 && vc.z < 0.0)
			continue;

		// put them all in! (x : -1, y : -1) => (u : 0, v : 0), (x : 1, y : 1) => (u : 1, v : 1)
		D3DXVECTOR3 nrm;

		nrm = D3DXVECTOR3(na.x + nb.x + nc.x, na.y + nb.y + nc.y, na.z + nb.z + nc.z);
		float nrmMod = sqrt(nrm.x * nrm.x + nrm.y * nrm.y + nrm.z * nrm.z);
		nrm.x /= nrmMod;
		nrm.y /= nrmMod;
		nrm.z /= nrmMod;
		float aa = -D3DXVec3Dot(&nrm, rayDir);
		if (aa < 0.0)
			continue;
		aa = (1.0 - nrmCoof) + nrmCoof * aa;

		a.tu = va.x * 0.5 + 0.5;
		a.tv = va.y * 0.5 + 0.5;
		a.a = aa;

		b.tu = vb.x * 0.5 + 0.5;
		b.tv = vb.y * 0.5 + 0.5;
		b.a = aa;

		c.tu = vc.x * 0.5 + 0.5;
		c.tv = vc.y * 0.5 + 0.5;
		c.a = aa;

		resVertices.push_back(vertexPAT4(a, 0.0, 0.0));
		resVertices.push_back(vertexPAT4(b, 0.0, 0.0));
		resVertices.push_back(vertexPAT4(c, 0.0, 0.0));*/


		/*D3DXVECTOR3 nrm;
		int backCount = 0;
		
		a.tu = va.x * 0.5 + 0.5;
		a.tv = va.y * 0.5 + 0.5;
		nrm = D3DXVECTOR3(a.nx, a.ny, a.nz);
		float aa = -D3DXVec3Dot(&nrm, rayDir);
		if (aa < 0.0)
		{
			aa = 0.0;
			backCount++;
		}
		a.a = aa;

		b.tu = vb.x * 0.5 + 0.5;
		b.tv = vb.y * 0.5 + 0.5;
		nrm = D3DXVECTOR3(b.nx, b.ny, b.nz);
		float ba = -D3DXVec3Dot(&nrm, rayDir);
		if (ba < 0.0)
		{
			ba = 0.0;
			backCount++;
		}
		b.a = ba;

		c.tu = vc.x * 0.5 + 0.5;
		c.tv = vc.y * 0.5 + 0.5;
		nrm = D3DXVECTOR3(c.nx, c.ny, c.nz);
		float ca = -D3DXVec3Dot(&nrm, rayDir);
		if (ca < 0.0)
		{
			ca = 0.0;
			backCount++;
		}
		c.a = ca;

		if (backCount < 3)
		{
			resVertices.push_back(a);
			resVertices.push_back(b);
			resVertices.push_back(c);
		}*/
	}

	if (resVertices.size() != 0)
		return new UNCRZ_decal(remAge, (vertexPAT4*)&resVertices.front(), (int*)&resIndicies.front(), (void*)vertexArray, VX_PCT, (int)resVertices.size() / 3);
	else
		return NULL;
}

void splatSquareDecal_Model(UNCRZ_model* mdl, LPDIRECT3DDEVICE9 dxDevice, int remAge, D3DXMATRIX* splatProj, D3DXVECTOR3* rayDir, float nrmCoof, char* textureFileName, std::vector<UNCRZ_texture*>* textures)
{
	LPDIRECT3DTEXTURE9 tex;

	createTexture(dxDevice, textureFileName, &tex, textures);

	for (int i = 0; i < mdl->sections.size(); i++)
	{
		UNCRZ_section* curSec = mdl->sections[i];

		if (curSec->acceptDecals == false)
			continue;

		if (mdl->vertexType == VX_PCT)
		{
			UNCRZ_decal* dec = splatSquareDecalPCT(remAge, (vertexPCT*)mdl->vertexArray, mdl->indexArray, curSec->vOffset, curSec->vLen, splatProj, rayDir, nrmCoof, mdl->transArr);
			if (dec != NULL)
			{
				dec->tex = tex;
				curSec->decals.push_back(dec);
			}
		}
	}
}

void simpleSplatSquareDecal_Model(UNCRZ_model* mdl, LPDIRECT3DDEVICE9 dxDevice, int remAge, D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float nrmCoof, float width, float height, float nearZ, float farZ, char* textureFileName, std::vector<UNCRZ_texture*>* textures, float rot, D3DXMATRIX* outSplatProj)
{
	D3DXVECTOR3 eyeVec = *rayPos;
	D3DXVECTOR3 targVec = eyeVec + *rayDir;
	D3DXVECTOR3 upVec(0, 1, 0);

	if (rayDir->x == 0 && rayDir->z == 0)
		upVec = D3DXVECTOR3(1, 0, 0);

	D3DXMATRIX rotM;
	D3DXMatrixRotationAxis(&rotM, rayDir, rot);
	D3DXVec3TransformNormal(&upVec, &upVec, &rotM);

	D3DXMATRIX viewM;
	D3DXMATRIX projM;
	D3DXMATRIX splatProj;

	D3DXMatrixLookAtLH(&viewM, &eyeVec, &targVec, &upVec);
	D3DXMatrixOrthoLH(&projM, width, height, nearZ, farZ);

	D3DXMatrixMultiply(&splatProj, &viewM, &projM);

	splatSquareDecal_Model(mdl, dxDevice, remAge, &splatProj, rayDir, nrmCoof, textureFileName, textures);

	*outSplatProj = splatProj;
}

UNCRZ_decal* splatSquareDecalPTW4(int remAge, vertexPTW4* vertexArray, short* indexArray, int vOffset, int vLen, D3DXMATRIX* splatProj, D3DXVECTOR3* rayDir, float nrmCoof)
{
	std::vector<vertexPAT4> resVertices;
	std::vector<int> resIndicies;

	vertexPTW4 a;
	vertexPTW4 b;
	vertexPTW4 c;
	D3DXVECTOR4 va;
	D3DXVECTOR4 vb;
	D3DXVECTOR4 vc;

	for (int i = vOffset; i < vOffset + vLen * 3; i += 3)
	{
		a = (((vertexPTW4*)vertexArray)[(int)indexArray[i]]);
		b = (((vertexPTW4*)vertexArray)[(int)indexArray[i + 1]]);
		c = (((vertexPTW4*)vertexArray)[(int)indexArray[i + 2]]);

		//D3DXMATRIX trans;

		//va = D3DXVECTOR4(a.x, a.y, a.z, 1.0f);
		//D3DXMatrixMultiply(&trans, transArr->getValue((int)a.tti), splatProj);
		//D3DXVec4Transform(&va, &va, &trans);
		//vb = D3DXVECTOR4(b.x, b.y, b.z, 1.0f);
		//D3DXMatrixMultiply(&trans, transArr->getValue((int)b.tti), splatProj);
		//D3DXVec4Transform(&vb, &vb, &trans);
		//vc = D3DXVECTOR4(c.x, c.y, c.z, 1.0f);
		//D3DXMatrixMultiply(&trans, transArr->getValue((int)c.tti), splatProj);
		//D3DXVec4Transform(&vc, &vc, &trans);

		va = D3DXVECTOR4(a.x, a.y, a.z, 1.0f);
		D3DXVec4Transform(&va, &va, splatProj);
		vb = D3DXVECTOR4(b.x, b.y, b.z, 1.0f);
		D3DXVec4Transform(&vb, &vb, splatProj);
		vc = D3DXVECTOR4(c.x, c.y, c.z, 1.0f);
		D3DXVec4Transform(&vc, &vc, splatProj);

		if (va.x > 1.0 && vb.x > 1.0 && vc.x > 1.0)
			continue;
		if (va.y > 1.0 && vb.y > 1.0 && vc.y > 1.0)
			continue;
		if (va.z > 1.0 && vb.z > 1.0 && vc.z > 1.0)
			continue;

		if (va.x < -1.0 && vb.x < -1.0 && vc.x < -1.0)
			continue;
		if (va.y < -1.0 && vb.y < -1.0 && vc.y < -1.0)
			continue;
		if (va.z < 0.0 && vb.z < 0.0 && vc.z < 0.0)
			continue;

		// put them all in! (x : -1, y : -1) => (u : 0, v : 0), (x : 1, y : 1) => (u : 1, v : 1)
				D3DXVECTOR3 nrm;

		nrm = D3DXVECTOR3((a.nx + b.nx + c.nx) / 3.0, (a.ny + b.ny + c.ny) / 3.0, (a.nz + b.nz + c.nz) / 3.0);
		float aa = -D3DXVec3Dot(&nrm, rayDir);
		if (aa < 0.0)
			continue;
		aa = (1.0 - nrmCoof) + nrmCoof * aa;

		a.tu = va.x * 0.5 + 0.5;
		a.tv = va.y * 0.5 + 0.5;

		b.tu = vb.x * 0.5 + 0.5;
		b.tv = vb.y * 0.5 + 0.5;

		c.tu = vc.x * 0.5 + 0.5;
		c.tv = vc.y * 0.5 + 0.5;

		resIndicies.push_back(indexArray[i]);
		resVertices.push_back(vertexPAT4(a, aa, a.tu, a.tv, va.z, 0.0));
		resIndicies.push_back(indexArray[i + 1]);
		resVertices.push_back(vertexPAT4(b, aa, b.tu, b.tv, vb.z, 0.0));
		resIndicies.push_back(indexArray[i + 2]);
		resVertices.push_back(vertexPAT4(c, aa, c.tu, c.tv, vc.z, 0.0));
		
		/*D3DXVECTOR3 nrm;
		int backCount = 0;

		a.tu = va.x * 0.5 + 0.5;
		a.tv = va.y * 0.5 + 0.5;
		nrm = D3DXVECTOR3(a.nx, a.ny, a.nz);
		float aa = -D3DXVec3Dot(&nrm, rayDir);
		if (aa < 0.0)
		{
			aa = 0.0;
			backCount++;
		}

		b.tu = vb.x * 0.5 + 0.5;
		b.tv = vb.y * 0.5 + 0.5;
		nrm = D3DXVECTOR3(b.nx, b.ny, b.nz);
		float ba = -D3DXVec3Dot(&nrm, rayDir);
		if (ba < 0.0)
		{
			ba = 0.0;
			backCount++;
		}

		c.tu = vc.x * 0.5 + 0.5;
		c.tv = vc.y * 0.5 + 0.5;
		nrm = D3DXVECTOR3(c.nx, c.ny, c.nz);
		float ca = -D3DXVec3Dot(&nrm, rayDir);
		if (ca < 0.0)
		{
			ca = 0.0;
			backCount++;
		}

		if (backCount < 3)
		{
			resVertices.push_back(a.quickPCT(aa, 1.0, 1.0, 1.0));
			resVertices.push_back(b.quickPCT(ba, 1.0, 1.0, 1.0));
			resVertices.push_back(c.quickPCT(ca, 1.0, 1.0, 1.0));
		}*/
	}

	if (resVertices.size() != 0)
		return new UNCRZ_decal(remAge, (vertexPAT4*)&resVertices.front(), (int*)&resIndicies.front(), (void*)vertexArray, VX_PTW4, (int)resVertices.size() / 3);
	else
		return NULL;
}

void splatSquareDecal_Terrain(UNCRZ_terrain* trn, LPDIRECT3DDEVICE9 dxDevice, int remAge, D3DXMATRIX* splatProj, D3DXVECTOR3* rayDir, float nrmCoof, char* textureFileName, std::vector<UNCRZ_texture*>* textures)
{
	if (trn->vertexType == VX_PTW4)
	{
		UNCRZ_decal* dec = splatSquareDecalPTW4(remAge, (vertexPTW4*)trn->vertexArray, trn->indexArray, 0, trn->numIndices / 3, splatProj, rayDir, nrmCoof);
		if (dec != NULL)
		{
			createTexture(dxDevice, textureFileName, &dec->tex, textures);
			trn->decals.push_back(dec);
		}
	}
}

void simpleSplatSquareDecal_Terrain(UNCRZ_terrain* trn, LPDIRECT3DDEVICE9 dxDevice, int remAge, D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float nrmCoof, float width, float height, float nearZ, float farZ, char* textureFileName, std::vector<UNCRZ_texture*>* textures, float rot, D3DXMATRIX* outSplatProj)
{
	D3DXVECTOR3 eyeVec = *rayPos;
	D3DXVECTOR3 targVec = eyeVec + *rayDir;
	D3DXVECTOR3 upVec(0, 1, 0);

	if (rayDir->x == 0 && rayDir->z == 0)
		upVec = D3DXVECTOR3(1, 0, 0);

	D3DXMATRIX rotM;
	D3DXMatrixRotationAxis(&rotM, rayDir, rot);
	D3DXVec3TransformNormal(&upVec, &upVec, &rotM);

	D3DXMATRIX viewM;
	D3DXMATRIX projM;
	D3DXMATRIX splatProj;

	D3DXMatrixLookAtLH(&viewM, &eyeVec, &targVec, &upVec);
	D3DXMatrixOrthoLH(&projM, width, height, nearZ, farZ);

	D3DXMatrixMultiply(&splatProj, &viewM, &projM);

	splatSquareDecal_Terrain(trn, dxDevice, remAge, &splatProj, rayDir, nrmCoof, textureFileName, textures);

	*outSplatProj = splatProj;
}

UNCRZ_decal* splatTriangleDecalPCT(int remAge, vertexPCT* vertexArray, short* indexArray, int vOffset, int vLen, D3DXMATRIX* splatProj, D3DXVECTOR3* rayDir, float nrmCoof, UNCRZ_trans_arr* transArr)
{
	std::vector<vertexPAT4> resVertices;
	std::vector<int> resIndicies;

	vertexPCT a;
	vertexPCT b;
	vertexPCT c;
	D3DXVECTOR4 va;
	D3DXVECTOR4 vb;
	D3DXVECTOR4 vc;
	//D3DXVECTOR4 na;
	//D3DXVECTOR4 nb;
	//D3DXVECTOR4 nc;
	D3DXVECTOR3 nrm;
	D3DXVECTOR3 fnt;
	D3DXVECTOR3 bck;

	for (int i = vOffset; i < vOffset + vLen * 3; i += 3)
	{
		a = (((vertexPCT*)vertexArray)[(int)indexArray[i]]);
		b = (((vertexPCT*)vertexArray)[(int)indexArray[i + 1]]);
		c = (((vertexPCT*)vertexArray)[(int)indexArray[i + 2]]);

		//D3DXMATRIX trans;

		//va = D3DXVECTOR4(a.x, a.y, a.z, 1.0f);
		//D3DXMatrixMultiply(&trans, transArr->getValue((int)a.tti), splatProj);
		//D3DXVec4Transform(&va, &va, &trans);
		//vb = D3DXVECTOR4(b.x, b.y, b.z, 1.0f);
		//D3DXMatrixMultiply(&trans, transArr->getValue((int)b.tti), splatProj);
		//D3DXVec4Transform(&vb, &vb, &trans);
		//vc = D3DXVECTOR4(c.x, c.y, c.z, 1.0f);
		//D3DXMatrixMultiply(&trans, transArr->getValue((int)c.tti), splatProj);
		//D3DXVec4Transform(&vc, &vc, &trans);

		/*va = D3DXVECTOR4(a.x, a.y, a.z, 1.0f);
		na = D3DXVECTOR4(a.nx, a.ny, a.nz, 1.0f);
		//va += na;
		D3DXVec4Transform(&va, &va, transArr->getValue((int)a.tti));
		D3DXVec4Transform(&va, &va, splatProj);

		vb = D3DXVECTOR4(b.x, b.y, b.z, 1.0f);
		nb = D3DXVECTOR4(b.nx, b.ny, b.nz, 1.0f);
		//vb += nb;
		D3DXVec4Transform(&vb, &vb, transArr->getValue((int)b.tti));
		D3DXVec4Transform(&vb, &vb, splatProj);

		vc = D3DXVECTOR4(c.x, c.y, c.z, 1.0f);
		nc = D3DXVECTOR4(c.nx, c.ny, c.nz, 1.0f);
		//vc += nc;
		D3DXVec4Transform(&vc, &vc, transArr->getValue((int)c.tti));
		D3DXVec4Transform(&vc, &vc, splatProj);*/

		va = D3DXVECTOR4(a.x, a.y, a.z, 1.0f);
		vb = D3DXVECTOR4(b.x, b.y, b.z, 1.0f);
		vc = D3DXVECTOR4(c.x, c.y, c.z, 1.0f);

		D3DXVec4Transform(&vb, &vb, transArr->getValue((int)b.tti));
		D3DXVec4Transform(&va, &va, transArr->getValue((int)a.tti));
		D3DXVec4Transform(&vc, &vc, transArr->getValue((int)c.tti));

		// find normal
		fnt = D3DXVECTOR3(vb.x - va.x, vb.y - va.y, vb.z - va.z);
		bck = D3DXVECTOR3(vc.x - va.x, vc.y - va.y, vc.z - va.z);

		nrm.x = fnt.y * bck.z - bck.y * fnt.z;
		nrm.y = fnt.z * bck.x - bck.z * fnt.x;
		nrm.z = fnt.x * bck.y - bck.x * fnt.y;

		float mod = sqrtf(nrm.x * nrm.x + nrm.y * nrm.y + nrm.z * nrm.z);

		if (mod == 0)
			nrm = D3DXVECTOR3(0, 1, 0);
		else
		{
			nrm.x /= mod;
			nrm.y /= mod;
			nrm.z /= mod;
		}

		// dot/filter
		float aa = -D3DXVec3Dot(&nrm, rayDir);
		if (aa < 0.0)
			continue;
		aa = (1.0 - nrmCoof) + nrmCoof * aa;

		// splat proj
		D3DXVec4Transform(&va, &va, splatProj);
		D3DXVec4Transform(&vb, &vb, splatProj);
		D3DXVec4Transform(&vc, &vc, splatProj);

		// filter
		if (va.x > 1.0 && vb.x > 1.0 && vc.x > 1.0)
			continue;
		if (va.y > 1.0 && vb.y > 1.0 && vc.y > 1.0)
			continue;
		if (va.z > 1.0 && vb.z > 1.0 && vc.z > 1.0)
			continue;

		if (va.x < -1.0 && vb.x < -1.0 && vc.x < -1.0)
			continue;
		if (va.y < -1.0 && vb.y < -1.0 && vc.y < -1.0)
			continue;
		if (va.z < 0.0 && vb.z < 0.0 && vc.z < 0.0)
			continue;

		// put them all in! (x : -1, y : -1) => (u : 0, v : 0), (x : 1, y : 1) => (u : 1, v : 1)

		a.tu = va.x * 0.5 + 0.5;
		a.tv = va.y * 0.5 + 0.5;
		a.a = aa;

		b.tu = vb.x * 0.5 + 0.5;
		b.tv = vb.y * 0.5 + 0.5;
		b.a = aa;

		c.tu = vc.x * 0.5 + 0.5;
		c.tv = vc.y * 0.5 + 0.5;
		c.a = aa;

		resIndicies.push_back(indexArray[i]);
		resVertices.push_back(vertexPAT4(a, 0.0, 0.0));
		resIndicies.push_back(indexArray[i + 1]);
		resVertices.push_back(vertexPAT4(b, 0.0, 0.0));
		resIndicies.push_back(indexArray[i + 2]);
		resVertices.push_back(vertexPAT4(c, 0.0, 0.0));

		/*va = D3DXVECTOR4(a.x, a.y, a.z, 1.0f);
		na = D3DXVECTOR4(a.nx, a.ny, a.nz, 0.0f);
		na += va;
		D3DXVec4Transform(&va, &va, transArr->getValue((int)a.tti));
		D3DXVec4Transform(&na, &na, transArr->getValue((int)a.tti));
		na -= va;
		D3DXVec4Transform(&va, &va, splatProj);

		vb = D3DXVECTOR4(b.x, b.y, b.z, 1.0f);
		nb = D3DXVECTOR4(b.nx, b.ny, b.nz, 0.0f);
		nb += vb;
		D3DXVec4Transform(&vb, &vb, transArr->getValue((int)b.tti));
		D3DXVec4Transform(&nb, &nb, transArr->getValue((int)b.tti));
		nb -= vb;
		D3DXVec4Transform(&vb, &vb, splatProj);

		vc = D3DXVECTOR4(c.x, c.y, c.z, 1.0f);
		nc = D3DXVECTOR4(c.nx, c.ny, c.nz, 0.0f);
		nc += vc;
		D3DXVec4Transform(&vc, &vc, transArr->getValue((int)c.tti));
		D3DXVec4Transform(&nc, &nc, transArr->getValue((int)c.tti));
		nc -= vc;
		D3DXVec4Transform(&vc, &vc, splatProj);

		if (va.x > 1.0 && vb.x > 1.0 && vc.x > 1.0)
			continue;
		if (va.y > 1.0 && vb.y > 1.0 && vc.y > 1.0)
			continue;
		if (va.z > 1.0 && vb.z > 1.0 && vc.z > 1.0)
			continue;

		if (va.x < -1.0 && vb.x < -1.0 && vc.x < -1.0)
			continue;
		if (va.y < -1.0 && vb.y < -1.0 && vc.y < -1.0)
			continue;
		if (va.z < 0.0 && vb.z < 0.0 && vc.z < 0.0)
			continue;

		// put them all in! (x : -1, y : -1) => (u : 0, v : 0), (x : 1, y : 1) => (u : 1, v : 1)
		D3DXVECTOR3 nrm;

		nrm = D3DXVECTOR3(na.x + nb.x + nc.x, na.y + nb.y + nc.y, na.z + nb.z + nc.z);
		float nrmMod = sqrt(nrm.x * nrm.x + nrm.y * nrm.y + nrm.z * nrm.z);
		nrm.x /= nrmMod;
		nrm.y /= nrmMod;
		nrm.z /= nrmMod;
		float aa = -D3DXVec3Dot(&nrm, rayDir);
		if (aa < 0.0)
			continue;
		aa = (1.0 - nrmCoof) + nrmCoof * aa;

		a.tu = va.x * 0.5 + 0.5;
		a.tv = va.y * 0.5 + 0.5;
		a.a = aa;

		b.tu = vb.x * 0.5 + 0.5;
		b.tv = vb.y * 0.5 + 0.5;
		b.a = aa;

		c.tu = vc.x * 0.5 + 0.5;
		c.tv = vc.y * 0.5 + 0.5;
		c.a = aa;

		resVertices.push_back(vertexPAT4(a, 0.0, 0.0));
		resVertices.push_back(vertexPAT4(b, 0.0, 0.0));
		resVertices.push_back(vertexPAT4(c, 0.0, 0.0));*/


		/*D3DXVECTOR3 nrm;
		int backCount = 0;
		
		a.tu = va.x * 0.5 + 0.5;
		a.tv = va.y * 0.5 + 0.5;
		nrm = D3DXVECTOR3(a.nx, a.ny, a.nz);
		float aa = -D3DXVec3Dot(&nrm, rayDir);
		if (aa < 0.0)
		{
			aa = 0.0;
			backCount++;
		}
		a.a = aa;

		b.tu = vb.x * 0.5 + 0.5;
		b.tv = vb.y * 0.5 + 0.5;
		nrm = D3DXVECTOR3(b.nx, b.ny, b.nz);
		float ba = -D3DXVec3Dot(&nrm, rayDir);
		if (ba < 0.0)
		{
			ba = 0.0;
			backCount++;
		}
		b.a = ba;

		c.tu = vc.x * 0.5 + 0.5;
		c.tv = vc.y * 0.5 + 0.5;
		nrm = D3DXVECTOR3(c.nx, c.ny, c.nz);
		float ca = -D3DXVec3Dot(&nrm, rayDir);
		if (ca < 0.0)
		{
			ca = 0.0;
			backCount++;
		}
		c.a = ca;

		if (backCount < 3)
		{
			resVertices.push_back(a);
			resVertices.push_back(b);
			resVertices.push_back(c);
		}*/
	}

	if (resVertices.size() != 0)
		return new UNCRZ_decal(remAge, (vertexPAT4*)&resVertices.front(), (int*)&resIndicies.front(), (void*)vertexArray, VX_PCT, (int)resVertices.size() / 3);
	else
		return NULL;
}

void splatTriangleDecal_Model(UNCRZ_model* mdl, LPDIRECT3DDEVICE9 dxDevice, int remAge, D3DXMATRIX* splatProj, D3DXVECTOR3* rayDir, float nrmCoof, char* textureFileName, std::vector<UNCRZ_texture*>* textures)
{
	LPDIRECT3DTEXTURE9 tex;

	createTexture(dxDevice, textureFileName, &tex, textures);

	for (int i = 0; i < mdl->sections.size(); i++)
	{
		UNCRZ_section* curSec = mdl->sections[i];

		if (curSec->acceptDecals == false)
			continue;

		if (mdl->vertexType == VX_PCT)
		{
			UNCRZ_decal* dec = splatTriangleDecalPCT(remAge, (vertexPCT*)mdl->vertexArray, mdl->indexArray, curSec->vOffset, curSec->vLen, splatProj, rayDir, nrmCoof, mdl->transArr);
			if (dec != NULL)
			{
				dec->tex = tex;
				curSec->decals.push_back(dec);
			}
		}
	}
}

void simpleSplatTriangleDecal_Model(UNCRZ_model* mdl, LPDIRECT3DDEVICE9 dxDevice, int remAge, D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float nrmCoof, float width, float height, float nearZ, float farZ, char* textureFileName, std::vector<UNCRZ_texture*>* textures, float rot, D3DXMATRIX* outSplatProj)
{
	D3DXVECTOR3 eyeVec = *rayPos;
	D3DXVECTOR3 targVec = eyeVec + *rayDir;
	D3DXVECTOR3 upVec(0, 1, 0);

	if (rayDir->x == 0 && rayDir->z == 0)
		upVec = D3DXVECTOR3(1, 0, 0);

	D3DXMATRIX rotM;
	D3DXMatrixRotationAxis(&rotM, rayDir, rot);
	D3DXVec3TransformNormal(&upVec, &upVec, &rotM);

	D3DXMATRIX viewM;
	D3DXMATRIX projM;
	D3DXMATRIX splatProj;

	D3DXMatrixLookAtLH(&viewM, &eyeVec, &targVec, &upVec);
	D3DXMatrixOrthoLH(&projM, width, height, nearZ, farZ);

	D3DXMatrixMultiply(&splatProj, &viewM, &projM);

	splatTriangleDecal_Model(mdl, dxDevice, remAge, &splatProj, rayDir, nrmCoof, textureFileName, textures);

	*outSplatProj = splatProj;
}

UNCRZ_decal* splatTriangleDecalPTW4(int remAge, vertexPTW4* vertexArray, short* indexArray, int vOffset, int vLen, D3DXMATRIX* splatProj, D3DXVECTOR3* rayDir, float nrmCoof)
{
	std::vector<vertexPAT4> resVertices;
	std::vector<int> resIndicies;

	vertexPTW4 a;
	vertexPTW4 b;
	vertexPTW4 c;
	D3DXVECTOR4 va;
	D3DXVECTOR4 vb;
	D3DXVECTOR4 vc;

	for (int i = vOffset; i < vOffset + vLen * 3; i += 3)
	{
		a = (((vertexPTW4*)vertexArray)[(int)indexArray[i]]);
		b = (((vertexPTW4*)vertexArray)[(int)indexArray[i + 1]]);
		c = (((vertexPTW4*)vertexArray)[(int)indexArray[i + 2]]);

		//D3DXMATRIX trans;

		//va = D3DXVECTOR4(a.x, a.y, a.z, 1.0f);
		//D3DXMatrixMultiply(&trans, transArr->getValue((int)a.tti), splatProj);
		//D3DXVec4Transform(&va, &va, &trans);
		//vb = D3DXVECTOR4(b.x, b.y, b.z, 1.0f);
		//D3DXMatrixMultiply(&trans, transArr->getValue((int)b.tti), splatProj);
		//D3DXVec4Transform(&vb, &vb, &trans);
		//vc = D3DXVECTOR4(c.x, c.y, c.z, 1.0f);
		//D3DXMatrixMultiply(&trans, transArr->getValue((int)c.tti), splatProj);
		//D3DXVec4Transform(&vc, &vc, &trans);

		va = D3DXVECTOR4(a.x, a.y, a.z, 1.0f);
		D3DXVec4Transform(&va, &va, splatProj);
		vb = D3DXVECTOR4(b.x, b.y, b.z, 1.0f);
		D3DXVec4Transform(&vb, &vb, splatProj);
		vc = D3DXVECTOR4(c.x, c.y, c.z, 1.0f);
		D3DXVec4Transform(&vc, &vc, splatProj);

		if (va.x > 1.0 && vb.x > 1.0 && vc.x > 1.0)
			continue;
		if (va.y > 1.0 && vb.y > 1.0 && vc.y > 1.0)
			continue;
		if (va.z > 1.0 && vb.z > 1.0 && vc.z > 1.0)
			continue;

		if (va.x < -1.0 && vb.x < -1.0 && vc.x < -1.0)
			continue;
		if (va.y < -1.0 && vb.y < -1.0 && vc.y < -1.0)
			continue;
		if (va.z < 0.0 && vb.z < 0.0 && vc.z < 0.0)
			continue;

		// put them all in! (x : -1, y : -1) => (u : 0, v : 0), (x : 1, y : 1) => (u : 1, v : 1)
				D3DXVECTOR3 nrm;

		nrm = D3DXVECTOR3((a.nx + b.nx + c.nx) / 3.0, (a.ny + b.ny + c.ny) / 3.0, (a.nz + b.nz + c.nz) / 3.0);
		float aa = -D3DXVec3Dot(&nrm, rayDir);
		if (aa < 0.0)
			continue;
		aa = (1.0 - nrmCoof) + nrmCoof * aa;

		a.tu = va.x * 0.5 + 0.5;
		a.tv = va.y * 0.5 + 0.5;

		b.tu = vb.x * 0.5 + 0.5;
		b.tv = vb.y * 0.5 + 0.5;

		c.tu = vc.x * 0.5 + 0.5;
		c.tv = vc.y * 0.5 + 0.5;

		resIndicies.push_back(indexArray[i]);
		resVertices.push_back(vertexPAT4(a, aa, a.tu, a.tv, va.z, 0.0));
		resIndicies.push_back(indexArray[i + 1]);
		resVertices.push_back(vertexPAT4(b, aa, b.tu, b.tv, vb.z, 0.0));
		resIndicies.push_back(indexArray[i + 2]);
		resVertices.push_back(vertexPAT4(c, aa, c.tu, c.tv, vc.z, 0.0));
		
		/*D3DXVECTOR3 nrm;
		int backCount = 0;

		a.tu = va.x * 0.5 + 0.5;
		a.tv = va.y * 0.5 + 0.5;
		nrm = D3DXVECTOR3(a.nx, a.ny, a.nz);
		float aa = -D3DXVec3Dot(&nrm, rayDir);
		if (aa < 0.0)
		{
			aa = 0.0;
			backCount++;
		}

		b.tu = vb.x * 0.5 + 0.5;
		b.tv = vb.y * 0.5 + 0.5;
		nrm = D3DXVECTOR3(b.nx, b.ny, b.nz);
		float ba = -D3DXVec3Dot(&nrm, rayDir);
		if (ba < 0.0)
		{
			ba = 0.0;
			backCount++;
		}

		c.tu = vc.x * 0.5 + 0.5;
		c.tv = vc.y * 0.5 + 0.5;
		nrm = D3DXVECTOR3(c.nx, c.ny, c.nz);
		float ca = -D3DXVec3Dot(&nrm, rayDir);
		if (ca < 0.0)
		{
			ca = 0.0;
			backCount++;
		}

		if (backCount < 3)
		{
			resVertices.push_back(a.quickPCT(aa, 1.0, 1.0, 1.0));
			resVertices.push_back(b.quickPCT(ba, 1.0, 1.0, 1.0));
			resVertices.push_back(c.quickPCT(ca, 1.0, 1.0, 1.0));
		}*/
	}

	if (resVertices.size() != 0)
		return new UNCRZ_decal(remAge, (vertexPAT4*)&resVertices.front(), (int*)&resIndicies.front(), (void*)vertexArray, VX_PTW4, (int)resVertices.size() / 3);
	else
		return NULL;
}

void splatTriangleDecal_Terrain(UNCRZ_terrain* trn, LPDIRECT3DDEVICE9 dxDevice, int remAge, D3DXMATRIX* splatProj, D3DXVECTOR3* rayDir, float nrmCoof, char* textureFileName, std::vector<UNCRZ_texture*>* textures)
{
	if (trn->vertexType == VX_PTW4)
	{
		UNCRZ_decal* dec = splatTriangleDecalPTW4(remAge, (vertexPTW4*)trn->vertexArray, trn->indexArray, 0, trn->numIndices / 3, splatProj, rayDir, nrmCoof);
		if (dec != NULL)
		{
			createTexture(dxDevice, textureFileName, &dec->tex, textures);
			trn->decals.push_back(dec);
		}
	}
}

void simpleSplatTriangleDecal_Terrain(UNCRZ_terrain* trn, LPDIRECT3DDEVICE9 dxDevice, int remAge, D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float nrmCoof, float width, float height, float nearZ, float farZ, char* textureFileName, std::vector<UNCRZ_texture*>* textures, float rot, D3DXMATRIX* outSplatProj)
{
	D3DXVECTOR3 eyeVec = *rayPos;
	D3DXVECTOR3 targVec = eyeVec + *rayDir;
	D3DXVECTOR3 upVec(0, 1, 0);

	if (rayDir->x == 0 && rayDir->z == 0)
		upVec = D3DXVECTOR3(1, 0, 0);

	D3DXMATRIX rotM;
	D3DXMatrixRotationAxis(&rotM, rayDir, rot);
	D3DXVec3TransformNormal(&upVec, &upVec, &rotM);

	D3DXMATRIX viewM;
	D3DXMATRIX projM;
	D3DXMATRIX splatProj;

	D3DXMatrixLookAtLH(&viewM, &eyeVec, &targVec, &upVec);
	D3DXMatrixOrthoLH(&projM, width, height, nearZ, farZ);

	D3DXMatrixMultiply(&splatProj, &viewM, &projM);

	splatTriangleDecal_Terrain(trn, dxDevice, remAge, &splatProj, rayDir, nrmCoof, textureFileName, textures);

	*outSplatProj = splatProj;
}


void genPCNormal(short index, vertexPC* vertexArray, short* indexArray, int numIndices, std::vector<vertexPC>* nrmVis)
{
	vertexPC* vPC = &vertexArray[index];
	D3DXVECTOR4 nrm;
	vertexPC b;
	vertexPC f;
	nrm.w = 0;

	for (int i = 0; i < numIndices; i += 3)
	{
		if (indexArray[i] == index)
		{
			f = vertexArray[indexArray[i + 1]];
			b = vertexArray[indexArray[i + 2]];
		}
		else if (indexArray[i + 1] == index)
		{
			f = vertexArray[indexArray[i + 2]];
			b = vertexArray[indexArray[i]];
		}
		else if (indexArray[i + 2] == index)
		{
			f = vertexArray[indexArray[i]];
			b = vertexArray[indexArray[i + 1]];
		}
		else
			continue;

		if (f.tti != vPC->tti || b.tti != vPC->tti)
			continue;

		f.x -= vPC->x;
		f.y -= vPC->y;
		f.z -= vPC->z;

		b.x -= vPC->x;
		b.y -= vPC->y;
		b.z -= vPC->z;

		nrm.x = f.y * b.z - b.y * f.z;
		nrm.y = f.z * b.x - b.z * f.x;
		nrm.z = f.x * b.y - b.x * f.y;

		float mod = sqrtf(nrm.x * nrm.x + nrm.y * nrm.y + nrm.z * nrm.z);

		nrm.x /= mod;
		nrm.y /= mod;
		nrm.z /= mod;
		break;
	}

	vPC->nx = nrm.x;
	vPC->ny = nrm.y;
	vPC->nz = nrm.z;
	vPC->nw = 0.0f;
}

void genPCTNormal(short index, vertexPCT* vertexArray, short* indexArray, int numIndices, std::vector<vertexPC>* nrmVis)
{
	vertexPCT* vPCT = &vertexArray[index];
	D3DXVECTOR4 nrm;
	vertexPCT b;
	vertexPCT f;
	nrm.w = 0;

	for (int i = 0; i < numIndices; i += 3)
	{
		if (indexArray[i] == index)
		{
			f = vertexArray[indexArray[i + 1]];
			b = vertexArray[indexArray[i + 2]];
		}
		else if (indexArray[i + 1] == index)
		{
			f = vertexArray[indexArray[i + 2]];
			b = vertexArray[indexArray[i]];
		}
		else if (indexArray[i + 2] == index)
		{
			f = vertexArray[indexArray[i]];
			b = vertexArray[indexArray[i + 1]];
		}
		else
			continue;

		if (f.tti != vPCT->tti || b.tti != vPCT->tti)
			continue;

		f.x -= vPCT->x;
		f.y -= vPCT->y;
		f.z -= vPCT->z;

		b.x -= vPCT->x;
		b.y -= vPCT->y;
		b.z -= vPCT->z;

		nrm.x = f.y * b.z - b.y * f.z;
		nrm.y = f.z * b.x - b.z * f.x;
		nrm.z = f.x * b.y - b.x * f.y;

		float mod = sqrtf(nrm.x * nrm.x + nrm.y * nrm.y + nrm.z * nrm.z);

		nrm.x /= mod;
		nrm.y /= mod;
		nrm.z /= mod;
		break;
	}

	vPCT->nx = nrm.x;
	vPCT->ny = nrm.y;
	vPCT->nz = nrm.z;
	vPCT->nw = 0.0f;

	nrmVis->push_back(vertexPC(vPCT->x, vPCT->y, vPCT->z, 255, 255, 0, 0));
	nrmVis->push_back(vertexPC(vPCT->x + vPCT->nx, vPCT->y + vPCT->ny, vPCT->z + vPCT->nz, 255, 255, 0, 0));
}

void genPTW4Normal(short index, vertexPTW4* vertexArray, short* indexArray, int numIndices, std::vector<vertexPC>* nrmVis)
{
	vertexPTW4* vPTW4 = &vertexArray[index];
	D3DXVECTOR4 nrm;
	vertexPTW4 b;
	vertexPTW4 f;
	nrm.w = 0;
	float mod;

	for (int i = 0; i < numIndices; i += 3)
	{
		if (indexArray[i] == index)
		{
			f = vertexArray[indexArray[i + 1]];
			b = vertexArray[indexArray[i + 2]];
		}
		else if (indexArray[i + 1] == index)
		{
			f = vertexArray[indexArray[i + 2]];
			b = vertexArray[indexArray[i]];
		}
		else if (indexArray[i + 2] == index)
		{
			f = vertexArray[indexArray[i]];
			b = vertexArray[indexArray[i + 1]];
		}
		else
			continue;

		f.x -= vPTW4->x;
		f.y -= vPTW4->y;
		f.z -= vPTW4->z;

		b.x -= vPTW4->x;
		b.y -= vPTW4->y;
		b.z -= vPTW4->z;

		nrm.x = f.y * b.z - b.y * f.z;
		nrm.y = f.z * b.x - b.z * f.x;
		nrm.z = f.x * b.y - b.x * f.y;

		mod = sqrtf(nrm.x * nrm.x + nrm.y * nrm.y + nrm.z * nrm.z);

		nrm.x /= mod;
		nrm.y /= mod;
		nrm.z /= mod;
		break;
	}

	vPTW4->nx = nrm.x;
	vPTW4->ny = nrm.y;
	vPTW4->nz = nrm.z;
	vPTW4->nw = 0.0f;

	nrmVis->push_back(vertexPC(vPTW4->x, vPTW4->y, vPTW4->z, 255, 255, 0, 0));
	nrmVis->push_back(vertexPC(vPTW4->x + vPTW4->nx, vPTW4->y + vPTW4->ny, vPTW4->z + vPTW4->nz, 255, 255, 0, 0));

	// ultra Debug only
	
	/*nrmVis.push_back(vertexPC(vPTW4->x, vPTW4->y, vPTW4->z, 255, 255, 0, 0));
	nrmVis.push_back(vertexPC(vPTW4->x + f.x, vPTW4->y + f.y, vPTW4->z + f.z, 255, 255, 0, 0));
	
	nrmVis.push_back(vertexPC(vPTW4->x, vPTW4->y, vPTW4->z, 255, 255, 0, 0));
	nrmVis.push_back(vertexPC(vPTW4->x + b.x, vPTW4->y + b.y, vPTW4->z + b.z, 255, 255, 0, 0));*/
}

void fillOutNrmsPCT(void* vertexArray, int numVertices, std::vector<vertexPC>* nrmVis)
{
	vertexPCT* vPCTs = (vertexPCT*)vertexArray;

	for (int i = 0; i < numVertices; i++)
	{
		vertexPCT* vPCT = &vPCTs[i];
		vPCT->nz = vPCT->nz;
		nrmVis->push_back(vertexPC(vPCT->x, vPCT->y, vPCT->z, 255, 255, 0, 0));
		nrmVis->push_back(vertexPC(vPCT->x + vPCT->nx, vPCT->y + vPCT->ny, vPCT->z + vPCT->nz, 255, 255, 0, 0));
	}
}

void autoGenNormals(void* vertexArray, short* indexArray, int numVertices, int numIndices, DWORD vertexType, std::vector<vertexPC>* nrmVis)
{
	vertexPC* vPCs;
	vertexPCT* vPCTs;
	vertexPTW4* vPTW4s;

start:

	if (vertexType == VX_PC)
	{
		vPCs = (vertexPC*)vertexArray;

		for (int i = 0; i < numVertices; i++)
		{
			genPCNormal(i, vPCs, indexArray, numIndices, nrmVis);
		}
	}
	else if (vertexType == VX_PCT)
	{
		vPCTs = (vertexPCT*)vertexArray;

		for (int i = 0; i < numVertices; i++)
		{
			genPCTNormal(i, vPCTs, indexArray, numIndices, nrmVis);
		}
	}
	else if (vertexType == VX_PTW4)
	{
		vPTW4s = (vertexPTW4*)vertexArray;

		for (int i = 0; i < numVertices; i++)
		{
			genPTW4Normal(i, vPTW4s, indexArray, numIndices, nrmVis);
		}
	}
	else
	{
		vertexType = VX_PTW4;
		goto start;
	}
}

struct viewTrans
{
public:
	float bbuffWidth, bbuffHeight;
	float textScaleX, textScaleY;
	float invTextScaleX, invTextScaleY;
	float winWidth, winHeight;
	float centreX, centreY;
	float scaleX, scaleY;
	float invScaleX, invScaleY;

	viewTrans() { };

	viewTrans(float bbuffSizeX, float bbuffSizeY, float winSizeX, float winSizeY)
	{
		bbuffWidth = bbuffSizeX;
		bbuffHeight = bbuffSizeY;

		winWidth = winSizeX;
		winHeight = winSizeY;

		textScaleX = bbuffWidth / winWidth;
		textScaleY = bbuffHeight / winHeight;
		invTextScaleX = 1.0 / textScaleX;
		invTextScaleY = 1.0 / textScaleY;

		centreX = winSizeX / 2.0;
		centreY = winSizeY / 2.0;
		invScaleX = centreX;
		invScaleY = -centreY;
		scaleX = 1.0 / invScaleX;
		scaleY = 1.0 / invScaleY;
	}

	float xToTextX(float x)
	{
		return x * textScaleX;
	}

	float yToTextY(float y)
	{
		return y * textScaleY;
	}

	float xToScreen(float x)
	{
		return (x - centreX) * scaleX;
	}

	float xToWindow(float x)
	{
		return x * invScaleX + centreX;
	}

	float yToScreen(float y)
	{
		return (y - centreY) * scaleY;
	}

	float yToWindow(float y)
	{
		return y * invScaleY + centreY;
	}

	float wToScreen(float w)
	{
		return w * scaleX;
	}

	float hToScreen(float h)
	{
		return h * -scaleY;
	}
};

const int UI_numkeys = 255;

// ui focusing
const DWORD UIF_lose = 0; // lose focus if I have it
const DWORD UIF_gain = 1; // gain focus if I don't have it / keep
const DWORD UIF_keep = 2; // keep focus if I don't have it
const DWORD UIF_clear = 3; // unfocus whatever is focused / lose
const DWORD UIF_keepOrClear = 4; // keep focus if I have it, otherwise clear it

// ui item action
const DWORD UIA_tick = 0; // once per frame ATM

// usertype - can spawn cmdtype events
const DWORD UIA_leftDown = 1;
const DWORD UIA_rightDown = 2;
const DWORD UIA_leftUp = 3;
const DWORD UIA_rightUp = 4;
const DWORD UIA_mouseMove = 5;
const DWORD UIA_mouseEnter = 6;
const DWORD UIA_mouseLeave = 7;
const DWORD UIA_keyDown = 8;
const DWORD UIA_keyUp = 9;

// cmdtype - should not spawn more events
const DWORD UIA_cmd = 10;
const DWORD UIA_stateChange = 11;
const DWORD UIA_gainFocus = 12;
const DWORD UIA_loseFocus = 13;

struct uiEvent
{
private:
	bool consumed;

public:
	uiItem* uii;
	DWORD action;
	DWORD* data;
	int datalen;

	uiEvent(uiItem* uiiN, DWORD actionN)
	{
		uii = uiiN;
		action = actionN;
		data = NULL;
		datalen = 0;
		consumed = false;
	}

	// copies dataN
	uiEvent(uiItem* uiiN, DWORD actionN, DWORD* dataN, int datalenN)
	{
		uii = uiiN;
		action = actionN;
		data = new DWORD[datalenN];
		for (int i = 0; i < datalenN; i++)
			data[i] = dataN[i];
		datalen = datalenN;
		consumed = false;
	}

	void consume()
	{
		consumed = true;
	}

	bool wasConsumed()
	{
		return consumed;
	}

	void release()
	{
		if (datalen > 0)
			delete[datalen](data);
	}
};

// tex alignment and modes (2 bits of horizontal, 2 bits for verticle, 2 bits for other stuff)
const DWORD TXA_horizontal = 3;
const DWORD TXA_fillh = 0;
const DWORD TXA_left = 1;
const DWORD TXA_right = 2;
const DWORD TXA_center = 3;

const DWORD TXA_verticle = 12;
const DWORD TXA_fillv = 0;
const DWORD TXA_top = 4;
const DWORD TXA_bottom = 8;
const DWORD TXA_middle = 12;

const DWORD TXA_offsetInset = 16;

const DWORD TXA_pixelOffset = 32;

// tex modes
const DWORD TXM_fit = 1; // rejects alignment, and everything else really, SIMPLE and CHEAP
const DWORD TXM_zoom = 2; // requires image dimensions, fits inside box
const DWORD TXM_flat = 3; // requires image dimensions, no scaling

struct uiEventManager
{
private:
	std::vector<uiEvent*> events;
	int top;

	void purge()
	{
		events.erase(events.begin(), events.begin() + top + 1);
		top = 0;
	}

public:
	uiEventManager()
	{
		top = 0;
	}

	// when you push an event, you better have finished using it, 'cause it's gonna get freed
	void pushEvent(uiEvent* uie)
	{
		events.push_back(uie);
	}

	uiEvent* popEvent()
	{
		if (top >= events.size())
		{
			top = events.size() - 1;
			purge();
			return NULL;
		}

		uiEvent* uie = events[top];

		if (top > 15) // no more than 16 empty spaces
			purge();
		else
			top++;

		return uie;
	}
};

struct uiItem
{
public:
	std::string name;
	bool enabled; // true initially
	bool visible; // true initially
	bool clickable; // false initially
	RECT rect;

	std::vector<uiItem*> uiItems;
	uiItem* parent;
	uiEventManager* uiem;

	bool drawChildren; // true by default
	bool updateChildren; // true by default
	bool tapChildren; // true by default

	bool focused; // false initially
	bool needsUpdate; // true initially

	// clc - this is stuff that is calculated in update()
	RECT clcRect;
	// end clc

	uiItem()
	{
		// never use
	}

	uiItem(char* nameN, uiItem* parentN, RECT rectN, uiEventManager* uiemN)
	{
		enabled = true;
		visible = true;
		clickable = false;

		drawChildren = true;
		updateChildren = true;
		tapChildren = true;

		focused = false;
		needsUpdate = true;

		name = nameN;
		parent = parentN;
		if (parent != NULL)
			parent->uiItems.push_back(this);
		rect = rectN;
		uiem = uiemN;
	}

protected:
	void registerUi(DWORD action)
	{
		uiem->pushEvent(new uiEvent(this, action));
	}

	void registerUi(DWORD action, DWORD* data, int datalen)
	{
		uiem->pushEvent(new uiEvent(this, action, data, datalen));
	}

public:

	bool getTaped(float x, float y, uiItem** tapedOut, float* xOut, float* yOut)
	{
		if (!enabled || !visible) // can't click disabled / invisble stuff
			return false;

		if (x >= clcRect.left && x <= clcRect.right && y >= clcRect.top && y <= clcRect.bottom)
		{
			if (tapChildren)
			{
				// check children - count backwards
				for (int i = uiItems.size() - 1; i >= 0; i--)
				{
					uiItem* uii = uiItems[i];
					if (uii->enabled && uii->getTaped(x, y, tapedOut, xOut, yOut))
					{
						return true; // stop on the first child to be taped
					}
				}
			}

			if (clickable)
			{
				// no children taped, return me
				*xOut = x - clcRect.left;
				*yOut = y - clcRect.top;
				*tapedOut = this;
				return true;
			}
		}

		return false;
	}

	void generateChildOffsets(float* offsetX, float* offsetY)
	{
		*offsetX = clcRect.left;
		*offsetY = clcRect.top;
	}

	virtual void updateMe(viewTrans* vt)
	{
	}

	virtual void drawMe(LPDIRECT3DDEVICE9 dxDevice)
	{
	}

	// clc time - if you want to just update this component, and not it's siblings, parents, etc. then pass the parent clcRext.left and clcRect.top
	void update(float offsetX, float offsetY, viewTrans* vt, bool force)
	{
		if (!visible)
			return;

		force = force || needsUpdate;

		if (force)
		{
			clcRect.left = rect.left + offsetX;
			clcRect.right = rect.right + offsetX;
			clcRect.top = rect.top + offsetY;
			clcRect.bottom = rect.bottom + offsetY;

			updateMe(vt);

			needsUpdate = false;
		}

		if (updateChildren)
		{
			// update children
			generateChildOffsets(&offsetX, &offsetY);
			for each (uiItem* uii in uiItems)
			{
				uii->update(offsetX, offsetY, vt, force); // stop on the first child to be taped
			}
		}
	}

	void draw(LPDIRECT3DDEVICE9 dxDevice)
	{
		if (!visible)
			return;

		drawMe(dxDevice);

		if (drawChildren)
		{
			for each (uiItem* uii in uiItems)
			{
				uii->draw(dxDevice);
			}
		}
	}

	// note that is parent == NULL initially then you will probably have this in 2 places!
	void addTo(uiItem* parentN)
	{
		// remove from old parent
		if (parent != NULL)
		{
			for (int i = 0; i < parent->uiItems.size(); i++)
			{
				if (parent->uiItems[i] == this)
					parent->uiItems.erase(parent->uiItems.begin() + i, parent->uiItems.begin() + i);
			}
		}

		parent = parentN;
		if (parent != NULL)
			parent->uiItems.push_back(this);
	}

	virtual DWORD handleUi(uiEvent* uie, bool keyDown[UI_numkeys])
	{
		return UIF_keep;
	}

	virtual DWORD handleFocusUi(uiEvent* uie, bool keyDown[UI_numkeys])
	{
		return UIF_keep;
	}

	void enfocus()
	{
		focused = true;
		registerUi(UIA_gainFocus);
	}

	void unfocus()
	{
		focused = false;
		registerUi(UIA_loseFocus);
	}
};

struct uiBlankItem : public uiItem
{
public:
	uiBlankItem()
	{
		// never use this
	}

	uiBlankItem(RECT rectN, uiEventManager* uiemN) : uiItem("", NULL, rectN, uiemN)
	{
	}

	virtual void updateMe(viewTrans *vt) override
	{
		// chill
	}

	virtual void drawMe(LPDIRECT3DDEVICE9 dxDevice) override
	{
		// chill
	}
};

struct uiTextItem : public uiItem
{
public:
	std::string text;
	LPD3DXFONT font;
	D3DCOLOR textCol;
	int textBufferSize;
	DWORD textAlign;

	// clc - this is stuff that is calculated in update()
	RECT clcTextRect;
	// end clc

	uiTextItem()
	{
		// never use this
	}

	// text constructor
	uiTextItem(LPDIRECT3DDEVICE9 dxDevice, char* nameN, uiItem* parentN, RECT rectN, uiEventManager* uiemN, char* textN, D3DCOLOR textColN, LPD3DXFONT fontN) : uiItem(nameN, parentN, rectN, uiemN)
	{
		textAlign = DT_LEFT;

		font = fontN;
		textCol = textColN;
		text = std::string(textN);
	}

	virtual void updateMe(viewTrans* vt) override
	{
		updateText(vt);
	}

	virtual void drawMe(LPDIRECT3DDEVICE9 dxDevice) override
	{
		drawText();
	}

	void updateText(viewTrans* vt)
	{
		clcTextRect.left = vt->xToTextX(clcRect.left);
		clcTextRect.right = vt->xToTextX(clcRect.right);
		clcTextRect.top = vt->yToTextY(clcRect.top);
		clcTextRect.bottom = vt->yToTextY(clcRect.bottom);
	}

	void drawText()
	{
		font->DrawTextA(NULL, text.c_str(), -1, &clcTextRect, textAlign, textCol);
	}
};

struct uiTexItem : public uiItem
{
public:
	UNCRZ_effect effect;
	D3DXHANDLE tech;
	LPDIRECT3DVERTEXDECLARATION9 vertexDec;

	bool useTex;
	LPDIRECT3DTEXTURE9 tex;
	bool useTex0;
	LPDIRECT3DTEXTURE9 tex0;
	bool useTex1;
	LPDIRECT3DTEXTURE9 tex1;
	bool useTex2;
	LPDIRECT3DTEXTURE9 tex2;
	bool useTex3;
	LPDIRECT3DTEXTURE9 tex3;

	D3DXVECTOR4 colMod;

	float texW;
	float texH;
	float texScaleX;
	float texScaleY;
	DWORD texAlign;
	DWORD texMode;
	float texHAlignOffset; // flex only
	float texVAlignOffset;

	// clc - this is stuff that is calculated in update()
	vertexPCT clcTexVerts[4];
	D3DXVECTOR4 clcTexData;
	bool clcUseTexData;
	// end clc

	void zeroIsh()
	{
		useTex = false;
		useTex0 = false;
		useTex1 = false;
		useTex2 = false;
		useTex3 = false;
	}

	uiTexItem()
	{
		// never use this
	}

	// tex construtor
	// vertexDecN must be vertexPecPCT
	uiTexItem(LPDIRECT3DDEVICE9 dxDevice, char* nameN, uiItem* parentN, LPDIRECT3DVERTEXDECLARATION9 vertexDecN, char* effectFileName, char* techName, char* texFileName, RECT rectN, uiEventManager* uiemN, std::vector<UNCRZ_effect>* effectList, std::vector<UNCRZ_texture*>* textureList) : uiItem(nameN, parentN, rectN, uiemN)
	{
		texW = -1; // means to assume we don't have this data (may be ignored)
		texH = -1;

		texMode = TXM_fit; // cheapest, doesn't need to know image dimensions, probably what everyone wants
		texAlign = TXA_fillh | TXA_fillv; // (0)
		texScaleX = 1.0f;
		texScaleY = 1.0f;
		texHAlignOffset = 0.0f;
		texVAlignOffset = 0.0f;

		zeroIsh();

		effect = createEffect(dxDevice, effectFileName, VX_PCT, effectFileName, effectList);
		tech = effect.effect->GetTechniqueByName(techName);
		loadTexture(dxDevice, TID_tex, texFileName, texFileName != NULL, textureList);
		vertexDec = vertexDecN;

		colMod = D3DXVECTOR4(0, 1, 1, 1);
	}

	// oh dear
	void loadTexture(LPDIRECT3DDEVICE9 dxDevice, DWORD texID, char* texFileName, bool enable, std::vector<UNCRZ_texture*>* textureList)
	{
		LPDIRECT3DTEXTURE9 texN = NULL;
		if (texFileName != NULL)
			createTexture(dxDevice, texFileName, &texN, textureList);
		
		loadTexture(texID, texN, enable);
	}

	void loadTexture(DWORD texID, LPDIRECT3DTEXTURE9 texN, bool enable)
	{
		switch (texID)
		{
		case TID_tex:
			loadTexture(&useTex, &tex, texN, enable);
			break;
		case TID_tex0:
			loadTexture(&useTex0, &tex0, texN, enable);
			break;
		case TID_tex1:
			loadTexture(&useTex1, &tex1, texN, enable);
			break;
		case TID_tex2:
			loadTexture(&useTex2, &tex2, texN, enable);
			break;
		case TID_tex3:
			loadTexture(&useTex3, &tex3, texN, enable);
			break;
		}
	}

	void loadTexture(bool* usePtr, LPDIRECT3DTEXTURE9* texPtr, LPDIRECT3DTEXTURE9 texN, bool enable)
	{
		*usePtr = enable;
		*texPtr = texN;
	}

	void setTextures()
	{
		if (useTex)
			effect.setTexture(tex);
		if (useTex0)
			effect.setTexture0(tex0);
		if (useTex1)
			effect.setTexture1(tex1);
		if (useTex2)
			effect.setTexture2(tex2);
		if (useTex3)
			effect.setTexture3(tex3);
	}

	// clc time - if you want to just update this component, and not it's siblings, parents, etc. then pass the parent clcRext.left and clcRect.top
	virtual void updateMe(viewTrans* vt) override
	{
		updateTex(vt);
	}

	virtual void drawMe(LPDIRECT3DDEVICE9 dxDevice) override
	{
		drawTex(dxDevice);
	}

	void updateTex(viewTrans* vt)
	{
		float left = vt->xToScreen(clcRect.left);
		float right = vt->xToScreen(clcRect.right);
		float top = vt->yToScreen(clcRect.top);
		float bottom = vt->yToScreen(clcRect.bottom);

		float tleft, tright, ttop, tbottom;

		if (texMode == TXM_fit)
		{
			// skip to answers
			tleft = 0;
			tright = 1;
			ttop = 0;
			tbottom = 1;
		}
		else if (texMode == TXM_flat || texMode == TXM_zoom)
		{
			float bw = right - left;
			float bh = top - bottom;
			float tw = vt->wToScreen(texW) * texScaleX;
			float th = vt->hToScreen(texH) * texScaleY;
			float sw = tw / bw; // suitably scaled
			float sh = th / bh;

			float hao;
			float vao;
			if (texAlign & TXA_offsetInset)
			{ // 0 - 1 is like left - right or top - bottom
				hao = texHAlignOffset * (1.0f - sw);
				vao = texVAlignOffset * (1.0f - sh);
			}
			else
			{ // 0 - 1 is like left - right or top - bottom from the topleft corner
				hao = texHAlignOffset;
				vao = texVAlignOffset;
			}

			DWORD tah = texAlign & TXA_horizontal;
			DWORD tav = texAlign & TXA_verticle;

			// zoomness
			if (texMode == TXM_zoom)
			{
				if (sw > sh)
				{
					sw = 1.0f;
					sh /= sw;
				}
				else
				{
					sw /= sh;
					sh = 1.0f;
				}
			}

			switch (tah)
			{
				case TXA_fillh:
					tleft = 0;
					tright = 1;
					break;
				case TXA_left:
					tleft = 0;
					tright = sw;
					break;
				case TXA_right:
					tleft = 1.0f - sw;
					tright = 1;
					break;
				case TXA_center:
					tleft = 0.5f - sw * 0.5;
					tright = 0.5f + sw * 0.5;
					break;
			}

			switch (tav)
			{
				case TXA_fillv:
					ttop = 0;
					tbottom = 1;
					break;
				case TXA_top:
					ttop = 0;
					tbottom = sh;
					break;
				case TXA_bottom:
					ttop = 1.0f - sh;
					tbottom = 1;
					break;
				case TXA_middle:
					ttop = 0.5f - sh * 0.5;
					tbottom = 0.5f + sh * 0.5;
					break;
			}

			// tcoords describe where the image should be, need to transform
			
			float tsx = 1.0 / (tright - tleft);
			float tsy = 1.0 / (tbottom - ttop);

			tleft += hao;
			tright += hao;
			ttop += vao;
			tbottom += vao;

			tleft = 0 - tleft * tsx;
			tright = 1.0f + (1.0f - tright) * tsx;
			ttop = 0 - ttop * tsy;
			tbottom = 1.0f + (1.0f - tbottom) * tsy;
		}

		clcTexVerts[0] = vertexPCT(vertexPC(left, top, 0, 1, 1, 1, -1), tleft, ttop); // negative tti means ignore tti
		clcTexVerts[1] = vertexPCT(vertexPC(right, top, 0, 1, 1, 1, -1), tright, ttop);
		clcTexVerts[2] = vertexPCT(vertexPC(left, bottom, 0, 1, 1, 1, -1), tleft, tbottom);
		clcTexVerts[3] = vertexPCT(vertexPC(right, bottom, 0, 1, 1, 1, -1), tright, tbottom);

		clcUseTexData = false; // default, may change below
		if (texAlign & TXA_pixelOffset)
		{
			if (texW == -1 || texH == -1)
			{
				// fix offsetness - this might need revising (currently does the job for a full screen texture, but not much else)
				clcTexData = D3DXVECTOR4(0.5 / (float)(rect.right - rect.left + 1), 0.5 / (float)(rect.bottom - rect.top + 1), 1.0 / (float)vt->bbuffWidth, 1.0 / (float)vt->bbuffHeight);
				clcUseTexData = true;
				
				for (int i = 0; i < 4; i++) // do ahead of shader
				{
					clcTexVerts[i].tu += clcTexData.x;
					clcTexVerts[i].tv += clcTexData.y;
				}
				// end of stuff that might need revising
			}
			else
			{
				// fix offsetness - this might need revising (currently does the job for a full screen texture, but not much else)
				clcTexData = D3DXVECTOR4(0.5 / texW, 0.5 / texH, 1.0 / (float)vt->bbuffWidth, 1.0 / (float)vt->bbuffHeight);
				clcUseTexData = true;
				
				for (int i = 0; i < 4; i++) // do ahead of shader
				{
					clcTexVerts[i].tu += clcTexData.x;
					clcTexVerts[i].tv += clcTexData.y;
				}
				// end of stuff that might need revising
			}
		}
	}

	void drawTex(LPDIRECT3DDEVICE9 dxDevice)
	{
		D3DXMATRIX idMat;
		D3DXMatrixIdentity(&idMat);

		dxDevice->SetVertexDeclaration(vertexDec);
		effect.setTechnique(tech);
		setTextures();
		effect.setcolMod((float*)&colMod);
		effect.setViewProj(&idMat);
		// effect.setTicker(ticker); would be nice to have this information

		if (clcUseTexData)
			effect.setTextureData((float*)&clcTexData.x);

		effect.effect->CommitChanges();

		UINT numPasses;
		effect.effect->Begin(&numPasses, 0);
		for (int i = 0; i < numPasses; i++)
		{
			effect.effect->BeginPass(i);

			dxDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, &clcTexVerts, sizeof(vertexPCT));

			effect.effect->EndPass();
		}
		effect.effect->End();
	}
};

struct uiCheckable
{
public:
	virtual void setChecked(bool checkedN)
	{
	}

	virtual bool isChecked()
	{
		return false;
	}
};

struct uiCheckItem : uiItem, uiCheckable
{
private:
	bool checked;

	uiTexItem box;
	uiTextItem label;

	LPDIRECT3DTEXTURE9 onTex;
	LPDIRECT3DTEXTURE9 offTex;

public:
	uiCheckItem()
	{
		// never use this
	}

	uiCheckItem(LPDIRECT3DDEVICE9 dxDevice, char* nameN, uiItem* parentN, char* labelText, D3DCOLOR textColN, LPD3DXFONT fontN, LPDIRECT3DVERTEXDECLARATION9 vertexDecN, char* effectFileName, char* techName, RECT rectN, uiEventManager* uiemN, std::vector<UNCRZ_effect>* effectList, std::vector<UNCRZ_texture*>* textureList) : uiItem(nameN, parentN, rectN, uiemN)
	{
		clickable = false;

		// rect doesn't matter, gets changed by updateMe() anyway
		box = uiTexItem(dxDevice, "", NULL, vertexDecN, effectFileName, techName, NULL, rect, uiem, effectList, textureList);
		box.enabled = true;
		box.colMod = D3DXVECTOR4(1, 1, 1, 1);
		box.texMode = TXM_fit;
		box.texAlign = TXA_fillh | TXA_fillv | TXA_pixelOffset;

		label = uiTextItem(dxDevice, "", NULL, rect, uiem, labelText, textColN, fontN);
		label.enabled = true;
		label.textAlign = DT_LEFT | DT_VCENTER;

		createTexture(dxDevice, "ui/checkOn.tga", &onTex, textureList);
		createTexture(dxDevice, "ui/checkOff.tga", &offTex, textureList);

		setChecked(false);
	}

	void toggleChecked()
	{
		setChecked(!checked);
	}

	virtual void setChecked(bool checkedN) override
	{
		if (checked == checkedN)
			return;

		checked = checkedN;

		if (checked)
			box.loadTexture(TID_tex, onTex, true);
		else
			box.loadTexture(TID_tex, offTex, true);
		
		registerUi(UIA_stateChange);
		needsUpdate = true;
	}

	virtual bool isChecked() override
	{
		return checked;
	}

	// clc time - if you want to just update this component, and not it's siblings, parents, etc. then pass the parent clcRext.left and clcRect.top
	virtual void updateMe(viewTrans* vt) override
	{
		int w = rect.bottom - rect.top;

		RECT boxRect;
		boxRect.left = 0;
		boxRect.right = w;
		boxRect.top = 0;
		boxRect.bottom = w;
		box.rect = boxRect;

		RECT labelRect;
		labelRect.left = w + 5;
		labelRect.right = rect.right;
		labelRect.top = 0;
		labelRect.bottom = w;
		label.rect = labelRect;

		box.update(clcRect.left, clcRect.top, vt, true);
		label.update(clcRect.left, clcRect.top, vt, true);

		// hack to make this less clickable
		clcRect = box.clcRect;
	}

	virtual void drawMe(LPDIRECT3DDEVICE9 dxDevice) override
	{
		box.draw(dxDevice);
		label.draw(dxDevice);
	}
	
	// return true if it should take focus
	virtual DWORD handleUi(uiEvent* uie, bool keyDown[UI_numkeys]) override
	{
		if (uie->action == UIA_leftDown || uie->action == UIA_rightDown)
		{
			toggleChecked();
		}

		return UIF_keep;
	}
};

struct uiRadioGroup
{
private:
	uiCheckable* checkedItem;

public:
	uiRadioGroup()
	{
		checkedItem = NULL;
	}

	void clearChecked()
	{
		if (checkedItem != NULL)
		{
			checkedItem->setChecked(false);
			checkedItem = NULL;
		}
	}

	void setCheckedItem(uiCheckable* uii)
	{
		clearChecked();
		checkedItem = uii;
	}

	void registerChecked(uiCheckable* uii, bool checked)
	{
		if (checkedItem == uii)
		{
			if (!checked)
			{
				checkedItem = NULL; // don't call clear to avoid re-calling caller
			}
		}
		else if (checked)
		{
			setCheckedItem(uii);
		}
	}

	uiCheckable* getCheckedItem()
	{
		return checkedItem;
	}
};

struct uiRadioItem : uiItem, uiCheckable
{
private:
	bool checked;

	uiTexItem box;
	uiTextItem label;

	LPDIRECT3DTEXTURE9 onTex;
	LPDIRECT3DTEXTURE9 offTex;

	uiRadioGroup* uirg;

public:
	uiRadioItem()
	{
		// never use this
	}

	uiRadioItem(LPDIRECT3DDEVICE9 dxDevice, char* nameN, uiItem* parentN, char* labelText, D3DCOLOR textColN, LPD3DXFONT fontN, LPDIRECT3DVERTEXDECLARATION9 vertexDecN, char* effectFileName, char* techName, RECT rectN, uiEventManager* uiemN, std::vector<UNCRZ_effect>* effectList, std::vector<UNCRZ_texture*>* textureList, uiRadioGroup* uirgN) : uiItem(nameN, parentN, rectN, uiemN)
	{
		clickable = false;

		// rect doesn't matter, gets changed by updateMe() anyway
		box = uiTexItem(dxDevice, "", NULL, vertexDecN, effectFileName, techName, NULL, rect, uiem, effectList, textureList);
		box.enabled = true;
		box.colMod = D3DXVECTOR4(1, 1, 1, 1);
		box.texMode = TXM_fit;
		box.texAlign = TXA_fillh | TXA_fillv | TXA_pixelOffset;

		label = uiTextItem(dxDevice, "", NULL, rect, uiem, labelText, textColN, fontN);
		label.enabled = true;
		label.textAlign = DT_LEFT | DT_VCENTER;

		createTexture(dxDevice, "ui/radioOn.tga", &onTex, textureList);
		createTexture(dxDevice, "ui/radioOff.tga", &offTex, textureList);

		uirg = uirgN;

		setChecked(false);
	}

	virtual void setChecked(bool checkedN) override
	{
		if (checked == checkedN)
			return;

		checked = checkedN;

		if (checked)
			box.loadTexture(TID_tex, onTex, true);
		else
			box.loadTexture(TID_tex, offTex, true);
		
		uirg->registerChecked(this, checked);
		registerUi(UIA_stateChange);
		needsUpdate = true;
	}

	virtual bool isChecked() override
	{
		return checked;
	}

	// clc time - if you want to just update this component, and not it's siblings, parents, etc. then pass the parent clcRext.left and clcRect.top
	virtual void updateMe(viewTrans* vt) override
	{
		int w = rect.bottom - rect.top;

		RECT boxRect;
		boxRect.left = 0;
		boxRect.right = w;
		boxRect.top = 0;
		boxRect.bottom = w;
		box.rect = boxRect;

		RECT labelRect;
		labelRect.left = w + 5;
		labelRect.right = rect.right;
		labelRect.top = 0;
		labelRect.bottom = w;
		label.rect = labelRect;

		box.update(clcRect.left, clcRect.top, vt, true);
		label.update(clcRect.left, clcRect.top, vt, true);

		// hack to make this less clickable
		clcRect = box.clcRect;
	}

	virtual void drawMe(LPDIRECT3DDEVICE9 dxDevice) override
	{
		box.draw(dxDevice);
		label.draw(dxDevice);
	}
	
	// return true if it should take focus
	virtual DWORD handleUi(uiEvent* uie, bool keyDown[UI_numkeys]) override
	{
		if (uie->action == UIA_leftDown || uie->action == UIA_rightDown)
		{
			setChecked(true);
		}

		return UIF_keep;
	}
};

struct uiTextInputItem : uiItem
{
private:
	uiTexItem box;
	uiTextItem label;

	int cursorPos;

public:
	uiTextInputItem()
	{
		// never use this
	}

	uiTextInputItem(LPDIRECT3DDEVICE9 dxDevice, char* nameN, uiItem* parentN, char* textN, D3DCOLOR textColN, LPD3DXFONT fontN, LPDIRECT3DVERTEXDECLARATION9 vertexDecN, char* effectFileName, char* techName, RECT rectN, uiEventManager* uiemN, std::vector<UNCRZ_effect>* effectList, std::vector<UNCRZ_texture*>* textureList) : uiItem(nameN, parentN, rectN, uiemN)
	{
		// rect doesn't matter, gets changed by updateMe() anyway
		box = uiTexItem(dxDevice, "", NULL, vertexDecN, effectFileName, techName, NULL, rect, uiem, effectList, textureList);
		box.enabled = true;
		box.colMod = D3DXVECTOR4(1, 1, 1, 1);
		box.texMode = TXM_fit;
		box.texAlign = TXA_fillh | TXA_fillv | TXA_pixelOffset;

		label = uiTextItem(dxDevice, "", NULL, rect, uiem, textN, textColN, fontN);
		label.enabled = true;
		label.textAlign = DT_LEFT | DT_TOP;

		cursorPos = strlen(textN);
		box.loadTexture(dxDevice, TID_tex, "ui/bland.tga", true, textureList  );
	}

	void insertText(char* ptext)
	{
		label.text = label.text.insert(cursorPos, ptext);
		cursorPos += strlen(ptext);
	}

	void removeText(int count)
	{
		if (count < 0)
		{ // backspace
			count = -count;
			if (count > cursorPos)
				count = cursorPos;
			
			label.text = label.text.erase(cursorPos - count, count);
			cursorPos -= count;
		}
		else if (count > 0)
		{ // delete
			if (count > label.text.size() - cursorPos)
				count = label.text.size() - cursorPos;
			
			label.text = label.text.erase(cursorPos, count);
		}
	}

	// clc time - if you want to just update this component, and not it's siblings, parents, etc. then pass the parent clcRext.left and clcRect.top
	virtual void updateMe(viewTrans* vt) override
	{
		int w = rect.right - rect.left;
		int h = rect.bottom - rect.top;

		RECT boxRect;
		boxRect.left = 0;
		boxRect.right = w;
		boxRect.top = 0;
		boxRect.bottom = h;
		box.rect = boxRect;

		RECT labelRect;
		labelRect.left = 2;
		labelRect.right = w - 2;
		labelRect.top = 0;
		labelRect.bottom = h;
		label.rect = labelRect;

		box.update(clcRect.left, clcRect.top, vt, true);
		label.update(clcRect.left, clcRect.top, vt, true);
	}

	virtual void drawMe(LPDIRECT3DDEVICE9 dxDevice) override
	{
		box.draw(dxDevice);

		// put in cursor if we want one
		if (focused)
			label.text = label.text.insert(cursorPos, "|");
		label.draw(dxDevice);
		if (focused)
			label.text = label.text.erase(cursorPos, 1);
	}
	
	virtual DWORD handleUi(uiEvent* uie, bool keyDown[UI_numkeys]) override
	{
		if (uie->action == UIA_leftDown || uie->action == UIA_rightDown)
		{
			needsUpdate = true;
			return UIF_gain; // get focus
		}
		else if (uie->action == UIA_keyDown)
		{
			uie->consume(); // hmm

			DWORD wParam = uie->data[0]; // copied from wndProc
			if (wParam >= 64 + 1 && wParam < 64 + 26 + 1) // letters
			{
				insertText((char*)&wParam);
			}
			else if (wParam >= 48 && wParam < 48 + 10) // numbers
			{
				insertText((char*)&wParam);
			}
			else if (wParam == VK_SPACE)
			{
				insertText(" ");
			}
			else if (wParam == VK_OEM_PERIOD)
			{
				insertText(".");
			}
			else if (wParam == VK_OEM_COMMA)
			{
				insertText(",");
			}
			else if (wParam == VK_OEM_MINUS)
			{
				insertText("-");
			}
			else if (wParam == VK_OEM_PLUS)
			{
				insertText("+");
			}
			else if (wParam == VK_DIVIDE)
			{
				insertText("/");
			}
			else if (wParam == VK_MULTIPLY)
			{
				insertText("*");
			}
			else if (wParam == VK_BACK)
			{
				removeText(-1);
			}
			else if (wParam == VK_DELETE)
			{
				removeText(1);
			}
			else if (wParam == VK_LEFT)
			{
				if (cursorPos > 0)
					cursorPos--;
			}
			else if (wParam == VK_RIGHT)
			{
				if (cursorPos < label.text.size())
					cursorPos++;
			}
			else
			{
				if (wParam == VK_ESCAPE || wParam == VK_RETURN)
					return UIF_lose;
				return UIF_keep;
			}

			registerUi(UIA_stateChange);
			return UIF_keep;
		}

		return UIF_keep;
	}
};

struct uiButtonItem : uiItem
{
private:
	uiTexItem box;
	uiTextItem label;

	LPDIRECT3DTEXTURE9 nrmTex;
	LPDIRECT3DTEXTURE9 pressTex;

	bool pressed;

public:
	uiButtonItem()
	{
		// never use this
	}

	uiButtonItem(LPDIRECT3DDEVICE9 dxDevice, char* nameN, uiItem* parentN, char* labelText, D3DCOLOR textColN, LPD3DXFONT fontN, LPDIRECT3DVERTEXDECLARATION9 vertexDecN, char* effectFileName, char* techName, RECT rectN, uiEventManager* uiemN, std::vector<UNCRZ_effect>* effectList, std::vector<UNCRZ_texture*>* textureList) : uiItem(nameN, parentN, rectN, uiemN)
	{
		// rect doesn't matter, gets changed by updateMe() anyway
		box = uiTexItem(dxDevice, "", NULL, vertexDecN, effectFileName, techName, NULL, rect, uiem, effectList, textureList);
		box.enabled = true;
		box.colMod = D3DXVECTOR4(1, 1, 1, 1);
		box.texMode = TXM_fit;
		box.texAlign = TXA_fillh | TXA_fillv | TXA_pixelOffset;

		label = uiTextItem(dxDevice, "", NULL, rect, uiem, labelText, textColN, fontN);
		label.enabled = true;
		label.textAlign = DT_CENTER | DT_VCENTER;

		createTexture(dxDevice, "ui/button.tga", &nrmTex, textureList);
		createTexture(dxDevice, "ui/buttonPress.tga", &pressTex, textureList);

		pressed = true; // so that setPressed doesn't cancel itself
		setPressed(false);
	}

private:
	void setPressed(bool pressedN)
	{
		if (pressed == pressedN)
			return;

		pressed = pressedN;
		if (pressed)
			box.loadTexture(TID_tex, pressTex, true);
		else
			box.loadTexture(TID_tex, nrmTex, true);
		needsUpdate = true;
	}

	// clc time - if you want to just update this component, and not it's siblings, parents, etc. then pass the parent clcRext.left and clcRect.top
	virtual void updateMe(viewTrans* vt) override
	{
		int w = rect.right - rect.left;
		int h = rect.bottom - rect.top;

		RECT boxRect;
		boxRect.left = 0;
		boxRect.right = w;
		boxRect.top = 0;
		boxRect.bottom = h;
		box.rect = boxRect;

		RECT labelRect;
		labelRect.left = 0;
		labelRect.right = w;
		labelRect.top = 0;
		labelRect.bottom = h;
		label.rect = labelRect;

		box.update(clcRect.left, clcRect.top, vt, true);
		label.update(clcRect.left, clcRect.top, vt, true);
	}

	virtual void drawMe(LPDIRECT3DDEVICE9 dxDevice) override
	{
		box.draw(dxDevice);
		label.draw(dxDevice);
	}
	
	virtual DWORD handleUi(uiEvent* uie, bool keyDown[UI_numkeys]) override
	{
		// make us mouseUp/mouseDown
		if (uie->action == UIA_leftDown)
		{
			if (uie->uii == this)
			{ // check it's me we are talking about
				setPressed(true);
				return UIF_gain;
			}
		}
		else if (uie->action == UIA_leftUp)
		{
			if (pressed)
			{
				setPressed(false);
				registerUi(UIA_cmd); // dispatch a cmd event
			}
		}
		else if (uie->action == UIA_loseFocus)
		{
			setPressed(false); // don't even THINK about re-requesting focus
		}
		return UIF_keep;
	}
	
	virtual DWORD handleFocusUi(uiEvent* uie, bool keyDown[UI_numkeys]) override
	{
		if (uie->action == UIA_leftUp)
		{
			setPressed(false);
		}
		return UIF_keep;
	}
};

// joy of joys!
struct uiRenderer
{
public:
	std::string name;
	int texWidth;
	int texHeight;
	LPDIRECT3DSURFACE9 targetSurface;
	LPDIRECT3DTEXTURE9 targetTex;
	LPDIRECT3DSURFACE9 zSurface;
	
	bool clearTarget;
	D3DCOLOR clearColor;

	std::vector<uiItem*> uiItems;

	uiRenderer(char* nameN, bool doClear, D3DCOLOR clearColN)
	{
		name = nameN;
		clearTarget = doClear;
		clearColor = clearColN;
	}

	void init(LPDIRECT3DDEVICE9 dxDevice, int texWidthN, int texHeightN, D3DFORMAT targetFormat)
	{
		texWidth = texWidthN;
		texHeight = texHeightN;
		D3DXCreateTexture(dxDevice, texWidth, texHeight, 0, D3DUSAGE_RENDERTARGET, targetFormat, D3DPOOL_DEFAULT, &targetTex);
		targetTex->GetSurfaceLevel(0, &targetSurface);
	}

	void init(LPDIRECT3DSURFACE9 targetSurfaceN)
	{
		targetSurface = targetSurfaceN;
	}

	void initStencil(LPDIRECT3DSURFACE9 zSurfaceN)
	{
		zSurface = zSurfaceN;
	}

	void initStencil(LPDIRECT3DDEVICE9 dxDevice)
	{
		dxDevice->CreateDepthStencilSurface(texWidth, texHeight, D3DFMT_D16, UNCRZ_STDSAMPLE, 0, TRUE, &zSurface, NULL);
	}

	void update(float offsetX, float offsetY, viewTrans* vt, bool force)
	{
		for each (uiItem* uii in uiItems)
		{
			if (uii->enabled)
				uii->update(offsetX, offsetY, vt, force);
		}
	}

	void draw(LPDIRECT3DDEVICE9 dxDevice)
	{
		dxDevice->SetRenderTarget(0, targetSurface);
		dxDevice->SetDepthStencilSurface(zSurface);

		if (clearTarget)
			dxDevice->Clear(0, NULL, D3DCLEAR_TARGET, clearColor, 1.0f, 0);
		dxDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

		dxDevice->SetRenderState(D3DRS_CLIPPING, false);

		dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		dxDevice->SetRenderState(D3DRS_ZENABLE, false);
		dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);

		dxDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		dxDevice->BeginScene();

		for each (uiItem* uii in uiItems)
		{
			uii->draw(dxDevice);
		}

		dxDevice->EndScene();
	}
};

// very much untested
struct uiRenderedItem : uiTexItem
{
public:
	uiRenderedItem()
	{
		// never use this
	}

	uiRenderedItem(LPDIRECT3DDEVICE9 dxDevice, char* nameN, int texWN, int texHN, uiItem* parentN, LPDIRECT3DVERTEXDECLARATION9 vertexDecN, char* effectFileName, char* techName, char* texFileName, RECT rectN, uiEventManager* uiemN, std::vector<UNCRZ_effect>* effectList, std::vector<UNCRZ_texture*>* textureList) : uiTexItem(dxDevice, nameN, parentN, vertexDecN, effectFileName, techName, texFileName, rectN, uiemN, effectList, textureList)
	{
		drawChildren = false; // uiRenderer should sort this out
		updateChildren = false; // uiRenderer should sort this out
		tapChildren = true;

		texW = texWN; // means to assume we don't have this data (may be ignored)
		texH = texHN;

		texMode = TXM_flat; // could be handling mis-fit textures
		texAlign = TXA_fillh | TXA_fillv; // (0)
		texScaleX = 1.0f;
		texScaleY = 1.0f;
		texHAlignOffset = 0.0f;
		texVAlignOffset = 0.0f;
	}
};

void drawData::startTimer(bool preFlushDx)
	{
		if (preFlushDx)
		{
			DEBUG_DX_FLUSH();
		}
		DEBUG_HR_START(&hridstart);
	}

void drawData::stopTimer(int genericIndex, bool flushDx, char* label, DWORD outFormat)
	{
		if (flushDx)
		{
			if (!debugFlushing)
			{
				genericLabel[genericIndex]->visible = false;
				genericBar[genericIndex]->visible = false;
				return;
			}
			DEBUG_DX_FLUSH();
		}
		float timedSecs;
		DEBUG_HR_END(&hridstart, &hridend, &timedSecs);
		
		float textScale;
		float barScale;
		if (outFormat == GDOF_ms)
		{
			textScale = 1000.0;
			barScale = 1.0;
			genericLabel[genericIndex]->text = std::string(label) + std::string(itoa((int)(timedSecs * textScale), (char*)&buff, 10)) + "ms";
			genericBar[genericIndex]->rect.right = 10 + (int)((float)(genericDebugView->rect.right - genericDebugView->rect.left - 20) * timedSecs * barScale);
		}
		else if (outFormat == GDOF_dms)
		{
			textScale = 10000.0;
			barScale = 10.0;
			genericLabel[genericIndex]->text = std::string(label) + std::string(itoa((int)(timedSecs * textScale), (char*)&buff, 10)) + "dms";
			genericBar[genericIndex]->rect.right = 10 + (int)((float)(genericDebugView->rect.right - genericDebugView->rect.left - 20) * timedSecs * barScale);
		}
		genericLabel[genericIndex]->visible = true;
		genericBar[genericIndex]->visible = true;
	}

void drawData::nontimed(int genericIndex, char* label, float value, DWORD outFormat)
	{
		float textScale;
		float barScale;
		if (outFormat == GDOF_ms)
		{
			textScale = 1000.0;
			barScale = 1.0;
			genericLabel[genericIndex]->text = std::string(label) + std::string(itoa((int)(value * textScale), (char*)&buff, 10)) + "ms";
			genericBar[genericIndex]->rect.right = 10 + (int)((float)(genericDebugView->rect.right - genericDebugView->rect.left - 20) * value * barScale);
		}
		else if (outFormat == GDOF_dms)
		{
			textScale = 10000.0;
			barScale = 10.0;
			genericLabel[genericIndex]->text = std::string(label) + std::string(itoa((int)(value * textScale), (char*)&buff, 10)) + "dms";
			genericBar[genericIndex]->rect.right = 10 + (int)((float)(genericDebugView->rect.right - genericDebugView->rect.left - 20) * value * barScale);
		}
		else if (outFormat == GDOF_prop100)
		{
			textScale = 100.0;
			barScale = 1.0;
			genericLabel[genericIndex]->text = std::string(label) + std::string(itoa((int)(value * textScale), (char*)&buff, 10)) + "%";
			genericBar[genericIndex]->rect.right = 10 + (int)((float)(genericDebugView->rect.right - genericDebugView->rect.left - 20) * value * barScale);
		}
		genericLabel[genericIndex]->visible = true;
		genericBar[genericIndex]->visible = true;
	}

const DWORD VM_persp = 0;
const DWORD VM_ortho = 1;

struct UNCRZ_view
{
public:
	std::string name;
	int texWidth;
	int texHeight;
	LPDIRECT3DSURFACE9 targetSurface;
	LPDIRECT3DTEXTURE9 targetTex;
	LPDIRECT3DSURFACE9 sideSurface;
	LPDIRECT3DTEXTURE9 sideTex;
	LPDIRECT3DSURFACE9 zTargetSurface;
	LPDIRECT3DSURFACE9 zSideSurface;
	
	DWORD viewMode;
	DWORD alphaMode; // be careful when considering using this, will almost certainly achieve nothing

	float projectionNear;
	float projectionFar;
	float dimX;
	float dimY;

	D3DXVECTOR3 camPos;
	D3DXVECTOR3 camDir;
	D3DXVECTOR3 camUp;

	std::vector<UNCRZ_obj*> zSortedObjs; // for anything that needs to be drawn back-to-front
	std::vector<int> zsoLocalIndexes; // for anything that needs to be drawn back-to-front

	D3DVIEWPORT9 viewViewPort;
	D3DXMATRIX viewViewMat;
	D3DXMATRIX viewProjMat;
	D3DXMATRIX viewViewProj; // (texAligned, x and y are 0..1)
	D3DXMATRIX viewViewProjVP; // (x and y are -1..1)
	
	bool clearView;
	D3DCOLOR clearColor;

	bool viewEnabled;

	bool useClips;
	bool useClip[maxClips];
	D3DXVECTOR4 clips[maxClips];

	UNCRZ_view(std::string nameN)
	{
		zeroIsh();

		name = nameN;

		// loads of defaults
		camPos = D3DXVECTOR3(0, 0, 0);
		camDir = D3DXVECTOR3(1, 0, 0);
		camUp = D3DXVECTOR3(0, 1, 0);

		viewMode = VM_persp;

		viewEnabled = true;
	}

	void initTarget(LPDIRECT3DDEVICE9 dxDevice, int texWidthN, int texHeightN, D3DFORMAT targetFormat)
	{
		texWidth = texWidthN;
		texHeight = texHeightN;
		D3DXCreateTexture(dxDevice, texWidth, texHeight, 0, D3DUSAGE_RENDERTARGET, targetFormat, D3DPOOL_DEFAULT, &targetTex);
		targetTex->GetSurfaceLevel(0, &targetSurface);
	}

	// for if you allready have a targetSurface (i.e. THE target surface)
	void initTarget(LPDIRECT3DSURFACE9 targetSurfaceN, int texWidthN, int texHeightN)
	{
		texWidth = texWidthN;
		texHeight = texHeightN;
		targetSurface = targetSurfaceN;
	}

	void initSide(LPDIRECT3DDEVICE9 dxDevice, D3DFORMAT sideFormat)
	{
		D3DXCreateTexture(dxDevice, texWidth, texHeight, 0, D3DUSAGE_RENDERTARGET, sideFormat, D3DPOOL_DEFAULT, &sideTex);
		sideTex->GetSurfaceLevel(0, &sideSurface);
	}

	// for if you allready have a sideSurface
	void initSide(LPDIRECT3DTEXTURE9 sideTexN, LPDIRECT3DSURFACE9 sideSurfaceN)
	{
		sideTex = sideTexN;
		sideSurface = sideSurfaceN;
	}

	void initTargetStencil(LPDIRECT3DSURFACE9 zTargetSurfaceN)
	{
		zTargetSurface = zTargetSurfaceN;
	}

	void initTargetStencil(LPDIRECT3DDEVICE9 dxDevice)
	{
		dxDevice->CreateDepthStencilSurface(texWidth, texHeight, D3DFMT_D16, UNCRZ_STDSAMPLE, 0, TRUE, &zTargetSurface, NULL);
	}

	void initSideStencil(LPDIRECT3DSURFACE9 zSideSurfaceN)
	{
		zSideSurface = zSideSurfaceN;
	}

	void initSideStencil(LPDIRECT3DDEVICE9 dxDevice)
	{
		dxDevice->CreateDepthStencilSurface(texWidth, texHeight, D3DFMT_D16, UNCRZ_STDSAMPLE, 0, TRUE, &zSideSurface, NULL);
	}

	void zeroIsh()
	{
		useClips = false;
		for (int i = 0; i < maxClips; i++)
		{
			useClip[i] = false;
		}
	}

	void dirNormalAt(D3DXVECTOR3 camTarg)
	{
		camDir = camTarg - camPos;
		float mod = sqrtf(camDir.x * camDir.x + camDir.y * camDir.y + camDir.z * camDir.z);
		camDir.x /= mod;
		camDir.y /= mod;
		camDir.z /= mod;
	}

	bool getClipEnable(int idx)
	{
		return useClip[idx];
	}

	void setClipEnable(int idx, bool enable)
	{
		useClip[idx] = enable;
	}

	void setClip(int idx, D3DXVECTOR4* clip, bool enable)
	{
		clips[idx] = *clip;
		setClipEnable(idx, enable);
	}

	void setClip(int idx, D3DXVECTOR3* nrm, float dist, bool enable)
	{
		clips[idx] = D3DXVECTOR4(nrm->x, nrm->y, nrm->z, dist);
		setClipEnable(idx, enable);
	}

	void setClip(int idx, float x, float y, float z, float dist, bool enable)
	{
		clips[idx] = D3DXVECTOR4(x, y, z, dist);
		setClipEnable(idx, enable);
	}
};

struct UNCRZ_over
{
public:
	std::string name;
	int texWidth;
	int texHeight;
	LPDIRECT3DSURFACE9 targetSurface;
	LPDIRECT3DTEXTURE9 targetTex;
	LPDIRECT3DSURFACE9 zSurface;

	DWORD alphaMode;

	UNCRZ_effect effect;
	D3DXHANDLE overTech;
	D3DXVECTOR4 colMod;

	bool useTex;
	LPDIRECT3DTEXTURE9 tex;
	bool useTex0;
	LPDIRECT3DTEXTURE9 tex0;
	bool useTex1;
	LPDIRECT3DTEXTURE9 tex1;
	bool useTex2;
	LPDIRECT3DTEXTURE9 tex2;
	bool useTex3;
	LPDIRECT3DTEXTURE9 tex3;
	
	bool clearView;
	D3DCOLOR clearColor;
	
	bool overEnabled;

	void zeroIsh()
	{
		useTex = false;
		useTex0 = false;
		useTex1 = false;
		useTex2 = false;
		useTex3 = false;
	}

	void setTextures()
	{
		if (useTex)
			effect.setTexture(tex);
		if (useTex0)
			effect.setTexture0(tex0);
		if (useTex1)
			effect.setTexture1(tex1);
		if (useTex2)
			effect.setTexture2(tex2);
		if (useTex3)
			effect.setTexture3(tex3);
	}

	UNCRZ_over(std::string nameN)
	{
		name = nameN;
		zeroIsh();
		overEnabled = true;
	}

	void init(LPDIRECT3DDEVICE9 dxDevice, int texWidthN, int texHeightN, char* effectFileName, char* techName, std::vector<UNCRZ_effect>* effectList, D3DFORMAT targetFormat)
	{
		texWidth = texWidthN;
		texHeight = texHeightN;
		D3DXCreateTexture(dxDevice, texWidth, texHeight, 0, D3DUSAGE_RENDERTARGET, targetFormat, D3DPOOL_DEFAULT, &targetTex);
		targetTex->GetSurfaceLevel(0, &targetSurface);
		
		effect = createEffect(dxDevice, effectFileName, VX_PCT, effectFileName, effectList);
		overTech = effect.effect->GetTechniqueByName(techName);
	}

	void init(LPDIRECT3DDEVICE9 dxDevice, LPDIRECT3DSURFACE9 targetSurfaceN, int texWidthN, int texHeightN, char* effectFileName, char* techName, std::vector<UNCRZ_effect>* effectList)
	{
		texWidth = texWidthN;
		texHeight = texHeightN;
		targetSurface = targetSurfaceN;
		
		effect = createEffect(dxDevice, effectFileName, VX_PCT, effectFileName, effectList);
		overTech = effect.effect->GetTechniqueByName(techName);
	}

	void initStencil(LPDIRECT3DSURFACE9 zSurfaceN)
	{
		zSurface = zSurfaceN;
	}

	void initStencil(LPDIRECT3DDEVICE9 dxDevice)
	{
		dxDevice->CreateDepthStencilSurface(texWidth, texHeight, D3DFMT_D16, UNCRZ_STDSAMPLE, 0, TRUE, &zSurface, NULL);
	}
};

// :O
// :O
// END OF UNCRZ STUFF
// :S
// :S

// global lists
std::vector<UNCRZ_effect> effects;
std::vector<UNCRZ_sprite*> sprites;
std::vector<UNCRZ_model*> models;
std::vector<UNCRZ_texture*> textures;
std::vector<UNCRZ_matrix*> matrices;
std::vector<UNCRZ_font*> fonts;
std::vector<UNCRZ_FBF_anim*> anims;

std::vector<UNCRZ_model*> plantArr;
std::vector<UNCRZ_sprite_data> fireSprites;
std::vector<UNCRZ_sprite_data> smokeSprites;
std::vector<UNCRZ_sprite_data> laserSprites;
std::vector<UNCRZ_sprite_data> lpointSprites;

UNCRZ_obj* mapObj; // map, for detailed collision
UNCRZ_obj* cloudObj; // clouds
std::vector<UNCRZ_obj*> objs; // stuff you may want to access
std::vector<UNCRZ_obj*> zSortedObjs; // for anything that needs to be drawn back-to-front
std::vector<int> zsoLocalIndexes;

UNCRZ_sprite_buffer sbuff;

UNCRZ_sprite* fireSprite;
UNCRZ_sprite* smokeSprite;
UNCRZ_sprite* laserSprite;
UNCRZ_sprite* lpointSprite;

// ui
uiEventManager mainUiem;
std::vector<uiItem*> uiItems;
uiRenderer* mainUiRenderer; // shares uiItems as above
uiItem* focusItem;
uiItem* mouseItem;
viewTrans mainVt;
uiTexItem* mainView;
uiTextItem* bannerText;

// menu
uiTexItem* menuView;

// debuging (ui items)
bool debugData = false;
bool debugFlushing = false; // only done if debugData is true
uiTexItem* debugView;
uiTextItem* fpsLabel;

uiTextItem* frameTimeLabel;
uiTexItem* frameTimeBar;

uiTextItem* evalTimeLabel;
uiTexItem* evalTimeBar;
uiTextItem* evalEventsLabel;
uiTexItem* evalEventsBar;
uiTextItem* evalObjsLabel;
uiTexItem* evalObjsBar;
uiTextItem* evalSpritesLabel;
uiTexItem* evalSpritesBar;

uiTextItem* drawTimeLabel;
uiTexItem* drawTimeBar;
uiTextItem* drawAnimsLabel;
uiTexItem* drawAnimsBar;
uiTextItem* drawUpdateLabel;
uiTexItem* drawUpdateBar;
uiTextItem* drawLightsLabel;
uiTexItem* drawLightsBar;
uiTextItem* drawTerrainLabel;
uiTexItem* drawTerrainBar;
uiTextItem* drawCloudsLabel;
uiTexItem* drawCloudsBar;
uiTextItem* drawObjsLabel;
uiTexItem* drawObjsBar;
uiTextItem* drawSpritesLabel;
uiTexItem* drawSpritesBar;
uiTextItem* drawUiLabel;
uiTexItem* drawUiBar;
uiTextItem* drawPresentLabel;
uiTexItem* drawPresentBar;
uiTextItem* drawOverLabel;
uiTexItem* drawOverBar;

const int genericUiCount = 10;
uiTexItem* genericDebugView;
uiTextItem* genericLabel[genericUiCount];
uiTexItem* genericBar[genericUiCount];

// forward defs
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LPDIRECT3DDEVICE9 initDevice(HWND);
void initEffect(LPDIRECT3DDEVICE9);
void drawFrame(LPDIRECT3DDEVICE9);
void drawScene(LPDIRECT3DDEVICE9, drawData*, UNCRZ_view*, DWORD, DWORD);
void drawOver(LPDIRECT3DDEVICE9, drawData*, UNCRZ_over*);
void drawUi(LPDIRECT3DDEVICE9);
void drawLight(LPDIRECT3DDEVICE9, lightData*);
void prepDynamicDecal(LPDIRECT3DDEVICE9, dynamicDecalData*);
void attachDynamicDecal(LPDIRECT3DDEVICE9, UNCRZ_obj* o);
void initObjs(LPDIRECT3DDEVICE9);
void initSprites(LPDIRECT3DDEVICE9);
void initFont(LPDIRECT3DDEVICE9);
void initUi(LPDIRECT3DDEVICE9);
void initTextures(LPDIRECT3DDEVICE9);
void initViews(LPDIRECT3DDEVICE9);
void initOvers(LPDIRECT3DDEVICE9);
void initLights(LPDIRECT3DDEVICE9);
void initOther();
void updateDecals();
LARGE_INTEGER getTime();
void term();
void deSelect();
void preventInput(float);
bool takeInput();
void eval();
void registerUi(uiEvent*);
int handleUiEvents(uiEventManager*);
void handleUi(uiEvent*);
void reload(LPDIRECT3DDEVICE9);
void g_startGo();
void g_endGo();
void g_startGame();
void moveCamera(LPDIRECT3DDEVICE9);
void moveCameraView(LPDIRECT3DDEVICE9, UNCRZ_view*);
void moveCamerawaterReflect(LPDIRECT3DDEVICE9);
void moveCamerawaterRefract(LPDIRECT3DDEVICE9);
void moveCameraLight(LPDIRECT3DDEVICE9, lightData*);
void moveCameraDynamicDecal(LPDIRECT3DDEVICE9, dynamicDecalData*);
int rnd(int);
drawData createDrawData(float, D3DVIEWPORT9* vp);
drawData createDrawDataView(LPDIRECT3DDEVICE9, float, UNCRZ_view*);
drawData createDrawDataOver(float, D3DVIEWPORT9* vp, UNCRZ_over*);
D3DVIEWPORT9 createViewPort(float);
D3DVIEWPORT9 createViewPort(UINT, UINT);

float ticker = 0;

char* appName = "Barembs";
char* windowText = "Barembs";
int windowSizeX = 1000;
int windowSizeY = 700;

D3DXVECTOR3 camPos(0.0f, 40.0f, 0.0f);
float rotY = 0;
float rotPar = 0.0f;
float targRotY = 0;
float targRotPar = 0.0f;
float moveVel = 0.4f;
float focalDist = 10.0f;

D3DXVECTOR4 eyePos; // for shaders
D3DXVECTOR4 eyeDir;

// OMG OMG
bool disableOpenTarget = true; // disables the historical target->postprocess->screen system (use UI Items to show stuff)
DWORD viewMode = VM_persp;
// OMG OMG

D3DXMATRIXA16 viewMatrix;
D3DXMATRIX projMatrix;
D3DXMATRIX viewProj;
D3DXMATRIX vpMat; // view port matrix

D3DCOLOR textCol;
LPD3DXFONT textFont;

D3DXVECTOR4 white(1.0, 1.0, 1.0, 1.0);

bool wireFrame;
bool vertigo = false;
bool running = true;
HINSTANCE hInst;
HWND mainHWnd;
LPDIRECT3DDEVICE9 mainDxDevice;

//float rippleCycle = 0.0f;
//float vertigoCycle = 1.0f;
float farDepth = 1000.0f;
std::vector<vertexPC> normalVis;

LARGE_INTEGER hrticks; // ticks since some time in the past
LARGE_INTEGER hrsec; // ticks per second
LARGE_INTEGER hrlsec; // tick of the last second
LARGE_INTEGER hrbstart; // start of a timed block
LARGE_INTEGER hrbend; // end of a timed block
LARGE_INTEGER hrsbstart; // start of a timed sub block
LARGE_INTEGER hrsbend; // end of a timed sub block
LARGE_INTEGER hrfstart; // start of frame
LARGE_INTEGER hrfend; // end of frame
int frames = 0;
int fps = 0;

// measurements
float hrframeTime; // seconds

float hrevalTime; // seconds
float hrevalSpritesTime; // seconds
float hrevalEventsTime; // seconds
float hrevalObjsTime; // seconds

float hrdrawTime; // seconds
float hrdrawAnimsTime; // seconds
float hrdrawUpdateTime; // seconds
float hrdrawLightsTime; // seconds
float hrdrawTerrainTime; // seconds
float hrdrawCloudsTime; // seconds
float hrdrawObjsTime; // seconds
float hrdrawSpritesTime; // seconds
float hrdrawOverTime; // seconds
float hrdrawUiTime; // seconds
float hrdrawPresentTime; // seconds

float hrgenericTime[genericUiCount];

// scary queries for flushing
IDirect3DQuery9* flushQuery;

void createFlushQuery(LPDIRECT3DDEVICE9 dxDevice)
{
	dxDevice->CreateQuery(D3DQUERYTYPE_EVENT, &flushQuery);
}

// measureFunctions
void hr_start(LARGE_INTEGER* start)
{
	QueryPerformanceCounter(start);
}

void hr_zero(float* outTime)
{
	*outTime = 0.0;
}

void hr_end(LARGE_INTEGER* start, LARGE_INTEGER* end, float* outTime)
{
	QueryPerformanceCounter(end);
	*outTime = ((double)(end->QuadPart - start->QuadPart) / (double)(hrsec.QuadPart));
}

void hr_accend(LARGE_INTEGER* start, LARGE_INTEGER* end, float* outTime)
{ // cumulative
	QueryPerformanceCounter(end);
	*outTime += ((double)(end->QuadPart - start->QuadPart) / (double)(hrsec.QuadPart));
}

float tInc = 0;
bool keyDown[UI_numkeys];

// vpData
UINT vpWidth;
UINT vpHeight;

float targetTexScale = 1.0f; // SSAA.... ish
float lightTexScale = 5.0f;

// textures
LPDIRECT3DTEXTURE9 targetTex;
LPDIRECT3DTEXTURE9 waterReflectTex;
LPDIRECT3DTEXTURE9 waterRefractTex;
LPDIRECT3DTEXTURE9 underTex;
LPDIRECT3DTEXTURE9 sideTex;
LPDIRECT3DTEXTURE9 ripplesTex;

// surfaces in use
LPDIRECT3DSURFACE9 finalTargetSurface;
LPDIRECT3DSURFACE9 targetSurface;
LPDIRECT3DSURFACE9 sideSurface; // for side rendering (stuff that will ultimatly be on the target)

// depth surfaces
LPDIRECT3DSURFACE9 zSurface;
LPDIRECT3DSURFACE9 zSideSurface;
LPDIRECT3DSURFACE9 zLightSurface;

// shady stuff
LPD3DXEFFECT effectA = NULL;
UNCRZ_effect aeffect;
LPDIRECT3DTEXTURE9 tex;

// views
std::vector<UNCRZ_view*> views;

// overs
std::vector<UNCRZ_over*> overs;

// lighting
std::vector<lightData*> lights;

// dynamic decals
std::vector<dynamicDecalData*> dynamicDecals;

//
// game stuff
//

bool g_pause = false;
bool initialised = false;
long g_ticks = 0;

//
// end game stuff
//

int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,
				   int nCmdShow)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = GetSysColorBrush(COLOR_BTNFACE);
	wcex.lpszMenuName   = NULL;
	wcex.lpszClassName  = appName;
	wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			windowText,
			NULL);

		return 1;
	}

	hInst = hInstance; // Store instance handle in our global variable

	// The parameters to CreateWindow explained:
	// szWindowClass: the name of the application
	// szTitle: the text that appears in the title bar
	// WS_OVERLAPPEDWINDOW: the type of window to create
	// CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
	// 500, 100: initial size (width, length)
	// NULL: the parent of this window
	// NULL: this application does not have a menu bar
	// hInstance: the first parameter from WinMain
	// NULL: not used in this application
	HWND hWnd = CreateWindowEx(0,
		appName,
		windowText,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		windowSizeX, windowSizeY,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hWnd)
	{
		MessageBox(NULL,
			_T("Call to CreateWindow failed!"),
			windowText,
			NULL);

		return 1;
	}

	mainHWnd = hWnd;

	SetTimer(hWnd, 77, 100, NULL);

	// The parameters to ShowWindow explained:
	// hWnd: the value returned from CreateWindow
	// nCmdShow: the fourth parameter from WinMain
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	mainDxDevice = initDevice(hWnd);
	initVertexDec(mainDxDevice);
	initEffect(mainDxDevice); // Enable to test for shader errors
	initTextures(mainDxDevice); // want to do this early, everything relies on this setup
	initViews(mainDxDevice);
	initOvers(mainDxDevice);
	initLights(mainDxDevice);
	initFont(mainDxDevice);
	initUi(mainDxDevice);
	initOther();

	reload(mainDxDevice);

	initialised = true; // this is rather important

	// Main message loop:
	MSG msg;
	while (running && GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}

int getTapedObj(D3DXVECTOR3* rayPos, D3DXVECTOR3* rayDir, float* distRes)
{
	int best = -1;
	float bestDist = -1;
	float localDistRes = 0.0;

	for (int i = objs.size() - 1; i >= 0; i--)
	{
		if (objs[i]->model->collides(rayPos, rayDir, &localDistRes))
		{
			if (best == -1 || localDistRes < bestDist)
			{
				best = i;
				bestDist = localDistRes;
			}
		}
	}

	*distRes = bestDist;

	return best;
}

uiItem* getTapedUiItem(float x, float y, float* xOut, float* yOut)
{
	uiItem* taped;
	for (int i = uiItems.size() - 1; i >= 0; i--)
	{
		uiItem* uii = uiItems[i];
		if (uii->enabled && uii->getTaped(x, y, &taped, xOut, yOut))
			return taped;
	}
	return NULL;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	float sx, sy;
	float lx, ly;
	uiItem* tapedUii;

	switch (message)
	{
	case WM_TIMER:
		/*if (wParam == 77)
		{
			drawFrame(mainDxDevice);
			fps = frames;
			frames = 0;
		}*/
		break;
	case WM_MOUSEMOVE:
		if (!takeInput())
			break;
		
		sx = (float)LOWORD(lParam);
		sy = (float)HIWORD(lParam);

		tapedUii = getTapedUiItem(sx, sy, &lx, &ly);
		if (tapedUii != NULL)
		{
			DWORD hudat[2] = { lx, ly };
			if (tapedUii != mouseItem)
			{
				if (mouseItem != NULL)
					registerUi(new uiEvent(mouseItem, UIA_mouseLeave));
				registerUi(new uiEvent(mouseItem, UIA_mouseEnter));
			}
			registerUi(new uiEvent(tapedUii, UIA_mouseMove, hudat, 2));
			break;
		}
		else if (mouseItem != NULL)
		{
			registerUi(new uiEvent(mouseItem, UIA_mouseLeave));
		}
		break;
	case WM_LBUTTONDOWN:
		if (!takeInput())
			break;

		sx = (float)LOWORD(lParam);
		sy = (float)HIWORD(lParam);

		tapedUii = getTapedUiItem(sx, sy, &lx, &ly);
		if (tapedUii != NULL)
		{
			DWORD hudat[2] = { lx, ly};
			registerUi(new uiEvent(tapedUii, UIA_leftDown, hudat, 2));
			break;
		}
		break;
	case WM_RBUTTONDOWN:
		if (!takeInput())
			break;

		sx = (float)LOWORD(lParam);
		sy = (float)HIWORD(lParam);

		tapedUii = getTapedUiItem(sx, sy, &lx, &ly);
		if (tapedUii != NULL)
		{
			DWORD hudat[2] = { lx, ly };
			registerUi(new uiEvent(tapedUii, UIA_rightDown, hudat, 2));
			break;
		}
		break;
	case WM_LBUTTONUP:
		if (!takeInput())
			break;

		sx = (float)LOWORD(lParam);
		sy = (float)HIWORD(lParam);

		tapedUii = getTapedUiItem(sx, sy, &lx, &ly);
		if (tapedUii != NULL)
		{
			DWORD hudat[2] = { lx, ly};
			registerUi(new uiEvent(tapedUii, UIA_leftUp, hudat, 2));
			break;
		}
		break;
	case WM_RBUTTONUP:
		if (!takeInput())
			break;

		sx = (float)LOWORD(lParam);
		sy = (float)HIWORD(lParam);

		tapedUii = getTapedUiItem(sx, sy, &lx, &ly);
		if (tapedUii != NULL)
		{
			DWORD hudat[2] = { lx, ly };
			registerUi(new uiEvent(tapedUii, UIA_rightUp, hudat, 2));
			break;
		}
		break;
	case WM_KEYDOWN:
		if (takeInput())
		{
			keyDown[wParam] = true;

			DWORD hudat[1] = { (DWORD)wParam };
			registerUi(new uiEvent(focusItem, UIA_keyDown, hudat, 1));
		}
		break;
	case WM_KEYUP:
		if (takeInput())
		{
			keyDown[wParam] = false;

			DWORD hudat[1] = { (DWORD)wParam };
			registerUi(new uiEvent(focusItem, UIA_keyUp, hudat, 1));
		}
		break;
	case WM_DESTROY:
		mainDxDevice->Release();
		running = false;
		for each(UNCRZ_model* m in models)
		{
			m->release();
		}
		break;
	case WM_PAINT:
		if (!initialised || g_pause)
			break;
		
		DEBUG_HR_START(&hrfstart);

		frames++;
		// hr perf stuff
		QueryPerformanceFrequency(&hrsec);
		QueryPerformanceCounter(&hrticks);
		if (hrticks.QuadPart > hrlsec.QuadPart + hrsec.QuadPart)
		{
			// 1sec elapsed
			fps = frames;
			hrlsec = hrticks;
			frames = 0;
		}

		// everything interesting
		DEBUG_HR_START(&hrbstart);
		eval();
		DEBUG_HR_END(&hrbstart, &hrbend, &hrevalTime);

		DEBUG_HR_START(&hrbstart);
		drawFrame(mainDxDevice);
		DEBUG_HR_END(&hrbstart, &hrbend, &hrdrawTime);

		DEBUG_HR_END(&hrfstart, &hrfend, &hrframeTime);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

float getDist(float x0, float y0, float x1, float y1)
{
	float x = x0 - x1;
	float y = y0 - y1;
	return sqrt(x * x + y * y);
}

float getDistNoSqrt(float x0, float y0, float x1, float y1)
{
	float x = x0 - x1;
	float y = y0 - y1;
	return x * x + y * y;
}

float getDist(float x0, float y0, float z0, float x1, float y1, float z1)
{
	float x = x0 - x1;
	float y = y0 - y1;
	float z = z0 - z1;
	return sqrt(x * x + y * y + z * z);
}

float getDistNoSqrt(float x0, float y0, float z0, float x1, float y1, float z1)
{
	float x = x0 - x1;
	float y = y0 - y1;
	float z = z0 - z1;
	return x * x + y * y + z * z;
}

void prepBMap(std::ifstream* file, int* width, int* height)
{
	for (int i = 0; i < 10; i++)
	{
		file->get();
	}

	// 11
	int offset;
	offset = file->get();
	offset += file->get() * 256;
	offset += file->get() * 256 * 256;
	offset += file->get() * 256 * 256 * 256;
	// 15

	for (int i = 0; i < 4; i++)
		file->get();
	
	// 19
	*width = file->get();
	*width += file->get() * 256;
	*width += file->get() * 256 * 256;
	*width += file->get() * 256 * 256 * 256;
	
	*height = file->get();
	*height += file->get() * 256;
	*height += file->get() * 256 * 256;
	*height += file->get() * 256 * 256 * 256;

	// 27
	for (int i = 27; i <= offset; i++)
		file->get();
}

void setFocus(uiItem* focusN)
{
	if (focusItem == focusN)
		return; // don't refocus

	if (focusItem != NULL)
		focusItem->unfocus();

	focusItem = focusN;

	if (focusItem != NULL)
		focusItem->enfocus();
}

// dodgy stuff
void handleUiBefore(uiEvent* uie)
{
	uiItem* uii = uie->uii;
	DWORD action = uie->action;
	DWORD* data = uie->data;

	switch (action)
	{
	}
}

// less dodgy stuff - not allowed to mess with the uiEvent
void handleUiAfter(uiEvent* uie)
{
	uiItem* uii = uie->uii;
	DWORD action = uie->action;
	DWORD* data = uie->data;

	RECT crect;
	D3DVIEWPORT9 vp;
	D3DXMATRIX mehMatrix;
	D3DXMATRIX viewProjInv;
	D3DXVECTOR3 nearVec, farVec, screenVec, rayPos, rayDir;
	float distRes = 0.0f;
	float rayDirMod;
	int taped;
	D3DXVECTOR3 simpleDir;
	D3DXVECTOR3 simpleLoc;

	switch (action)
	{
		// tick
	case UIA_tick:
		{
			D3DXVECTOR3 dirVec;

			if (keyDown[VK_UP])
			{
				rotPar -= 0.1f;
				targRotPar -= 0.1f;
			}
			if (keyDown[VK_DOWN])
			{
				rotPar += 0.1f;
				targRotPar += 0.1f;
			}
			if (keyDown[VK_RIGHT])
			{
				rotY -= 0.1f;
				targRotY -= 0.1f;
			}
			if (keyDown[VK_LEFT])
			{
				rotY += 0.1f;
				targRotY += 0.1f;
			}
			if (keyDown[VK_SPACE])
			{
				//D3DXSaveTextureToFile("mehReflect.bmp", D3DXIFF_BMP, waterReflectTex, NULL);
				//D3DXSaveTextureToFile("mehRefract.bmp", D3DXIFF_BMP, waterRefractTex, NULL);
				//D3DXSaveTextureToFile("mehUnder.bmp", D3DXIFF_BMP, underTex, NULL);
				//D3DXSaveTextureToFile("mehLight.bmp", D3DXIFF_BMP, lights[2]->lightTex, NULL);
				//D3DXSaveTextureToFile("mehSun.bmp", D3DXIFF_BMP, lights[0]->lightTex, NULL);

				//D3DXSaveTextureToFile("mehSide.bmp", D3DXIFF_BMP, sideTex, NULL);
				//D3DXSaveTextureToFile("mehTarget.bmp", D3DXIFF_BMP, targetTex, NULL);
				//D3DXSaveTextureToFile("mehMainV.bmp", D3DXIFF_BMP, views[0]->targetTex, NULL);
				//D3DXSaveTextureToFile("mehMainO.bmp", D3DXIFF_BMP, overs[0]->targetTex, NULL);

			}
			if (keyDown[64 + 23]) // w
			{ // FORWARD
				rotPar -= 0.1f;
				targRotPar -= 0.1f;
			}
			if (keyDown[64 + 1]) // a
			{ // LEFT
				rotY += 0.1f;
				targRotY += 0.1f;
			}
			if (keyDown[64 + 19]) // s
			{ // BACKWARD
				rotPar += 0.1f;
				targRotPar += 0.1f;
			}
			if (keyDown[64 + 4]) // d
			{ // RIGHT
				rotY -= 0.1f;
				targRotY -= 0.1f;
			}
		}
		break;

		// nice stuff (commands, stuff spat out by uiItems, etc.)
	case UIA_cmd:

		break;

		// ugly stuff
	case UIA_keyDown:
		{
			DWORD wParam = data[0]; // copied from wndProc
			if (wParam == 64 + 17) // q
			{
				for (int i = 0; i < 8; i++)
				{
					dynamicDecals[i]->decalEnabled = !dynamicDecals[i]->decalEnabled;
				}
			}
			else if (wParam == 64 + 5) // e
			{
				for (int i = 1; i < 9; i++)
				{
					lights[i]->lightEnabled = !lights[i]->lightEnabled;
				}
			}
			else if (wParam == 64 + 6) // f
			{
				wireFrame = !wireFrame;
			}
			else if (wParam == 64 + 7) // g
			{
			}
			else if (wParam == 64 + 18) // r
			{
			}
			else if (wParam == VK_PAUSE || wParam == 64 + 16) // p
			{
				// DOES NOT WORK!!! CANNOT UNPAUSE!!!
				//g_pause = !g_pause;
				for each (uiItem* uii in uiItems)
				{
					uii->needsUpdate = true; // good opertunity to update everything
				}
			}
			else if (wParam == VK_HOME)
			{
				if (debugFlushing)
				{
					debugData = false;
					debugFlushing = false;
				}
				else if (debugData)
				{
					debugFlushing = true;
				}
				else
				{
					debugData = true;
				}
			}
			else if (wParam == VK_END)
			{
//				D3DXSaveTextureToFile("mehReflect.bmp", D3DXIFF_BMP, waterReflectTex, NULL);
//				D3DXSaveTextureToFile("mehRefract.bmp", D3DXIFF_BMP, waterRefractTex, NULL);
//				D3DXSaveTextureToFile("mehUnder.bmp", D3DXIFF_BMP, underTex, NULL);
//				D3DXSaveTextureToFile("mehLight.bmp", D3DXIFF_BMP, lights[2]->lightTex, NULL);
//				D3DXSaveTextureToFile("mehSun.bmp", D3DXIFF_BMP, lights[0]->lightTex, NULL);

				D3DXSaveTextureToFile("mehSide.bmp", D3DXIFF_BMP, sideTex, NULL);
				D3DXSaveTextureToFile("meh0.bmp", D3DXIFF_BMP, views[0]->targetTex, NULL);
				D3DXSaveTextureToFile("meh1.bmp", D3DXIFF_BMP, views[1]->targetTex, NULL);
				D3DXSaveTextureToFile("meh2.bmp", D3DXIFF_BMP, views[2]->targetTex, NULL);
				//D3DXSaveTextureToFile("meh3.bmp", D3DXIFF_BMP, views[3]->targetTex, NULL);
//				D3DXSaveTextureToFile("mehTarget.bmp", D3DXIFF_BMP, targetTex, NULL);
//				D3DXSaveTextureToFile("mehMainV.bmp", D3DXIFF_BMP, views[0]->targetTex, NULL);
//				D3DXSaveTextureToFile("mehMainO.bmp", D3DXIFF_BMP, overs[0]->targetTex, NULL);
			}
		}
		break;
	case UIA_keyUp:
		{
			DWORD wParam = data[0]; // copied from wndProc
			//if (wParam == 64 + index) // alphabet[index]
			//{
			//}
		}
		break;
	case UIA_leftDown:
		{
			if (uii->name == "mainover")
			{
				setFocus(NULL);

				//mainDxDevice->GetViewport(&vp);
				vp = createViewPort(1);
				GetClientRect(mainHWnd, &crect);
				vp = createViewPort(crect.right - crect.left, crect.bottom - crect.top);

				screenVec.x = data[0];
				screenVec.y = data[1];

				screenVec.z = 0.1f;

				D3DXMatrixIdentity(&mehMatrix);

				D3DXVec3Unproject(&nearVec, &screenVec, &vp, &views[0]->viewProjMat, &views[0]->viewViewMat, &mehMatrix);

				screenVec.z = 1.0f;
				D3DXVec3Unproject(&farVec, &screenVec, &vp, &views[0]->viewProjMat, &views[0]->viewViewMat, &mehMatrix);

				rayPos = nearVec;
				rayDir = farVec - nearVec;
				rayDirMod = sqrt(rayDir.x * rayDir.x + rayDir.y * rayDir.y + rayDir.z * rayDir.z);
				rayDir.x /= rayDirMod;
				rayDir.y /= rayDirMod;
				rayDir.z /= rayDirMod;

				//taped = getTapedObj(&rayPos, &rayDir, &distRes, g_go);
			}
		}
		break;
	case UIA_rightDown:
		{
			if (uii->name == "mainover")
			{
				setFocus(NULL);

				//mainDxDevice->GetViewport(&vp);
				vp = createViewPort(1);
				GetClientRect(mainHWnd, &crect);
				vp = createViewPort(crect.right - crect.left, crect.bottom - crect.top);

				screenVec.x = data[0];
				screenVec.y = data[1];

				screenVec.z = 0.1f;

				D3DXMatrixIdentity(&mehMatrix);

				D3DXVec3Unproject(&nearVec, &screenVec, &vp, &views[0]->viewProjMat, &views[0]->viewViewMat, &mehMatrix);

				screenVec.z = 1.0f;
				D3DXVec3Unproject(&farVec, &screenVec, &vp, &views[0]->viewProjMat, &views[0]->viewViewMat, &mehMatrix);

				rayPos = nearVec;
				rayDir = farVec - nearVec;
				rayDirMod = sqrt(rayDir.x * rayDir.x + rayDir.y * rayDir.y + rayDir.z * rayDir.z);
				rayDir.x /= rayDirMod;
				rayDir.y /= rayDirMod;
				rayDir.z /= rayDirMod;
			}
		}
		break;
	}
}

void registerUi(uiItem* uii, DWORD action)
{
	registerUi(new uiEvent(uii, action));
}

void registerUi(uiEvent* uie)
{
	mainUiem.pushEvent(uie);
}

// return number of events processed, because we can
int handleUiEvents(uiEventManager* uiem)
{
	int c = 0;

	uiEvent* uie;
	while ((uie = uiem->popEvent()) != NULL)
	{
		handleUi(uie);
		uie->release();
		delete(uie);
		c++;
	}

	return c;
}

void handleUi(uiEvent* uie)
{
	handleUiBefore(uie); // can consume, preventing it getting to the uiItem
	if (uie->wasConsumed())
		return;

	// focus - only send stuff to handleFocusUi that won't be sent to handleUi
	if (focusItem != NULL && focusItem != uie->uii)
	{
		DWORD uif = focusItem->handleFocusUi(uie, keyDown);
		if (uif == UIF_clear || uif == UIF_lose)
			setFocus(NULL);
	}

	// relevant
	if (uie->uii != NULL)
	{
		DWORD uif = uie->uii->handleUi(uie, keyDown);

		if (uif != UIF_keep) // usually UIF_keep
		{
			if (uif == UIF_clear)
				setFocus(NULL);
			else if (uie->uii == focusItem)
			{
				if (uif == UIF_lose)
					setFocus(NULL);
			}
			else
			{
				if (uif == UIF_gain)
					setFocus(uie->uii);
				else if (uif == UIF_keepOrClear)
					setFocus(NULL);
			}
		}
	}
	if (uie->wasConsumed())
		return;

	handleUiAfter(uie);
}

float compTime(LARGE_INTEGER a, LARGE_INTEGER b)
{
	return (float)(a.QuadPart - b.QuadPart) / (float)hrsec.QuadPart;
}

LARGE_INTEGER getTime()
{
	LARGE_INTEGER tt;
	QueryPerformanceCounter(&tt);
	return tt;
}

void preventInput(float seconds)
{
	//g_noinput = getTime().QuadPart + (LONGLONG)((float)hrsec.QuadPart * seconds);
}

bool takeInput()
{
	return true;
	//return !(getTime().QuadPart < g_noinput);
}

void evalTime()
{
	g_ticks++;
	ticker += 0.01;
}

void eval()
{
	evalTime();

	if (!takeInput())
		return;

	DEBUG_HR_START(&hrsbstart);
	registerUi(NULL, UIA_tick);
	int eventCount = handleUiEvents(&mainUiem); // do events
	DEBUG_HR_END(&hrsbstart, &hrsbend, &hrevalEventsTime);

	char buff[100];
	genericBar[0]->rect.right = genericBar[0]->rect.left + eventCount;
	genericLabel[0]->text = "Events: " + std::string(itoa(eventCount, buff, 10));
	genericBar[0]->visible = true;
	genericLabel[0]->visible = true;

	float distRes;
	D3DXVECTOR3 nearVec, farVec, rayDir;

	// sprites
	DEBUG_HR_START(&hrsbstart);

	// fire
	for (int i = (int)fireSprites.size() - 1; i >= 0; i--)
	{
		if (++fireSprites[i].other.x >= fireSprites[i].other.w)
			fireSprites.erase(fireSprites.begin() + i, fireSprites.begin() + i + 1);
		else
		{
			fireSprites[i].pos.x += 0.001 * (float)rnd(100);
			fireSprites[i].pos.y += 0.1;
			fireSprites[i].pos.z += 0.001 * (float)rnd(100);
		}
	}

	// smoke
	for (int i = (int)smokeSprites.size() - 1; i >= 0; i--)
	{
		smokeSprites[i].pos.x += 0.001 * (float)rnd(100);
		smokeSprites[i].pos.y += 0.1;
		smokeSprites[i].pos.z += 0.001 * (float)rnd(100);
		smokeSprites[i].other.x += smokeSprites[i].other.y * (float)rnd(10);
		if (smokeSprites[i].other.x >= 1)
			smokeSprites.erase(smokeSprites.begin() + i, smokeSprites.begin() + i + 1);
	}

	// laser
	for (int i = laserSprites.size() - 1; i >= 0; i--)
	{
		laserSprites[i].other.w -= 0.1;
		if (laserSprites[i].other.w <= 0)
		{
			laserSprites.erase(laserSprites.begin() + i, laserSprites.begin() + i + 1);
		}
	}

	// lpoint
	for (int i = lpointSprites.size() - 1; i >= 0; i--)
	{
		lpointSprites[i].other.w -= 0.2;
		if (lpointSprites[i].other.w <= 0)
		{
			lpointSprites.erase(lpointSprites.begin() + i, lpointSprites.begin() + i + 1);
		}
	}

	DEBUG_HR_END(&hrsbstart, &hrsbend, &hrevalSpritesTime);





	// units
	DEBUG_HR_START(&hrsbstart);

	DEBUG_HR_END(&hrsbstart, &hrsbend, &hrevalObjsTime);


	// lights
	int n = 8;
	double ang = (double)ticker * 3.0;
	double da = D3DX_PI * 2.0 / (double)n;
	for (int i = 1; i < n + 1; i++)
	{
		lights[i]->lightPos = D3DXVECTOR4(sin(ang) * 90.0, 12 + cos(ang * 2.0 + ticker) * 5 , cos(ang) * 90, 0);
		lights[i]->allowSkip = true;
		lights[i]->updateBox();
		ang += da;

		dynamicDecals[i - 1]->lightPos = lights[i]->lightPos;

		for (int j = 0; j < 12; j++)
		{ // wow
			lpointSprites.push_back(UNCRZ_sprite_data(D3DXVECTOR4(
			lights[i]->lightPos.x + (double)(rnd(21) - 10) / 10.0,
			lights[i]->lightPos.y + (double)(rnd(21) - 10) / 10.0,
			lights[i]->lightPos.z + (double)(rnd(21) - 10) / 10.0,
			1), D3DXVECTOR4(0.0, 0.0, 0.0, (double)rnd(20) / 10.0 + 0.2)));
		}
	}



	// views
	
	// rotY
	while (rotY > targRotY + D3DX_PI)
	{
		rotY -= D3DX_PI * 2;
	}
	while (rotY < targRotY - D3DX_PI)
	{
		rotY += D3DX_PI * 2;
	}
	float dRotY = targRotY - rotY;

	// rotPar
	while (rotPar > targRotPar + D3DX_PI)
	{
		rotPar -= D3DX_PI * 2;
	}
	while (rotPar < targRotPar - D3DX_PI)
	{
		rotPar += D3DX_PI * 2;
	}
	float dRotPar = targRotPar - rotPar;

	// change
	float dRotMod = sqrtf(dRotY * dRotY + dRotPar * dRotPar) * 10.0;
	if (dRotMod > 1.0)
	{
		dRotY /= dRotMod;
		dRotPar /= dRotMod;

		rotY += dRotY;
		rotPar += dRotPar;

		// set if over
		if (dRotY < 0.0f && rotY < targRotY)
			rotY = targRotY;
		if (dRotY > 0.0f && rotY > targRotY)
			rotY = targRotY;
		if (dRotPar < 0.0f && rotPar < targRotPar)
			rotPar = targRotPar;
		if (dRotPar > 0.0f && rotPar > targRotPar)
			rotPar = targRotPar;
	}
	else
	{
		rotY = targRotY;
		rotPar = targRotPar;
	}

	RECT crect;
	GetClientRect(mainHWnd, &crect);
	int winWidth = crect.right - crect.left;
	int winHeight = crect.bottom - crect.top;

	// get normaled vecs
	camPos = D3DXVECTOR3(0.0f, 3.0f, 0.0f);

	D3DXMATRIX mehMatrix;
	D3DXVECTOR3 dirVec = D3DXVECTOR3(80.0f, 0.0f, 0.0f);
	D3DXMatrixRotationZ(&mehMatrix, rotPar);
	D3DXVec3TransformNormal(&dirVec, &dirVec, &mehMatrix);
	D3DXMatrixRotationY(&mehMatrix, rotY);
	D3DXVec3TransformNormal(&dirVec, &dirVec, &mehMatrix);

	camPos -= dirVec;
	D3DXVECTOR3 targVec = camPos + dirVec;
	camPos.y -= (camPos.y - 3) / 20;

	views[0]->dimY = (float)winWidth / (float)winHeight;
	views[0]->camPos = camPos;
	views[0]->dirNormalAt(targVec);

	// refract
	views[2]->dimY = (float)winWidth / (float)winHeight;
	views[2]->camPos = camPos;
	views[2]->dirNormalAt(targVec);

	camPos.y = -camPos.y;
	targVec.y = -targVec.y;

	// reflect
	views[1]->dimY = (float)winWidth / (float)winHeight;
	views[1]->camPos = camPos;
	views[1]->dirNormalAt(targVec);
}

void reload(LPDIRECT3DDEVICE9 dxDevice)
{
	objs.clear();
	zSortedObjs.clear();

	for (int i = models.size() - 1; i >= 0; i--)
	{
		models[i]->release();
	}
	models.clear();
	sprites.clear();
	anims.clear();

	loadModelsFromFile("text.uncrz", mainDxDevice, &models, &effects, &textures, &matrices, &normalVis);
	loadSpritesFromFile("textS.uncrz", mainDxDevice, &sprites, &effects, &textures, &matrices);
	loadAnimsFromFile("textA.uncrz", &anims, &models);

	initObjs(dxDevice);
	initSprites(dxDevice);
}

LPDIRECT3DDEVICE9 initDevice(HWND hWnd)
{	
	LPDIRECT3D9 dxObj;
	dxObj = Direct3DCreate9(D3D_SDK_VERSION);

	if (dxObj == NULL)
	{
		MessageBox(hWnd, "ARGH!!!!", "ARGH!!!", MB_OK);
	}

	D3DPRESENT_PARAMETERS dxPresParams;
	ZeroMemory(&dxPresParams, sizeof(dxPresParams));
	dxPresParams.Windowed = true;
	dxPresParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	dxPresParams.EnableAutoDepthStencil = TRUE;
	dxPresParams.AutoDepthStencilFormat = D3DFMT_D16;
    dxPresParams.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	dxPresParams.MultiSampleType = D3DMULTISAMPLE_NONE;
	dxPresParams.MultiSampleType = UNCRZ_STDSAMPLE;
	
	//dxPresParams.BackBufferWidth = 1600;
	//dxPresParams.BackBufferHeight = 1200;
	//dxPresParams.BackBufferWidth = 1280;
	//dxPresParams.BackBufferHeight = 960;
	//dxPresParams.BackBufferWidth = 920;
	//dxPresParams.BackBufferHeight = 690;
	//dxPresParams.BackBufferWidth = 800;
	//dxPresParams.BackBufferHeight = 600;

	// fit buffers to size of screen (for testing only?)
	RECT crect;
	GetClientRect(mainHWnd, &crect);
	dxPresParams.BackBufferWidth = crect.right - crect.left;// + 1;
	dxPresParams.BackBufferHeight = crect.bottom - crect.top;// + 1;

	if (dxPresParams.Windowed)
		dxPresParams.FullScreen_RefreshRateInHz = 0;
	else
	{
		dxPresParams.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
		dxPresParams.BackBufferCount = 1;
		dxPresParams.BackBufferFormat = D3DFMT_R5G6B5;
	}

	LPDIRECT3DDEVICE9 dxDevice;
	LRESULT createRes = dxObj->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &dxPresParams, &dxDevice);

	dxDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	dxDevice->SetRenderState(D3DRS_LIGHTING, false);
	dxDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	//dxDevice->GetTexture(0, (IDirect3DBaseTexture9**)&targetTex);
	dxDevice->GetRenderTarget(0, &finalTargetSurface);

	createFlushQuery(dxDevice);

	return dxDevice;
}

void initFont(LPDIRECT3DDEVICE9 dxDevice)
{
	D3DXCreateFont(dxDevice, 20, 0, FW_NORMAL, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Tahoma", &textFont);

	textCol = D3DCOLOR_ARGB(255, 0, 0, 128);
}

void initUi(LPDIRECT3DDEVICE9 dxDevice)
{
	D3DVIEWPORT9 vp = createViewPort(1);

	RECT crect;
	GetClientRect(mainHWnd, &crect);
	int winWidth = crect.right - crect.left;
	int winHeight = crect.bottom - crect.top;
	mainVt = viewTrans(vp.Width, vp.Height, winWidth, winHeight);

	focusItem = NULL;
	mouseItem = NULL;

	// font
	LPD3DXFONT uiFont;
	createFont(dxDevice, (int)mainVt.yToTextY((float)20), "uifont", &uiFont, &fonts);

	// uiItems
	RECT rect;
	uiTexItem* tempTex;
	uiTextItem* tempText;
	uiCheckItem* tempCheck;
	uiRadioItem* tempRadio;
	uiTextInputItem* tempTextInput;
	uiButtonItem* tempButton;

	// (view one view)
	rect.left = 0;
	rect.right = winWidth;// + 1;
	rect.top = 0;
	rect.bottom = winHeight;// + 1;
	tempTex = new uiTexItem(dxDevice, "mainover", NULL, vertexDecPCT, "un_shade.fx", "over_final_flatalpha", "over_main", rect, &mainUiem, &effects, &textures);	
	tempTex->clickable = true;
	uiItems.push_back(tempTex);
	tempTex->colMod = D3DXVECTOR4(1, 1, 1, 1);
	tempTex->texAlign = TXA_pixelOffset;
	tempTex->texW = vp.Width;
	tempTex->texH = vp.Height;
	mainView = tempTex;


	// debug view
	rect.left = 175;
	rect.right = 395;
	rect.top = 45;
	rect.bottom = 500;
	tempTex = new uiTexItem(dxDevice, "debugItem", NULL, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/bland.tga", rect, &mainUiem, &effects, &textures);
	tempTex->visible = false;
	tempTex->clickable = false;
	uiItems.push_back(tempTex);
	tempTex->colMod = D3DXVECTOR4(0, 0.5, 1, 0.5);
	debugView = tempTex;

	// vlines
	rect.left = 9;
	rect.right = 10;
	rect.top = 10;
	rect.bottom = debugView->rect.bottom - debugView->rect.top - 10;
	tempTex = new uiTexItem(dxDevice, "vlineLeft", debugView, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/pure.tga", rect, &mainUiem, &effects, &textures);
	tempTex->colMod = D3DXVECTOR4(0, 0, 0, 1);
	tempTex->clickable = false;

	rect.left = debugView->rect.right - debugView->rect.left - 10;
	rect.right = debugView->rect.right - debugView->rect.left - 9;
	rect.top = 10;
	rect.bottom = debugView->rect.bottom - debugView->rect.top - 10;
	tempTex = new uiTexItem(dxDevice, "vlineLeft", debugView, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/pure.tga", rect, &mainUiem, &effects, &textures);
	tempTex->colMod = D3DXVECTOR4(0, 0, 0, 1);
	tempTex->enabled = true;
	tempTex->clickable = false;

	int dg = 10;
	// fps
	rect.left = 15;
	rect.right = 210;
	rect.top = dg;
	rect.bottom = dg + 20;
	tempText = new uiTextItem(dxDevice, "fpsLabel", debugView, rect, &mainUiem, "~~ fps ~~", D3DCOLOR_ARGB(255, 0, 0, 128), uiFont);
	tempText->clickable = false;
	fpsLabel = tempText;

	dg += 30;
	// frame time
	rect.left = 10;
	rect.right = 10; // this will change
	rect.top = dg;
	rect.bottom = dg + 20;
	tempTex = new uiTexItem(dxDevice, "frameTimeBar", debugView, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/pure.tga", rect, &mainUiem, &effects, &textures);
	tempTex->colMod = D3DXVECTOR4(1, 1, 1, 1);
	tempTex->clickable = false;
	frameTimeBar = tempTex;

	rect.left = 15;
	rect.right = 190;
	rect.top = dg;
	rect.bottom = dg + 20;
	tempText = new uiTextItem(dxDevice, "frameTimeLabel", debugView, rect, &mainUiem, "~~ frame time ~~", D3DCOLOR_ARGB(255, 0, 0, 128), uiFont);
	tempText->clickable = false;
	frameTimeLabel = tempText;

	dg += 30;
	// eval time
	rect.left = 10;
	rect.right = 10; // this will change
	rect.top = dg;
	rect.bottom = dg + 20;
	tempTex = new uiTexItem(dxDevice, "evalTimeBar", debugView, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/pure.tga", rect, &mainUiem, &effects, &textures);
	tempTex->colMod = D3DXVECTOR4(1, 0, 0, 1);
	tempTex->clickable = false;
	evalTimeBar = tempTex;

	rect.left = 15;
	rect.right = 190;
	rect.top = dg;
	rect.bottom = dg + 20;
	tempText = new uiTextItem(dxDevice, "evalTimeLabel", debugView, rect, &mainUiem, "~~ eval time ~~", D3DCOLOR_ARGB(255, 0, 0, 128), uiFont);
	tempText->clickable = false;
	evalTimeLabel = tempText;

	dg += 25;
	// eval sprites time
	rect.left = 10;
	rect.right = 10; // this will change
	rect.top = dg;
	rect.bottom = dg + 20;
	tempTex = new uiTexItem(dxDevice, "evalEventsBar", debugView, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/pure.tga", rect, &mainUiem, &effects, &textures);
	tempTex->colMod = D3DXVECTOR4(1, 0.5, 0.5, 1);
	tempTex->clickable = false;
	evalEventsBar = tempTex;

	rect.left = 15;
	rect.right = 190;
	rect.top = dg;
	rect.bottom = dg + 20;
	tempText = new uiTextItem(dxDevice, "evalEventsLabel", debugView, rect, &mainUiem, "~~ eval events time ~~", D3DCOLOR_ARGB(255, 0, 0, 128), uiFont);
	tempText->clickable = false;
	evalEventsLabel = tempText;

	dg += 25;
	// eval sprites time
	rect.left = 10;
	rect.right = 10; // this will change
	rect.top = dg;
	rect.bottom = dg + 20;
	tempTex = new uiTexItem(dxDevice, "evalSpritesBar", debugView, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/pure.tga", rect, &mainUiem, &effects, &textures);
	tempTex->colMod = D3DXVECTOR4(1, 0.5, 0.5, 1);
	tempTex->clickable = false;
	evalSpritesBar = tempTex;

	rect.left = 15;
	rect.right = 190;
	rect.top = dg;
	rect.bottom = dg + 20;
	tempText = new uiTextItem(dxDevice, "evalSpritesLabel", debugView, rect, &mainUiem, "~~ eval sprites time ~~", D3DCOLOR_ARGB(255, 0, 0, 128), uiFont);
	tempText->clickable = false;
	evalSpritesLabel = tempText;

	dg += 25;
	// eval objs time
	rect.left = 10;
	rect.right = 10; // this will change
	rect.top = dg;
	rect.bottom = dg + 20;
	tempTex = new uiTexItem(dxDevice, "evalObjsBar", debugView, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/pure.tga", rect, &mainUiem, &effects, &textures);
	tempTex->colMod = D3DXVECTOR4(1, 0.5, 0.5, 1);
	tempTex->clickable = false;
	evalObjsBar = tempTex;

	rect.left = 15;
	rect.right = 190;
	rect.top = dg;
	rect.bottom = dg + 20;
	tempText = new uiTextItem(dxDevice, "evalObjsLabel", debugView, rect, &mainUiem, "~~ draw sprites time ~~", D3DCOLOR_ARGB(255, 0, 0, 128), uiFont);
	tempText->clickable = false;
	evalObjsLabel = tempText;

	dg += 30;
	// draw time
	rect.left = 10;
	rect.right = 10; // this will change
	rect.top = dg;
	rect.bottom = dg + 20;
	tempTex = new uiTexItem(dxDevice, "drawTimeBar", debugView, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/pure.tga", rect, &mainUiem, &effects, &textures);
	tempTex->colMod = D3DXVECTOR4(0, 1, 0, 1);
	tempTex->clickable = false;
	drawTimeBar = tempTex;

	rect.left = 15;
	rect.right = 190;
	rect.top = dg;
	rect.bottom = dg + 20;
	tempText = new uiTextItem(dxDevice, "drawTimeLabel", debugView, rect, &mainUiem, "~~ draw time ~~", D3DCOLOR_ARGB(255, 0, 0, 128), uiFont);
	tempText->clickable = false;
	drawTimeLabel = tempText;

	dg += 25;
	// draw anims time
	rect.left = 10;
	rect.right = 10; // this will change
	rect.top = dg;
	rect.bottom = dg + 20;
	tempTex = new uiTexItem(dxDevice, "drawAnimsBar", debugView, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/pure.tga", rect, &mainUiem, &effects, &textures);
	tempTex->colMod = D3DXVECTOR4(0.5, 1, 0.5, 1);
	tempTex->clickable = false;
	drawAnimsBar = tempTex;

	rect.left = 15;
	rect.right = 190;
	rect.top = dg;
	rect.bottom = dg + 20;
	tempText = new uiTextItem(dxDevice, "drawAnimsLabel", debugView, rect, &mainUiem, "~~ draw anims time ~~", D3DCOLOR_ARGB(255, 0, 0, 128), uiFont);
	tempText->clickable = false;
	drawAnimsLabel = tempText;

	dg += 25;
	// draw update time
	rect.left = 10;
	rect.right = 10; // this will change
	rect.top = dg;
	rect.bottom = dg + 20;
	tempTex = new uiTexItem(dxDevice, "drawUpdateBar", debugView, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/pure.tga", rect, &mainUiem, &effects, &textures);
	tempTex->colMod = D3DXVECTOR4(0.5, 1, 0.5, 1);
	tempTex->clickable = false;
	drawUpdateBar = tempTex;

	rect.left = 15;
	rect.right = 190;
	rect.top = dg;
	rect.bottom = dg + 20;
	tempText = new uiTextItem(dxDevice, "drawUpdateLabel", debugView, rect, &mainUiem, "~~ draw update time ~~", D3DCOLOR_ARGB(255, 0, 0, 128), uiFont);
	tempText->clickable = false;
	drawUpdateLabel = tempText;

	dg += 25;
	// draw lights time
	rect.left = 10;
	rect.right = 10; // this will change
	rect.top = dg;
	rect.bottom = dg + 20;
	tempTex = new uiTexItem(dxDevice, "drawLightsBar", debugView, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/pure.tga", rect, &mainUiem, &effects, &textures);
	tempTex->colMod = D3DXVECTOR4(0.5, 1, 0.5, 1);
	tempTex->clickable = false;
	drawLightsBar = tempTex;

	rect.left = 15;
	rect.right = 190;
	rect.top = dg;
	rect.bottom = dg + 20;
	tempText = new uiTextItem(dxDevice, "drawLightsLabel", debugView, rect, &mainUiem, "~~ draw lights time ~~", D3DCOLOR_ARGB(255, 0, 0, 128), uiFont);
	tempText->clickable = false;
	drawLightsLabel = tempText;

	dg += 25;
	// draw terrain time
	rect.left = 10;
	rect.right = 10; // this will change
	rect.top = dg;
	rect.bottom = dg + 20;
	tempTex = new uiTexItem(dxDevice, "drawTerrainBar", debugView, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/pure.tga", rect, &mainUiem, &effects, &textures);
	tempTex->colMod = D3DXVECTOR4(0.5, 1, 0.5, 1);
	tempTex->clickable = false;
	drawTerrainBar = tempTex;

	rect.left = 15;
	rect.right = 190;
	rect.top = dg;
	rect.bottom = dg + 20;
	tempText = new uiTextItem(dxDevice, "drawTerrainLabel", debugView, rect, &mainUiem, "~~ draw terrain time ~~", D3DCOLOR_ARGB(255, 0, 0, 128), uiFont);
	tempText->clickable = false;
	drawTerrainLabel = tempText;

	dg += 25;
	// draw clouds time
	rect.left = 10;
	rect.right = 10; // this will change
	rect.top = dg;
	rect.bottom = dg + 20;
	tempTex = new uiTexItem(dxDevice, "drawCloudsBar", debugView, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/pure.tga", rect, &mainUiem, &effects, &textures);
	tempTex->colMod = D3DXVECTOR4(0.5, 1, 0.5, 1);
	tempTex->clickable = false;
	drawCloudsBar = tempTex;

	rect.left = 15;
	rect.right = 190;
	rect.top = dg;
	rect.bottom = dg + 20;
	tempText = new uiTextItem(dxDevice, "drawCloudsLabel", debugView, rect, &mainUiem, "~~ draw cloud time ~~", D3DCOLOR_ARGB(255, 0, 0, 128), uiFont);
	tempText->clickable = false;
	drawCloudsLabel = tempText;

	dg += 25;
	// draw objs time
	rect.left = 10;
	rect.right = 10; // this will change
	rect.top = dg;
	rect.bottom = dg + 20;
	tempTex = new uiTexItem(dxDevice, "drawObjsBar", debugView, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/pure.tga", rect, &mainUiem, &effects, &textures);
	tempTex->colMod = D3DXVECTOR4(0.5, 1, 0.5, 1);
	tempTex->clickable = false;
	drawObjsBar = tempTex;

	rect.left = 15;
	rect.right = 190;
	rect.top = dg;
	rect.bottom = dg + 20;
	tempText = new uiTextItem(dxDevice, "drawObjsLabel", debugView, rect, &mainUiem, "~~ draw objs time ~~", D3DCOLOR_ARGB(255, 0, 0, 128), uiFont);
	tempText->clickable = false;
	drawObjsLabel = tempText;

	dg += 25;
	// draw sprites time
	rect.left = 10;
	rect.right = 10; // this will change
	rect.top = dg;
	rect.bottom = dg + 20;
	tempTex = new uiTexItem(dxDevice, "drawSpritesBar", debugView, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/pure.tga", rect, &mainUiem, &effects, &textures);
	tempTex->colMod = D3DXVECTOR4(0.5, 1, 0.5, 1);
	tempTex->clickable = false;
	drawSpritesBar = tempTex;

	rect.left = 15;
	rect.right = 190;
	rect.top = dg;
	rect.bottom = dg + 20;
	tempText = new uiTextItem(dxDevice, "drawSpritesLabel", debugView, rect, &mainUiem, "~~ draw sprites time ~~", D3DCOLOR_ARGB(255, 0, 0, 128), uiFont);
	tempText->clickable = false;
	drawSpritesLabel = tempText;

	dg += 25;
	// draw over time
	rect.left = 10;
	rect.right = 10; // this will change
	rect.top = dg;
	rect.bottom = dg + 20;
	tempTex = new uiTexItem(dxDevice, "drawOverBar", debugView, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/pure.tga", rect, &mainUiem, &effects, &textures);
	tempTex->colMod = D3DXVECTOR4(0.5, 1, 0.5, 1);
	tempTex->clickable = false;
	drawOverBar = tempTex;

	rect.left = 15;
	rect.right = 190;
	rect.top = dg;
	rect.bottom = dg + 20;
	tempText = new uiTextItem(dxDevice, "drawOverLabel", debugView, rect, &mainUiem, "~~ draw over time ~~", D3DCOLOR_ARGB(255, 0, 0, 128), uiFont);
	tempText->clickable = false;
	drawOverLabel = tempText;

	dg += 25;
	// draw ui time
	rect.left = 10;
	rect.right = 10; // this will change
	rect.top = dg;
	rect.bottom = dg + 20;
	tempTex = new uiTexItem(dxDevice, "drawUiBar", debugView, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/pure.tga", rect, &mainUiem, &effects, &textures);
	tempTex->colMod = D3DXVECTOR4(0.5, 1, 0.5, 1);
	tempTex->clickable = false;
	drawUiBar = tempTex;

	rect.left = 15;
	rect.right = 190;
	rect.top = dg;
	rect.bottom = dg + 20;
	tempText = new uiTextItem(dxDevice, "drawUiLabel", debugView, rect, &mainUiem, "~~ draw ui time ~~", D3DCOLOR_ARGB(255, 0, 0, 128), uiFont);
	tempText->clickable = false;
	drawUiLabel = tempText;

	dg += 25;
	// draw present time
	rect.left = 10;
	rect.right = 10; // this will change
	rect.top = dg;
	rect.bottom = dg + 20;
	tempTex = new uiTexItem(dxDevice, "drawPresentBar", debugView, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/pure.tga", rect, &mainUiem, &effects, &textures);
	tempTex->colMod = D3DXVECTOR4(0.5, 1, 0.5, 1);
	tempTex->clickable = false;
	drawPresentBar = tempTex;

	rect.left = 15;
	rect.right = 190;
	rect.top = dg;
	rect.bottom = dg + 20;
	tempText = new uiTextItem(dxDevice, "drawPresentLabel", debugView, rect, &mainUiem, "~~ draw present time ~~", D3DCOLOR_ARGB(255, 0, 0, 128), uiFont);
	tempText->clickable = false;
	drawPresentLabel = tempText;



	// generic debug view
		// debug view
	rect.left = 400;
	rect.right = 620;
	rect.top = 45;
	rect.bottom = 500;
	tempTex = new uiTexItem(dxDevice, "debugView", NULL, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/bland.tga", rect, &mainUiem, &effects, &textures);
	tempTex->clickable = false;
	uiItems.push_back(tempTex);
	tempTex->colMod = D3DXVECTOR4(0, 0.5, 1, 0.5);
	genericDebugView = tempTex;

	// vlines
	rect.left = 9;
	rect.right = 10;
	rect.top = 10;
	rect.bottom = debugView->rect.bottom - debugView->rect.top - 10;
	tempTex = new uiTexItem(dxDevice, "vlineLeft", genericDebugView, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/pure.tga", rect, &mainUiem, &effects, &textures);
	tempTex->colMod = D3DXVECTOR4(0, 0, 0, 1);
	tempTex->clickable = false;

	rect.left = debugView->rect.right - debugView->rect.left - 10;
	rect.right = debugView->rect.right - debugView->rect.left - 9;
	rect.top = 10;
	rect.bottom = debugView->rect.bottom - debugView->rect.top - 10;
	tempTex = new uiTexItem(dxDevice, "vlineLeft", genericDebugView, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/pure.tga", rect, &mainUiem, &effects, &textures);
	tempTex->colMod = D3DXVECTOR4(0, 0, 0, 1);
	tempTex->clickable = false;

	dg = 10 - 30;
	for (int i = 0; i < genericUiCount; i++)
	{
		dg += 30;
		// frame time
		rect.left = 10;
		rect.right = 10; // this will change
		rect.top = dg;
		rect.bottom = dg + 20;
		tempTex = new uiTexItem(dxDevice, "", genericDebugView, vertexDecPCT, "un_shade.fx", "simpleUi", "ui/pure.tga", rect, &mainUiem, &effects, &textures);
		tempTex->colMod = D3DXVECTOR4(1, 1, 1, 1);
		tempTex->clickable = false;
		genericBar[i] = tempTex;

		rect.left = 15;
		rect.right = 190;
		rect.top = dg;
		rect.bottom = dg + 20;
		tempText = new uiTextItem(dxDevice, "", genericDebugView, rect, &mainUiem, "~~ clint ~~", D3DCOLOR_ARGB(255, 0, 0, 128), uiFont);
		tempText->clickable = false;
		genericLabel[i] = tempText;
	}


	// testness
	rect.left = 0;
	rect.right = winWidth;// + 1;
	rect.top = 40;
	rect.bottom = winHeight;// + 1;
	tempTex = new uiTexItem(dxDevice, "testness", NULL, vertexDecPCT, "un_shade.fx", "simpleUiBorder", "white_bordered.tga", rect, &mainUiem, &effects, &textures);
	tempTex->enabled = false;
	tempTex->visible = false;
	tempTex->clickable = false;
	uiItems.push_back(tempTex);
	tempTex->colMod = D3DXVECTOR4(1, 1, 1, 1);
	tempTex->texH = 65;
	tempTex->texW = 65;
	tempTex->texMode = TXM_flat;
	tempTex->texAlign = TXA_left | TXA_bottom | TXA_offsetInset;
	tempTex->texScaleX = 5.0f;
	tempTex->texScaleY = 1.0f;


	mainUiRenderer = new uiRenderer("mainuirenderer", false, 0);
	mainUiRenderer->init(finalTargetSurface);
	mainUiRenderer->initStencil(zSurface);
	mainUiRenderer->uiItems = uiItems;
}

void initEffect(LPDIRECT3DDEVICE9 dxDevice)
{
	LPD3DXBUFFER errs = NULL;
	HRESULT res = D3DXCreateEffectFromFile(dxDevice, "un_shade.fx", NULL, NULL, 0, NULL, &effectA, &errs);

	if (errs != NULL)
	{
		LPVOID meh = errs->GetBufferPointer();
		int mehhem = (int)errs->GetBufferSize();

		if (res == D3DERR_INVALIDCALL)
		{
			mehhem++;
		}
		if (res == D3DXERR_INVALIDDATA)
		{
			mehhem++;
		}
		if (res == E_OUTOFMEMORY)
		{
			mehhem++;
		}
		if (res == D3D_OK)
		{
			mehhem++;
		}
		char* mehb = new char[mehhem];
		memcpy(mehb, meh, mehhem);

		std::cout << mehb;
		mehhem++;
	}

	effectA->SetTechnique("simple");


	// now do aeffect

	aeffect = createEffect(dxDevice, "un_shade.fx", VX_PCT, "un_shade.fx", &effects);
}

int rnd(int top)
{
	return rand() % top;
}

void runAnims()
{
	for (int i = objs.size() - 1; i >= 0; i--)
	{
		objs[i]->run();
	}
}

void initObjs(LPDIRECT3DDEVICE9 dxDevice)
{
	D3DXVECTOR3 simpleDir;
	D3DXVECTOR3 simpleLoc;

	UNCRZ_FBF_anim* mapAnim = getFBF_anim(&anims, "map_idle");
	UNCRZ_FBF_anim* cloudAnim = getFBF_anim(&anims, "clouds_idle");

	mapObj = new UNCRZ_obj(getModel(&models, "map"));
	mapObj->changeAnim(mapAnim);
	cloudObj = new UNCRZ_obj(getModel(&models, "clouds"));
	cloudObj->changeAnim(cloudAnim);

	mapObj->update(true);
	cloudObj->update(true);

	UNCRZ_obj* temp;

	getModel(&models, "anubis")->changeAnim(getFBF_anim(&anims, "anubis_spin"));
	temp = new UNCRZ_obj(new UNCRZ_model(getModel(&models, "anubis")));
	//temp->changeAnim(getFBF_anim(&anims, "anubis_spin"));
	temp->offset = D3DXVECTOR3(0, -2, 0);
	temp->update(true);
	objs.push_back(temp);
	zSortedObjs.push_back(temp);

	temp = new UNCRZ_obj(new UNCRZ_model(getModel(&models, "anubis")));
	//temp->changeAnim(getFBF_anim(&anims, "anubis_spin"));
	temp->offset = D3DXVECTOR3(15, -2, 0);
	temp->update(true);
	objs.push_back(temp);
	zSortedObjs.push_back(temp);

	temp = new UNCRZ_obj(new UNCRZ_model(getModel(&models, "anubis")));
	//temp->changeAnim(getFBF_anim(&anims, "anubis_spin"));
	temp->offset = D3DXVECTOR3(0, -2, 15);
	temp->update(true);
	objs.push_back(temp);
	zSortedObjs.push_back(temp);
}

void initSprites(LPDIRECT3DDEVICE9 dxDevice)
{
	sbuff = UNCRZ_sprite_buffer();
	sbuff.create(dxDevice, vertexDecPCT, 50);

	fireSprite = getSprite(&sprites, "fire");
	smokeSprite = getSprite(&sprites, "smoke");
	laserSprite = getSprite(&sprites, "laser");
	lpointSprite = getSprite(&sprites, "lpoint");
}


void disableClip(LPDIRECT3DDEVICE9 dxDevice)
{
	dxDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
	dxDevice->SetRenderState(D3DRS_CLIPPING, false);
}

void setClip(LPDIRECT3DDEVICE9 dxDevice, int idx, D3DXVECTOR4* planeCoofs)
{
	D3DXVECTOR4 transPlaneCoofs;
	D3DXMATRIX viewProjInv;
	D3DXMatrixInverse(&viewProjInv, NULL, &viewProj);
	D3DXMatrixTranspose(&viewProjInv, &viewProjInv);
	D3DXVec4Transform(&transPlaneCoofs, planeCoofs, &viewProjInv);

	D3DXPLANE plane((const float*)&transPlaneCoofs);

	dxDevice->SetClipPlane(idx, (const float*)&plane);
}

void enableClip(LPDIRECT3DDEVICE9 dxDevice, DWORD value)
{
	dxDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, value);
	dxDevice->SetRenderState(D3DRS_CLIPPING, true);
}

void drawZBackToFront(std::vector<UNCRZ_obj*>& objList, std::vector<int>& mol, LPDIRECT3DDEVICE9 dxDevice, drawData* ddat, DWORD drawArgs) // mutable objs list
{
	int osize = (int)objList.size();
	int msize = (int)mol.size();
	if (msize > osize)
	{
		// lost items somewhere (current implmentation is poor)
		mol.resize(osize);
		
		for (int i = 0; i < osize; i++)
		{
			mol[i] = i;
		}
	}
	else if (msize < osize)
	{
		// rarely going to be adding more than 1 item when speed matters
		for (int i = msize; i < osize; i++)
		{
			mol.insert(mol.begin(), i);
		}
	}

	int top = osize - 1;

	if (top < 0)
		return;

	for (int i = top; i >= 0; i--)
	{
		objList[mol[i]]->zSortDepth = getDistNoSqrt(objList[mol[i]]->offset.x, objList[mol[i]]->offset.y, objList[mol[i]]->offset.z, camPos.x, camPos.y, camPos.z);
	}
	
	bool noSwaps = false;
	int curTop = top + 1;
	int curIdx;

next:
	curTop--;

	curIdx = mol[0];

	noSwaps = true;
	for (int i = 0; i < curTop; i++)
	{
		if (objList[mol[i + 1]]->zSortDepth < objList[curIdx]->zSortDepth)
		{ // swap
			noSwaps = false;
			mol[i] = mol[i + 1];
			mol[i + 1] = curIdx;
		}
		else
		{
			curIdx = mol[i + 1];
		}
	}

	objList[curIdx]->draw(dxDevice, ddat, drawArgs);

	if (noSwaps || curTop <= 0)
		goto sorted;
	goto next;

sorted:
	for (int i = curTop - 1; i >= 0; i--)
	{
		objList[mol[i]]->draw(dxDevice, ddat, drawArgs);
	}
}

//void drawZBackToFront(std::vector<UNCRZ_obj*>& mol, LPDIRECT3DDEVICE9 dxDevice, drawData* ddat, DWORD drawArgs) // mutable objs list
//{
//	int top = (int)mol.size() - 1;
//
//	if (top < 0)
//		return;
//
//	for (int i = top; i >= 0; i--)
//	{
//		mol[i]->zSortDepth = getDistNoSqrt(mol[i]->offset.x, mol[i]->offset.y, mol[i]->offset.z, camPos.x, camPos.y, camPos.z);
//	}
//	
//	bool noSwaps = false;
//	int curTop = top + 1;
//	UNCRZ_obj* curObj;
//
//next:
//	curTop--;
//
//	curObj = mol[0];
//
//	noSwaps = true;
//	for (int i = 0; i < curTop; i++)
//	{
//		if (mol[i + 1]->zSortDepth < curObj->zSortDepth)
//		{ // swap
//			noSwaps = false;
//			mol[i] = mol[i + 1];
//			mol[i + 1] = curObj;
//		}
//		else
//		{
//			curObj = mol[i + 1];
//		}
//	}
//
//	curObj->draw(dxDevice, ddat, drawArgs);
//
//	if (noSwaps || curTop <= 0)
//		goto sorted;
//	goto next;
//
//sorted:
//	for (int i = curTop; i >= 0; i--)
//	{
//		mol[i]->draw(dxDevice, ddat, drawArgs);
//	}
//}

// takes from side target
void drawTargetOverFinalTarget(LPDIRECT3DDEVICE9 dxDevice, drawData* ddat, UNCRZ_effect* effect)
{
	D3DXMATRIX idMat;
	D3DXMatrixIdentity(&idMat);
	dxDevice->SetRenderTarget(0, ddat->targetSurface);
	dxDevice->SetViewport(ddat->targetVp);

	dxDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	dxDevice->SetRenderState(D3DRS_ZENABLE, false);
	dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);

	vertexOver overVerts[4]; // make this const / VBuff
	overVerts[0] = vertexOver(-1, -1, 0, 0, 1);
	overVerts[1] = vertexOver(-1, 1, 0, 0, 0);
	overVerts[2] = vertexOver(1, -1, 0, 1, 1);
	overVerts[3] = vertexOver(1, 1, 0, 1, 0);

	D3DXVECTOR4 texData = D3DXVECTOR4(0.5 / (float)ddat->targetVp->Width, 0.5 / (float)ddat->targetVp->Height, 1.0 / (float)ddat->targetVp->Width, 1.0 / (float)ddat->targetVp->Height);
	effect->setTextureData((float*)&texData.x);
	
	for (int i = 0; i < 4; i++) // do ahead of shader
	{
		overVerts[i].tu += texData.x;
		overVerts[i].tv += texData.y;
	}

	effect->setEyePos(&ddat->eyePos);
	effect->setEyeDir(&ddat->eyeDir);
	effect->setTicker(ddat->ticker);
	effect->setTexture(ddat->sideTex);
	effect->effect->SetTechnique("over_final");
	effect->setViewProj(&idMat);
	effect->effect->CommitChanges();

	dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

	UINT numPasses, pass;
	effect->effect->Begin(&numPasses, 0);
	effect->effect->BeginPass(0);

	dxDevice->SetVertexDeclaration(vertexDecOver);
	dxDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, overVerts, sizeof(vertexOver));

	effect->effect->EndPass();
	effect->effect->End();
	
	dxDevice->SetRenderState(D3DRS_ZENABLE, true);
	dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
}

void drawUi(LPDIRECT3DDEVICE9 dxDevice)
{
	// need to update a few labels
	char buff[500];

	// update debug stuff
	if (debugData)
	{
		debugView->visible = true;
		genericDebugView->visible = true;

		fpsLabel->text = "  FPS: " + std::string(itoa(fps, (char*)&buff, 10));

		frameTimeLabel->text = " Frame:    " + std::string(itoa((int)(hrframeTime * 1000.0), (char*)&buff, 10)) + "ms";
		frameTimeBar->rect.right = 10 + (int)((float)(debugView->rect.right - debugView->rect.left - 20) * hrframeTime);

		evalTimeLabel->text = " Eval:     " + std::string(itoa((int)(hrevalTime * 1000.0), (char*)&buff, 10)) + "ms";
		evalTimeBar->rect.right = 10 + (int)((float)(debugView->rect.right - debugView->rect.left - 20) * hrevalTime);
		evalEventsLabel->text = "Events:  " + std::string(itoa((int)(hrevalEventsTime / hrevalTime * 100.0), (char*)&buff, 10)) + "%";
		evalEventsBar->rect.right = 10 + (int)((float)(debugView->rect.right - debugView->rect.left - 20) * hrevalEventsTime / hrevalTime);
		evalSpritesLabel->text = "Sprites:  " + std::string(itoa((int)(hrevalSpritesTime / hrevalTime * 100.0), (char*)&buff, 10)) + "%";
		evalSpritesBar->rect.right = 10 + (int)((float)(debugView->rect.right - debugView->rect.left - 20) * hrevalSpritesTime / hrevalTime);
		evalObjsLabel->text = "Objs:     " + std::string(itoa((int)(hrevalObjsTime / hrevalTime * 100.0), (char*)&buff, 10)) + "%";
		evalObjsBar->rect.right = 10 + (int)((float)(debugView->rect.right - debugView->rect.left - 20) * hrevalObjsTime / hrevalTime);

		drawTimeLabel->text = " Draw:     " + std::string(itoa((int)(hrdrawTime * 1000.0), (char*)&buff, 10)) + "ms";
		drawTimeBar->rect.right = 10 + (int)((float)(debugView->rect.right - debugView->rect.left - 20) * hrdrawTime);
		drawAnimsLabel->text = "Anims:    " + std::string(itoa((int)(hrdrawAnimsTime / hrdrawTime * 100.0), (char*)&buff, 10)) + "%";
		drawAnimsBar->rect.right = 10 + (int)((float)(debugView->rect.right - debugView->rect.left - 20) * hrdrawAnimsTime / hrdrawTime);
		drawUpdateLabel->text = "Update:   " + std::string(itoa((int)(hrdrawUpdateTime / hrdrawTime * 100.0), (char*)&buff, 10)) + "%";
		drawUpdateBar->rect.right = 10 + (int)((float)(debugView->rect.right - debugView->rect.left - 20) * hrdrawUpdateTime / hrdrawTime);
		drawLightsLabel->text = "Lights:   " + std::string(itoa((int)(hrdrawLightsTime / hrdrawTime * 100.0), (char*)&buff, 10)) + "%";
		drawLightsBar->rect.right = 10 + (int)((float)(debugView->rect.right - debugView->rect.left - 20) * hrdrawLightsTime / hrdrawTime);
		drawLightsLabel->visible = debugFlushing;
		drawLightsBar->visible = debugFlushing;
		drawTerrainLabel->text = "Terrain:  " + std::string(itoa((int)(hrdrawTerrainTime / hrdrawTime * 100.0), (char*)&buff, 10)) + "%";
		drawTerrainBar->rect.right = 10 + (int)((float)(debugView->rect.right - debugView->rect.left - 20) * hrdrawTerrainTime / hrdrawTime);
		drawTerrainLabel->visible = debugFlushing;
		drawTerrainBar->visible = debugFlushing;
		drawCloudsLabel->text = "Clouds:  " + std::string(itoa((int)(hrdrawCloudsTime / hrdrawTime * 100.0), (char*)&buff, 10)) + "%";
		drawCloudsBar->rect.right = 10 + (int)((float)(debugView->rect.right - debugView->rect.left - 20) * hrdrawCloudsTime / hrdrawTime);
		drawCloudsLabel->visible = debugFlushing;
		drawCloudsBar->visible = debugFlushing;
		drawObjsLabel->text = "Objs:     " + std::string(itoa((int)(hrdrawObjsTime / hrdrawTime * 100.0), (char*)&buff, 10)) + "%";
		drawObjsBar->rect.right = 10 + (int)((float)(debugView->rect.right - debugView->rect.left - 20) * hrdrawObjsTime / hrdrawTime);
		drawObjsLabel->visible = debugFlushing;
		drawObjsBar->visible = debugFlushing;
		drawSpritesLabel->text = "Sprites:  " + std::string(itoa((int)(hrdrawSpritesTime / hrdrawTime * 100.0), (char*)&buff, 10)) + "%";
		drawSpritesBar->rect.right = 10 + (int)((float)(debugView->rect.right - debugView->rect.left - 20) * hrdrawSpritesTime / hrdrawTime);
		drawSpritesLabel->visible = debugFlushing;
		drawSpritesBar->visible = debugFlushing;
		drawOverLabel->text = "Over:     " + std::string(itoa((int)(hrdrawOverTime / hrdrawTime * 100.0), (char*)&buff, 10)) + "%";
		drawOverBar->rect.right = 10 + (int)((float)(debugView->rect.right - debugView->rect.left - 20) * hrdrawOverTime / hrdrawTime);
		drawOverLabel->visible = debugFlushing;
		drawOverBar->visible = debugFlushing;
		drawUiLabel->text = "Ui:       " + std::string(itoa((int)(hrdrawUiTime / hrdrawTime * 100.0), (char*)&buff, 10)) + "%";
		drawUiBar->rect.right = 10 + (int)((float)(debugView->rect.right - debugView->rect.left - 20) * hrdrawUiTime / hrdrawTime);
		drawUiLabel->visible = debugFlushing;
		drawUiBar->visible = debugFlushing;
		drawPresentLabel->text = "Present:  " + std::string(itoa((int)(hrdrawPresentTime / hrdrawTime * 100.0), (char*)&buff, 10)) + "%";
		drawPresentBar->rect.right = 10 + (int)((float)(debugView->rect.right - debugView->rect.left - 20) * hrdrawPresentTime / hrdrawTime);
		
		debugView->needsUpdate = true; // will force update of all children
		genericDebugView->needsUpdate = true;
	}
	else
	{
		debugView->visible = false;
		genericDebugView->visible = false;
	}

	// update and draw
	for each (uiItem* uii in uiItems)
	{
		uii->update(0, 0, &mainVt, false);
	}
	mainUiRenderer->draw(dxDevice);

	// disable all the generics
	for (int i = 0; i < genericUiCount; i++)
	{
		genericLabel[i]->visible = false;
		genericBar[i]->visible = false;
	}
}

void drawFrame(LPDIRECT3DDEVICE9 dxDevice)
{
	// reset some stuff (these all do cumulative timing (because they can be done more than once per frame);
	DEBUG_HR_ZERO(&hrdrawCloudsTime);
	DEBUG_HR_ZERO(&hrdrawObjsTime);
	DEBUG_HR_ZERO(&hrdrawSpritesTime);
	DEBUG_HR_ZERO(&hrdrawTerrainTime);

	// this hasn't been organised yet
	dxDevice->SetRenderState(D3DRS_ZENABLE, true);
	dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);

	float lightCoof = 1.0f; // this may be purged

	DEBUG_HR_START(&hrsbstart);
	// anims
	runAnims();
	DEBUG_HR_END(&hrsbstart, &hrsbend, &hrdrawAnimsTime);

	// update models
	DEBUG_HR_START(&hrsbstart);
	//cloudObj->offset = camPos;
	//cloudObj->offset.y -= 400;
	cloudObj->update(true);
	cloudObj->model->noCull = true;
	mapObj->update(true);
	mapObj->model->noCull = true;
	for (int i = objs.size() - 1; i >= 0; i--)
	{
		objs[i]->update();
	}
	DEBUG_HR_END(&hrsbstart, &hrsbend, &hrdrawUpdateTime);

	DEBUG_HR_START(&hrsbstart);
	// drawLight
	for (int i = lights.size() - 1; i >= 0; i--)
	{
		if (lights[i]->lightEnabled)
			drawLight(dxDevice, lights[i]);
	}
	DEBUG_DX_FLUSH();
	
	DEBUG_HR_END(&hrsbstart, &hrsbend, &hrdrawLightsTime);

	// prep DynamicDecals
	for (int i = dynamicDecals.size() - 1; i >= 0; i--)
	{
		dynamicDecalData* ddd = dynamicDecals[i];
		if (ddd->decalEnabled)
		{
			prepDynamicDecal(dxDevice, ddd);
		}
	}

	D3DVIEWPORT9 vp = createViewPort(targetTexScale);
	dxDevice->SetViewport(&vp);

	if (wireFrame)
		dxDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	else
		dxDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	// drawView
	for (int i = views.size() - 1; i >= 0; i--)
	{
		if (views[i]->viewEnabled)
		{
			moveCameraView(dxDevice, views[i]); // sets eyePos/eyeDir - needed for ddat
			drawData viewDdat = createDrawDataView(dxDevice, lightCoof, views[i]);
			drawScene(dxDevice, &viewDdat, views[i], DF_default, SF_default); // timed
		}
	}

	disableClip(dxDevice);
	dxDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	// drawOver
	DEBUG_HR_START(&hrsbstart);
	for (int i = overs.size() - 1; i >= 0; i--)
	{
		if (overs[i]->overEnabled)
		{
			drawData overDdat = createDrawDataOver(lightCoof, &vp, overs[i]);
			drawOver(dxDevice, &overDdat, overs[i]); // timed
		}
	}
	DEBUG_DX_FLUSH();
	DEBUG_HR_END(&hrsbstart, &hrsbend, &hrdrawOverTime);

	if (!disableOpenTarget)
	{
		// draw scene
		moveCamera(dxDevice);
		dxDevice->SetRenderTarget(0, targetSurface);
		dxDevice->SetDepthStencilSurface(zSurface);

		dxDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 100, 200), 1.0f, 0); // don't really need this
		dxDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
		dxDevice->SetRenderState(D3DRS_ZENABLE, true);
		dxDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		dxDevice->BeginScene();

		if (wireFrame)
			dxDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		else
			dxDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

		D3DXMATRIX meh;
		D3DXMatrixIdentity(&meh);
		UINT numPasses, pass;

		drawData ddat = createDrawData(lightCoof, &vp);
		
		// special version of drawScene (because the main camera doesn't take a UNCRZ_view* yet)
		DEBUG_HR_START(&hrsbstart);
		cloudObj->draw(dxDevice, &ddat, DF_default);
		DEBUG_DX_FLUSH();
		DEBUG_HR_ACCEND(&hrsbstart, &hrsbend, &hrdrawCloudsTime);

		DEBUG_HR_START(&hrsbstart);
		mapObj->draw(dxDevice, &ddat, DF_default);
		DEBUG_DX_FLUSH();
		DEBUG_HR_ACCEND(&hrsbstart, &hrsbend, &hrdrawTerrainTime);

		DEBUG_HR_START(&hrsbstart);
		drawZBackToFront(zSortedObjs, zsoLocalIndexes, dxDevice, &ddat, DF_default);
		DEBUG_DX_FLUSH();
		DEBUG_HR_ACCEND(&hrsbstart, &hrsbend, &hrdrawObjsTime);

		DEBUG_HR_START(&hrsbstart);
		// colour
		if (fireSprites.size() > 0)
			fireSprite->draw(dxDevice, &ddat, &sbuff, &fireSprites.front(), 0, fireSprites.size(), DF_default, SD_colour);
		if (smokeSprites.size() > 0)
			smokeSprite->draw(dxDevice, &ddat, &sbuff, &smokeSprites.front(), 0, smokeSprites.size(), DF_default, SD_colour);
		if (laserSprites.size() > 0)
			laserSprite->draw(dxDevice, &ddat, &sbuff, &laserSprites.front(), 0, laserSprites.size(), DF_default, SD_colour);
		if (lpointSprites.size() > 0)
			lpointSprite->draw(dxDevice, &ddat, &sbuff, &lpointSprites.front(), 0, lpointSprites.size(), DF_default, SD_colour);
		// alpha
		if (fireSprites.size() > 0)
			fireSprite->draw(dxDevice, &ddat, &sbuff, &fireSprites.front(), 0, fireSprites.size(), DF_default, SD_depth);
		if (smokeSprites.size() > 0)
			smokeSprite->draw(dxDevice, &ddat, &sbuff, &smokeSprites.front(), 0, smokeSprites.size(), DF_default, SD_depth);
		if (laserSprites.size() > 0)
			laserSprite->draw(dxDevice, &ddat, &sbuff, &laserSprites.front(), 0, laserSprites.size(), DF_default, SD_depth);
		if (lpointSprites.size() > 0)
			lpointSprite->draw(dxDevice, &ddat, &sbuff, &lpointSprites.front(), 0, lpointSprites.size(), DF_default, SD_depth);
		DEBUG_DX_FLUSH();
		DEBUG_HR_ACCEND(&hrsbstart, &hrsbend, &hrdrawSpritesTime);

		// reset D3DRS_ZWRITEENABLE
		dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

		// while we still have access to the ddat
		ddat.nontimed(2, "CulledObjs: ", (float)ddat.cullCount / (float)(ddat.cullCount + ddat.drawCount), GDOF_prop100);


		// draw waste of space light indicators
		/*terrain->effect.effect->Begin(&numPasses, 0);
		terrain->effect.effect->BeginPass(0);

		terrain->effect.setViewProj(&viewProj);
		terrain->effect.effect->CommitChanges();

		dxDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		dxDevice->SetVertexDeclaration(vertexDecPC);
		vertexPC lVs[3];
		for (int i = lights.size() - 1; i >= 0; i--)
		{
			D3DXVECTOR3 lPos = D3DXVECTOR3(lights[i]->lightPos.x, lights[i]->lightPos.y, lights[i]->lightPos.z);
			lVs[0] = vertexPC(lPos, D3DXVECTOR3(1.0, 0.0, 0.0), -1);
			lVs[1] = vertexPC(lPos + D3DXVECTOR3(lights[i]->lightDir.x, lights[i]->lightDir.y, lights[i]->lightDir.z) , D3DXVECTOR3(0.0, 1.0, 0.0), -1);
			lVs[2] = vertexPC(lPos + lights[i]->lightUp, D3DXVECTOR3(0.0, 0.0, 1.0), -1);
			
			dxDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, &(lVs[0]), sizeof(vertexPC));
		}

		terrain->effect.effect->EndPass();	
		terrain->effect.effect->End();*/




		// from the old days of testing
		//for (int i = objs.size() - 1; i >= 1; i--)
		//{
		//	objs[i]->model->drawBBoxDebug(dxDevice, &ddat);
		//}



		// normals
		/*dxDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		terrain->effect.effect->SetTechnique("dull");
		terrain->effect.setViewProj(&viewProj);
		terrain->effect.effect->CommitChanges();

		terrain->effect.effect->Begin(&numPasses, 0);

		dxDevice->SetVertexDeclaration(vertexDecPC);

		for (pass = 0; pass < numPasses; pass++)
		{
			terrain->effect.effect->BeginPass(pass);

			dxDevice->DrawPrimitiveUP(D3DPT_LINELIST, (UINT)normalVis.size() / 2, (vertexPC*)&normalVis.front(), sizeof(vertexPC));

			terrain->effect.effect->EndPass();
		}
		terrain->effect.effect->End();*/

		dxDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

		dxDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		DEBUG_HR_START(&hrsbstart);
		ddat.sideSurface = targetSurface;
		ddat.sideTex = targetTex;
		ddat.targetSurface = finalTargetSurface;
		*ddat.targetVp = createViewPort(1.0); // this ensures the final draw and UI draws are done with a suitable VP

		aeffect.effect->SetTechnique(aeffect.effect->GetTechniqueByName("simple"));
		drawTargetOverFinalTarget(dxDevice, &ddat, &aeffect);

		DEBUG_DX_FLUSH();
		DEBUG_HR_END(&hrsbstart, &hrsbend, &hrdrawOverTime);
		
		dxDevice->EndScene();
	}

	DEBUG_HR_START(&hrsbstart);
	drawUi(dxDevice);
	DEBUG_DX_FLUSH();
	DEBUG_HR_END(&hrsbstart, &hrsbend, &hrdrawUiTime);


	dxDevice->GetViewport(&vp);
	RECT crect;
	GetClientRect(mainHWnd, &crect);
	int winWidth = crect.right - crect.left;
	int winHeight = crect.bottom - crect.top;
	mainVt = viewTrans(vpWidth, vpHeight, winWidth, winHeight);


	DEBUG_HR_START(&hrsbstart);
	dxDevice->Present(NULL, NULL, NULL, NULL);
	DEBUG_HR_END(&hrsbstart, &hrsbend, &hrdrawPresentTime);
}

void createVPMatrix(D3DXMATRIX* out, int x, int y, int w, int h, float minZ, float maxZ)
{
	*out = D3DXMATRIX(
		(float)w * 0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, (float)h * -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, maxZ - minZ, 0.0f,
		(float)w * 0.5f + x, (float)h * 0.5f + y, minZ, 1.0f);
}

drawData createDrawData(float lightCoof, D3DVIEWPORT9* vp)
{
	drawData ddat = drawData(&viewProj, &viewMatrix, &projMatrix, lights, dynamicDecals, lightCoof);

	ddat.performPlainPass = true;
	
	ddat.eyePos = eyePos;
	ddat.eyeDir = eyeDir;
	ddat.ticker = ticker;

	ddat.targetVp = vp;
	ddat.sideVp = new D3DVIEWPORT9();
	*ddat.sideVp = createViewPort(targetTexScale);
	
	ddat.targetTex = targetTex;
	ddat.targetSurface = targetSurface;
	ddat.sideTex = sideTex;
	ddat.sideSurface = sideSurface;

	ddat.zSideSurface = zSideSurface;
	ddat.zTargetSurface = zSurface;

	// debugging/hr
	ddat.hrsec = hrsec;
	ddat.debugData = debugData;
	ddat.debugFlushing = debugFlushing;
	ddat.flushQuery = flushQuery;
	ddat.genericDebugView = genericDebugView;
	ddat.genericLabel = genericLabel;
	ddat.genericBar = genericBar;

	return ddat;
}

drawData createDrawDataView(LPDIRECT3DDEVICE9 dxDevice, float lightCoof, UNCRZ_view* view)
{
	drawData ddat = drawData(&viewProj, &viewMatrix, &projMatrix, lights, dynamicDecals, lightCoof);

	ddat.performPlainPass = true;
	
	ddat.eyePos = eyePos; // err
	ddat.eyeDir = eyeDir; // err
	ddat.ticker = ticker;

	ddat.targetVp = &view->viewViewPort;
	ddat.sideVp = &view->viewViewPort;

	ddat.targetTex = view->targetTex;
	ddat.targetSurface = view->targetSurface;
	ddat.sideTex = view->sideTex;
	ddat.sideSurface = view->sideSurface;

	ddat.zTargetSurface = view->zTargetSurface;
	ddat.zSideSurface = view->zSideSurface;

	ddat.farDepth = view->projectionFar;

	// debugging/hr
	ddat.hrsec = hrsec;
	ddat.debugData = debugData;
	ddat.debugFlushing = debugFlushing;
	ddat.flushQuery = flushQuery;
	ddat.genericDebugView = genericDebugView;
	ddat.genericLabel = genericLabel;
	ddat.genericBar = genericBar;

	// clips
	if (view->useClips)
	{
		DWORD clipEnable = 0;
		for (int i = 0; i < maxClips; i++)
		{
			if (view->getClipEnable(i))
			{
				setClip(dxDevice, i, &view->clips[i]);
				clipEnable |= (1 << i);
			}
		}
		ddat.clipEnable = clipEnable;
	}
	else
	{
		ddat.clipEnable = 0;
	}

	ddat.enableClip(dxDevice);

	return ddat;
}

// this can be stripped considerably... (and createDrawDataView too
drawData createDrawDataOver(float lightCoof, D3DVIEWPORT9* vp, UNCRZ_over* over)
{
	drawData ddat = drawData(&viewProj, &viewMatrix, &projMatrix, lights, dynamicDecals, lightCoof);

	ddat.performPlainPass = true;
	
	ddat.eyePos = eyePos; // err
	ddat.eyeDir = eyeDir; // err
	ddat.ticker = ticker;

	ddat.targetVp = vp;
	ddat.sideVp = new D3DVIEWPORT9();
	*ddat.sideVp = createViewPort(targetTexScale);
	
	ddat.targetTex = over->targetTex;
	ddat.targetSurface = over->targetSurface;
	ddat.sideTex = sideTex;
	ddat.sideSurface = sideSurface;

	ddat.zSideSurface = zSideSurface;
	ddat.zTargetSurface = zSurface;

	// debugging/hr
	ddat.hrsec = hrsec;
	ddat.debugData = debugData;
	ddat.debugFlushing = debugFlushing;
	ddat.flushQuery = flushQuery;
	ddat.genericDebugView = genericDebugView;
	ddat.genericLabel = genericLabel;
	ddat.genericBar = genericBar;

	return ddat;
}

D3DVIEWPORT9 createViewPort(float scale)
{
	D3DVIEWPORT9 vp;
	vp.X = 0;
	vp.Y = 0;
	vp.Width = (float)vpWidth * scale;
	vp.Height = (float)vpHeight * scale;
	vp.MaxZ = 1.0f;
	vp.MinZ = 0.0f;

	return vp;
}

D3DVIEWPORT9 createViewPort(UINT w, UINT h)
{
	D3DVIEWPORT9 vp;
	vp.X = 0;
	vp.Y = 0;
	vp.Width = w;
	vp.Height = h;
	vp.MaxZ = 1.0f;
	vp.MinZ = 0.0f;

	return vp;
}

void setCloudStates(LPDIRECT3DDEVICE9 dxDevice)
{
	//dxDevice->SetRenderState(D3DRS_CLIPPING, false);
	dxDevice->SetRenderState(D3DRS_ZENABLE, false); // clouds don't suffer z-ness
	dxDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);
}

// note that sprites do their own thing
void setSceneStates(LPDIRECT3DDEVICE9 dxDevice)
{
	//dxDevice->SetRenderState(D3DRS_CLIPPING, true);
	dxDevice->SetRenderState(D3DRS_ZENABLE, true);
	dxDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	//dxDevice->SetRenderState(D3DRS_STENCILENABLE, false);
	//dxDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
	//dxDevice->SetRenderState(D3DRS_BLENDFACTOR, 0xffffffff);
}

void setLightStates(LPDIRECT3DDEVICE9 dxDevice)
{
	//dxDevice->SetRenderState(D3DRS_CLIPPING, true);
	dxDevice->SetRenderState(D3DRS_ZENABLE, true);
	dxDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
}

void setOverStates(LPDIRECT3DDEVICE9 dxDevice)
{
	//dxDevice->SetRenderState(D3DRS_CLIPPING, false);
	dxDevice->SetRenderState(D3DRS_ZENABLE, false);
	dxDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);
}

void setAlpha(LPDIRECT3DDEVICE9 dxDevice, DWORD alphaMode)
{
	switch (alphaMode)
	{
	case AM_none:
		dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		break;
	case AM_nice:
		dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		break;
	case AM_add:
		dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE); // formerly D3DBLEND_SRCALPHA
		dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		break;
	}
}

// does some actual drawing
void drawScene(LPDIRECT3DDEVICE9 dxDevice, drawData* ddat, UNCRZ_view* view, DWORD drawArgs, DWORD sceneArgs)
{
	// horrid stuff
	if (view->name == "main_over" || view->name == "main_under")
	{
		getSec(mapObj->model, "water")->sectionEnabled = false;
	}
	else
	{
		getSec(mapObj->model, "water")->sectionEnabled = true;
	}
	//

	dxDevice->SetRenderTarget(0, view->targetSurface);
	dxDevice->SetDepthStencilSurface(view->zSideSurface);

	//D3DVIEWPORT9 vp = createViewPort(view->texWidth, view->texHeight);
	//view->viewViewPort = vp;
	//dxDevice->SetViewport(&vp);

	if (view->clearView)
		dxDevice->Clear(0, NULL, D3DCLEAR_TARGET, view->clearColor, 1.0f, 0);
	dxDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	
	setAlpha(dxDevice, view->alphaMode); // this generally won't achieve anything (because models will mess with stuff themselves)

	dxDevice->BeginScene();

	if (sceneArgs & SF_notimed)
	{
		if ((sceneArgs & SF_noclouds) == false)
		{
			setCloudStates(dxDevice);
			cloudObj->draw(dxDevice, ddat, drawArgs);
		}
		setSceneStates(dxDevice);

		mapObj->draw(dxDevice, ddat, drawArgs);
		drawZBackToFront(zSortedObjs, view->zsoLocalIndexes, dxDevice, ddat, drawArgs);

		// colour
		if (fireSprites.size() > 0)
			fireSprite->draw(dxDevice, ddat, &sbuff, &fireSprites.front(), 0, fireSprites.size(), DF_default, SD_colour);
		if (smokeSprites.size() > 0)
			smokeSprite->draw(dxDevice, ddat, &sbuff, &smokeSprites.front(), 0, smokeSprites.size(), DF_default, SD_colour);
		if (laserSprites.size() > 0)
			laserSprite->draw(dxDevice, ddat, &sbuff, &laserSprites.front(), 0, laserSprites.size(), DF_default, SD_colour);
		if (lpointSprites.size() > 0)
			lpointSprite->draw(dxDevice, ddat, &sbuff, &lpointSprites.front(), 0, lpointSprites.size(), DF_default, SD_colour);
		// alpha
		if (fireSprites.size() > 0)
			fireSprite->draw(dxDevice, ddat, &sbuff, &fireSprites.front(), 0, fireSprites.size(), DF_default, SD_depth);
		if (smokeSprites.size() > 0)
			smokeSprite->draw(dxDevice, ddat, &sbuff, &smokeSprites.front(), 0, smokeSprites.size(), DF_default, SD_depth);
		if (laserSprites.size() > 0)
			laserSprite->draw(dxDevice, ddat, &sbuff, &laserSprites.front(), 0, laserSprites.size(), DF_default, SD_depth);
		if (lpointSprites.size() > 0)
			lpointSprite->draw(dxDevice, ddat, &sbuff, &lpointSprites.front(), 0, lpointSprites.size(), DF_default, SD_depth);
	}
	else
	{
		if ((sceneArgs & SF_noclouds) == false)
		{
			DEBUG_HR_START(&hrsbstart);
			setCloudStates(dxDevice);
			cloudObj->draw(dxDevice, ddat, drawArgs);
			DEBUG_DX_FLUSH();
			DEBUG_HR_ACCEND(&hrsbstart, &hrsbend, &hrdrawCloudsTime);
		}
		setSceneStates(dxDevice);

		DEBUG_HR_START(&hrsbstart);
		mapObj->draw(dxDevice, ddat, drawArgs);
		DEBUG_DX_FLUSH();
		DEBUG_HR_ACCEND(&hrsbstart, &hrsbend, &hrdrawTerrainTime);

		DEBUG_HR_START(&hrsbstart);
		drawZBackToFront(zSortedObjs, view->zsoLocalIndexes, dxDevice, ddat, drawArgs);
		DEBUG_DX_FLUSH();
		DEBUG_HR_ACCEND(&hrsbstart, &hrsbend, &hrdrawObjsTime);

		DEBUG_HR_START(&hrsbstart);
		// colour
		if (fireSprites.size() > 0)
			fireSprite->draw(dxDevice, ddat, &sbuff, &fireSprites.front(), 0, fireSprites.size(), DF_default, SD_colour);
		if (smokeSprites.size() > 0)
			smokeSprite->draw(dxDevice, ddat, &sbuff, &smokeSprites.front(), 0, smokeSprites.size(), DF_default, SD_colour);
		if (laserSprites.size() > 0)
			laserSprite->draw(dxDevice, ddat, &sbuff, &laserSprites.front(), 0, laserSprites.size(), DF_default, SD_colour);
		if (lpointSprites.size() > 0)
			lpointSprite->draw(dxDevice, ddat, &sbuff, &lpointSprites.front(), 0, lpointSprites.size(), DF_default, SD_colour);
		// alpha
		if (fireSprites.size() > 0)
			fireSprite->draw(dxDevice, ddat, &sbuff, &fireSprites.front(), 0, fireSprites.size(), DF_default, SD_depth);
		if (smokeSprites.size() > 0)
			smokeSprite->draw(dxDevice, ddat, &sbuff, &smokeSprites.front(), 0, smokeSprites.size(), DF_default, SD_depth);
		if (laserSprites.size() > 0)
			laserSprite->draw(dxDevice, ddat, &sbuff, &laserSprites.front(), 0, laserSprites.size(), DF_default, SD_depth);
		if (lpointSprites.size() > 0)
			lpointSprite->draw(dxDevice, ddat, &sbuff, &lpointSprites.front(), 0, lpointSprites.size(), DF_default, SD_depth);
		DEBUG_DX_FLUSH();
		DEBUG_HR_ACCEND(&hrsbstart, &hrsbend, &hrdrawSpritesTime);
	}

	dxDevice->EndScene();
	
	ddat->nontimed(2, "CulledObjs: ", (float)ddat->cullCount / (float)(ddat->cullCount + ddat->drawCount), GDOF_prop100);
}

void drawOver(LPDIRECT3DDEVICE9 dxDevice, drawData* ddat, UNCRZ_over* over)
{
	D3DXMATRIX idMat;
	D3DXMatrixIdentity(&idMat);
	dxDevice->SetRenderTarget(0, ddat->targetSurface);
	dxDevice->SetDepthStencilSurface(over->zSurface);

	D3DVIEWPORT9 vp = createViewPort(over->texWidth, over->texHeight);
	dxDevice->SetViewport(&vp);

	dxDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	setOverStates(dxDevice);

	dxDevice->BeginScene();

	vertexOver overVerts[4]; // make this const / VBuff
	overVerts[0] = vertexOver(-1, -1, 0, 0, 1);
	overVerts[1] = vertexOver(-1, 1, 0, 0, 0);
	overVerts[2] = vertexOver(1, -1, 0, 1, 1);
	overVerts[3] = vertexOver(1, 1, 0, 1, 0);

	D3DXVECTOR4 texData = D3DXVECTOR4(0.5 / (float)ddat->targetVp->Width, 0.5 / (float)ddat->targetVp->Height, 1.0 / (float)ddat->targetVp->Width, 1.0 / (float)ddat->targetVp->Height);
	over->effect.setTextureData((float*)&texData.x);
	
	for (int i = 0; i < 4; i++) // do ahead of shader
	{
		overVerts[i].tu += texData.x;
		overVerts[i].tv += texData.y;
	}

	over->setTextures();
	over->effect.setEyePos(&ddat->eyePos);
	over->effect.setEyeDir(&ddat->eyeDir);
	over->effect.setTicker(ddat->ticker);
	over->effect.effect->SetTechnique(over->overTech);
	over->effect.setViewProj(&idMat);
	over->effect.effect->CommitChanges();
	over->effect.setcolMod(over->colMod);

	setAlpha(dxDevice, over->alphaMode);

	UINT numPasses;
	over->effect.effect->Begin(&numPasses, 0);
	for (int i = 0; i < numPasses; i++)
	{
		over->effect.effect->BeginPass(i);

		dxDevice->SetVertexDeclaration(vertexDecOver);
		dxDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, overVerts, sizeof(vertexOver));

		over->effect.effect->EndPass();
	}
	over->effect.effect->End();
	
	dxDevice->SetRenderState(D3DRS_ZENABLE, true);
	dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	
	dxDevice->EndScene();
}

void drawLight(LPDIRECT3DDEVICE9 dxDevice, lightData* ld)
{
	if (ld->useLightMap == false)
		return;

	dxDevice->SetRenderTarget(0, ld->lightSurface);
	dxDevice->SetDepthStencilSurface(zLightSurface);

	D3DVIEWPORT9 vp = createViewPort(ld->texWidth, ld->texHeight);
	dxDevice->SetViewport(&vp);

	moveCameraLight(dxDevice, ld);
	//disableClip(dxDevice);

	dxDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(255, 0, 0), 1.0f, 0); // (all of texture used for depth (ish))
	dxDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	setLightStates(dxDevice);
	dxDevice->BeginScene();

	drawData ddat = createDrawData(0.0f, &vp);
	ddat.lightType = ld->lightType;
	ddat.lightDepth = ld->lightDepth;
	ddat.lightConeness = ld->coneness;

	// disgusting clouds states
	//dxDevice->SetRenderState(D3DRS_ZENABLE, false); // clouds don't suffer z-ness
	//dxDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	//dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);
	//cloudObj->draw(dxDevice, &ddat, DF_light); // just gets in the way
	//dxDevice->SetRenderState(D3DRS_ZENABLE, true);
	//dxDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	//dxDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	mapObj->draw(dxDevice, &ddat, DF_light);
	drawZBackToFront(zSortedObjs, ld->zsoLocalIndexes, dxDevice, &ddat, DF_light);
	if (fireSprites.size() > 0)
		fireSprite->draw(dxDevice, &ddat, &sbuff, &fireSprites.front(), 0, fireSprites.size(), DF_light, SD_default);
	if (smokeSprites.size() > 0)
		smokeSprite->draw(dxDevice, &ddat, &sbuff, &smokeSprites.front(), 0, smokeSprites.size(), DF_light, SD_default);
	if (laserSprites.size() > 0)
		laserSprite->draw(dxDevice, &ddat, &sbuff, &laserSprites.front(), 0, laserSprites.size(), DF_light, SD_default);
	if (lpointSprites.size() > 0)
		lpointSprite->draw(dxDevice, &ddat, &sbuff, &lpointSprites.front(), 0, lpointSprites.size(), DF_light, SD_default);

	dxDevice->EndScene();
}

void prepDynamicDecal(LPDIRECT3DDEVICE9 dxDevice, dynamicDecalData* ddd)
{
	moveCameraDynamicDecal(dxDevice, ddd);
}

void initTextures(LPDIRECT3DDEVICE9 dxDevice)
{
	D3DVIEWPORT9 vp;
	dxDevice->GetViewport(&vp);

	vp.Width;
	vp.Height;

	vpWidth = vp.Width;
	vpHeight = vp.Height;

	//D3DXCreateTexture(dxDevice, vp.Width, vp.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &waterReflectTex);
	//waterReflectTex->GetSurfaceLevel(0, &waterReflectSurface);
	//
	//D3DXCreateTexture(dxDevice, vp.Width, vp.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &waterRefractTex);
	//waterRefractTex->GetSurfaceLevel(0, &waterRefractSurface);
	//
	//D3DXCreateTexture(dxDevice, vp.Width, vp.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &underTex);
	//underTex->GetSurfaceLevel(0, &underSurface);

	// side
	D3DXCreateTexture(dxDevice, vp.Width * targetTexScale, vp.Height * targetTexScale, 0, D3DUSAGE_RENDERTARGET, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, &sideTex);
	sideTex->GetSurfaceLevel(0, &sideSurface);
	dxDevice->CreateDepthStencilSurface(vp.Width * targetTexScale, vp.Height * targetTexScale, D3DFMT_D16, UNCRZ_STDSAMPLE, 0, TRUE, &zSideSurface, NULL);
	
	// target
	D3DXCreateTexture(dxDevice, vp.Width * targetTexScale, vp.Height * targetTexScale, 0, D3DUSAGE_RENDERTARGET, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, &targetTex);
	targetTex->GetSurfaceLevel(0, &targetSurface);
	dxDevice->CreateDepthStencilSurface(vp.Width * targetTexScale, vp.Height * targetTexScale, D3DFMT_D16, UNCRZ_STDSAMPLE, 0, TRUE, &zSurface, NULL);
	
	// light
	dxDevice->CreateDepthStencilSurface(vp.Width * lightTexScale, vp.Height * lightTexScale, D3DFMT_D16, UNCRZ_STDSAMPLE, 0, TRUE, &zLightSurface, NULL);

	HRESULT res;
	//res = D3DXCreateTextureFromFile(dxDevice, "ripples.tga", &ripplesTex);
	//res = res;


	// targettexture linking

	UNCRZ_texture* tex = new UNCRZ_texture("target", targetTex);
	textures.push_back(tex);
}

void initViews(LPDIRECT3DDEVICE9 dxDevice)
{
	UNCRZ_view* tempView;

	tempView = new UNCRZ_view("main");
	tempView->camPos = D3DXVECTOR3(-30, 20, -30);
	tempView->camPos += D3DXVECTOR3(0.5, 0, -0.5);
	tempView->dirNormalAt(D3DXVECTOR3(0, 1, 0));
	tempView->initTarget(dxDevice, vpWidth * targetTexScale, vpHeight * targetTexScale, D3DFMT_A8B8G8R8);
	tempView->initSide(sideTex, sideSurface);
	tempView->initTargetStencil(zSurface);
	tempView->initSideStencil(zSideSurface);
	tempView->dimX = FOV;
	tempView->dimY = vpWidth / (float)vpHeight;// / 2.0;
	tempView->viewMode = VM_persp;
	tempView->alphaMode = AM_none;
	tempView->clearView = true;
	tempView->clearColor = D3DCOLOR_XRGB(0, 0, 0);
	tempView->projectionNear = 1;
	tempView->projectionFar = 1500;
	views.push_back(tempView);

	float waterTexScale = 0.5;
	tempView = new UNCRZ_view("main_under");
	tempView->camPos = D3DXVECTOR3(-30, 20, -30);
	tempView->camPos += D3DXVECTOR3(0.5, 0, -0.5);
	tempView->dirNormalAt(D3DXVECTOR3(0, 1, 0));
	tempView->initTarget(dxDevice, vpWidth * targetTexScale * waterTexScale, vpHeight * targetTexScale * waterTexScale, D3DFMT_A8B8G8R8);
	tempView->initSide(dxDevice, D3DFMT_A8B8G8R8);
	tempView->initTargetStencil(zSurface);
	tempView->initSideStencil(zSideSurface);
	tempView->dimX = FOV;
	tempView->dimY = vpWidth / (float)vpHeight;// / 2.0;
	tempView->viewMode = VM_persp;
	tempView->alphaMode = AM_none;
	tempView->clearView = true;
	tempView->clearColor = D3DCOLOR_XRGB(0, 0, 0);
	tempView->projectionNear = 1;
	tempView->projectionFar = 1500;
	tempView->setClip(0, 0, 1, 0, 0, true);
	tempView->useClips = true;
	views.push_back(tempView);

	tempView = new UNCRZ_view("main_over");
	tempView->camPos = D3DXVECTOR3(-30, 20, -30);
	tempView->camPos += D3DXVECTOR3(0.5, 0, -0.5);
	tempView->dirNormalAt(D3DXVECTOR3(0, 1, 0));
	tempView->initTarget(dxDevice, vpWidth * targetTexScale * waterTexScale, vpHeight * targetTexScale * waterTexScale, D3DFMT_A8B8G8R8);
	tempView->initSide(dxDevice, D3DFMT_A8B8G8R8);
	tempView->initTargetStencil(zSurface);
	tempView->initSideStencil(zSideSurface);
	tempView->dimX = FOV;
	tempView->dimY = vpWidth / (float)vpHeight;// / 2.0;
	tempView->viewMode = VM_persp;
	tempView->alphaMode = AM_none;
	tempView->clearView = true;
	tempView->clearColor = D3DCOLOR_XRGB(0, 0, 0);
	tempView->projectionNear = 1;
	tempView->projectionFar = 1500;
	tempView->setClip(0, 0, -1, 0, 0.4, true);
	tempView->useClips = true;
	views.push_back(tempView);

	// this bit is IMPERATIVE (targettexture linking)
	for (int i = views.size() - 1; i >= 0; i--)
	{
		UNCRZ_texture* tex = new UNCRZ_texture(std::string("view_") + views[i]->name, views[i]->targetTex);
		textures.push_back(tex);
		setMatrix(std::string("view_") + views[i]->name + "_viewproj", &views[i]->viewViewProjVP, &matrices);
	}
}

void initOvers(LPDIRECT3DDEVICE9 dxDevice)
{
	UNCRZ_over* tempOver;

	tempOver = new UNCRZ_over("main");
	tempOver->init(dxDevice, vpWidth * targetTexScale, vpHeight * targetTexScale, "un_shade.fx", "over_final_fun", &effects, D3DFMT_A8B8G8R8);
	tempOver->initStencil(zSurface);
	tempOver->useTex = true; // write a "loadtex" func or something?
	createTexture(dxDevice, "view_main", &tempOver->tex, &textures);
	tempOver->useTex0 = true;
	createTexture(dxDevice, "ui/overtex0.tga", &tempOver->tex0, &textures);
	tempOver->useTex1 = true;
	createTexture(dxDevice, "ui/overtex1.tga", &tempOver->tex1, &textures);
	tempOver->useTex2 = true;
	createTexture(dxDevice, "ui/overtex2.tga", &tempOver->tex2, &textures);
	tempOver->useTex3 = true;
	createTexture(dxDevice, "ui/overtex3.tga", &tempOver->tex3, &textures);
	tempOver->alphaMode = AM_nice;
	tempOver->clearView = true;
	tempOver->clearColor = D3DCOLOR_XRGB(0, 0, 0);
	tempOver->colMod = D3DXVECTOR4(1, 1, 1, 1);
	overs.push_back(tempOver);

	// this bit is IMPERATIVE (targettexture linking)
	for (int i = overs.size() - 1; i >= 0; i--)
	{
		UNCRZ_texture* tex = new UNCRZ_texture(std::string("over_") + overs[i]->name, overs[i]->targetTex);
		textures.push_back(tex);
	}
}

void initLights(LPDIRECT3DDEVICE9 dxDevice)
{
	// note that lightColMod W value is not really used by the simple shaders

	dynamicDecalData* ddd;
	lightData* ld;

	ld = new lightData("pointlight");

	ld->lightEnabled = true;
	ld->lightType = LT_point;
	ld->lightDepth = 200;
	ld->lightDir = D3DXVECTOR4(0, 0, 1, 0.0); // not used
	ld->lightPos = D3DXVECTOR4(0, 100, 0, 0.0);
	ld->lightUp = D3DXVECTOR3(1, 0, 0); // not used
	ld->lightAmbient = D3DXVECTOR4(0.1, 0.1, 0.1, 1);
	ld->lightColMod = D3DXVECTOR4(1.0, 1.0, 1.0, 1);
	ld->useLightMap = false;

	lights.push_back(ld);

	int n = 8;
	for (int i = 0; i < n; i++)
	{
		ld = new lightData("spinlight");

		ld->lightEnabled = true;
		ld->lightType = LT_point;
		ld->lightDepth = 30;
		ld->lightDir = D3DXVECTOR4(0, 0, 1, 0.0); // not used
		ld->lightPos = D3DXVECTOR4(0, 10, 0, 0.0);
		ld->lightUp = D3DXVECTOR3(1, 0, 0); // not used
		ld->lightAmbient = D3DXVECTOR4(0.0, 0.0, 0.0, 0.0);
		ld->lightColMod = D3DXVECTOR4(5.0, 5.0, 5.0, 1);
		ld->useLightMap = false;

		lights.push_back(ld);

		ddd = new dynamicDecalData();

		ddd->lightType = LT_ortho;
		ddd->lightDepth = 200;
		ddd->lightDir = D3DXVECTOR4(0, -1, 0, 0.0);
		ddd->lightPos = D3DXVECTOR4(90, 100, 0, 0.0);
		ddd->lightUp = D3DXVECTOR3(1, 0, 0); // not used
		ddd->dimX = 20;
		ddd->dimY = 20;
		ddd->lightColMod = D3DXVECTOR4(1.0, 1.0, 1.0, 1);
		ddd->init(dxDevice, "b_ring.tga", &textures);
		ddd->decalEnabled = true;

		dynamicDecals.push_back(ddd);
	}

	//

	//ld = new lightData("sun");

	//ld->lightEnabled = true;
	//ld->lightType = LT_ortho;
	//ld->dimX = 50;
	//ld->dimY = 50;
	//ld->lightDepth = 80;
	//ld->lightDir = D3DXVECTOR4(0, -0.9, 0.1, 0.0);
	//ld->lightPos = D3DXVECTOR4(0, 0, 0, 0.0) - ld->lightDir * 40;
	//ld->lightUp = D3DXVECTOR3(1, 0, 0);
	//ld->lightAmbient = D3DXVECTOR4(0, 0, 0, 0);
	//ld->lightColMod = D3DXVECTOR4(1, 1, 1, 1);
	//ld->init(dxDevice, vpWidth * lightTexScale, vpHeight * lightTexScale, "lightPattern.tga", &textures); // MAXIMUM SIZE
	//ld->useLightMap = true;

	//lights.push_back(ld);



	// disable these lights while testing
	/*ld = new lightData();

	ld->lightEnabled = true;
	ld->lightType = LT_point;
	ld->lightDepth = 100;
	ld->lightDir = D3DXVECTOR4(0, 0, 1, 0.0); // not used
	ld->lightPos = D3DXVECTOR4(0, 20, 0, 0.0);
	ld->lightUp = D3DXVECTOR3(1, 0, 0); // not used
	ld->useLightMap = false;

	lights.push_back(ld);


	ld = new lightData();

	ld->lightType = LT_persp;
	ld->dimX = FOV;
	ld->dimY = 1;
	ld->lightDepth = 50;
	ld->lightDir = D3DXVECTOR4(0.0, -10, 0.0, 0.0);
	ld->lightPos = D3DXVECTOR4(0, 20.0, 0, 0.0);
	ld->lightUp = D3DXVECTOR3(0, 1, 0);
	ld->init(dxDevice, vpWidth, vpHeight, "lightPattern1.tga");
	ld->useLightMap = true;

	lights.push_back(ld);


	ld = new lightData();

	ld->lightType = LT_persp;
	ld->dimX = FOV * 0.6f;
	ld->dimY = 1;
	ld->lightDepth = 100;
	ld->lightDir = D3DXVECTOR4(0.1, -10, 0.0, 0.0);
	ld->lightPos = D3DXVECTOR4(100, 20.0, 0.0, 0.0);
	ld->lightUp = D3DXVECTOR3(0, 1, 0);
	ld->init(dxDevice, vpWidth, vpHeight, "lightPattern2.tga");
	ld->useLightMap = true;

	lights.push_back(ld);


	ld = new lightData();

	ld->lightType = LT_point;
	ld->lightDepth = 100;
	ld->lightDir = D3DXVECTOR4(0, 0, 1, 0.0); // not used
	ld->lightPos = D3DXVECTOR4(0, 20, 0, 0.0);
	ld->lightUp = D3DXVECTOR3(1, 0, 0); // not useds
	ld->useLightMap = false;

	lights.push_back(ld);*/

	// this bit is IMPERATIVE (targettexture linking)
	for (int i = lights.size() - 1; i >= 0; i--)
	{
		if (lights[i]->useLightMap)
		{
			UNCRZ_texture* tex = new UNCRZ_texture(std::string("light_") + lights[i]->name, lights[i]->lightTex);
			textures.push_back(tex);
		}
	}
}

void initOther()
{
}

void texAlignViewProj(LPD3DXMATRIX vp)
{
	D3DXMATRIX translate, scale;

	D3DXMatrixScaling(&scale, 0.5, -0.5, 1.0);
	D3DXMatrixMultiply(vp, vp, &scale);
	D3DXMatrixTranslation(&translate, 0.5, 0.5, 0);
	D3DXMatrixMultiply(vp, vp, &translate);
}

void moveCamera_persp()
{
	D3DXMATRIXA16 rotMatrix;

	D3DXVECTOR3 eyeVec = camPos;
	D3DXVECTOR3 targVec(1, 0, 0);  // ??
	D3DXVECTOR3 upVec(0, 1, 0);

	D3DXMatrixRotationZ(&rotMatrix, rotPar);
	D3DXVec3TransformNormal(&targVec, &targVec, &rotMatrix);

	D3DXMatrixRotationY(&rotMatrix, rotY);
	D3DXVec3TransformNormal(&targVec, &targVec, &rotMatrix);

	targVec += eyeVec;

	D3DXMatrixLookAtLH(&viewMatrix, &eyeVec, &targVec, &upVec);
 
	D3DXMatrixPerspectiveFovLH(&projMatrix, FOV, mainVt.winWidth / mainVt.winHeight, 1, farDepth);
	D3DXMatrixMultiply(&viewProj, &viewMatrix, &projMatrix);

	eyePos = D3DXVECTOR4(eyeVec, 0);
	eyeDir = D3DXVECTOR4(targVec, 0) - eyePos;
}

void moveCamera(LPDIRECT3DDEVICE9 dxDevice)
{
	switch (viewMode)
	{
	case VM_persp:
		moveCamera_persp();
		break;
	}
}

void moveCameraView_ortho(UNCRZ_view* view)
{
	D3DXVECTOR3 eyeVec = view->camPos;
	D3DXVECTOR3 targVec = eyeVec + view->camDir;
	D3DXVECTOR3 upVec = view->camUp;

	D3DXMatrixLookAtLH(&viewMatrix, &eyeVec, &targVec, &upVec);
 
	D3DXMatrixOrthoLH(&projMatrix, view->dimX, view->dimY, view->projectionNear, view->projectionFar);
	D3DXMatrixMultiply(&viewProj, &viewMatrix, &projMatrix);

	texAlignViewProj(&viewProj);

	eyePos = D3DXVECTOR4(eyeVec, 0);
	eyeDir = D3DXVECTOR4(targVec, 0) - eyePos;
	
	float edMod = sqrtf(eyeDir.x * eyeDir.x + eyeDir.y * eyeDir.y + eyeDir.z * eyeDir.z);
	eyeDir.x /= edMod;
	eyeDir.y /= edMod;
	eyeDir.z /= edMod;
}

void moveCameraView_persp(UNCRZ_view* view)
{
	D3DXVECTOR3 eyeVec = view->camPos;
	D3DXVECTOR3 targVec = eyeVec + view->camDir;
	D3DXVECTOR3 upVec = view->camUp;

	D3DXMatrixLookAtLH(&viewMatrix, &eyeVec, &targVec, &upVec);
 
	D3DXMatrixPerspectiveFovLH(&projMatrix, view->dimX, view->dimY, view->projectionNear, view->projectionFar);
	D3DXMatrixMultiply(&viewProj, &viewMatrix, &projMatrix);

	eyePos = D3DXVECTOR4(eyeVec, 0);
	eyeDir = D3DXVECTOR4(targVec, 0) - eyePos;
	
	float edMod = sqrtf(eyeDir.x * eyeDir.x + eyeDir.y * eyeDir.y + eyeDir.z * eyeDir.z);
	eyeDir.x /= edMod;
	eyeDir.y /= edMod;
	eyeDir.z /= edMod;
}

void moveCameraView(LPDIRECT3DDEVICE9 dxDevice, UNCRZ_view* view)
{
	D3DVIEWPORT9 vp = createViewPort(view->texWidth, view->texHeight);
	view->viewViewPort = vp;
	dxDevice->SetViewport(&vp);

	if (view->viewMode == VM_ortho)
		moveCameraView_ortho(view);
	else if (view->viewMode == VM_persp)
		moveCameraView_persp(view);

	view->viewViewMat = viewMatrix;
	view->viewProjMat = projMatrix;
	view->viewViewProjVP = viewProj;
	view->viewViewProj = viewProj;

	texAlignViewProj(&view->viewViewProj);
}

/*void moveCamerawaterReflect(LPDIRECT3DDEVICE9 dxDevice)
{
	D3DXMATRIXA16 rotMatrix;

	D3DXVECTOR3 eyeVec = camPos;
	eyeVec.y = 1.0f - eyeVec.y;
	D3DXVECTOR3 targVec(1, 0, 0);  // ??
	D3DXVECTOR3 upVec(0, 1, 0);

	D3DXMatrixRotationZ(&rotMatrix, rotPar + xVPM);
	D3DXVec3TransformNormal(&targVec, &targVec, &rotMatrix);

	targVec.y = 0.0f - targVec.y;

	D3DXMatrixRotationY(&rotMatrix, rotY + yVPM);
	D3DXVec3TransformNormal(&targVec, &targVec, &rotMatrix);

	targVec += eyeVec;

	D3DXMatrixLookAtLH(&viewMatrix, &eyeVec, &targVec, &upVec);
 
	D3DXMatrixPerspectiveFovLH(&projMatrix, FOV, 500/500, 1, farDepth);
	D3DXMatrixMultiply(&viewProj, &viewMatrix, &projMatrix);
}

void moveCamerawaterRefract(LPDIRECT3DDEVICE9 dxDevice)
{
	D3DXMATRIXA16 rotMatrix;

	D3DXVECTOR3 eyeVec = camPos;
	//eyeVec.y = 1.0f - eyeVec.y;
	D3DXVECTOR3 targVec(1, 0, 0);  // ??
	D3DXVECTOR3 upVec(0, 1, 0);

	D3DXMatrixRotationZ(&rotMatrix, rotPar + xVPM);
	D3DXVec3TransformNormal(&targVec, &targVec, &rotMatrix);

	//targVec.y = - targVec.y;

	D3DXMatrixRotationY(&rotMatrix, rotY + yVPM);
	D3DXVec3TransformNormal(&targVec, &targVec, &rotMatrix);

	targVec += eyeVec;

	D3DXMatrixLookAtLH(&viewMatrix, &eyeVec, &targVec, &upVec);
 
	D3DXMatrixPerspectiveFovLH(&projMatrix, FOV, 500/500, 1, farDepth);
	D3DXMatrixMultiply(&viewProj, &viewMatrix, &projMatrix);
}*/

void moveCameraLight_ortho(LPDIRECT3DDEVICE9 dxDevice, lightData* ld)
{
	D3DXVECTOR3 eyeVec(ld->lightPos.x, ld->lightPos.y, ld->lightPos.z);
	D3DXVECTOR3 targVec(ld->lightDir.x, ld->lightDir.y, ld->lightDir.z);
	D3DXVECTOR3 upVec = ld->lightUp;

	targVec += eyeVec;

	D3DXMatrixLookAtLH(&viewMatrix, &eyeVec, &targVec, &upVec);
 
	D3DXMatrixOrthoLH(&projMatrix, ld->dimX, ld->dimY, 0, ld->lightDepth);
	D3DXMatrixMultiply(&viewProj, &viewMatrix, &projMatrix);
}

void moveCameraLight_persp(LPDIRECT3DDEVICE9 dxDevice, lightData* ld)
{
	D3DXVECTOR3 eyeVec = ld->lightPos;
	D3DXVECTOR3 targVec(ld->lightDir.x, ld->lightDir.y, ld->lightDir.z);
	D3DXVECTOR3 upVec = ld->lightUp;

	targVec += eyeVec;

	D3DXMatrixLookAtLH(&viewMatrix, &eyeVec, &targVec, &upVec);
 
	D3DXMatrixPerspectiveFovLH(&projMatrix, ld->dimX, ld->dimY, 0, ld->lightDepth);
	D3DXMatrixMultiply(&viewProj, &viewMatrix, &projMatrix);
}

void moveCameraLight(LPDIRECT3DDEVICE9 dxDevice, lightData* ld)
{
	if (ld->lightType == LT_ortho)
		moveCameraLight_ortho(dxDevice, ld);
	else if (ld->lightType == LT_persp)
		moveCameraLight_persp(dxDevice, ld);

	ld->lightViewProjVP = viewProj;
	ld->lightViewProj = viewProj;

	texAlignViewProj(&ld->lightViewProj);
}

void moveCameraDynamicDecal_ortho(LPDIRECT3DDEVICE9 dxDevice, dynamicDecalData* ddd)
{
	D3DXVECTOR3 eyeVec(ddd->lightPos.x, ddd->lightPos.y, ddd->lightPos.z);
	D3DXVECTOR3 targVec(ddd->lightDir.x, ddd->lightDir.y, ddd->lightDir.z);
	D3DXVECTOR3 upVec = ddd->lightUp;

	targVec += eyeVec;

	D3DXMatrixLookAtLH(&viewMatrix, &eyeVec, &targVec, &upVec);
 
	D3DXMatrixOrthoLH(&projMatrix, ddd->dimX, ddd->dimY, 0, ddd->lightDepth);
	D3DXMatrixMultiply(&viewProj, &viewMatrix, &projMatrix);

	texAlignViewProj(&viewProj);

	ddd->lightViewProj = viewProj;
}

void moveCameraDynamicDecal_persp(LPDIRECT3DDEVICE9 dxDevice, dynamicDecalData* ddd)
{
	D3DXVECTOR3 eyeVec = ddd->lightPos;
	D3DXVECTOR3 targVec(ddd->lightDir.x, ddd->lightDir.y, ddd->lightDir.z);
	D3DXVECTOR3 upVec = ddd->lightUp;

	targVec += eyeVec;

	D3DXMatrixLookAtLH(&viewMatrix, &eyeVec, &targVec, &upVec);

	D3DXMatrixPerspectiveFovLH(&projMatrix, ddd->dimX, ddd->dimY, 0, ddd->lightDepth);
	D3DXMatrixMultiply(&viewProj, &viewMatrix, &projMatrix);

	texAlignViewProj(&viewProj);

	ddd->lightViewProj = viewProj;
}

void moveCameraDynamicDecal(LPDIRECT3DDEVICE9 dxDevice, dynamicDecalData* ddd)
{
	if (ddd->lightType == DDT_ortho)
		moveCameraDynamicDecal_ortho(dxDevice, ddd);
	else if (ddd->lightType == DDT_persp)
		moveCameraDynamicDecal_persp(dxDevice, ddd);
}

void term()
{
	DestroyWindow(mainHWnd);
}
