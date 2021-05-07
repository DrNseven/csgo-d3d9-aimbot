#include <windows.h>
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

//dx sdk dir
#include "DXSDK\d3dx9.h"
#if defined _M_X64
#pragma comment(lib, "DXSDK/x64/d3dx9.lib") 
#elif defined _M_IX86
#pragma comment(lib, "DXSDK/x86/d3dx9.lib")
#endif


typedef HRESULT(APIENTRY* tEndScene)(LPDIRECT3DDEVICE9 pDevice);
typedef HRESULT(APIENTRY* tDrawIndexedPrimitive)(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);
typedef HRESULT(APIENTRY* tSetVertexShaderConstantF)(LPDIRECT3DDEVICE9 pDevice, UINT, const float*, UINT);


tEndScene oEndScene = nullptr;
tDrawIndexedPrimitive oDrawIndexedPrimitive = nullptr;
tSetVertexShaderConstantF oSetVertexShaderConstantF = nullptr;

LPDIRECT3DDEVICE9 pD3DDevice = nullptr;
void* d3d9Device[119];

bool bInit = false;
UINT mStartRegister;
UINT mVector4fCount;
bool makeTmagenta = true;

int aimbot = 0;
DWORD Daimkey = VK_SHIFT;		//aimkey
float aimsens = 1.0f;			//aim sensitivity, makes aim smoother
int aimfov = 4;				    //aim field of view in % 

#include "main.h"


//magic
#define CT_HANDS (Stride == 32 && NumVertices > 500 && mStartRegister == 58 && mVector4fCount == 150)
#define T_HANDS ((Stride == 32 && NumVertices > 500 && mStartRegister == 58)&&(mVector4fCount == 108||mVector4fCount == 144||mVector4fCount == 126))
//#define CT_HANDS ((Stride == 32 && NumVertices == 2052 && primCount == 2966 && mStartRegister == 58 && mVector4fCount == 150)||(Stride == 32 && NumVertices == 2112 && primCount == 2998 && mStartRegister == 58 && mVector4fCount == 150))
//#define T_HANDS ((Stride == 32 && NumVertices == 1006 && primCount == 1560 && mStartRegister == 58)&&(mVector4fCount == 108||mVector4fCount == 144||mVector4fCount == 126))


HRESULT APIENTRY hkSetVertexShaderConstantF(LPDIRECT3DDEVICE9 pDevice, UINT StartRegister, const float* pConstantData, UINT Vector4fCount)
{
    if (pConstantData)
    {
        mStartRegister = StartRegister;
        mVector4fCount = Vector4fCount;
    }

    return oSetVertexShaderConstantF(pDevice, StartRegister, pConstantData, Vector4fCount);
}

HRESULT APIENTRY hkEndScene(LPDIRECT3DDEVICE9 pDevice)
{
    pDevice->GetViewport(&Viewport); //get viewport
    ScreenCenterX = Viewport.Width / 2.0f;
    ScreenCenterY = Viewport.Height / 2.0f;

    //esp 
    if (ModelInfo.size() != NULL)
    {
        for (unsigned int i = 0; i < ModelInfo.size(); i++)
        {
            if (ModelInfo[i].vOutX > 1 && ModelInfo[i].vOutY > 1 && ModelInfo[i].vOutX < Viewport.Width && ModelInfo[i].vOutY < Viewport.Height)
            DrawFilledRect(pDevice, (int)ModelInfo[i].vOutX, (int)ModelInfo[i].vOutY, 6, 6, 0xFFFFFF00);
        }
    }

    if (ModelInfo.size() != NULL)
    {
        UINT BestTarget = -1;
        DOUBLE fClosestPos = 99999;

        for (unsigned int i = 0; i < ModelInfo.size(); i++)
        {
            //aimfov
            float radiusx = (aimfov * 10.0f) * (ScreenCenterX / 100);
            float radiusy = (aimfov * 10.0f) * (ScreenCenterY / 100);

            if (aimfov == 0)
            {
                radiusx = 10.0f * (ScreenCenterX / 100);
                radiusy = 10.0f * (ScreenCenterY / 100);
            }

            //get crosshairdistance
            ModelInfo[i].CrosshairDistance = GetDistance(ModelInfo[i].vOutX, ModelInfo[i].vOutY, ScreenCenterX, ScreenCenterY);

            //aim at team 1 or 2 (not needed)
            //if (aimbot == ModelInfo[i].iTeam)

                //if in fov
                if (ModelInfo[i].vOutX >= ScreenCenterX - radiusx && ModelInfo[i].vOutX <= ScreenCenterX + radiusx && ModelInfo[i].vOutY >= ScreenCenterY - radiusy && ModelInfo[i].vOutY <= ScreenCenterY + radiusy)

                    //get closest/nearest target to crosshair
                    if (ModelInfo[i].CrosshairDistance < fClosestPos)
                    {
                        fClosestPos = ModelInfo[i].CrosshairDistance;
                        BestTarget = i;
                    }
        }


        //if nearest target to crosshair
        if (BestTarget != -1)
        {
            float DistX = ModelInfo[BestTarget].vOutX - ScreenCenterX;
            float DistY = ModelInfo[BestTarget].vOutY - ScreenCenterY;

            DistX /= aimsens;
            DistY /= aimsens;

            //aim
            if(GetAsyncKeyState(Daimkey) & 0x8000)
                MouseMove(DistX, DistY);
            //rip mouse_event
        }
    }
    ModelInfo.clear();

    return oEndScene(pDevice);
}


