#include <windows.h>
#include <fstream>
#include <string>
#include <vector>
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "winmm.lib")//time

//dx sdk 
//#include <d3dx9.h"
//#pragma comment(lib, "d3dx9.lib")

//dx sdk if files are in ..\DXSDK dir
#include "DXSDK\d3dx9.h"
#if defined _M_X64
#pragma comment(lib, "DXSDK/x64/d3dx9.lib") 
#elif defined _M_IX86
#pragma comment(lib, "DXSDK/x86/d3dx9.lib")
#endif

//if using ms detours
//#include "detours\detours.h"
//#pragma comment(lib, "detours/Detours.lib")

//if using DX Includes
//#include <DirectXMath.h>
//using namespace DirectX;

//if using minhook where files are in MinHook dir
#include "MinHook/include/MinHook.h" //detour
using namespace std;

#pragma warning (disable: 4244) 
#pragma warning (disable: 4996)
#define _CRT_SECURE_NO_DEPRECATE

//==========================================================================================================================

HMODULE Hand;
LPDIRECT3DDEVICE9 pDevice;

UINT Stride;

D3DVIEWPORT9 Viewport; //use this Viewport
float ScreenCX;
float ScreenCY;

LPD3DXFONT Font; //font

IDirect3DVertexShader9* vShader;
UINT vSize;

D3DVERTEXBUFFER_DESC vdesc;

bool InitOnce = true;
LPDIRECT3DTEXTURE9 Red, Green, Blue, Yellow;

int countnum = -1;

static BOOL screenshot_taken = FALSE;

//IDirect3DTexture9 *texture;

//==========================================================================================================================

//features

//visuals
int wallhack = 1;				//wallhack
int distanceesp = 1;			//distance esp
int shaderesp = 0;				//shader esp
int lineesp = 10;				//line esp
int boxesp = 0;					//box esp
int picesp = 0;					//pic esp
int nograss = 1;				//nograss
int nofog = 1;					//nofog

//aimbot settings
int aimbot = 1;
int aimkey = 2;
DWORD Daimkey = VK_RBUTTON;		//aimkey
int aimsens = 1;				//aim sensitivity, makes aim smoother
int aimfov = 3;					//aim field of view in % 
int aimheight = 3;				//aim height value, high value aims higher

//autoshoot settings
int autoshoot = 0;
unsigned int asdelay = 49;		//use x-999 (shoot for xx millisecs, looks more legit)
bool IsPressed = false;			//
DWORD astime = timeGetTime();	//autoshoot timer
//==========================================================================================================================

// getdir & log
char dlldir[320];
char* GetDirFile(char *name)
{
	static char pldir[320];
	strcpy_s(pldir, dlldir);
	strcat_s(pldir, name);
	return pldir;
}

void Log(const char *fmt, ...)
{
	if (!fmt)	return;

	char		text[4096];
	va_list		ap;
	va_start(ap, fmt);
	vsprintf_s(text, fmt, ap);
	va_end(ap);

	ofstream logfile(GetDirFile((PCHAR)"log.txt"), ios::app);
	if (logfile.is_open() && text)	logfile << text << endl;
	logfile.close();
}

//==========================================================================================================================

// Parameters:
//
//   float4 CameraPos;
//   float4 FogInfo;
//   float4 PointLightAttr[5];
//   float4 ShadowLightAttr[5];
//   row_major float4x4 texTrans0;
//   row_major float4x4 world;
//   row_major float4x4 wvp;
//
//
// Registers:
//
//   Name            Reg   Size
//   --------------- ----- ----
//   PointLightAttr  c0       5
//   world           c5       4
//   ShadowLightAttr c9       4
//   wvp             c13      4
//   texTrans0       c17      3
//   FogInfo         c20      1
//   CameraPos       c21      1

//calc distance
float GetDistance(float Xx, float Yy, float xX, float yY)
{
	return sqrt((yY - Yy) * (yY - Yy) + (xX - Xx) * (xX - Xx));
}

