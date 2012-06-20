/*
 * -----------------------------------------------------------------
 * $Revision: 1.2 $
 * $Date: 2008/12/17 19:38:48 $
 * -----------------------------------------------------------------
 * Programmer(s): Radu Serban @ LLNL
 * -----------------------------------------------------------------
 * Example (serial):
 *
 * This example solves a nonlinear system from.
 *
 * Source: "Handbook of Test Problems in Local and Global Optimization",
 *             C.A. Floudas, P.M. Pardalos et al.
 *             Kluwer Academic Publishers, 1999.
 * Test problem 4 from Section 14.1, Chapter 14: Ferraris and Tronconi
 * 
 * This problem involves a blend of trigonometric and exponential terms.
 *    0.5 sin(x1 x2) - 0.25 x2/pi - 0.5 x1 = 0
 *    (1-0.25/pi) ( exp(2 x1)-e ) + e x2 / pi - 2 e x1 = 0
 * such that
 *    0.25 <= x1 <=1.0
 *    1.5 <= x2 <= 2 pi
 * 
 * The treatment of the bound constraints on x1 and x2 is done using
 * the additional variables
 *    l1 = x1 - x1_min >= 0
 *    L1 = x1 - x1_max <= 0
 *    l2 = x2 - x2_min >= 0
 *    L2 = x2 - x2_max >= 0
 * 
 * and using the constraint feature in KINSOL to impose
 *    l1 >= 0    l2 >= 0
 *    L1 <= 0    L2 <= 0
 * 
 * The Ferraris-Tronconi test problem has two known solutions.
 * The nonlinear system is solved by KINSOL using different 
 * combinations of globalization and Jacobian update strategies 
 * and with different initial guesses (leading to one or the other
 * of the known solutions).
 *
 *
 * Constraints are imposed to make all components of the solution
 * positive.
 * -----------------------------------------------------------------
 */


#ifndef _DC_SUPERCAPACITOR_SOLVER_ 
#define _DC_SUPERCAPACITOR_SOLVER_

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <kinsol/kinsol.h>
#include <kinsol/kinsol_dense.h>
#include <kinsol/kinsol_impl.h>
#include <nvector/nvector_serial.h>
#include <sundials/sundials_types.h>
#include <sundials/sundials_math.h>

#ifdef __cplusplus
}
#endif

/* Problem Constants */

#define NVAR   1
#define NEQ    3*NVAR

#define FTOL   RCONST(1.e-5)  /* function tolerance */
#define STOL   RCONST(1.e-5) /* step tolerance */

#define ZERO   RCONST(0.0)
#define PT25   RCONST(0.25)
#define PT5    RCONST(0.5)
#define ONE    RCONST(1.0)
#define ONEPT5 RCONST(1.5)
#define TWO    RCONST(2.0)

#define PI     RCONST(3.1415926)
#define E      RCONST(2.7182818)

#define TOL	   1.e-3
#define ABSTOL RCONST(1.e-3)

const double MinManualStep = 1.e-4;
const double MaxManualStep = 1.e-1;

class DCSolver;

typedef struct {
  realtype lb[NVAR];
  realtype ub[NVAR];
} *UserData;

/* Accessor macro */
#define Ith(v,i)    NV_Ith_S(v,i-1)   

class supcapacitor;
class ees_bank;

class DCSolver {
public:
	DCSolver();
	~DCSolver();
    int SolveItGivenDCInput(double dc_vin, double dc_iin, double &dc_vout, double &dc_iout, double &dc_power, ees_bank *bank);
	int SolveItGivenDCOutput(double dc_vout, double dc_iout, double &dc_vin, double &dc_iin, double &dc_power, ees_bank *bank); 
	int ManualSolverGivenDCOutput();
    double ComputeDCPowerBuck(const double &Vin, const double &Vout, const double &Iout);
    double ComputeDCPowerBoost(const double &Vin, const double &Vout, const double &Iout);
    double ComputeDCPowerBuckApproximation(const double &Vin, const double &Vout, const double &Iout);
    double ComputeDCPowerBoostApproximation(const double &Vin, const double &Vout, const double &Iout);
private:
	int KINSolverWapper();
    // Data members for DC-DC converter
    std::vector<double> m_Rsw;
    std::vector<double> m_Qsw;
    double m_RL, m_RC;
    double m_Ictrl;
    double m_fs;
    double m_Lf;

	// Data members for the solver
	UserData data;
	realtype fnormtol, scsteptol;
	N_Vector u1, u, s, c;
	int glstr, mset, flag;
	void *kmem;

	int buck_failed, boost_failed;
};

#endif
