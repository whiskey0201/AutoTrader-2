#include "stdafx.h"
#include "DBWrapper.h"
#include <sstream>
#include <iostream>
#include <time.h>
#include "config.h"
#include "ThostFtdcDepthMDFieldWrapper.h"

bool CThostFtdcDepthMDFieldWrapper::firstlanuch = true;

namespace {
	/*
	Usage: "20150430" to 1430382950
	*/
	long long DateTimeToTimestamp(const char* date_src, const char* time_src){
		char szYear[5], szMonth[3], szDay[3], szHour[3], szMin[3], szSec[3];

		szYear[0] = *date_src++;
		szYear[1] = *date_src++;
		szYear[2] = *date_src++;
		szYear[3] = *date_src++;
		szYear[4] = 0x0;

		szMonth[0] = *date_src++;
		szMonth[1] = *date_src++;
		szMonth[2] = 0x0;

		szDay[0] = *date_src++;
		szDay[1] = *date_src++;
		szDay[2] = 0x0;

		szHour[0] = *time_src++;
		szHour[1] = *time_src++;
		szHour[2] = 0x0;
		time_src++;

		szMin[0] = *time_src++;
		szMin[1] = *time_src++;
		szMin[2] = 0x0;
		time_src++;

		szSec[0] = *time_src++;
		szSec[1] = *time_src++;
		szSec[2] = 0x0; 

		tm tmObj;

		tmObj.tm_year = atoi(szYear) - 1900;
		tmObj.tm_mon = atoi(szMonth) - 1;
		tmObj.tm_mday = atoi(szDay);
		tmObj.tm_hour = atoi(szHour);
		tmObj.tm_min = atoi(szMin);
		tmObj.tm_sec = atoi(szSec);
		tmObj.tm_isdst = -1;

		return mktime(&tmObj);
	}

	double StringtoDouble(const std::string& str)
	{
		std::stringstream ss;
		double ret;
		ss << str;
		ss >> ret;
		return ret;
	}

	int StringtoInt(const std::string& str)
	{
		std::stringstream ss;
		int ret;
		ss << str;
		ss >> ret;
		return ret;
	}

	long long Stringtolong(const std::string& str)
	{
		std::stringstream ss;
		long long ret;
		ss << str;
		ss >> ret;
		return ret;
	}

	std::string ConvertTime(const std::string& src){
		//from "2015-04-30 00:00:00" to "20150430"
		//assert(src.length() > 10);
		char ret[9];
		const char* _src = src.c_str();

		//year
		ret[0] = *_src++;
		ret[1] = *_src++;
		ret[2] = *_src++;
		ret[3] = *_src++;
		_src++;//skip '-'

		//month
		ret[4] = *_src++;
		ret[5] = *_src++;
		_src++;

		//day
		ret[6] = *_src++;
		ret[7] = *_src++;
		ret[8] = 0x0;

		return std::string(ret);
	}
}

CThostFtdcDepthMDFieldWrapper::CThostFtdcDepthMDFieldWrapper(CThostFtdcDepthMarketDataField* p):
	m_k5m(0.0),
	m_k3m(0.0),
	recoveryData(false)
{
	for (int i =0; i < 5; i++){
		m_ticktype[i] = TickType::Commom;
	}
	assert(p);
	memcpy(&m_MdData, p, sizeof(CThostFtdcDepthMarketDataField));
}

CThostFtdcDepthMDFieldWrapper::~CThostFtdcDepthMDFieldWrapper()
{
}

long long CThostFtdcDepthMDFieldWrapper::toTimeStamp() const{
	long long ret = DateTimeToTimestamp(m_MdData.TradingDay, m_MdData.UpdateTime) * 2 + m_MdData.UpdateMillisec / 500;
	return ret;
}

