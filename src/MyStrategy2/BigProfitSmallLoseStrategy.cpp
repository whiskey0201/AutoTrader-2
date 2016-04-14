#include "BigProfitSmallLoseStrategy.h"
#include "TickWrapper.h"

BigProfitSmallLoseStrategy::BigProfitSmallLoseStrategy(double max_profit_ratio, double max_lose_ratio, int threshold_volume)
	: m_curOrder(new Order())
	, _max_profit_ratio(max_profit_ratio)
	, _max_lose_ratio(max_lose_ratio)
	, _threshold_volume(threshold_volume)
{
}

BigProfitSmallLoseStrategy::~BigProfitSmallLoseStrategy()
{
	delete m_curOrder;
	m_curOrder = nullptr;
}

bool BigProfitSmallLoseStrategy::tryInvoke(const std::vector<TickWrapper>& data, TickWrapper& info){
	const size_t breakthrough_confirm_duration = 60; //tick

	if (info.Volume() > _threshold_volume){
		m_curOrder->SetInstrumentId(info.InstrumentId());
		m_curOrder->SetOrderType(Order::LimitPriceFOKOrder);

		auto stopIter = data.rbegin();
		if (data.size() > breakthrough_confirm_duration)
			std::advance(stopIter, breakthrough_confirm_duration);
		else
			return false;
 
		int volume_total = 0;
		long long totalmulti = 0;
		auto iter = data.rbegin(); 
		iter++;
		for (; iter != stopIter; iter++){
			totalmulti += (iter->LastPrice() * iter->Volume());
			volume_total += iter->Volume();
		}
		double breakReferPrice = totalmulti / volume_total;

		if (info.LastPrice() > breakReferPrice){
			m_curOrder->SetRefExchangePrice(info.LastPrice() + 1);
			m_curOrder->SetExchangeDirection(THOST_FTDC_D_Buy);
		}else{
			m_curOrder->SetRefExchangePrice(info.LastPrice() - 1);
			m_curOrder->SetExchangeDirection(THOST_FTDC_D_Sell);
		}

		return true;
	}
}

bool BigProfitSmallLoseStrategy::tryInvoke(const std::vector<TickWrapper>& tickdata, const std::vector<KData>& data, const std::vector<TickWrapper>& curmindata, TickWrapper& info){
	return false;
}

Order BigProfitSmallLoseStrategy::GetCurOrder() const{
	return *m_curOrder;
}