struct WeaponEspInfo_t
{
	float pOutX, pOutY, RealDistance, pOut2X, pOut2Y, vSize;
	float CrosshairDistance;
};
std::vector<WeaponEspInfo_t>WeaponEspInfo;

//w2s for weapons
void AddWeapons(LPDIRECT3DDEVICE9 Device)
{
	D3DXMATRIX matrix;
	Device->GetVertexShaderConstantF(13, matrix, 4);

	D3DXVECTOR3 pOut, pIn(0.0f, (float)aimheight, -2.0f);
	float distance = pIn.x * matrix._14 + pIn.y * matrix._24 + pIn.z * matrix._34 + matrix._44;
	D3DXVec3TransformCoord(&pOut, &pIn, &matrix);

	pOut.x = Viewport.X + (1.0f + pOut.x) *Viewport.Width / 2.0f;
	pOut.y = Viewport.Y + (1.0f - pOut.y) *Viewport.Height / 2.0f;

	float x1, y1;
	if (pOut.x > 0.0f && pOut.y > 0.0f && pOut.x < Viewport.Width && pOut.y < Viewport.Height)
	{
		x1 = pOut.x;
		y1 = pOut.y;
	}
	else
	{
		x1 = -1.0f;
		y1 = -1.0f;
	}

	float x2, y2;
	if (lineesp == 1)
	{
		D3DXVECTOR3 pOut2, pIn2(0, (float)aimheight, 75.0f);//esp 1 line length
		D3DXVec3TransformCoord(&pOut2, &pIn2, &matrix);

		pOut2.x = Viewport.X + (1.0f + pOut2.x) *Viewport.Width / 2.0f;
		pOut2.y = Viewport.Y + (1.0f - pOut2.y) *Viewport.Height / 2.0f;

		if (pOut2.x > 0.0f && pOut2.y > 0.0f && pOut2.x < Viewport.Width && pOut2.y < Viewport.Height)
		{
			x2 = pOut2.x;
			y2 = pOut2.y;
		}
		else
		{
			x2 = -1.0f;
			y2 = -1.0f;
		}
	}

	WeaponEspInfo_t pWeaponEspInfo = { static_cast<float>(x1), static_cast<float>(y1), static_cast<float>(distance*0.1f), static_cast<float>(x2), static_cast<float>(y2), static_cast<float>(vSize) };
	WeaponEspInfo.push_back(pWeaponEspInfo);
}

//==========================================================================================================================

HRESULT GenerateTexture(IDirect3DDevice9 *pDevice, IDirect3DTexture9 **ppD3Dtex, DWORD colour32)
{
	if (FAILED(pDevice->CreateTexture(8, 8, 1, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, ppD3Dtex, NULL)))
		return E_FAIL;

	WORD colour16 = ((WORD)((colour32 >> 28) & 0xF) << 12)
		| (WORD)(((colour32 >> 20) & 0xF) << 8)
		| (WORD)(((colour32 >> 12) & 0xF) << 4)
		| (WORD)(((colour32 >> 4) & 0xF) << 0);

	D3DLOCKED_RECT d3dlr;
	(*ppD3Dtex)->LockRect(0, &d3dlr, 0, 0);
	WORD *pDst16 = (WORD*)d3dlr.pBits;

	for (int xy = 0; xy < 8 * 8; xy++)
		*pDst16++ = colour16;

	(*ppD3Dtex)->UnlockRect(0);

	return S_OK;
}

//IDirect3DPixelShader9* oldsShader;
void DrawBox(IDirect3DDevice9 *pDevice, float x, float y, float w, float h, D3DCOLOR Color)
{
	struct Vertex
	{
		float x, y, z, ht;
		DWORD Color;
	}
	V[4] = { { x, y + h, 0.0f, 0.0f, Color },{ x, y, 0.0f, 0.01f, Color },
	{ x + w, y + h, 0.0f, 0.0f, Color },{ x + w, y, 0.0f, 0.0f, Color } };
	pDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	//pDevice->GetPixelShader(&oldsShader);

	pDevice->SetTexture(0, NULL);
	pDevice->SetPixelShader(0);

	// mix texture color
	//pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	//pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	//pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	// mix texture alpha 
	//pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	//pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	//pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	//pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	//pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	//pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	//pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);

	pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, V, sizeof(Vertex));

	//pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);

	//pDevice->SetPixelShader(oldsShader);
}