void CThostFtdcDepthMDFieldWrapper::serializeToDB(DBWrapper& db) const {
	// if this item is recovered from db, so that we don't need serialize it to db again. 
	if (recoveryData)
		return;

	std::string tableName(m_MdData.InstrumentID);

	DBUtils::CreateTickTableIfNotExists(Config::Instance()->DBName(), tableName);

	std::stringstream sql;
	sql << "INSERT INTO `" << tableName << "` (`";
	sql << "Date" << "`,`";
	sql << "InstrumentID" << "`,`";
	sql << "ExchangeID" << "`,`";
	sql << "ExchangeInstID" << "`,`";
	sql << "LastPrice" << "`,`";
	sql << "PreSettlementPrice" << "`,`";
	sql << "PreClosePrice" << "`,`";
	sql << "PreOpenInterest" << "`,`";
	sql << "OpenPrice" << "`,`";
	sql << "HighestPrice" << "`,`";
	sql << "LowestPrice" << "`,`";
	sql << "Volume" << "`,`";
	sql << "Turnover" << "`,`";
	sql << "OpenInterest" << "`,`";
	sql << "ClosePrice" << "`,`";
	sql << "SettlementPrice" << "`,`";
	sql << "UpperLimitPrice" << "`,`";
	sql << "LowerLimitPrice" << "`,`";
	sql << "PreDelta" << "`,`";
	sql << "CurrDelta"<< "`,`";
	sql << "UpdateTime"<< "`,`";
	sql << "UpdateMillisec" << "`,`";
	sql << "BidPrice1" << "`,`";
	sql << "BidVolume1" << "`,`";
	sql << "AskPrice1" << "`,`";
	sql << "AskVolume1" << "`,`";
	sql << "BidPrice2" << "`,`";
	sql << "BidVolume2" << "`,`";
	sql << "AskPrice2" << "`,`";
	sql << "AskVolume2" << "`,`";
	sql << "BidPrice3" << "`,`";
	sql << "BidVolume3" << "`,`";
	sql << "AskPrice3" << "`,`";
	sql << "AskVolume3" << "`,`";
	sql << "BidPrice4" << "`,`";
	sql << "BidVolume4" << "`,`";
	sql << "AskPrice4" << "`,`";
	sql << "AskVolume4" << "`,`";
	sql << "BidPrice5" << "`,`";
	sql << "BidVolume5" << "`,`";
	sql << "AskPrice5" << "`,`";
	sql << "AskVolume5" << "`,`";
	sql << "AveragePrice" << "`,`";
	sql << "ActionDay" << "`,`";
	sql << "k3m" << "`,`";
	sql << "k5m" << "`,`";
	sql << "Strategy1" << "`,`";
	sql << "Strategy2" << "`,`";
	sql << "Strategy3" << "`,`";
	sql << "Strategy4" << "`,`";
	sql << "Strategy5" << "`";
	sql << ") VALUES(\"";
	sql << m_MdData.TradingDay << "\", \"";//m_MdData.TradingDay
	sql <<  m_MdData.InstrumentID << "\", \"";
	sql <<  m_MdData.ExchangeID << "\", \"";
	sql <<  m_MdData.ExchangeInstID << "\", ";
	sql <<  m_MdData.LastPrice << ", ";
	sql <<  m_MdData.PreSettlementPrice << ", ";
	sql <<  m_MdData.PreClosePrice << ", ";
	sql <<  m_MdData.PreOpenInterest << ", ";
	sql <<  m_MdData.OpenPrice << ", ";
	sql <<  m_MdData.HighestPrice << ", ";
	sql <<  m_MdData.LowestPrice << ", ";
	sql <<  m_MdData.Volume << ", ";
	sql <<  m_MdData.Turnover << ", ";
	sql <<  m_MdData.OpenInterest << ", ";
	sql <<  m_MdData.ClosePrice << ", ";
	sql <<  m_MdData.SettlementPrice << ", ";
	sql <<  m_MdData.UpperLimitPrice << ", ";
	sql <<  m_MdData.LowerLimitPrice << ", ";
	sql <<  m_MdData.PreDelta << ", ";
	sql <<  m_MdData.CurrDelta << ", \"";
	sql <<  m_MdData.UpdateTime << "\", ";  // m_MdData.UpdateTime
	sql <<  m_MdData.UpdateMillisec << ", ";
	sql <<  m_MdData.BidPrice1 << ", ";
	sql <<  m_MdData.BidVolume1 << ", ";
	sql <<  m_MdData.AskPrice1 << ", ";
	sql <<  m_MdData.AskVolume1 << ", ";
	sql <<  m_MdData.BidPrice2 << ", ";
	sql <<  m_MdData.BidVolume2 << ", ";
	sql <<  m_MdData.AskPrice2 << ", ";
	sql <<  m_MdData.AskVolume2 << ", ";
	sql <<  m_MdData.BidPrice3 << ", ";
	sql <<  m_MdData.BidVolume3 << ", ";
	sql <<  m_MdData.AskPrice3 << ", ";
	sql <<  m_MdData.AskVolume3 << ", ";
	sql <<  m_MdData.BidPrice4 << ", ";
	sql <<  m_MdData.BidVolume4 << ", ";
	sql <<  m_MdData.AskPrice4 << ", ";
	sql <<  m_MdData.AskVolume4 << ", ";
	sql <<  m_MdData.BidPrice5 << ", ";
	sql <<  m_MdData.BidVolume5 << ", ";
	sql <<  m_MdData.AskPrice5 << ", ";
	sql <<  m_MdData.AskVolume5 << ", ";
	sql <<  m_MdData.AveragePrice << ", \"";
	sql <<  m_MdData.ActionDay << "\", "; // m_MdData.ActionDay
	sql <<  m_k3m << ", ";
	sql << m_k5m << ", ";
	sql << (int)m_ticktype[0] << ", ";
	sql << (int)m_ticktype[1] << ", ";
	sql << (int)m_ticktype[2] << ", ";
	sql << (int)m_ticktype[3] << ", ";
	sql << (int)m_ticktype[4] << ")";
	//"INSERT INTO `test` (`name`) VALUES (1234) 
	std::cerr << sql.str() << std::endl;
	db.ExecuteNoResult(sql.str());
}

