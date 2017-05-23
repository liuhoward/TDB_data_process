// TestTDBAPI_v2.cpp : Defines the entry point for the console application.
//

//#include "Platform.h"
//#include "PathHelper.h"

#ifdef __PLATFORM_WINDOWS__
//#include "stdafx.h"
#include <windows.h>
#else
#include <stdarg.h>
#endif

#include "TDBAPI.h"
#include <vector>
#include <assert.h>
#include <string>
#include <time.h>
#include <iostream>
#include <cstdlib>
#define ELEMENT_COUNT(arr) (sizeof(arr)/sizeof(arr[0]))


int Print(const char* szFormat, ...);


#define TICK_BEGIN(Topic) \
    int nBefore##Topic##Tick_ = GetTickCount();

#define TICK_END(Topic) \
    int nAfter##Topic##Tick_ = GetTickCount(); \
    Print("Topic:%s time-sep:%d\n", #Topic, nAfter##Topic##Tick_ - nBefore##Topic##Tick_);


void GetTick(THANDLE hTdb, const std::string& strCode, bool bWithAB, int nStartDay, int nEndDay, int nStartTime =0, int nEndTime = 0);
void GetK(THANDLE hTdb, const std::string& strCode, CYCTYPE nCycType,int nCycDef, REFILLFLAG nFlag,  int nAutoComplete, int nStartDay, int nEndDay, int nStartTime=0, int nEndTime = 0);

int main(int argc, char* argv[])
{

    OPEN_SETTINGS settings={"192.168.14.200","10010","sz1b_szqhdy","123123",30,2,0};
    //LoadIPFromCin(settings);
    TDBDefine_ResLogin loginAnswer={0};
    THANDLE hTdb = TDB_Open(&settings, &loginAnswer);
    
    if (!hTdb)
    {
        Print("TDB_Open failed:%s, program exit!\n", loginAnswer.szInfo);
        exit(0);
    }

    {
        TDBDefine_Code* pCode = NULL;
        int nCount = 0;
        TDB_GetCodeTable(hTdb, "SH", &pCode, &nCount);
        for (int i=0; i<nCount&&i<20; i++)
        {
           Print("%s %s\n", pCode[i].chCode, pCode[i].chCNName);
        }
        
        delete []pCode;
    }

    char* arrCode[] = {"000001.sz" ,"600000.sh"};
    int arrDays[] = {20170515, 20170516, 20170517};

    for (int i=0; i<sizeof(arrCode)/sizeof(arrCode[0]); i++)
    {
        for (int j=0; j<sizeof(arrDays)/sizeof(arrDays[j]); j++)
        {
            char* pCode = arrCode[i];
            int nDate = arrDays[j];
            GetTick(hTdb, pCode, false, nDate, nDate);
            GetK(hTdb, pCode, CYC_SECOND, 1, REFILL_NONE, 0, nDate, nDate);
            GetK(hTdb, pCode, CYC_SECOND, 1, REFILL_NONE, 1, nDate, nDate);
            GetK(hTdb, pCode, CYC_SECOND, 7, REFILL_NONE, 0, nDate, nDate);
        }
    }

    Print("------------程序自然退出-----------------\n");

    TDB_Close(hTdb);

    system("pause");

}

void GetTick(THANDLE hTdb, const std::string& strCode, bool bWithAB, int nStartDay, int nEndDay, int nStartTime/* =0*/, int nEndTime/* = 0*/)
{
    TDBDefine_ReqTick reqTick = {"",nStartDay, nEndDay, nStartTime, nEndTime};
    strncpy(reqTick.chCode, strCode.c_str(), sizeof(reqTick.chCode));

    if (!bWithAB)
    {
        TDBDefine_Tick* pTick = NULL;
        int nCount;
        int nRet = TDB_GetTick(hTdb, &reqTick, &pTick, &nCount);

        Print("---------------------收到%d项快照，错误码:%d -----------------\n", nCount,nRet);
        for (int i=0; i<nCount && i<10; i++)
        {
            TDBDefine_Tick& tdbTick = *(pTick+i);
            
        }
        delete [] pTick;
    }
    else
    {
        
        TDBDefine_TickAB* pTick = NULL;
        int nCount;
        int nRet = TDB_GetTickAB(hTdb, &reqTick, &pTick, &nCount);

        Print("---------------------收到%d项快照，错误码:%d -----------------\n", nCount, nRet);
        for (int i=0; i<nCount  && i<10; i++)
        {
            TDBDefine_TickAB& tdbTick = *(pTick+i);
            //if(20130409 != tdbTick.nDate)
#ifdef __PLATFORM_WINDOWS__
            Print("code:%s, date:%d, time:%d, nprice:%d, vol:%I64d, turover:%I64d, acc vol:%I64d, acc turover:%I64d\n", tdbTick.chWindCode, tdbTick.nDate, tdbTick.nTime, tdbTick.nPrice, tdbTick.iVolume, tdbTick.iTurover, tdbTick.iAccVolume, tdbTick.iAccTurover);
#else
            Print("code:%s, date:%d, time:%d, nprice:%d, vol:%lld, turover:%lld, acc vol:%lld, acc turover:%lld\n", tdbTick.chWindCode, tdbTick.nDate, tdbTick.nTime, tdbTick.nPrice, tdbTick.iVolume, tdbTick.iTurover, tdbTick.iAccVolume, tdbTick.iAccTurover);
#endif
            
        }
        delete [] pTick;
    }
    
}

void GetK(THANDLE hTdb, const std::string& strCode, CYCTYPE nCycType, int nCycDef, REFILLFLAG nFlag,  int nAutoComplete, int nStartDay, int nEndDay, int nStartTime/*=0*/, int nEndTime/* = 0*/)
{
    TDBDefine_ReqKLine reqK = {"", nFlag, 0, 0, nCycType, nCycDef, nAutoComplete, nStartDay, nEndDay, nStartTime, nEndTime};
    strncpy(reqK.chCode, strCode.c_str(), sizeof(reqK.chCode));
    TDBDefine_KLine* pKLine = NULL;
    int nCount =0;
    //TICK_BEGIN(minitue);1
    int nRet = TDB_GetKLine(hTdb, &reqK, &pKLine, &nCount);
    //TICK_END(minitue);
    Print("---------------------收到%d项K线，错误码:%d -----------------\n", nCount, nRet);
    for (int i=0; i<nCount && i<10; i++)
    {
        TDBDefine_KLine& tdbK = *(pKLine+i);
#ifdef __PLATFORM_WINDOWS__
        Print("code:%s, date:%d, time:%d, open:%d, high:%d, low:%d, close:%d, volume:%I64d, turover:%I64d\n", tdbK.chWindCode, tdbK.nDate, tdbK.nTime, tdbK.nOpen, tdbK.nHigh, tdbK.nLow, tdbK.nClose, tdbK.iVolume, tdbK.iTurover);
#else
        Print("code:%s, date:%d, time:%d, open:%d, high:%d, low:%d, close:%d, volume:%lld, turover:%lld\n", tdbK.chWindCode, tdbK.nDate, tdbK.nTime, tdbK.nOpen, tdbK.nHigh, tdbK.nLow, tdbK.nClose, tdbK.iVolume, tdbK.iTurover);
#endif
        
    }
    delete[] pKLine;
}


int Print(const char* szFormat, ...)
{
    const int MAX_OUTPUT_LEN = 65534;
    int nBufSize = MAX_OUTPUT_LEN+1;
    va_list vArgs;
    va_start(vArgs, szFormat);

    char* szBuf = new char [nBufSize];
    vsnprintf_s(szBuf, nBufSize,nBufSize-1 , szFormat, vArgs);
    va_end(vArgs);

    printf(szBuf);

    delete[] szBuf;
    return 0;
}