void DrawP(LPDIRECT3DDEVICE9 Device, int baseX, int baseY, int baseW, int baseH, D3DCOLOR Cor)
{
	D3DRECT BarRect = { baseX, baseY, baseX + baseW, baseY + baseH };
	Device->Clear(1, &BarRect, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, Cor, 0, 0);
}

void DrawCornerBox(LPDIRECT3DDEVICE9 Device, int x, int y, int w, int h, int borderPx, DWORD borderColor)
{
	DrawP(Device, x - (w / 2), (y - h + borderPx), w / 3, borderPx, borderColor); //bottom 
	DrawP(Device, x - (w / 2) + w - w / 3, (y - h + borderPx), w / 3, borderPx, borderColor); //bottom 
	DrawP(Device, x - (w / 2), (y - h + borderPx), borderPx, w / 3, borderColor); //left 
	DrawP(Device, x - (w / 2), (y - h + borderPx) + h - w / 3, borderPx, w / 3, borderColor); //left 
	DrawP(Device, x - (w / 2), y, w / 3, borderPx, borderColor); //top 
	DrawP(Device, x - (w / 2) + w - w / 3, y, w / 3, borderPx, borderColor); //top 
	DrawP(Device, (x + w - borderPx) - (w / 2), (y - h + borderPx), borderPx, w / 3, borderColor);//right 
	DrawP(Device, (x + w - borderPx) - (w / 2), (y - h + borderPx) + h - w / 3, borderPx, w / 3, borderColor);//right 
}

class D3DTLVERTEX
{
public:
	FLOAT X, Y, X2, Y2;
	DWORD Color;
};

//IDirect3DPixelShader9* oldlShader;
void DrawLine(IDirect3DDevice9* pDevice, float X, float Y, float X2, float Y2, float Width, D3DCOLOR Color, bool AntiAliased)
{
	D3DTLVERTEX qV[2] = {
		{ (float)X , (float)Y, 0.0f, 1.0f, Color },
		{ (float)X2 , (float)Y2 , 0.0f, 1.0f, Color },
	};
	const DWORD D3DFVF_TL = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;

	pDevice->SetFVF(D3DFVF_TL);

	//pDevice->GetPixelShader(&oldlShader);

	//pDevice->SetTexture(0, Yellow);
	pDevice->SetTexture(0, NULL);
	pDevice->SetPixelShader(0);

	//pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, (AntiAliased ? TRUE : FALSE));

	pDevice->DrawPrimitiveUP(D3DPT_LINELIST, 2, qV, sizeof(D3DTLVERTEX));

	pDevice->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, FALSE);
	//pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);

	//pDevice->SetPixelShader(oldlShader);
}

LPD3DXLINE pLine;
VOID DrawLine2(IDirect3DDevice9* pDevice, FLOAT startx, FLOAT starty, FLOAT endx, FLOAT endy, FLOAT width, D3DCOLOR dColor)
{
	D3DXVECTOR2 lines[] = { D3DXVECTOR2(startx, starty), D3DXVECTOR2(endx, endy) };

	pDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
	pDevice->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE);

	pLine->SetAntialias(TRUE);

	pLine->SetWidth(width);
	pLine->Begin();
	pLine->Draw(lines, 2, dColor);
	pLine->End();

	pDevice->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
	pDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);
}

//fix me
//void DrawLine3(LPDIRECT3DDEVICE9 pDevice, D3DCOLOR Color, int x, int y, int w)
//{
	//DrawP(pDevice, x, y, w, 2, Color);
//}

//=====================================================================================================================
 
// draw sprite
LPD3DXSPRITE pSprite = NULL; 
LPDIRECT3DTEXTURE9 pSpriteTextureImage = NULL;
bool SpriteCreated = false;

