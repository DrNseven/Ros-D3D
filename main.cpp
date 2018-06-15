//Ros D3D 1.5 by n7
//
//Credits: JoshRose
//
//how to fix safesh "error" with ms detours 1.5:
//project -> properties -> configuration properties -> linker -> advanced -> image has safe exception handler -> No (/SAFESEH:NO)


#include "main.h"


typedef HRESULT(APIENTRY* tEndScene)(LPDIRECT3DDEVICE9 pDevice);
tEndScene oEndScene = NULL;

typedef HRESULT(APIENTRY* tSetStreamSource)(IDirect3DDevice9*, UINT, IDirect3DVertexBuffer9*, UINT, UINT);
tSetStreamSource oSetStreamSource = 0;

typedef HRESULT(APIENTRY* tSetTexture)(IDirect3DDevice9*, DWORD, IDirect3DBaseTexture9 *);
tSetTexture oSetTexture = 0;

typedef HRESULT(APIENTRY* tPresent) (IDirect3DDevice9*, const RECT *, const RECT *, HWND, const RGNDATA *);
tPresent oPresent = 0;

typedef HRESULT(APIENTRY* tReset)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
tReset oReset = 0;

//optional:
//drawindexedprimitive

//setvertexshaderconstantf

//==========================================================================================================================

HRESULT APIENTRY hkEndScene(LPDIRECT3DDEVICE9 pDevice)
{
	while (!npDevice) {
		npDevice = pDevice;
	}

	return oEndScene(pDevice);
}

//==========================================================================================================================

HRESULT APIENTRY hkSetStreamSource(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT sStride)
{
	if (StreamNumber == 0)
		Stride = sStride;

	return oSetStreamSource(pDevice, StreamNumber, pStreamData, OffsetInBytes, sStride);
}

//==========================================================================================================================