CThostFtdcDepthMDFieldWrapper CThostFtdcDepthMDFieldWrapper::RecoverFromDB(const CThostFtdcDepthMDFieldDBStruct& vec)
{
	CThostFtdcDepthMarketDataField mdStuct;
	memset(&mdStuct, 0, sizeof(mdStuct));
	strcpy_s(mdStuct.TradingDay, ConvertTime(vec[1]).c_str());// todo: from "2015-04-30 00:00:00" to "20150430"
	strcpy_s(mdStuct.InstrumentID, vec[2].c_str());
	strcpy_s(mdStuct.ExchangeID, vec[3].c_str());
	strcpy_s(mdStuct.ExchangeInstID, vec[4].c_str());
	mdStuct.LastPrice = StringtoDouble(vec[5]);
	mdStuct.PreSettlementPrice = StringtoDouble(vec[6]);
	mdStuct.PreClosePrice = StringtoDouble(vec[7]);
	mdStuct.PreOpenInterest = StringtoDouble(vec[8]);
	mdStuct.OpenPrice = StringtoDouble(vec[9]);
	mdStuct.HighestPrice = StringtoDouble(vec[10]);
	mdStuct.LowestPrice = StringtoDouble(vec[11]);
	mdStuct.Volume = Stringtolong(vec[12]);
	mdStuct.Turnover = StringtoDouble(vec[13]);
	mdStuct.OpenInterest = StringtoDouble(vec[14]);
	mdStuct.ClosePrice = StringtoDouble(vec[15]);
	mdStuct.SettlementPrice = StringtoDouble(vec[16]);

	mdStuct.UpperLimitPrice = StringtoDouble(vec[17]);
	mdStuct.LowerLimitPrice = StringtoDouble(vec[18]);
	mdStuct.PreDelta = StringtoDouble(vec[19]);
	mdStuct.CurrDelta = StringtoDouble(vec[20]);
	strcpy_s(mdStuct.UpdateTime, vec[21].c_str());// todo: from "00:00:00" to "150430" // enhance :specify the size of str
	mdStuct.UpdateMillisec = StringtoInt(vec[22]);
	mdStuct.BidPrice1 = StringtoDouble(vec[23]);
	mdStuct.BidVolume1 = Stringtolong(vec[24]);
	mdStuct.AskPrice1 = StringtoDouble(vec[25]);
	mdStuct.AskVolume1 = Stringtolong(vec[26]);
	mdStuct.BidPrice2 = StringtoDouble(vec[27]);
	mdStuct.BidVolume2 = Stringtolong(vec[28]);
	mdStuct.AskPrice2 = StringtoDouble(vec[29]);
	mdStuct.AskVolume2 = Stringtolong(vec[30]);
	mdStuct.BidPrice3 = StringtoDouble(vec[31]);
	mdStuct.BidVolume3 = Stringtolong(vec[32]);
	mdStuct.AskPrice3 = StringtoDouble(vec[33]);
	mdStuct.AskVolume3 = Stringtolong(vec[34]);
	mdStuct.BidPrice4 = StringtoDouble(vec[35]);
	mdStuct.BidVolume4 = Stringtolong(vec[36]);
	mdStuct.AskPrice4 = StringtoDouble(vec[37]);
	mdStuct.AskVolume4 = Stringtolong(vec[38]);
	mdStuct.BidPrice5 = StringtoDouble(vec[39]);
	mdStuct.BidVolume5 = Stringtolong(vec[40]);
	mdStuct.AskPrice5 = StringtoDouble(vec[41]);
	mdStuct.AskVolume5 = Stringtolong(vec[42]);
	mdStuct.AveragePrice = StringtoDouble(vec[43]);
	strcpy_s(mdStuct.ActionDay, ConvertTime(vec[44]).c_str());
	CThostFtdcDepthMDFieldWrapper mdObject(&mdStuct);
	mdObject.setK3(StringtoDouble(vec[45]));
	mdObject.setK5(StringtoDouble(vec[46]));
	mdObject.SetTickType((TickType)StringtoInt(vec[47]), 0);
	mdObject.SetTickType((TickType)StringtoInt(vec[48]), 1);
	mdObject.SetTickType((TickType)StringtoInt(vec[49]), 2);
	mdObject.SetTickType((TickType)StringtoInt(vec[50]), 3);
	mdObject.SetTickType((TickType)StringtoInt(vec[51]), 4);
	mdObject.recoveryData = true;
	return mdObject;
}