HRESULT APIENTRY hkDrawIndexedPrimitive(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
    //if(!texMagenta)
    //GenerateTexture(pDevice, &texMagenta, D3DCOLOR_ARGB(255, 255, 0, 255));

    //if (!texGreen)
    //GenerateTexture(pDevice, &texGreen, D3DCOLOR_ARGB(255, 0, 255, 0));   

    if (!sMagenta)
        GenerateShader(pDevice, &sMagenta, 1.0f, 0.0f, 1.0f, false);

    if (!sGreen)
        GenerateShader(pDevice, &sGreen, 0.0f, 1.0f, 0.0f, false);

	LPDIRECT3DVERTEXBUFFER9 StreamData;
	UINT Offset = 0;
	UINT Stride = 0;

	if (pDevice->GetStreamSource(0, &StreamData, &Offset, &Stride) == D3D_OK)
		StreamData->Release();


    //toggle team colors
    if (GetAsyncKeyState(VK_F11) & 1) //<------------------------------------------------------------------------------------- F11
    {
        aimbot=!aimbot;
    }

    if (aimbot == 0 && T_HANDS || aimbot == 1 && CT_HANDS)
    {
        pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        oDrawIndexedPrimitive(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
        //pDevice->SetTexture(0, texMagenta);
        pDevice->SetPixelShader(sMagenta);
        pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
    }

    if (aimbot == 0 && T_HANDS || aimbot == 1 && CT_HANDS)
    {
        AddModel(pDevice, 1);
    }


    //hold down P key until a texture changes, press I to log values of those textures
    if (GetAsyncKeyState('O') & 1) //-
        countnum--;
    if (GetAsyncKeyState('P') & 1) //+
        countnum++;
    if ((GetAsyncKeyState(VK_MENU)) && (GetAsyncKeyState('9') & 1)) //reset, set to -1
        countnum = -1;
    //if (countnum == NumVertices)
        //if ((Stride > NULL) && (GetAsyncKeyState('I') & 1)) //press I to log to log.txt
            //Log("Stride == %d && NumVertices == %d && primCount == %d && mStartRegister == %d && mVector4fCount == %d", Stride, NumVertices, primCount, mStartRegister, mVector4fCount);

    if (countnum == NumVertices)
    {
        //return D3D_OK; //delete texture
    }

	return oDrawIndexedPrimitive(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

bool Hook(char* src, char* dst, int len)
{
    if (len < 5) return false;

    DWORD curProtection;

    VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &curProtection);

    memset(src, 0x90, len);

    uintptr_t relativeAddress = (uintptr_t)(dst - src - 5);

    *src = (char)0xE9;
    *(uintptr_t*)(src + 1) = (uintptr_t)relativeAddress;

    DWORD temp;
    VirtualProtect(src, len, curProtection, &temp);

    return true;
}

char* TrampHook(char* src, char* dst, unsigned int len)
{
    if (len < 5) return 0;

    // Create the gateway (len + 5 for the overwritten bytes + the jmp)
    char* gateway = (char*)VirtualAlloc(0, len + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    // Put the bytes that will be overwritten in the gateway
    memcpy(gateway, src, len);

    // Get the gateway to destination addy
    uintptr_t gateJmpAddy = (uintptr_t)(src - gateway - 5);

    // Add the jmp opcode to the end of the gateway
    *(gateway + len) = (char)0xE9;

    // Add the address to the jmp
    *(uintptr_t*)(gateway + len + 1) = gateJmpAddy;

    // Place the hook at the destination
    if (Hook(src, dst, len))
    {
        return gateway;
    }
    else return nullptr;
}

DWORD WINAPI Init(HMODULE hModule)
{
	if (GetD3D9Device(d3d9Device, sizeof(d3d9Device)))
	{
		oEndScene = (tEndScene)TrampHook((char*)d3d9Device[42], (char*)hkEndScene, 7);
		oDrawIndexedPrimitive = (tDrawIndexedPrimitive)TrampHook((char*)d3d9Device[82], (char*)hkDrawIndexedPrimitive, 7);
        oSetVertexShaderConstantF = (tSetVertexShaderConstantF)TrampHook((char*)d3d9Device[94], (char*)hkSetVertexShaderConstantF, 7);
	}
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)Init, hModule, 0, nullptr));
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}