HRESULT APIENTRY hkSetTexture(LPDIRECT3DDEVICE9 pDevice, DWORD Sampler, IDirect3DBaseTexture9 *pTexture)
{
	//texture = static_cast<IDirect3DTexture9*>(pTexture);

	if (InitOnce)
	{
		InitOnce = false;

		//get viewport
		pDevice->GetViewport(&Viewport);
		ScreenCX = (float)Viewport.Width / 2.0f;
		ScreenCY = (float)Viewport.Height / 2.0f;

		//generate texture
		GenerateTexture(pDevice, &Red, D3DCOLOR_ARGB(255, 255, 0, 0));
		GenerateTexture(pDevice, &Green, D3DCOLOR_RGBA(0, 255, 0, 255));
		GenerateTexture(pDevice, &Blue, D3DCOLOR_ARGB(255, 0, 0, 255));
		GenerateTexture(pDevice, &Yellow, D3DCOLOR_ARGB(255, 255, 255, 0));
		GenerateTexture(pDevice, &White, D3DCOLOR_ARGB(255, 255, 255, 255));
		GenerateTexture(pDevice, &Black, D3DCOLOR_ARGB(0, 0, 0, 0));

		//load settings
		LoadCfg();
	}

	//create ellipse shader
	if (shaderesp == 1 && !ellipse)
		DX9CreateEllipseShader(pDevice, &ellipse);

	//get vSize
	if (SUCCEEDED(pDevice->GetVertexShader(&vShader)))
		if (vShader != NULL)
			if (SUCCEEDED(vShader->GetFunction(NULL, &vSize)))
				if (vShader != NULL) { vShader->Release(); vShader = NULL; }

	//model rec sample
	//Stride == 20 && vSize == 2008
	//Stride == 36 && vSize == 2356

	if (wallhack>0)
	{
		pDevice->SetRenderState(D3DRS_DEPTHBIAS, 0);
		if ((vSize == 2356 || vSize == 900 || vSize == 2008 || vSize == 640) || (Stride == 36 && vSize == 1436) || (Stride == 48 && vSize == 1436)) //vSize == 600 //blood, gun shots & explosions
		{
			if (wallhack == 2 && vSize != 1436)
			{
				float sColor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };//green
				pDevice->SetPixelShaderConstantF(0, sColor, 1);
				//oSetTexture(pDevice, 0, Red);
				//oSetTexture(pDevice, 1, Red);
			}

			float bias = 1000.0f;
			float bias_float = static_cast<float>(-bias);
			bias_float /= 10000.0f;
			pDevice->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)&bias_float);
		}
	}

	//worldtoscreen weapons in hand
	if ((aimbot == 1 || distanceesp == 1 || shaderesp == 1 || lineesp > 0 || boxesp == 1 || picesp == 1))
		if ((Stride == 48 && vSize > 1328) || (vSize == 2356 || vSize == 2008 || vSize == 1552))
			AddWeapons(pDevice);

	//no fog for models
	if (nofog == 1)
		if (Stride == 48 || vSize == 2008 || vSize == 2356)
		{
			D3DXMATRIX matLegs;
			D3DXMatrixScaling(&matLegs, 0.0f, 0.0f, 0.0f);
			pDevice->SetVertexShaderConstantF(20, matLegs, 1);//legs

			D3DXMATRIX matChest;
			D3DXMatrixScaling(&matChest, 0.0f, 0.0f, 0.0f);
			pDevice->SetVertexShaderConstantF(25, matChest, 1);//chest
		}


	pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	//no grass by mtsagossi
	if (nograss == 1)
		if (vSize == 1660 || vSize == 1704)//grass
		{
			pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);
		}

	//draw square on heads
	if (depthcheck == 1)
	{
		//get pSize
		if (SUCCEEDED(pDevice->GetPixelShader(&pShader)))
			if (pShader != NULL)
				if (SUCCEEDED(pShader->GetFunction(NULL, &pSize)))
					if (pShader != NULL) { pShader->Release(); pShader = NULL; }

		//grab data
		pDevice->GetVertexDeclaration(&pDecl);
		if (pDecl != NULL)
			pDecl->GetDeclaration(decl, &numElements);
		if (pDecl != NULL) { pDecl->Release(); pDecl = NULL; }

		//get texture/desc size
		if (numElements == 6 && Sampler == 0 && pTexture)
		{
			texture = static_cast<IDirect3DTexture9*>(pTexture);
			if (texture)
			{
				if (FAILED(texture->GetLevelDesc(0, &sDesc)))
				{
					goto out;
				}

				if (sDesc.Pool == D3DPOOL_MANAGED && texture->GetType() == D3DRTYPE_TEXTURE && sDesc.Width == 512 && sDesc.Height == 512)
				{
					HRESULT hr = texture->LockRect(0, &pLockedRect, NULL, D3DLOCK_DONOTWAIT | D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK);

					if (SUCCEEDED(hr))
					{
						if (pLockedRect.pBits != NULL)
							qCRC = QuickChecksum((DWORD*)pLockedRect.pBits, pLockedRect.Pitch); //get crc
																								//qCRC = QuickChecksum((DWORD*)pLockedRect.pBits, 12);
					}
					texture->UnlockRect(0);
				}
			}
		}
	out:

		//if (Stride == 36 && vSize == 2008 && pSize == 1724) //helmet
		//if (GetAsyncKeyState(VK_F10) & 1)
		//Log("Stride == %d && vSize == %d && pSize == %d && pLockedRect.Pitch == %d && qCRC == %x && qCRC == %d && numElements == %d && decl->Type == %d", Stride, vSize, pSize, pLockedRect.Pitch, qCRC, qCRC, numElements, decl->Type);

		//log hair
		//if ((numElements == 6 || numElements == 10) && (vSize == 2356 && pSize != 1848))//hair
		//if (GetAsyncKeyState(VK_F10) & 1)
		//Log("Stride == %d && vSize == %d && pSize == %d && pLockedRect.Pitch == %d && qCRC == %x && numElements == %d && decl->Type == %d", Stride, vSize, pSize, pLockedRect.Pitch, qCRC, numElements, decl->Type);

		//log helmet (logs wrong values most of the time)
		//if (numElements == 6 && Stride == 36 && vSize == 2008 && pSize == 1724) //helmet
		//if (GetAsyncKeyState(VK_F10) & 1)
		//Log("Stride == %d && vSize == %d && pSize == %d && pLockedRect.Pitch == %d && qCRC == %x && qCRC == %d && numElements == %d && decl->Type == %d", Stride, vSize, pSize, pLockedRect.Pitch, qCRC, qCRC, numElements, decl->Type);

		//green hair
		if ((Stride == 36 && vSize == 2356 && pSize != 1848 && pLockedRect.Pitch == 2048 && numElements == 6) || //hair	
			(Stride == 44 && vSize == 2356 && pSize == 2272 && pLockedRect.Pitch == 1024 && numElements == 10)) //hair2
		{
			oSetTexture(pDevice, 0, Green);
			oSetTexture(pDevice, 1, Green);
		}

		//red helmets
		if ((vSize == 2008 && qCRC == 0xc46ee841) ||//helmet 1
			(vSize == 2008 && qCRC == 0x9590d282) ||//helmet 2
			(vSize == 2008 && qCRC == 0xe248e914))//helmet 3
		{
			oSetTexture(pDevice, 0, Red);
			oSetTexture(pDevice, 1, Red);
		}

		//make bigger
		if ((Stride == 36 && vSize == 2356 && pSize != 1848 && pLockedRect.Pitch == 2048 && numElements == 6) || //hair	
			(Stride == 44 && vSize == 2356 && pSize == 2272 && pLockedRect.Pitch == 1024 && numElements == 10) || //hair2
			(vSize == 2008 && qCRC == 0xc46ee841) ||//helmet 1
			(vSize == 2008 && qCRC == 0x9590d282) ||//helmet 2
			(vSize == 2008 && qCRC == 0xe248e914))//helmet 3
		{
			pDevice->SetRenderState(D3DRS_DEPTHBIAS, 0);
			pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);
			DrawtoTarget(pDevice);
		}
	}

	/*
	//logger
	if (GetAsyncKeyState('O') & 1) //-
	countnum--;
	if (GetAsyncKeyState('P') & 1) //+
	countnum++;
	if (countnum == vSize / 100)//Stride)
	if (GetAsyncKeyState('I') & 1) //log
	Log("Stride == %d && vSize == %d && pSize == %d && sDesc.Width == %d && qCRC == %x && numElements == %d && decl->Type == %d", Stride, vSize, pSize, sDesc.Width, qCRC, numElements, decl->Type);
	if (countnum == vSize / 100)//Stride)
	{
	//SetTexture_orig(pDevice, 0, Yellow);
	//SetTexture_orig(pDevice, 1, Yellow);
	return D3D_OK; //delete texture
	}
	*/
	return oSetTexture(pDevice, Sampler, pTexture);
}

