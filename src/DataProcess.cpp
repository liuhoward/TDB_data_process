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
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <hash_set>

#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
//#include <boost/format.hpp>

#define ELEMENT_COUNT(arr) (sizeof(arr)/sizeof(arr[0]))

using namespace std;
using namespace boost::gregorian;
using namespace boost::posix_time;

int Print(const char* szFormat, ...);


#define TICK_BEGIN(Topic) \
    int nBefore##Topic##Tick_ = GetTickCount();

#define TICK_END(Topic) \
    int nAfter##Topic##Tick_ = GetTickCount(); \
    Print("Topic:%s time-sep:%d\n", #Topic, nAfter##Topic##Tick_ - nBefore##Topic##Tick_);


int GetTickAB(THANDLE hTdb, const std::string& strCode, int nStartDay, int nEndDay, int nStartTime/* =0*/, int nEndTime/* = 0*/, ofstream& origin_out, ofstream& ofile);
void GetK(THANDLE hTdb, const std::string& strCode, CYCTYPE nCycType,int nCycDef, REFILLFLAG nFlag,  int nAutoComplete, int nStartDay, int nEndDay, int nStartTime=0, int nEndTime = 0);

vector<string> importStockCode(string srcFile);


int main(int argc, char* argv[])
{

    int vacations[] = {

        20160101,
        20160102,
        20160103,
        20160207,
        20160208,
        20160209,
        20160210,
        20160211,
        20160212,
        20160213,
        20160402,
        20160403,
        20160404,
        20160430,
        20160501,
        20160502,
        20160609,
        20160610,
        20160611,
        20160915,
        20160916,
        20160917,
        20161001,
        20161002,
        20161003,
        20161004,
        20161005,
        20161006,
        20161007,


        20170101,
        20170102,
        20170127,
        20170128,
        20170129,
        20170130,
        20170131,
        20170201,
        20170202,
        20170402,
        20170403,
        20170404,
        20170429,
        20170430,
        20170501,
        20170528,
        20170529,
        20170530,
        20171001,
        20171002,
        20171003,
        20171004,
        20171005,
        20171006,
        20171007,
        20171008,
    };

    hash_set<int> vacationSet;
    int size = ELEMENT_COUNT(vacations);
    for(int i = 0; i < size; i++) {
        vacationSet.insert(vacations[i]);
    }

    date begin_dt(2016,1,1);
    date end_dt(day_clock::local_day());
    days duration=end_dt-begin_dt;

    cout<<"calendar days between begin & end date are:" << duration << '\n';


    //cout<<"argc: "<<argc<<endl;
    string dataPath = "..\\..\\data\\";
    string codeFile = dataPath + "zz500.csv";
    string stockTickOriginPath = dataPath + "zz500origin\\";
    string stockTickPath = dataPath + "zz500\\";
    vector<string> stockCodes = importStockCode(codeFile);
    
    OPEN_SETTINGS settings={"192.168.14.200","10010","sz1b_szqhdy","123123",30,2,0};
    //LoadIPFromCin(settings);
    TDBDefine_ResLogin loginAnswer={0};
    THANDLE hTdb = TDB_Open(&settings, &loginAnswer);
    
    if (!hTdb)
    {
        Print("TDB_Open failed:%s, program exit!\n", loginAnswer.szInfo);
        exit(0);
    }

    char* arrCode[] = {"000001.sz"};
    int arrDays[] = {20170515};

    vector<string>::iterator chWindCode;
    for (chWindCode = stockCodes.begin(); chWindCode != stockCodes.end(); chWindCode++)
    {
        string stockTickOriginFile = stockTickOriginPath + *chWindCode + ".csv";
        string stockTickFile = stockTickPath + *chWindCode + ".csv";
        ofstream origin_out(stockTickOriginFile, ios::binary);
        ofstream ofile(stockTickFile, ios::binary);
        for (day_iterator iter = begin_dt; iter != end_dt; ++iter) {

            if (iter->day_of_week() ==  boost::date_time::Saturday
                || iter->day_of_week() ==  boost::date_time::Sunday) {
                continue;
            }

            int nDate = atoi(to_iso_string(*iter).c_str());

            if (vacationSet.find(nDate) != vacationSet.end()) {
                continue;
            }

            const char *pCode = (*chWindCode).c_str();
            GetTickAB(hTdb, pCode, nDate, nDate, 0, 0, origin_out, ofile);

        }
        origin_out.flush();
        origin_out.close();
        ofile.flush();
        ofile.close();
        
    }

    cout <<"------------finished-----------------"<<endl;

    TDB_Close(hTdb);

    system("pause");

}

bool a_less_b(const TDBDefine_TickAB& a, const TDBDefine_TickAB& b)
{
    return a.nTime < b.nTime;
}

void formatTickAB(TDBDefine_TickAB& tdbTick, ofstream& ss)
{
    //stringstream ss;
    ss << tdbTick.chWindCode << ","
        << tdbTick.nDate << ","
        << tdbTick.nTime << ","
        << tdbTick.nPrice << ","
        << tdbTick.iVolume << ","
        << tdbTick.iTurover << ","
        << tdbTick.nMatchItems << ","
        << tdbTick.nInterest << ","
        << (int)tdbTick.chTradeFlag << ","
        << tdbTick.chBSFlag << ","
        << tdbTick.iAccVolume << ","
        << tdbTick.iAccTurover << ","
        << tdbTick.nHigh << ","
        << tdbTick.nLow << ","
        << tdbTick.nOpen << ","
        << tdbTick.nPreClose << ",";

    int size = ELEMENT_COUNT(tdbTick.nAskPrice);
    for(int i = 0; i < size; i++) {
        ss << tdbTick.nAskPrice[i] << ",";
    }
    for(int i = 0; i < size; i++) {
        ss << tdbTick.nBidPrice[i] << ",";
    }
    for(int i = 0; i < size; i++) {
        ss << tdbTick.nAskVolume[i] << ",";
    }
    for(int i = 0; i < size; i++) {
        ss << tdbTick.nBidVolume[i] << ",";
    }

    ss << tdbTick.nAskAvPrice << ","
        << tdbTick.nBidAvPrice << ""
        << tdbTick.iTotalAskVolume << ","
        << tdbTick.iTotalBidVolume << ","
        << tdbTick.nIndex << ","
        << tdbTick.nStocks << ","
        << tdbTick.nUps << ","
        << tdbTick.nDowns << ","
        << tdbTick.nHoldLines << "\n";

}

void formatTickABreset(TDBDefine_TickAB& tdbTick, int time, ofstream& ss)
{
    //stringstream ss;
    ss << tdbTick.chWindCode << ","
        << tdbTick.nDate << ","
        << time << ","
        << tdbTick.nPrice << ","
        << 0 << ","
        << 0 << ","
        << 0 << ","
        << tdbTick.nInterest << ","
        << (int)tdbTick.chTradeFlag << ","
        << tdbTick.chBSFlag << ","
        << tdbTick.iAccVolume << ","
        << tdbTick.iAccTurover << ","
        << tdbTick.nHigh << ","
        << tdbTick.nLow << ","
        << tdbTick.nOpen << ","
        << tdbTick.nPreClose << ",";

    int size = ELEMENT_COUNT(tdbTick.nAskPrice);
    for(int i = 0; i < size; i++) {
        ss << tdbTick.nAskPrice[i] << ",";
    }
    for(int i = 0; i < size; i++) {
        ss << tdbTick.nBidPrice[i] << ",";
    }
    for(int i = 0; i < size; i++) {
        ss << tdbTick.nAskVolume[i] << ",";
    }
    for(int i = 0; i < size; i++) {
        ss << tdbTick.nBidVolume[i] << ",";
    }

    ss << tdbTick.nAskAvPrice << ","
        << tdbTick.nBidAvPrice << ""
        << tdbTick.iTotalAskVolume << ","
        << tdbTick.iTotalBidVolume << ","
        << tdbTick.nIndex << ","
        << tdbTick.nStocks << ","
        << tdbTick.nUps << ","
        << tdbTick.nDowns << ","
        << tdbTick.nHoldLines << "\n";

}


int timeToindex(int nTime) {
    int index = 0;
    int base = 93001;
    int delta = 0;
    if(nTime > 130000000) {
        base = 130001;
        delta = 7200;
    }

    int tmp = (nTime / 1000 - base);
    index += tmp / 10000 * 3600;
    tmp %= 10000;
    index += tmp / 100 * 60;
    index += tmp % 100 + delta; 

    return index;
}

int indexTotime(int index) {
    index += 1;
    int nTime = 93000;
    if(index > 7200) {
        nTime = 130000;
        index = index - 7200;
    }
    nTime += index / 3600 * 10000;
    index %= 3600;
    nTime += index / 60 * 100;
    nTime += index % 60;

    return nTime * 1000;
}

int GetTickAB(THANDLE hTdb, const std::string& strCode, int nStartDay, int nEndDay, int nStartTime/* =0*/, int nEndTime/* = 0*/, ofstream& origin_out, ofstream& ofile)
{
    TDBDefine_ReqTick reqTick = {"",nStartDay, nEndDay, nStartTime, nEndTime};
    strncpy(reqTick.chCode, strCode.c_str(), sizeof(reqTick.chCode));

    TDBDefine_TickAB *pTick = NULL;
    int nCount;
    int nRet = TDB_GetTickAB(hTdb, &reqTick, &pTick, &nCount);

    if(nRet != 0) {
        nRet = TDB_GetTickAB(hTdb, &reqTick, &pTick, &nCount);
    }

    if(nRet != 0) {
        return -1;
    }

    Print("-- %s ---%d---receive %d records, error code:%d ------\n", strCode.c_str(), nStartDay, nCount, nRet);

    if(nCount <= 0) {
        return 0;
    }

    vector<TDBDefine_TickAB> origin_data(nCount);
    for (int i = 0; i < nCount; i++) {
        TDBDefine_TickAB &tdbTick = *(pTick + i);

        formatTickAB(tdbTick, origin_out);

        // filter ticks with wrong date or wrong price or wrong time
        tdbTick.nTime = tdbTick.nTime / 1000 * 1000;
        if(tdbTick.nDate != nStartDay || tdbTick.nPrice <= 0 || tdbTick.nTime < 93000000 || (113000000 < tdbTick.nTime && tdbTick.nTime < 130000000) || tdbTick.nTime > 150000000) {
            continue;
        }
        origin_data.push_back(tdbTick);
    }

    if(origin_data.size() == 0) {
        return 0;
    }

    // sort the ticks
    sort(origin_data.begin(), origin_data.end(), a_less_b);

    vector<TDBDefine_TickAB>::iterator curr = origin_data.begin();
    vector<TDBDefine_TickAB>::iterator next = origin_data.begin();
    int currIndex = timeToindex(curr->nTime);
    int nextIndex;

    // remove duplicates with the same HHMMSS
    while(next != origin_data.end() && next->nTime == curr->nTime) {
        next++;
    }
    
    // total number of indice
    int total = 60 * 60 * 4;
    // only one valid tick
    if(next == origin_data.end()) {
        nextIndex = total;
    }
    else {
        nextIndex = timeToindex(next->nTime);
    }

    int index = 0;
    while(true) {
        // start to complete ticks
        while(index < nextIndex) {
            
            if(index == currIndex) {
                formatTickAB(*curr, ofile);
            }
            else {
                formatTickABreset(*curr, indexTotime(index), ofile);
            }
            
            index++;
        }
        if(nextIndex == total) {
            break;
        }
        currIndex = nextIndex;
        curr = next;
        // remove duplicates with the same HHMMSS
        while (next != origin_data.end() && next->nTime == curr->nTime) {
            next++;
        }

        if (next == origin_data.end()) {
            nextIndex = total;
        } else {
            nextIndex = timeToindex(next->nTime);
        }
   
    }

    delete[] pTick;
    vector<TDBDefine_TickAB>().swap(origin_data);

    return 0;
    
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

std::string& trim(std::string &s) 
{
    if (s.empty()) 
    {
        return s;
    }
    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}

vector<string> importStockCode(string srcFile)
{
    vector<string> stockCodes;
    string line;
    ifstream infile(srcFile.c_str());
    
    if(infile) {
        getline(infile, line);
        while (getline(infile, line)) {
            string tmp = trim(line);
            stockCodes.push_back(tmp);
        }
    }
    
    infile.close();

    return stockCodes;
}
