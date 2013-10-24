#ifndef _EES_BANK_
#define _EES_BANK_

#include "DCCon_out.hpp"

class dcconvertOUT;

class ees_bank {
public:
	enum EESBankDischargingTypeFlag {InvalidForDischarging, ValidForDischarging};
public:
	ees_bank() {}
	virtual ~ees_bank();
	virtual double EESBankGetCacc() const = 0;
	virtual double EESBankGetQacc() const = 0;
	virtual double EESBankGetRacc() const = 0;
	virtual double EESBankGetVoc() const = 0;
	virtual double EESBankGetEnergy() const = 0;
	virtual bool EESBankOperating(double, double, double) = 0;
	virtual bool EESBankReconfiguration(double, double, dcconvertOUT *) = 0;
	virtual	void EESBankCharge(double Iin, double Vin, double Tdur, double &Vs, double &Qacc) = 0;
	virtual void EESBankSetDischargingFlag(EESBankDischargingTypeFlag flag);
	virtual bool EESBankIfInvalidForDischarging() const;
private:
	EESBankDischargingTypeFlag dischargingFlag;
};

#endif