// COM utils
template<class COMObject>
void SafeRelease(COMObject*& pRes)
{
	IUnknown *unknown = pRes;
	if (unknown)
	{
		unknown->Release();
	}
	pRes = NULL;
}

bool CreateSprite(IDirect3DDevice9* pDevice)
{
	HRESULT hr;

	hr = D3DXCreateTextureFromFileA(pDevice, GetDirFile((PCHAR)"circle.png"), &pSpriteTextureImage);

	if (FAILED(hr))
	{
		//Log("D3DXCreateTextureFromFile failed");
		SpriteCreated = false;
		return false;
	}

	hr = D3DXCreateSprite(pDevice, &pSprite);

	if (FAILED(hr))
	{
		//Log("D3DXCreateSprite failed");
		SpriteCreated = false;
		return false;
	}

	SpriteCreated = true;

	return true;
}

// Delete work surfaces when device gets reset
void DeleteSprite()
{
	if (pSprite != NULL)
	{
		//Log("SafeRelease(pSprite)");
		SafeRelease(pSprite);
	}

	SpriteCreated = false;
}

// Draw Sprite
void DrawPic(IDirect3DDevice9* pDevice, IDirect3DTexture9 *tex, int cx, int cy)
{
	if (SpriteCreated && pSprite != NULL)
	{
		//position = PicSize(in pixel) / 2, 
		//64 -> 32
		D3DXVECTOR3 position;
		position.x = (float)cx-32.0f;
		position.y = (float)cy-32.0f;
		position.z = 0.0f;

		//draw pic
		pSprite->Begin(D3DXSPRITE_ALPHABLEND);
		pSprite->Draw(tex, NULL, NULL, &position, 0xFFFFFFFF);
		pSprite->End();
	}
}

//==========================================================================================================================

//draw shader
int DX9CreateEllipseShader(IDirect3DDevice9* pDevice, IDirect3DPixelShader9 **pShader)
{
	char vers[100];
	char *strshader = (PCHAR)"\
float4 radius: register(c0);\
sampler mytexture;\
struct VS_OUTPUT\
{\
float4 Pos : SV_POSITION;\
float4 Color : COLOR;\
float2 TexCoord : TEXCOORD;\
};\
float4 PS(VS_OUTPUT input) : SV_TARGET\
{\
if( ( (input.TexCoord[0]-0.5)*(input.TexCoord[0]-0.5) + (input.TexCoord[1]-0.5)*(input.TexCoord[1]-0.5) <= 0.5*0.5) &&\
( (input.TexCoord[0]-0.5)*(input.TexCoord[0]-0.5) + (input.TexCoord[1]-0.5)*(input.TexCoord[1]-0.5) >= radius[0]*radius[0]) )\
return input.Color;\
else return float4(0,0,0,0);\
};";

	D3DCAPS9 caps;
	pDevice->GetDeviceCaps(&caps);
	UINT V1 = D3DSHADER_VERSION_MAJOR(caps.PixelShaderVersion);
	UINT V2 = D3DSHADER_VERSION_MINOR(caps.PixelShaderVersion);
	sprintf_s(vers, "ps_%d_%d", V1, V2);
	LPD3DXBUFFER pShaderBuf;
	D3DXCompileShader(strshader, strlen(strshader), 0, 0, "PS", vers, 0, &pShaderBuf, 0, 0);
	if (pShaderBuf == NULL)
	{
		MessageBoxA(0, "pshader == NULL", 0, 0);
		return 1;
	}
	pDevice->CreatePixelShader((DWORD*)pShaderBuf->GetBufferPointer(), pShader);
	if (!pShader)
	{
		MessageBoxA(0, "ellipseshader == NULL", 0, 0);
		return 2;
	}

	memset(strshader, 0, strlen(strshader));
	pShaderBuf->Release();
	return 0;
}

