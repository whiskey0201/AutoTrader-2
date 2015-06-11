#include "stdafx.h"
#include "config.h"
#include "DBWrapper.h"
#include "TechVec.h"
#include "k3UpThroughK5.h"
#include "Order.h"

#include <sstream>
#include <assert.h>


k3UpThroughK5::k3UpThroughK5()
:m_curOrder(new Order())
{
}

k3UpThroughK5::~k3UpThroughK5()
{
}

double k3UpThroughK5::calculateK(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds) const
{
	//datetime to timestamp
	double totalExchangePrice = current.TurnOver();
	long long totalVolume = current.Volume();

	long long leftedge = current.toTimeStamp() - seconds * 2;
	for (auto it = data.begin(); it != data.end(); it++)
	{
		if (it->toTimeStamp() > leftedge){
			totalExchangePrice += it->TurnOver();
			totalVolume += it->Volume();
		}
		else{
			break;
		}
	}

	//assert(totalVolume != 0);
	//assert(totalExchangePrice >= 0.0);

	return totalExchangePrice / totalVolume;
}


bool k3UpThroughK5::TryInvoke(const std::list<CThostFtdcDepthMDFieldWrapper>& data, CThostFtdcDepthMDFieldWrapper& info)
{
	TickType direction = TickType::Commom;
	const size_t breakthrough_confirm_duration = 100; //50ms
	k3UpThroughK5TechVec* curPtr = new k3UpThroughK5TechVec(info.UUID(), info.InstrumentId(), info.Time(), info.LastPrice());
	bool orderSingal = false;
	double k3 = calculateK(data, info, 3 * 60);
	double k5 = calculateK(data, info, 5 * 60);
	curPtr->setK3m(k3);
	curPtr->setK5m(k5);

	if (!data.empty()){
		if (curPtr->IsUpThough()){ // up
			if (!data.empty() && data.size() > 500){
				std::list<CThostFtdcDepthMDFieldWrapper>::const_iterator stoper = data.begin();
				std::advance(stoper, breakthrough_confirm_duration);
				for (auto it = data.begin(); it != stoper; it++){
					StrategyTechVec* prePtr = it->GetTechVec();
					// if prePtr == NULL, mean it's recovered from db, so that md is not continuous. so it's should not be singal point.
					if (prePtr == NULL || !prePtr->IsUpThough())
					{ 
						// not special point
						orderSingal = false;
						break;
					}
					orderSingal = true;
				}
				//special point
				if (orderSingal){
					//m_curOrder->SetInstrumentId(info.InstrumentId());
					//m_curOrder->SetRefExchangePrice(info.LastPrice());
					//m_curOrder->SetExchangeDirection(ExchangeDirection::Buy);
					m_curOrder = new Order(info.InstrumentId(), info.LastPrice(), ExchangeDirection::Buy, Order::FAK);
					curPtr->SetTickType(TickType::BuyPoint);
				}
			}
		}
		else{ // down
			if (!data.empty() && data.size() > 500){
				std::list<CThostFtdcDepthMDFieldWrapper>::const_iterator stoper = data.begin();
				std::advance(stoper, breakthrough_confirm_duration);
				for (auto it = data.begin(); it != stoper; it++){
					StrategyTechVec* prePtr = it->GetTechVec();
					if (prePtr == NULL || prePtr->IsUpThough())
					{
						// not special point
						orderSingal = false;
						break;
					}
					orderSingal = true;
				}
				if (orderSingal){
					//special point
					//m_curOrder->SetInstrumentId(info.InstrumentId());
					//m_curOrder->SetRefExchangePrice(info.LastPrice());
					//m_curOrder->SetExchangeDirection(ExchangeDirection::Sell);
					m_curOrder = new Order(info.InstrumentId(), info.LastPrice(), ExchangeDirection::Sell, Order::FAK);
					curPtr->SetTickType(TickType::SellPoint);
				}
			}
		}
	}

	//info.SetTechVec((StrategyTechVec*)curPtr);
	info.m_techvec = curPtr;
	return orderSingal;
}

Order k3UpThroughK5::generateOrder(){
	assert(m_curOrder);
	return *m_curOrder;
}

bool k3UpThroughK5TechVec::IsTableCreated = false;

k3UpThroughK5TechVec::k3UpThroughK5TechVec(long long uuid, const std::string& instrumentID, const std::string& time, double lastprice)
: m_id(uuid)
, m_ticktype(TickType::Commom)
, m_lastprice(lastprice)
{
	strcpy_s(m_time, time.c_str());
	strcpy_s(m_instrumentId, instrumentID.c_str());
}

bool k3UpThroughK5TechVec::IsUpThough() const {
	return m_k3m > m_k5m;
}

int k3UpThroughK5TechVec::CreateTableIfNotExists(const std::string& dbname, const std::string& tableName)
{
	if (k3UpThroughK5TechVec::IsTableCreated == true){
		return 0;
	}
	else
	{
		k3UpThroughK5TechVec::IsTableCreated = true;
		const char* sqltempl = "CREATE TABLE IF NOT EXISTS `%s`.`%s` (\
			`id` INT NOT NULL AUTO_INCREMENT, \
			`uuid` BIGINT NOT NULL, \
			`k5m` Double(20,5) NULL, \
			`k3m` Double(20,5) NULL, \
			`Ticktype` int NULL, \
			`Time` VARCHAR(64) NULL, \
			`LastPrice` Double NULL, \
			PRIMARY KEY(`id`));";
		char sqlbuf[2046];
		sprintf_s(sqlbuf, sqltempl, dbname.c_str(), tableName.c_str());
		DBWrapper db;
		return db.ExecuteNoResult(sqlbuf);
	}
}

void k3UpThroughK5TechVec::serializeToDB(DBWrapper& db, const std::string& mark)
{
	std::string tableName(m_instrumentId);

	tableName += "_k3UpThroughK5_";
	tableName += mark;

	k3UpThroughK5TechVec::CreateTableIfNotExists(Config::Instance()->DBName(), tableName);

	std::stringstream sql;
	sql.precision(12);
	sql << "INSERT INTO `" << tableName << "` (`";
	sql << "uuid" << "`,`";
	sql << "k5m" << "`,`";
	sql << "k3m" << "`,`";
	sql << "Ticktype" << "`,`";
	sql << "Time" << "`,`";
	sql << "LastPrice" << "`";
	sql << ") VALUES(";
	sql << m_id << ", ";
	sql << m_k5m << ", ";
	sql << m_k3m << ", ";
	sql << (int)m_ticktype << ", \"";
	sql << m_time << "\", ";
	sql << m_lastprice << ")";

	//std::cerr << sql.str() << std::endl;
	db.ExecuteNoResult(sql.str());
}