//==========================================================================================================================

HRESULT APIENTRY hkPresent(IDirect3DDevice9* pDevice, const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion)
{
	if (GetAsyncKeyState(VK_ESCAPE) & 1 || GetAsyncKeyState(VK_INSERT))
	{
		//get viewport again
		pDevice->GetViewport(&Viewport);
		ScreenCX = (float)Viewport.Width / 2.0f;
		ScreenCY = (float)Viewport.Height / 2.0f;
	}

	//create font
	if (Font == NULL)
		D3DXCreateFont(pDevice, 14, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Italic"), &Font);

	//create dxLine
	if (!pLine)
		D3DXCreateLine(pDevice, &pLine);

	//create sprite
	if (!SpriteCreated)
		CreateSprite(pDevice);

	//draw Animation
	//if (ShowMenu)
		//DrawAnim(pDevice, 114, 173, 1.52f, 0.8f, 1.0f);
		//DrawAnim(pDevice, 114, 116, 1.5f, 1.5f, 1.5f);
		//DrawAnim(pDevice, Viewport.Width / 2, Viewport.Height*0.90f, 1.0f, 1.0f, 1.0f);//middle/bottom

	//draw background
	if (ShowMenu)
		DrawBox(pDevice, 71.0f, 86.0f, 200.0f, 275.0f, D3DCOLOR_ARGB(120, 30, 200, 200));//200 = left/right, 275 = up/down

	//draw menu
	if (Font)
		DrawMenu(pDevice);

	//Shift|RMouse|LMouse|Ctrl|Alt|Space|X|C
	if (aimkey == 0) Daimkey = 0;
	if (aimkey == 1) Daimkey = VK_SHIFT;
	if (aimkey == 2) Daimkey = VK_RBUTTON;
	if (aimkey == 3) Daimkey = VK_LBUTTON;
	if (aimkey == 4) Daimkey = VK_CONTROL;
	if (aimkey == 5) Daimkey = VK_MENU;
	if (aimkey == 6) Daimkey = VK_SPACE;
	if (aimkey == 7) Daimkey = 0x58; //X
	if (aimkey == 8) Daimkey = 0x43; //C


	//do distance esp
	if (distanceesp > 0 && WeaponEspInfo.size() != NULL)
	{
		for (unsigned int i = 0; i < WeaponEspInfo.size(); i++)
		{
			if (WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].RealDistance > 4.0f && (float)WeaponEspInfo[i].RealDistance <= 200.0f) // 4 - 200 yellow
				DrawCenteredString(Font, (int)WeaponEspInfo[i].pOutX, (int)WeaponEspInfo[i].pOutY - 20, D3DCOLOR_ARGB(255, 255, 255, 0), "%.f", (float)WeaponEspInfo[i].RealDistance);
			else if (WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].RealDistance > 200.0f && (float)WeaponEspInfo[i].RealDistance <= 1000.0f) //200 - 1000 white
				DrawCenteredString(Font, (int)WeaponEspInfo[i].pOutX, (int)WeaponEspInfo[i].pOutY - 20, D3DCOLOR_ARGB(255, 255, 255, 255), "%.f", (float)WeaponEspInfo[i].RealDistance);
			else if (WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].RealDistance > 1000.0f) //> 1000 gray
				DrawCenteredString(Font, (int)WeaponEspInfo[i].pOutX, (int)WeaponEspInfo[i].pOutY - 20, D3DCOLOR_ARGB(255, 128, 128, 128), "%.f", (float)WeaponEspInfo[i].RealDistance);
		}
	}

	//do shader esp
	if (shaderesp > 0 && WeaponEspInfo.size() != NULL)
	{
		for (unsigned int i = 0; i < WeaponEspInfo.size(); i++)
		{
			DWORD col[4] = { 0xffffffff,0xffffffff,0xffffffff,0xffffffff };//white
			//DWORD col[4] = { 0xffff0000,0xffff0000,0xffff0000,0xffff0000 };//gradient color (red)
			if (WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].RealDistance > 4.0f)
				DX9DrawEllipse(pDevice, (float)WeaponEspInfo[i].pOutX - 6.0f, (float)WeaponEspInfo[i].pOutY - 8.0f, 16.0f, 32.0f, 1.0f, col);//-8 or -9
		}
	}

	//do line esp
	if (lineesp > 0 && WeaponEspInfo.size() != NULL)
	{
		for (unsigned int i = 0; i < WeaponEspInfo.size(); i++)
		{
			//show where enemy is looking or aiming at
			if (lineesp == 1 && WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].RealDistance > 4.0f && WeaponEspInfo[i].pOut2X > 1.0f && WeaponEspInfo[i].pOut2Y > 1.0f && (float)WeaponEspInfo[i].vSize == 2008)//long range weapon
				DrawLine(pDevice, (float)WeaponEspInfo[i].pOutX, (float)WeaponEspInfo[i].pOutY, (float)WeaponEspInfo[i].pOut2X, (float)WeaponEspInfo[i].pOut2Y, 2.0f, D3DCOLOR_ARGB(255, 0, 0, 255), 0);

			else if (lineesp == 2 && WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].RealDistance > 4.0f)
				DrawLine(pDevice, (float)WeaponEspInfo[i].pOutX, (float)WeaponEspInfo[i].pOutY, ScreenCX, ScreenCY * 0.2f, 1, D3DCOLOR_ARGB(255, 255, 255, 255), 0);//0.1up, 1.0middle, 2.0down
				//DrawLine2(pDevice, (float)WeaponEspInfo[i].pOutX, (float)WeaponEspInfo[i].pOutY, ScreenCX, ScreenCY * ((float)esp * 0.2f), 1.0f, D3DCOLOR_ARGB(255, 255, 255, 255));
				//DrawLine3(pDevice, D3DCOLOR_ARGB(255, 255, 255, 255), (float)WeaponEspInfo[i].pOutX, (float)WeaponEspInfo[i].pOutY, 1.0f);//no

			else if (lineesp == 3 && WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].RealDistance > 4.0f)
				DrawLine(pDevice, (float)WeaponEspInfo[i].pOutX, (float)WeaponEspInfo[i].pOutY, ScreenCX, ScreenCY * 0.4f, 1.0f, D3DCOLOR_ARGB(255, 255, 255, 255), 0);

			else if (lineesp == 4 && WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].RealDistance > 4.0f)
				DrawLine(pDevice, (float)WeaponEspInfo[i].pOutX, (float)WeaponEspInfo[i].pOutY, ScreenCX, ScreenCY * 0.6f, 1.0f, D3DCOLOR_ARGB(255, 255, 255, 255), 0);

			else if (lineesp == 5 && WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].RealDistance > 4.0f)
				DrawLine(pDevice, (float)WeaponEspInfo[i].pOutX, (float)WeaponEspInfo[i].pOutY, ScreenCX, ScreenCY * 0.8f, 1.0f, D3DCOLOR_ARGB(255, 255, 255, 255), 0);

			else if (lineesp == 6 && WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].RealDistance > 4.0f)
				DrawLine(pDevice, (float)WeaponEspInfo[i].pOutX, (float)WeaponEspInfo[i].pOutY, ScreenCX, ScreenCY * 1.0f, 1.0f, D3DCOLOR_ARGB(255, 255, 255, 255), 0);

			else if (lineesp == 7 && WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].RealDistance > 4.0f)
				DrawLine(pDevice, (float)WeaponEspInfo[i].pOutX, (float)WeaponEspInfo[i].pOutY, ScreenCX, ScreenCY * 1.2f, 1.0f, D3DCOLOR_ARGB(255, 255, 255, 255), 0);

			else if (lineesp == 8 && WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].RealDistance > 4.0f)
				DrawLine(pDevice, (float)WeaponEspInfo[i].pOutX, (float)WeaponEspInfo[i].pOutY, ScreenCX, ScreenCY * 1.4f, 1.0f, D3DCOLOR_ARGB(255, 255, 255, 255), 0);

			else if (lineesp == 9 && WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].RealDistance > 4.0f)
				DrawLine(pDevice, (float)WeaponEspInfo[i].pOutX, (float)WeaponEspInfo[i].pOutY, ScreenCX, ScreenCY * 1.6f, 1.0f, D3DCOLOR_ARGB(255, 255, 255, 255), 0);

			else if (lineesp == 10 && WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].RealDistance > 4.0f)
				DrawLine(pDevice, (float)WeaponEspInfo[i].pOutX, (float)WeaponEspInfo[i].pOutY, ScreenCX, ScreenCY * 1.8f, 1.0f, D3DCOLOR_ARGB(255, 255, 255, 255), 0);

			else if (lineesp == 11 && WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].RealDistance > 4.0f)
				DrawLine(pDevice, (float)WeaponEspInfo[i].pOutX, (float)WeaponEspInfo[i].pOutY, ScreenCX, ScreenCY * 2.0f, 1.0f, D3DCOLOR_ARGB(255, 255, 255, 255), 0);
		}
	}

	//do box esp
	if (boxesp > 0 && WeaponEspInfo.size() != NULL)
	{
		for (unsigned int i = 0; i < WeaponEspInfo.size(); i++)
		{
			//box esp
			if (WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].RealDistance > 4.0f)
				DrawCornerBox(pDevice, (int)WeaponEspInfo[i].pOutX + 2, (int)WeaponEspInfo[i].pOutY + 2 + 20, 20, 30, 1, D3DCOLOR_ARGB(255, 255, 255, 255));
		}
	}

	//do pic esp
	if (picesp == 1 && WeaponEspInfo.size() != NULL)
	{
		for (unsigned int i = 0; i < WeaponEspInfo.size(); i++)
		{
			if (WeaponEspInfo[i].pOutX > 1.0f && WeaponEspInfo[i].pOutY > 1.0f && (float)WeaponEspInfo[i].RealDistance > 4.0f)
				DrawPic(pDevice, pSpriteTextureImage, (int)WeaponEspInfo[i].pOutX, (int)WeaponEspInfo[i].pOutY);
		}
	}


	//do aim
	if (aimbot == 1 && WeaponEspInfo.size() != NULL)
	{
		UINT BestTarget = -1;
		DOUBLE fClosestPos = 99999;

		for (unsigned int i = 0; i < WeaponEspInfo.size(); i++)
		{
			//aimfov
			float radiusx = (aimfov*5.0f) * (ScreenCX / 100.0f);
			float radiusy = (aimfov*5.0f) * (ScreenCY / 100.0f);

			if (aimfov == 0)
			{
				radiusx = 5.0f * (ScreenCX / 100.0f);
				radiusy = 5.0f * (ScreenCY / 100.0f);
			}

			//get crosshairdistance
			WeaponEspInfo[i].CrosshairDistance = GetDistance(WeaponEspInfo[i].pOutX, WeaponEspInfo[i].pOutY, ScreenCX, ScreenCY);

			//if in fov
			if (WeaponEspInfo[i].pOutX >= ScreenCX - radiusx && WeaponEspInfo[i].pOutX <= ScreenCX + radiusx && WeaponEspInfo[i].pOutY >= ScreenCY - radiusy && WeaponEspInfo[i].pOutY <= ScreenCY + radiusy)

				//get closest/nearest target to crosshair
				if (WeaponEspInfo[i].CrosshairDistance < fClosestPos)
				{
					fClosestPos = WeaponEspInfo[i].CrosshairDistance;
					BestTarget = i;
				}
		}


		//if nearest target to crosshair
		if (BestTarget != -1 && WeaponEspInfo[BestTarget].RealDistance > 4.0f)//do not aim at self
		{
			double DistX = WeaponEspInfo[BestTarget].pOutX - ScreenCX;
			double DistY = WeaponEspInfo[BestTarget].pOutY - ScreenCY;

			DistX /= (0.5f + (float)aimsens*0.5f);
			DistY /= (0.5f + (float)aimsens*0.5f);

			//aim
			if (GetAsyncKeyState(Daimkey) & 0x8000)
				mouse_event(MOUSEEVENTF_MOVE, (float)DistX, (float)DistY, 0, NULL);

			//autoshoot on
			if ((!GetAsyncKeyState(VK_LBUTTON) && (autoshoot == 1) && (GetAsyncKeyState(Daimkey) & 0x8000))) //
			{
				if (autoshoot == 1 && !IsPressed)
				{
					IsPressed = true;
					mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
				}
			}
		}
	}
	WeaponEspInfo.clear();

	//autoshoot off
	if (autoshoot == 1 && IsPressed)
	{
		if (timeGetTime() - astime >= asdelay)
		{
			IsPressed = false;
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			astime = timeGetTime();
		}
	}

	/*
	//draw logger
	if (Font && countnum != 0)
	{
	char szString[255];
	sprintf_s(szString, "countnum = %d", countnum);
	DrawString(Font, 219, 99, D3DCOLOR_ARGB(255, 0, 0, 0), (char*)&szString[0]);
	DrawString(Font, 221, 101, D3DCOLOR_ARGB(255, 0, 0, 0), (char*)&szString[0]);
	DrawString(Font, 220, 100, D3DCOLOR_ARGB(255, 255, 255, 255), (char*)&szString[0]);
	DrawString(Font, 220, 110, D3DCOLOR_ARGB(255, 255, 255, 255), (PCHAR)"hold P to +");
	DrawString(Font, 220, 120, D3DCOLOR_ARGB(255, 255, 255, 255), (PCHAR)"hold O to -");
	}
	*/

	return oPresent(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

//==========================================================================================================================

HRESULT APIENTRY hkReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	DeleteSprite();

	if (Font)
		Font->OnLostDevice();

	if (pLine)
		pLine->OnLostDevice();

	if (pSprite)
		pSprite->OnLostDevice();

	HRESULT ResetReturn = oReset(pDevice, pPresentationParameters);

	if (SUCCEEDED(ResetReturn))
	{
		if (Font)
			Font->OnResetDevice();

		if (pLine)
			pLine->OnResetDevice();

		if (pSprite)
			pSprite->OnResetDevice();

		InitOnce = true;
	}

	return ResetReturn;
}