DWORD deffault_color8[] = { 0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff };
struct VERTEX
{
	float x, y, z, rhw;
	DWORD color;
	float tu, tv;
};
DWORD FVF = D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE;
IDirect3DPixelShader9* ellipse;
int DX9DrawEllipse(IDirect3DDevice9* pDevice, float x, float y, float w, float h, float linew, DWORD *color)
{
	//if (!pDevice)return 1;
	static IDirect3DVertexBuffer9 *vb = 0;
	static IDirect3DIndexBuffer9 *ib = 0;
	static IDirect3DSurface9 *surface = 0;
	static IDirect3DTexture9 *pstexture = 0;
	if (!vb)
	{
		pDevice->CreateVertexBuffer(sizeof(VERTEX) * 4, 0, FVF, D3DPOOL_MANAGED, &vb, NULL);
		//Log("1");
		if (!vb)
		{
			MessageBoxA(0, "DrawEllipse error vb", 0, 0);
			return 2;
		}
		pDevice->CreateIndexBuffer((3 * 2) * 2, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &ib, NULL);
		if (!ib)
		{
			MessageBoxA(0, "DrawEllipse error ib", 0, 0);
			return 3;
		}

	}
	else
	{
		if (!color)color = deffault_color8;
		float tu = 0, tv = 0;
		float tw = 1.0, th = 1.0;
		VERTEX v[4] = { { x,y,0,1,color[0],tu,tv },{ x + w,y,0,1,color[1],tu + tw,tv },{ x + w,y + h,0,1,color[2],tu + tw,tv + th },{ x,y + h,0,1,color[3],tu,tv + th } };
		WORD i[2 * 3] = { 0,1,2, 2,3,0 };
		void *p;
		vb->Lock(0, sizeof(v), &p, 0);
		memcpy(p, v, sizeof(v));
		vb->Unlock();

		ib->Lock(0, sizeof(i), &p, 0);
		memcpy(p, i, sizeof(i));
		ib->Unlock();

		float radius[4] = { 0,w,h,0 };

		radius[0] = (linew) / w;
		if (radius[0]>0.5)radius[0] = 0.5;
		radius[0] = 0.5f - radius[0];


		pDevice->SetPixelShaderConstantF(0, radius, 1);
		pDevice->SetFVF(FVF);
		//pDevice->SetTexture(0, 0);
		pDevice->SetPixelShader(ellipse);
		//pDevice->SetVertexShader(0);
		pDevice->SetStreamSource(0, vb, 0, sizeof(VERTEX));
		pDevice->SetIndices(ib);

		pDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);

		pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);

		pDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);
	}
	return 0;
};

//==========================================================================================================================

void SaveCfg()
{
	ofstream fout;
	fout.open(GetDirFile((PCHAR)"rosd3d.ini"), ios::trunc);
	fout << "wallhack " << wallhack << endl;
	fout << "distanceesp " << distanceesp << endl;
	fout << "shaderesp " << shaderesp << endl;
	fout << "lineesp " << lineesp << endl;
	fout << "boxesp " << boxesp << endl;
	fout << "picesp " << picesp << endl;
	fout << "aimbot " << aimbot << endl;
	fout << "aimkey " << aimkey << endl;
	fout << "aimsens " << aimsens << endl;
	fout << "aimfov " << aimfov << endl;
	fout << "aimheight " << aimheight << endl;
	fout << "autoshoot " << autoshoot << endl;
	fout << "nograss " << nograss << endl;
	fout << "nofog " << nofog << endl;
	fout.close();
}

void LoadCfg()
{
	ifstream fin;
	string Word = "";
	fin.open(GetDirFile((PCHAR)"rosd3d.ini"), ifstream::in);
	fin >> Word >> wallhack;
	fin >> Word >> distanceesp;
	fin >> Word >> shaderesp;
	fin >> Word >> lineesp;
	fin >> Word >> boxesp;
	fin >> Word >> picesp;
	fin >> Word >> aimbot;
	fin >> Word >> aimkey;
	fin >> Word >> aimsens;
	fin >> Word >> aimfov;
	fin >> Word >> aimheight;
	fin >> Word >> autoshoot;
	fin >> Word >> nograss;
	fin >> Word >> nofog;
	fin.close();
}

