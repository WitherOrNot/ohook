#define _WIN32_WINNT _WIN32_WINNT_WIN10
#include <windows.h>
#include <shlwapi.h>
#include "sl.h"

BOOL bIsHeartbeatRegistryModified = FALSE;

BOOL IsGracePeriodProduct(HSLC hSLC, SLID *pProductSkuId) {
    PBYTE pBuffer = 0;
    UINT cbSize = 0;

    if(SLGetProductSkuInformation(hSLC, pProductSkuId, L"Name", NULL, &cbSize, &pBuffer) != S_OK) {
        LocalFree(pBuffer);
        return FALSE;
    }

    if(StrStrNIW((PWSTR)pBuffer, L"Grace", cbSize) != NULL) {
        LocalFree(pBuffer);
        return TRUE;
    }

    LocalFree(pBuffer);
    return FALSE;
}

HRESULT WINAPI SLGetLicensingStatusInformationHook(
    HSLC hSLC,
    SLID *pAppID,
    SLID *pProductSkuId,
    PWSTR pwszRightName,
    UINT *pnStatusCount,
    SL_LICENSING_STATUS **ppLicensingStatus
) {
    HRESULT hResult = SLGetLicensingStatusInformation(
        hSLC,
        pAppID,
        pProductSkuId,
        pwszRightName,
        pnStatusCount,
        ppLicensingStatus
    );

    if(hResult != S_OK)
        return hResult;

    for(int i = 0; i < *pnStatusCount; i++) {
        if((*ppLicensingStatus+i)->eStatus == 0) continue;
        if(IsGracePeriodProduct(hSLC, &((*ppLicensingStatus+i)->SkuId))) continue;

        (*ppLicensingStatus+i)->eStatus = 1;
        (*ppLicensingStatus+i)->dwGraceTime = 0;
        (*ppLicensingStatus+i)->dwTotalGraceDays = 0;
        (*ppLicensingStatus+i)->hrReason = 0;
        (*ppLicensingStatus+i)->qwValidityExpiration = 0;
    }

    return hResult;
}

BOOL APIENTRY WINAPI DllMain(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved
) {
    return TRUE;
}
