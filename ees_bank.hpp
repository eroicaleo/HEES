#ifndef _EES_BANK_
#define _EES_BANK_

class ees_bank {
public:
	ees_bank() {}
	virtual ~ees_bank();
	virtual double EESBankGetCacc() const = 0;
	virtual double EESBankGetQacc() const = 0;
	virtual double EESBankGetRacc() const = 0;
	virtual double EESBankGetVoc() const = 0;
	virtual double EESBankGetEnergy() const = 0;
	virtual bool EESBankOperating(double, double, double) = 0;
	virtual	void EESBankCharge(double Iin, double Tdur, double &Vs, double &Qacc) = 0;
};
#endif