//==========================================================================================================================
/*
class CTimer
{
public:
	ULONGLONG dwTime;
	bool bEnable;
	CTimer()
	{
		dwTime = 0;
		bEnable = true;
	}
	bool delay(DWORD dwMsec);
	void reset();
	void stop();
};

bool CTimer::delay(DWORD dwMsec)
{
	if (!bEnable)
		return true;

	if (!dwTime)
		dwTime = GetTickCount64();

	if (dwTime + dwMsec < GetTickCount64())
	{
		//dwTime = 0;
		bEnable = false;
		return true;
	}
	else
	{
		return false;
	}
}

void CTimer::reset()
{
	dwTime = 0;
	bEnable = true;
}

void CTimer::stop()
{
	bEnable = false;
}
*/
/*
//1
static ULONGLONG timefirst = GetTickCount64();
if (GetTickCount64() - timefirst > 2000)
{
DrawCenteredString(Font, 180, 200, D3DCOLOR_ARGB(255, 255, 255, 0), "11111111111111");
timefirst = GetTickCount64();
}

//2 same
static CTimer TriggerDelayZoom;
if (TriggerDelayZoom.delay(2000))
{
TriggerDelayZoom.reset();
DrawCenteredString(Font, 180, 220, D3DCOLOR_ARGB(255, 255, 255, 0), "11111111111111");
}
else//optional
{
//DrawCenteredString(Font, 180, 240, D3DCOLOR_ARGB(255, 255, 255, 0), "22222222222222");
}
*/

//==========================================================================================================================

// menu stuff
HRESULT DrawString(LPD3DXFONT Font, INT X, INT Y, DWORD dColor, const char* cString, ...)
{
	HRESULT hRet;

	CHAR buf[512] = { NULL };
	va_list ArgumentList;
	va_start(ArgumentList, cString);
	_vsnprintf_s(buf, sizeof(buf), sizeof(buf) - strlen(buf), cString, ArgumentList);
	va_end(ArgumentList);

	RECT rc[2];
	SetRect(&rc[0], X, Y, X, 0);
	SetRect(&rc[1], X, Y, X + 50, 50);

	hRet = D3D_OK;

	if (SUCCEEDED(hRet))
	{
		Font->DrawTextA(NULL, buf, -1, &rc[0], DT_NOCLIP, 0xFF000000);
		hRet = Font->DrawTextA(NULL, buf, -1, &rc[1], DT_NOCLIP, dColor);
	}

	return hRet;
}

HRESULT DrawCenteredString(LPD3DXFONT Font, INT X, INT Y, DWORD dColor, const char* cString, ...)
{
	HRESULT hRet;

	CHAR buf[512] = { NULL };
	va_list ArgumentList;
	va_start(ArgumentList, cString);
	_vsnprintf_s(buf, sizeof(buf), sizeof(buf) - strlen(buf), cString, ArgumentList);
	va_end(ArgumentList);

	RECT rc[2];
	SetRect(&rc[0], X, Y, X, 0);
	SetRect(&rc[1], X, Y, X + 2, 2);

	hRet = D3D_OK;

	if (SUCCEEDED(hRet))
	{
		Font->DrawTextA(NULL, buf, -1, &rc[0], DT_NOCLIP | DT_CENTER, 0xFF000000);
		hRet = Font->DrawTextA(NULL, buf, -1, &rc[1], DT_NOCLIP | DT_CENTER, dColor);
	}

	return hRet;
}

int menuselect = 0;
int Current = true;

int PosX = 30;
int PosY = 27;

int ShowMenu = false; //off by default

POINT Pos;

int CheckTab(int x, int y, int w, int h)
{
	if (ShowMenu)
	{
		GetCursorPos(&Pos);
		ScreenToClient(GetForegroundWindow(), &Pos);
		if (Pos.x > x && Pos.x < x + w && Pos.y > y && Pos.y < y + h)
		{
			if (GetAsyncKeyState(VK_LBUTTON) & 1)
			{
				//return 1; //disabled mouse selection in menu
			}
			return 2;
		}
	}
	return 0;
}

