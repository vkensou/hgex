#include "renderdoc_helper.h"

#ifdef _WIN32
#include <Windows.h>

#endif
std::string locate_renderdoc()
{
#ifdef _WIN32
    HKEY hKey = NULL;
    DWORD dwType = 0;
    char szValueName[MAXBYTE], szValueKey[MAXBYTE] = { 0 };
    DWORD dwBufferSize = MAXBYTE, dwKeySize = MAXBYTE;

    LONG lRet = RegOpenKey(HKEY_LOCAL_MACHINE, "Software\\Classes\\CLSID\\{5D6BF029-A6BA-417A-8523-120492B1DCE3}\\InprocServer32", &hKey);
    if (lRet != 0)
        return "";

    if (RegEnumValue(hKey, 0, szValueName, &dwBufferSize, NULL,
        &dwType, (unsigned char*)szValueKey, &dwKeySize) == 0)
        return szValueKey;
#endif

    return "";
}

bool load_renderdoc(const std::string& path)
{
#ifdef _WIN32
	HINSTANCE hinstLib;
	hinstLib = LoadLibrary(path.c_str());
	return hinstLib != NULL;
#endif
	return false;
}

RENDERDOC_API_1_0_0* GetRenderDocApi()
{
	RENDERDOC_API_1_0_0* rdoc = nullptr;
	HMODULE module = GetModuleHandleA("renderdoc.dll");

	if (module == NULL)
	{
		return nullptr;
	}

	pRENDERDOC_GetAPI getApi = nullptr;
	getApi = (pRENDERDOC_GetAPI)GetProcAddress(module, "RENDERDOC_GetAPI");

	if (getApi == nullptr)
	{
		return nullptr;
	}

	if (getApi(eRENDERDOC_API_Version_1_0_0, (void**)&rdoc) != 1)
	{
		return nullptr;
	}

	return rdoc;
}
