/* Copyright (c) 1996-2016, OPC Foundation. All rights reserved.

   The source code in this file is covered under a dual-license scenario:
     - RCL: for OPC Foundation members in good-standing
     - GPL V2: everybody else

   RCL license terms accompanied with this source code. See http://opcfoundation.org/License/RCL/1.00/

   GNU General Public License as published by the Free Software Foundation;
   version 2 of the License are accompanied with this source code. See http://opcfoundation.org/License/GPLv2

   This source code is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* System Headers */
#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <wincrypt.h>

/* UA platform definitions */
#include <opcua_p_internal.h>
#include <opcua_p_memory.h>
#include <opcua_p_cryptofactory.h>

/* own headers */
#include <opcua_p_wincrypt.h>

#define MAX_GENERATED_OUTPUT_LEN  1024

HCRYPTPROV OpcUa_g_hCryptoProvider;


#ifdef UNICODE
#define OPCUA_P_CHAR_TYPE(xConstString) (LPCWSTR)L##xConstString
#else
#define OPCUA_P_CHAR_TYPE(xConstString) xConstString
#endif /* UNICODE */

/*============================================================================
 * OpcUa_P_WinCrypt_Random_Initialize
 *===========================================================================*/
OpcUa_StatusCode OpcUa_P_WinCrypt_Initialize()
{
    /*DWORD err = 0;*/

    OpcUa_InitializeStatus(OpcUa_Module_P_WinCrypt, "Initialize");

    if (!CryptAcquireContext(&OpcUa_g_hCryptoProvider, OPCUA_P_CHAR_TYPE("Container"), NULL, PROV_RSA_FULL, 0))
    {
        DWORD dwLastError = GetLastError();

#if 0
        printf("CryptAcquireContext LastError1 0x%X\n", dwLastError);
#endif

        if (dwLastError == NTE_BAD_KEYSET)
        {
            if (!CryptAcquireContext(&OpcUa_g_hCryptoProvider, 
                OPCUA_P_CHAR_TYPE("Container"), 
                NULL, 
                PROV_RSA_FULL, 
                CRYPT_NEWKEYSET))
            {
#if 0
                dwLastError = GetLastError();
                printf("CryptAcquireContext LastError2 0x%X\n", dwLastError);
#endif
                uStatus = OpcUa_Bad;
            }
#if 0
            else
            {
                printf("CryptAcquireContext OK\n");
            }
#endif
        }
        else
        {
            uStatus = OpcUa_Bad;
        }
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_P_WinCrypt_Random_Clean
 *===========================================================================*/
OpcUa_StatusCode OpcUa_P_WinCrypt_Cleanup()
{
    OpcUa_InitializeStatus(OpcUa_Module_P_WinCrypt, "Cleanup");

    OpcUa_ReturnErrorIfTrue(!CryptReleaseContext(OpcUa_g_hCryptoProvider, 0),OpcUa_Bad);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_P_WinCrypt_Random_Key_Generate
 *===========================================================================*/
OpcUa_StatusCode OpcUa_P_WinCrypt_Random_Key_Generate(
    OpcUa_CryptoProvider* a_pProvider,
    OpcUa_Int32           a_keyLen,
    OpcUa_Key*            a_pKey)
{
    OpcUa_CryptoProviderConfig* pConfig = OpcUa_Null;
    OpcUa_Int32                 keyLen  = 0;

    OpcUa_InitializeStatus(OpcUa_Module_P_WinCrypt, "Random_Key_Generate");

    OpcUa_ReturnErrorIfArgumentNull(a_pProvider);
    OpcUa_ReturnErrorIfArgumentNull(a_pKey);
   
    keyLen = a_keyLen;
    
    if(keyLen < 0) 
    {
        if(a_pProvider->Handle != OpcUa_Null)
        {
            /* get default configuration */
            pConfig = (OpcUa_CryptoProviderConfig*)a_pProvider->Handle;
            keyLen = pConfig->SymmetricKeyLength;
        }
        else
        {
            uStatus = OpcUa_BadInvalidArgument;
            OpcUa_GotoErrorIfBad(uStatus);
        }
    }
    else if(keyLen > MAX_GENERATED_OUTPUT_LEN)
    {
            uStatus = OpcUa_BadInvalidArgument;
            OpcUa_GotoErrorIfBad(uStatus);
    }

    a_pKey->Key.Length = keyLen;
    a_pKey->Type = OpcUa_Crypto_KeyType_Random;

    if(a_pKey->Key.Data == OpcUa_Null)
    {
        OpcUa_ReturnStatusCode;
    }

    if (!CryptGenRandom(OpcUa_g_hCryptoProvider, a_pKey->Key.Length, a_pKey->Key.Data))
    {
        DWORD dwLastError = GetLastError();
        switch(dwLastError)
        {
        case ERROR_INVALID_HANDLE:
            {
                uStatus = OpcUa_BadInvalidArgument;
            }
        case ERROR_INVALID_PARAMETER:
            {
                uStatus = OpcUa_BadInvalidArgument;
            }
        case NTE_BAD_UID:
            {
                uStatus = OpcUa_BadInvalidArgument;
            }
        case NTE_FAIL:
            {
                uStatus = OpcUa_BadUnexpectedError;
            }
        default:
            {
                uStatus = OpcUa_Bad;
            }
        }
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

OpcUa_FinishErrorHandling;
}