void WriteText(int x, int y, DWORD color, char *text)
{
	RECT rect;
	SetRect(&rect, x, y, x, y);
	Font->DrawTextA(0, text, -1, &rect, DT_NOCLIP | DT_LEFT, color);
}

void lWriteText(int x, int y, DWORD color, char *text)
{
	RECT rect;
	SetRect(&rect, x, y, x, y);
	Font->DrawTextA(0, text, -1, &rect, DT_NOCLIP | DT_RIGHT, color);
}

void Category(LPDIRECT3DDEVICE9 pDevice, char *text)
{
	if (ShowMenu)
	{
		int Check = CheckTab(PosX + 44, (PosY + 51) + (Current * 15), 190, 10);
		DWORD ColorText;

		ColorText = D3DCOLOR_ARGB(255, 255, 0, 255);

		if (Check == 2)
			ColorText = D3DCOLOR_ARGB(255, 255, 255, 255);

		if (menuselect == Current)
			ColorText = D3DCOLOR_ARGB(255, 255, 255, 255);

		WriteText(PosX + 44, PosY + 50 + (Current * 15) - 1, ColorText, text);
		lWriteText(PosX + 236, PosY + 50 + (Current * 15) - 1, ColorText, (PCHAR)"[-]");
		Current++;
	}
}

void AddItem(LPDIRECT3DDEVICE9 pDevice, char *text, int &var, char **opt, int MaxValue)
{
	if (ShowMenu)
	{
		int Check = CheckTab(PosX + 44, (PosY + 51) + (Current * 15), 190, 10);
		DWORD ColorText;

		if (var)
		{
			//DrawBox(pDevice, PosX+44, PosY+51 + (Current * 15), 10, 10, D3DCOLOR_ARGB(255, 0, 255, 0));
			ColorText = D3DCOLOR_ARGB(255, 0, 255, 0);
		}
		if (var == 0)
		{
			//DrawBox(pDevice, PosX+44, PosY+51 + (Current * 15), 10, 10, D3DCOLOR_ARGB(255, 255, 0, 0));
			ColorText = D3DCOLOR_ARGB(255, 255, 0, 0);
		}

		if (Check == 1)
		{
			var++;
			if (var > MaxValue)
				var = 0;
		}

		if (Check == 2)
			ColorText = D3DCOLOR_ARGB(255, 255, 255, 255);

		if (menuselect == Current)
		{
			static ULONGLONG lasttick_right = GetTickCount64();
			static ULONGLONG lasttick_left = GetTickCount64();
			if (GetAsyncKeyState(VK_RIGHT) && GetTickCount64() - lasttick_right > 100)
			{
				lasttick_right = GetTickCount64();
				var++;
				if (var > MaxValue)
					var = 0;
			}
			else if (GetAsyncKeyState(VK_LEFT) && GetTickCount64() - lasttick_left > 100)
			{
				lasttick_left = GetTickCount64();
				var--;
				if (var < 0)
					var = MaxValue;
			}
		}

		if (menuselect == Current)
			ColorText = D3DCOLOR_ARGB(255, 255, 255, 255);


		WriteText(PosX + 44, PosY + 50 + (Current * 15) - 1, D3DCOLOR_ARGB(255, 50, 50, 50), text);
		WriteText(PosX + 45, PosY + 51 + (Current * 15) - 1, ColorText, text);

		lWriteText(PosX + 236, PosY + 50 + (Current * 15) - 1, D3DCOLOR_ARGB(255, 100, 100, 100), opt[var]);
		lWriteText(PosX + 237, PosY + 51 + (Current * 15) - 1, ColorText, opt[var]);
		Current++;
	}
}

//==========================================================================================================================

