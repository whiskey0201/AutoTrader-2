#ifndef INSTRUMENT_MGR_H 
#define INSTRUMENT_MGR_H

#include "ThostFtdcUserApiDataType.h"
#include "ThostFtdcUserApiStruct.h"
#include <map>
#include "CommonUtils.h"

#include "stdafx.h"
#include "InstrumentInfoMgr.h"

#define THOST_FTDC_EIDT_WRONG 0

namespace Instrument{

	struct STRATEGY_API Information
	{
		Information(){};
		Information(const CThostFtdcInstrumentField& ins_field)
		{
			memcpy(&InstruField, &ins_field, sizeof(CThostFtdcInstrumentField));
			memset(&MgrRateField, 0, sizeof(CThostFtdcInstrumentMarginRateField));
			memset(&ComRateField, 0, sizeof(CThostFtdcInstrumentCommissionRateField));
		};

		Information& operator = (const Information& info){
			memcpy(&InstruField, &info.InstruField, sizeof(CThostFtdcInstrumentField));
			memcpy(&MgrRateField, &info.MgrRateField, sizeof(CThostFtdcInstrumentMarginRateField));
			memcpy(&ComRateField, &info.ComRateField, sizeof(CThostFtdcInstrumentCommissionRateField));
			return *this;
		}

		CThostFtdcInstrumentField					InstruField;
		CThostFtdcInstrumentMarginRateField			MgrRateField;
		CThostFtdcInstrumentCommissionRateField		ComRateField;
	};

	class STRATEGY_API InformationMgr
	{
	public:
		typedef std::map< std::string, Information > InstrumentDict;
		typedef InstrumentDict::iterator             InstrumentDictIterator;
		typedef InstrumentDict::const_iterator       InstrumentDictConstIterator;
		InformationMgr();
		~InformationMgr();

		InformationMgr(const InformationMgr& mgr) = delete;
		InformationMgr& operator = (const InformationMgr& mgr) = delete;

	public:
		const Information& Get(const std::string& instrumentID) const;
		int VolumeMultiple(const std::string& instrumentID) const;
		double GetMargin(const std::string& instrumentID, int trade_volume, double trade_price, TThostFtdcDirectionType direction);
		double GetCommission(const std::string& instrumentID, int trade_volume, double trade_price, TThostFtdcOffsetFlagType flag);
		void Add(const std::string& instrumentID, const Information& info);
		bool SetMarginRate(const std::string& instrumentID, const CThostFtdcInstrumentMarginRateField& mgrRate);
		bool SetCommissionRate(const std::string& instrumentID, const CThostFtdcInstrumentCommissionRateField& comRate);
		std::string AllInstruments() const;
		InstrumentDictIterator begin() { return m_InfoDict.begin(); } ;
		InstrumentDictConstIterator end() const { return m_InfoDict.end(); };
		void serialize() const;
		TThostFtdcExchangeIDTypeType GetExchangeID(const std::string& instrumentID) const;

	private:
		void unserialize();
	private:
		//the pair-first is instrument Name
		std::map< std::string, Information > m_InfoDict;
		std::vector< std::string >           m_InfoVec;
		bool m_isSetup;
	};

	//InformationMgr& GetManager();
	
};

extern STRATEGY_API Instrument::InformationMgr& InstrumentManager;

#endif
	