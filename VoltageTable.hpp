#ifndef _VOLTAGE_TABLE_
#define _VOLTAGE_TABLE_

#include <cmath>
#include <vector>

using std::vector;

// The base class for a voltage table corresponding to a processor model
class VoltageTable {
public:
	VoltageTable(const vector<double> &, double nominalVol);
	virtual ~VoltageTable() {}

	virtual double VoltageCurrentRelation(double targetVol, double nominalCur) const;
	virtual double VoltageFrequencyRelation(double targetVol, double nominalLen) const;

	const vector<double> &getVoltageTable() const { return m_voltageTable; }
	double GetNominalVoltage() const { return m_nominalVol; }

private:
	vector<double> m_voltageTable;
	double m_nominalVol;
};

class TaskVoltageTable {
public:
	TaskVoltageTable(const VoltageTable &vt, double nomCur, double nomLen);
	double getVoltage(size_t level) const { return m_VCLTable[level].v; }
	double getNominalVoltage() const { return m_nominalVoltage; }
	double getCurrent(size_t level) const { return m_VCLTable[level].c; }
	double getCurrent(double voltage) const;
	int getScaledCeilLength(size_t level, int scale) const { return ceil(scale*m_VCLTable[level].l); }
	int getScaledCeilLength(double voltage, int scale) const;

private:
	size_t getVoltageLevel(double v) const;

private:
	struct VolCurLenEntry {
		double v;
		double c;
		double l;
	};
	vector<VolCurLenEntry> m_VCLTable;
	vector<double> m_voltageTable;
	double m_nominalCurrent;
	double m_nominalLength;
	double m_nominalVoltage;
};

const size_t syntheticVoltageLevel = 5;
const double syntheticVoltageTable[syntheticVoltageLevel] = {0.8, 0.9, 1.0, 1.1, 1.2};

extern const VoltageTable syntheticCPUVoltageTable;
extern const TaskVoltageTable idleTaskVoltageTable;
#endif
