#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include "VoltageTable.hpp"

using namespace std;

VoltageTable::VoltageTable(const vector<double> &vt, double nominalVol) :
	m_voltageTable(vt),
	m_nominalVol(nominalVol)
{}

inline double VoltageTable::VoltageCurrentRelation(double targetVol, double nominalCur) const {
	return nominalCur * (targetVol/m_nominalVol) * (targetVol/m_nominalVol);
}

inline double VoltageTable::VoltageFrequencyRelation(double targetVol, double nominalLen) const {
	return nominalLen * (m_nominalVol/targetVol);
}

size_t TaskVoltageTable::getVoltageLevel(double v) const {
	vector<double>::const_iterator it = find(m_voltageTable.begin(), m_voltageTable.end(), v);
	if (it == m_voltageTable.end()) {
		cerr << "Can not find the voltage: " << v << " in the voltage table! Abort!" << endl;
		exit(66);
	}
	return (it - m_voltageTable.begin());
}

double TaskVoltageTable::getCurrent(double voltage) const {
	size_t level = getVoltageLevel(voltage);
	return getCurrent(level);
}

int TaskVoltageTable::getScaledCeilLength(double voltage, int scale) const {
	size_t level = getVoltageLevel(voltage);
	return getScaledCeilLength(level, scale);
}

TaskVoltageTable::TaskVoltageTable(const VoltageTable &vt, double nomCur, double nomLen) :
	m_voltageTable(vt.getVoltageTable()),
	m_nominalCurrent(nomCur),
	m_nominalLength(nomLen),
	m_nominalVoltage(vt.GetNominalVoltage())
{
	m_VCLTable = vector<VolCurLenEntry>(m_voltageTable.size());
	for (size_t i = 0; i < m_VCLTable.size(); ++i) {
		m_VCLTable[i].v = m_voltageTable[i];
		m_VCLTable[i].c = vt.VoltageCurrentRelation(m_VCLTable[i].v, m_nominalCurrent);
		m_VCLTable[i].l = vt.VoltageFrequencyRelation(m_VCLTable[i].v, m_nominalLength);
	}
}

void BuildTaskVoltageTableVectorFromFile(const char *filename, vector<TaskVoltageTable> &vec_tvt, const VoltageTable &vt) {
	// Clear the vector first
	vec_tvt.clear();

	ifstream ifs(filename);
	double len(0.0);
	double power(0.0);
	double energy(0.0);
	double nominalCurrent(0.0);
	while ((ifs >> len >> power >> energy).good()) {
		nominalCurrent = power / vt.GetNominalVoltage();
		vec_tvt.push_back(TaskVoltageTable(vt, nominalCurrent, len));
	}
	ifs.close();
	return;
}

VoltageTable GenerateSingleVoltageTable(double vol, double nomvol) {
	return VoltageTable(vector<double>(vol), nomvol);
}

const VoltageTable syntheticCPUVoltageTable(vector<double>(syntheticVoltageTable, syntheticVoltageTable+syntheticVoltageLevel), 1.0);
const TaskVoltageTable idleTaskVoltageTable(syntheticCPUVoltageTable, 0.0, 0.0);