//==========================================================================================================================

#define ENDSCENE 0 
#define PRESENT 1 
#define SETTEXTURE 2
#define SETSTREAMSOURCE 3
#define RESET 4
//#define DRAWINDEXEDPRIMITIVE 5
//#define SETVERTEXSHADERCONSTANTF 6

bool hooked = false;
#define HOOK(func,addy) o##func = (t##func)DetourFunction((PBYTE)addy,(PBYTE)hk##func)
#define UNHOOK(func,addy) o##func = (t##func)DetourFunction((PBYTE)addy,(PBYTE)o##func)

PBYTE HookVTable(PDWORD* dwVTable, PBYTE dwHook, INT Index)
{
	DWORD dwOld = 0;
	VirtualProtect((void*)((*dwVTable) + (Index * 4)), 4, PAGE_EXECUTE_READWRITE, &dwOld);
	PBYTE pOrig = ((PBYTE)(*dwVTable)[Index]);
	(*dwVTable)[Index] = (DWORD)dwHook;
	VirtualProtect((void*)((*dwVTable) + (Index * 4)), 4, dwOld, &dwOld);
	return pOrig;
}

LRESULT CALLBACK MsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { return DefWindowProc(hwnd, uMsg, wParam, lParam); }
void DX_Init(DWORD* table)
{
	WNDCLASSEXA wc = { sizeof(WNDCLASSEX),CS_CLASSDC,MsgProc,0L,0L,GetModuleHandleA(NULL),NULL,NULL,NULL,NULL,"DX",NULL };
	RegisterClassExA(&wc);
	HWND hWnd = CreateWindowA("DX", NULL, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, GetDesktopWindow(), NULL, wc.hInstance, NULL);
	LPDIRECT3D9 pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	LPDIRECT3DDEVICE9 pd3dDevice;
	pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pd3dDevice);
	DWORD* pVTable = (DWORD*)pd3dDevice;
	pVTable = (DWORD*)pVTable[0];

	table[ENDSCENE] = pVTable[42];
	table[PRESENT] = pVTable[17];
	table[SETTEXTURE] = pVTable[65];                    
	table[SETSTREAMSOURCE] = pVTable[100];
	table[RESET] = pVTable[16];
	//table[DRAWINDEXEDPRIMITIVE] = pVTable[82];
	//table[SETVERTEXSHADERCONSTANTF] = pVTable[94];


	DestroyWindow(hWnd);
}

