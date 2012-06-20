#include "ees_bank.hpp"

ees_bank::~ees_bank() {
}

void ees_bank::EESBankSetDischargingFlag(ees_bank::EESBankDischargingTypeFlag flag) {
	dischargingFlag = flag;
}

bool ees_bank::EESBankIfInvalidForDischarging() const {
	return (dischargingFlag == InvalidForDischarging);
}
