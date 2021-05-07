static HWND window;

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
	DWORD wndProcId;
	GetWindowThreadProcessId(handle, &wndProcId);

	if (GetCurrentProcessId() != wndProcId)
		return TRUE; // skip to next window

	window = handle;
	return FALSE; // window found abort search
}

HWND GetProcessWindow()
{
	window = NULL;
	EnumWindows(EnumWindowsCallback, NULL);
	return window;
}

bool GetD3D9Device(void** pTable, size_t Size)
{
	if (!pTable)
		return false;

	IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	if (!pD3D)
		return false;

	IDirect3DDevice9* pDummyDevice = NULL;

	// options to create dummy device
	D3DPRESENT_PARAMETERS d3dpp = {};
	d3dpp.Windowed = false;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = GetProcessWindow();

	HRESULT dummyDeviceCreated = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDummyDevice);

	if (dummyDeviceCreated != S_OK)
	{
		// may fail in windowed fullscreen mode, trying again with windowed mode
		d3dpp.Windowed = !d3dpp.Windowed;

		dummyDeviceCreated = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDummyDevice);

		if (dummyDeviceCreated != S_OK)
		{
			pD3D->Release();
			return false;
		}
	}

	memcpy(pTable, *reinterpret_cast<void***>(pDummyDevice), Size);

	pDummyDevice->Release();
	pD3D->Release();
	return true;
}

void DrawFilledRect(LPDIRECT3DDEVICE9 Device, int x, int y, int w, int h, D3DCOLOR color)
{
	D3DRECT BarRect = { x, y, x + w, y + h };
	Device->Clear(1, &BarRect, D3DCLEAR_TARGET | D3DCLEAR_TARGET, color, 0, 0);
}

LPDIRECT3DPIXELSHADER9 sMagenta, sGreen;
#include <stdio.h>
HRESULT GenerateShader(IDirect3DDevice9* pD3Ddev, IDirect3DPixelShader9** pShader, float r, float g, float b, bool setzBuf)
{
	char szShader[256];
	ID3DXBuffer* pShaderBuf = NULL;
	if (setzBuf)
		sprintf_s(szShader, "ps_3_0\ndef c0, %f, %f, %f, %f\nmov oC0,c0\nmov oDepth, c0.x", r, g, b, 1.0f);
	else
		sprintf_s(szShader, "ps_3_0\ndef c0, %f, %f, %f, %f\nmov oC0,c0", r, g, b, 1.0f);
	//sprintf_s(szShader, "ps_3_0\ndef c0, %f, %f, %f, %f\nmov r0,c0", r, g, b, 1.0f);
	D3DXAssembleShader(szShader, (strlen(szShader) + 1), NULL, NULL, 0, &pShaderBuf, NULL);
	if (FAILED(pD3Ddev->CreatePixelShader((const DWORD*)pShaderBuf->GetBufferPointer(), pShader)))return E_FAIL;
	return S_OK;
}

/*
LPDIRECT3DTEXTURE9 texGreen = NULL;
LPDIRECT3DTEXTURE9 texMagenta = NULL;
HRESULT GenerateTexture(LPDIRECT3DDEVICE9 pDevice, IDirect3DTexture9** ppD3Dtex, DWORD color)
{
	HRESULT hr = pDevice->CreateTexture(8, 8, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, ppD3Dtex, NULL);

	if (FAILED(hr))
		return hr;

	D3DLOCKED_RECT d3dlr;
	(*ppD3Dtex)->LockRect(0, &d3dlr, 0, 0);
	BYTE* pDstRow = (BYTE*)d3dlr.pBits;
	DWORD* pDst32;

	for (int y = 0; y < 8; y++)
	{
		pDst32 = (DWORD*)pDstRow;
		for (int x = 0; x < 8; x++) *pDst32++ = color;
		pDstRow += d3dlr.Pitch;
	}

	(*ppD3Dtex)->UnlockRect(0);

	return S_OK;
}
*/

D3DVIEWPORT9 Viewport; //use this viewport
float ScreenCenterX;
float ScreenCenterY;

//get distance
float GetDistance(float Xx, float Yy, float xX, float yY)
{
	return sqrt((yY - Yy) * (yY - Yy) + (xX - Xx) * (xX - Xx));
}

#include <vector>
struct ModelInfo_t
{
	float vOutX, vOutY;
	INT       iTeam;
	float CrosshairDistance;
};
std::vector<ModelInfo_t>ModelInfo;

void MouseMove(float x, float y)
{
	INPUT Input = { 0 };
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_MOVE;
	Input.mi.dx = x; 
	Input.mi.dy = y; 
	SendInput(1, &Input, sizeof(INPUT));
}

int countnum = 0;
void AddModel(LPDIRECT3DDEVICE9 pDevice, int iTeam)
{
	
	D3DXVECTOR4 input, position;
	D3DXMATRIX matrix, world, world2, world3, worldcenter;

	pDevice->GetVertexShaderConstantF(8, matrix, 4); // 8 = cmatrix
	pDevice->GetVertexShaderConstantF(58, world, 4); // 58 = cModel

	if (aimbot == 0)//T
		pDevice->GetVertexShaderConstantF(136, world2, 4);
	if (aimbot == 1)//CT
		pDevice->GetVertexShaderConstantF(163, world2, 4);//163, 175, ..

	worldcenter = (world * 0.5f) + (world2 * 0.5f); //center

	input.x = worldcenter._14;
	input.y = worldcenter._24;
	input.z = worldcenter._34;
	input.w = 0.0f; //worldcenter._44;

	D3DXMatrixTranspose(&matrix, &matrix);
	//D3DXVec4Transform(&position, &input, (D3DXMATRIX*)&matrix);

	position.x = input.x * matrix._11 + input.y * matrix._21 + input.z * matrix._31 + matrix._41;
	position.y = input.x * matrix._12 + input.y * matrix._22 + input.z * matrix._32 + matrix._42;
	position.z = input.x * matrix._13 + input.y * matrix._23 + input.z * matrix._33 + matrix._43;
	position.w = input.x * matrix._14 + input.y * matrix._24 + input.z * matrix._34 + matrix._44;

	float xx, yy, zz;
	zz = Viewport.MinZ + position.z * (Viewport.MaxZ - Viewport.MinZ); //real distance

	xx = (((position.x / position.w) * (Viewport.Width / 2)) + Viewport.X + (Viewport.Width / 2));
	yy = (Viewport.Y + (Viewport.Height / 2) - ((position.y / position.w) * (Viewport.Height / 2)));

	if (zz >= 1.0f)
	{
		ModelInfo_t pModelInfo = { static_cast<float>(xx), static_cast<float>(yy), iTeam };
		ModelInfo.push_back(pModelInfo);
	}
	
}