unsigned _stdcall VMT_Patching(LPVOID  Param)
{
	while (1)
	{
		Sleep(100);
		HookVTable((PDWORD*)npDevice, (PBYTE)hkEndScene, 42);
		HookVTable((PDWORD*)npDevice, (PBYTE)hkPresent, 17);
		HookVTable((PDWORD*)npDevice, (PBYTE)hkSetTexture, 65);
		HookVTable((PDWORD*)npDevice, (PBYTE)hkSetStreamSource, 100);
		HookVTable((PDWORD*)npDevice, (PBYTE)hkReset, 16);
		//HookVTable((PDWORD*)npDevice, (PBYTE)hkDrawIndexedPrimitive, 82);
		//HookVTable((PDWORD*)npDevice, (PBYTE)hkSetVertexShaderConstantF, 94);
	}
	return 1;
}

unsigned _stdcall LoopFunction(LPVOID lpParam)
{
	while (1) 
	{
		if (hooked == false)
		{
			DWORD VTable[3] = { 0 };
			while (GetModuleHandleA("d3d9.dll") == NULL)
			{
				Sleep(250);
			}
			DX_Init(VTable);

			HOOK(EndScene, VTable[ENDSCENE]);
			//pPresent = (oPresent)DetourFunction((PBYTE)VTable[ENDSCENE], (PBYTE)myPresent);

			while (!npDevice)
			{
				Sleep(50);
			}
			UNHOOK(EndScene, VTable[ENDSCENE]);
			//pPresent = (oPresent)DetourFunction((PBYTE)VTable[ENDSCENE], (PBYTE)pPresent);

			*(PDWORD)&oEndScene = VTable[ENDSCENE];
			*(PDWORD)&oPresent = VTable[PRESENT];
			*(PDWORD)&oSetTexture = VTable[SETTEXTURE];
			*(PDWORD)&oSetStreamSource = VTable[SETSTREAMSOURCE];
			*(PDWORD)&oReset = VTable[RESET];

			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)VMT_Patching, NULL, 0, NULL);
			//_beginthreadex(0, 0, VMT_Patching, 0, 0, 0);

			hooked = true;

			Sleep(200);
		}
	}
	//return hooked;
	return 0;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule); //disable unwanted thread notifications to reduce overhead
		GetModuleFileNameA(hModule, dlldir, 512);
		for (int i = (int)strlen(dlldir); i > 0; i--)
		{
			if (dlldir[i] == '\\')
			{
				dlldir[i + 1] = 0;
				break;
			}
		}

		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)LoopFunction, 0, 0, 0);
		//_beginthreadex(0,0,LoopFunction,0,0,0);
	}
	return TRUE;
}