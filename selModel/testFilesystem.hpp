#ifndef _TEST_FILESYSMTE_HPP_
#define _TEST_FILESYSMTE_HPP_

#include <map>

#include "../nnet/nnet.hpp"

typedef std::map<double, nnetmodel> Level2Map;

void buildNnetMap(std::map<double, Level2Map> &nnetMap);

const nnetmodel &recallNnetMap(const std::map<double, Level2Map> &nnetMap, double bankCharge, double solarPower);

#endif