// menu part
char *opt_OnOff[] = { (PCHAR)"[OFF]", (PCHAR)"[On]" };
char *opt_WhChams[] = { (PCHAR)"[OFF]", (PCHAR)"[On]", (PCHAR)"[Color]" };
char *opt_ZeroTen[] = { (PCHAR)"[0]", (PCHAR)"[1]", (PCHAR)"[2]", (PCHAR)"[3]", (PCHAR)"[4]", (PCHAR)"[5]", (PCHAR)"[6]", (PCHAR)"[7]", (PCHAR)"[8]", (PCHAR)"[9]", (PCHAR)"[10]", (PCHAR)"[11]" };
char *opt_Keys[] = { (PCHAR)"[OFF]", (PCHAR)"[Shift]", (PCHAR)"[RMouse]", (PCHAR)"[LMouse]", (PCHAR)"[Ctrl]", (PCHAR)"[Alt]", (PCHAR)"[Space]", (PCHAR)"[X]", (PCHAR)"[C]" };
char *opt_aimfov[] = { (PCHAR)"[0]", (PCHAR)"[5%]", (PCHAR)"[10%]", (PCHAR)"[15%]", (PCHAR)"[20%]", (PCHAR)"[25%]", (PCHAR)"[30%]", (PCHAR)"[35%]", (PCHAR)"[40%]", (PCHAR)"[45%]" };
char *opt_autoshoot[] = { (PCHAR)"[OFF]", (PCHAR)"[OnKeyDown]" };

void DrawMenu(LPDIRECT3DDEVICE9 pDevice)
{
	static ULONGLONG lasttick_insert = GetTickCount64();
	if (GetAsyncKeyState(VK_INSERT) && GetTickCount64() - lasttick_insert > 150)
	{
		lasttick_insert = GetTickCount64();
		ShowMenu = !ShowMenu;
		//save settings
		SaveCfg();
	}

	if (ShowMenu)
	{
		static ULONGLONG lasttick_up = GetTickCount64();
		if (GetAsyncKeyState(VK_UP) && GetTickCount64() - lasttick_up > 100)
		{
			lasttick_up = GetTickCount64();
			menuselect--;
		}

		static ULONGLONG lasttick_down = GetTickCount64();
		if (GetAsyncKeyState(VK_DOWN) && GetTickCount64() - lasttick_down > 100)
		{
			lasttick_down = GetTickCount64();
			menuselect++;
		}

		Current = 1;

		AddItem(pDevice, (PCHAR)"Wallhack", wallhack, opt_WhChams, 2);
		AddItem(pDevice, (PCHAR)"Distance Esp", distanceesp, opt_OnOff, 1);
		AddItem(pDevice, (PCHAR)"Shader Esp", shaderesp, opt_OnOff, 1);
		AddItem(pDevice, (PCHAR)"Line Esp", lineesp, opt_ZeroTen, 11);
		AddItem(pDevice, (PCHAR)"Box Esp", boxesp, opt_OnOff, 1);
		AddItem(pDevice, (PCHAR)"Pic Esp", picesp, opt_OnOff, 1);
		AddItem(pDevice, (PCHAR)"Aimbot", aimbot, opt_OnOff, 1);
		AddItem(pDevice, (PCHAR)"Aimkey", aimkey, opt_Keys, 8);
		AddItem(pDevice, (PCHAR)"Aimsens", aimsens, opt_ZeroTen, 10);
		AddItem(pDevice, (PCHAR)"Aimfov", aimfov, opt_aimfov, 9);
		AddItem(pDevice, (PCHAR)"Aimheight", aimheight, opt_ZeroTen, 5);
		AddItem(pDevice, (PCHAR)"Autoshoot", autoshoot, opt_autoshoot, 1);
		AddItem(pDevice, (PCHAR)"No Grass", nograss, opt_OnOff, 1);
		AddItem(pDevice, (PCHAR)"No Fog", nofog, opt_OnOff, 1);

		if (menuselect >= Current)
			menuselect = 1;

		if (menuselect < 1)
			menuselect = 14;//Current;
	}
}

//=====================================================================================================================