#ifndef BOLL_TECH_H
#define BOLL_TECH_H

#include "stdafx.h"

struct STRATEGY_API BOLLTech{
	BOLLTech(){};
	BOLLTech(double ma, double var, double curVal = 0)
	: mMAValue(ma)
	, mVariance(var)
	, mCurValue(curVal)
	{};
	~BOLLTech(){};

	double BollWidth() const {
		return mVariance * m_boll_k * 2;
	}

	double BollUp() const {
		return mMAValue + m_boll_k * mVariance;
	}

	double BollDown() const{
		return mMAValue - m_boll_k * mVariance;
	}

	double BollMid() const{
		return mMAValue;
	}

	double Variance() const {
		return mVariance;
	}
	bool IsTriggerPoint() const;

	double mMAValue;;
	double mVariance;
	double mCurValue;
private:
	static const unsigned int m_boll_k = 2;
};

#endif