//
//long long CThostFtdcDepthMDFieldWrapper::toTimeStamp() const{
//	const char* pDate = m_MdData.TradingDay;
//
//	char szYear[5], szMonth[3], szDay[3], szHour[3], szMin[3], szSec[3];
//
//	szYear[0] = *pDate++;
//	szYear[1] = *pDate++;
//	szYear[2] = *pDate++;
//	szYear[3] = *pDate++;
//	szYear[4] = 0x0;
//
//	szMonth[0] = *pDate++;
//	szMonth[1] = *pDate++;
//	szMonth[2] = 0x0;
//
//	szDay[0] = *pDate++;
//	szDay[1] = *pDate++;
//	szDay[2] = 0x0;
//
//	const char* pTime = m_MdData.UpdateTime;
//
//	szHour[0] = *pTime++;
//	szHour[1] = *pTime++;
//	szHour[2] = 0x0;
//	pTime++;
//
//	szMin[0] = *pTime++;
//	szMin[1] = *pTime++;
//	szMin[2] = 0x0;
//	pTime++;
//
//	szSec[0] = *pTime++;
//	szSec[1] = *pTime++;
//	szSec[2] = 0x0;
//
//	tm tmObj;
//
//	tmObj.tm_year = atoi(szYear) - 1900;
//	tmObj.tm_mon = atoi(szMonth) - 1;
//	tmObj.tm_mday = atoi(szDay);
//	tmObj.tm_hour = atoi(szHour);
//	tmObj.tm_min = atoi(szMin);
//	tmObj.tm_sec = atoi(szSec);
//	tmObj.tm_isdst = -1;
//
//	long long ret = mktime(&tmObj) * 2 + m_MdData.UpdateMillisec / 500;
//	return ret;
//}