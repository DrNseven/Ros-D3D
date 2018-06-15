/*
* Ros D3D 1.5b by n7
How to compile:
- compile with visual studio community 2017 (..\Microsoft Visual Studio\2017\Community\Common7\IDE\devenv.exe)
- select Release x86
- click: project -> properties -> configuration properties -> general -> character set -> change to "not set"
- compile with CTRL + Shift + B
Optional: remove dependecy on vs runtime:
- click: project -> properties -> configuration properties -> C/C++ -> code generation -> runtime library: Multi-threaded (/MT)
*/

#include "main.h" //less important stuff & helper funcs here

typedef HRESULT(APIENTRY *SetStreamSource_t)(IDirect3DDevice9*, UINT, IDirect3DVertexBuffer9*, UINT, UINT);
SetStreamSource_t SetStreamSource_orig = 0;

typedef HRESULT(APIENTRY *SetTexture_t)(IDirect3DDevice9*, DWORD, IDirect3DBaseTexture9 *);
SetTexture_t SetTexture_orig = 0;

typedef HRESULT(APIENTRY* Present_t) (IDirect3DDevice9*, const RECT *, const RECT *, HWND, const RGNDATA *);
Present_t Present_orig = 0;

typedef HRESULT(APIENTRY *Reset_t)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
HRESULT APIENTRY Reset_hook(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
Reset_t Reset_orig = 0;

//==========================================================================================================================

HRESULT APIENTRY SetStreamSource_hook(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT sStride)
{
	if (StreamNumber == 0)
		Stride = sStride;

	return SetStreamSource_orig(pDevice, StreamNumber, pStreamData, OffsetInBytes, sStride);
}

//==========================================================================================================================

HRESULT APIENTRY SetTexture_hook(LPDIRECT3DDEVICE9 pDevice, DWORD Sampler, IDirect3DBaseTexture9 *pTexture)
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
			SetTexture_orig(pDevice, 0, Green);
			SetTexture_orig(pDevice, 1, Green);
		}

		//red helmets
		if ((vSize == 2008 && qCRC == 0xc46ee841) ||//helmet 1
			(vSize == 2008 && qCRC == 0x9590d282) ||//helmet 2
			(vSize == 2008 && qCRC == 0xe248e914))//helmet 3
		{
			SetTexture_orig(pDevice, 0, Red);
			SetTexture_orig(pDevice, 1, Red);
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
			//DrawtoTarget(pDevice);
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
	return SetTexture_orig(pDevice, Sampler, pTexture);
}

//==========================================================================================================================

HRESULT APIENTRY Present_hook(IDirect3DDevice9* pDevice, const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion)
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
	if (!SpriteCreateda)
		CreateSpritea(pDevice);

	//draw Animation
	//if (ShowMenu)
	//DrawAnima(pDevice, 114, 173, 1.52f, 0.8f, 1.0f);
	//DrawAnima(pDevice, 114, 116, 1.5f, 1.5f, 1.5f);
	//DrawAnima(pDevice, Viewport.Width / 2, Viewport.Height*0.90f, 1.0f, 1.0f, 1.0f);//middle/bottom

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
				DrawPica(pDevice, pSpriteTextureImagea, (int)WeaponEspInfo[i].pOutX, (int)WeaponEspInfo[i].pOutY);
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
	return Present_orig(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

//==========================================================================================================================

HRESULT APIENTRY Reset_hook(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	DeleteSpritea();

	if (Font)
		Font->OnLostDevice();

	if (pLine)
		pLine->OnLostDevice();

	if (pSpritea)
		pSpritea->OnLostDevice();

	HRESULT ResetReturn = Reset_orig(pDevice, pPresentationParameters);

	if (SUCCEEDED(ResetReturn))
	{
		if (Font)
			Font->OnResetDevice();

		if (pLine)
			pLine->OnResetDevice();

		if (pSpritea)
			pSpritea->OnResetDevice();

		InitOnce = true;
	}

	return ResetReturn;
}

//==========================================================================================================================

DWORD WINAPI RosD3D(LPVOID lpParameter)
{
	HMODULE dDll = NULL;
	while (!dDll)
	{
		dDll = GetModuleHandleA("d3d9.dll");
		Sleep(100);
	}
	CloseHandle(dDll);

	IDirect3D9* d3d = NULL;
	IDirect3DDevice9* d3ddev = NULL;

	HWND tmpWnd = CreateWindowA("BUTTON", "DX", WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, NULL, NULL, Hand, NULL);
	if (tmpWnd == NULL)
	{
		//Log("[DirectX] Failed to create temp window");
		return 0;
	}

	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (d3d == NULL)
	{
		DestroyWindow(tmpWnd);
		//Log("[DirectX] Failed to create temp Direct3D interface");
		return 0;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = tmpWnd;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	HRESULT result = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, tmpWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3ddev);
	if (result != D3D_OK)
	{
		d3d->Release();
		DestroyWindow(tmpWnd);
		//Log("[DirectX] Failed to create temp Direct3D device");
		return 0;
	}

	// We have the device, so walk the vtable to get the address of all the dx functions in d3d9.dll
#if defined _M_X64
	DWORD64* dVtable = (DWORD64*)d3ddev;
	dVtable = (DWORD64*)dVtable[0];
#elif defined _M_IX86
	DWORD* dVtable = (DWORD*)d3ddev;
	dVtable = (DWORD*)dVtable[0]; // == *d3ddev
#endif
	//Log("[DirectX] dVtable: %x", dVtable);

	//for(int i = 0; i < 95; i++)
	//{
			//Log("[DirectX] vtable[%i]: %x, pointer at %x", i, dVtable[i], &dVtable[i]);
	//}

	// Detour functions x86 & x64
	if (MH_Initialize() != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[17], &Present_hook, reinterpret_cast<void**>(&Present_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[17]) != MH_OK) { return 1; }
	
	if (MH_CreateHook((DWORD_PTR*)dVtable[100], &SetStreamSource_hook, reinterpret_cast<void**>(&SetStreamSource_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[100]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[65], &SetTexture_hook, reinterpret_cast<void**>(&SetTexture_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[65]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[16], &Reset_hook, reinterpret_cast<void**>(&Reset_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[16]) != MH_OK) { return 1; }
	
	//Log("[Detours] Detours attached\n");

	d3ddev->Release();
	d3d->Release();
	DestroyWindow(tmpWnd);

	return 1;
}

//==========================================================================================================================

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Hand = hModule;
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
		CreateThread(0, 0, RosD3D, 0, 0, 0); //init our hooks

		break;
	case DLL_PROCESS_DETACH:
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}
