
#include "DCSolver.hpp"
#include "SuperCap.hpp"

#include <algorithm>
#include <iostream>
#include <tr1/functional>

using namespace std;
using namespace std::tr1;

/* Functions Called by the KINSOL Solver */
static int func_given_input(N_Vector u, N_Vector f, void *user_data);
static int func_given_output(N_Vector u, N_Vector f, void *user_data);

/* Private Helper Functions */
static void SetInitialGuess1(N_Vector u, UserData data);
static void SetInitialGuess2(N_Vector u, UserData data);
static void PrintOutput(N_Vector u);
static void PrintFinalStats(void *kmem);
static int check_flag(void *flagvalue, char *funcname, int opt);

/* Private varible */
static double g_dc_vout, g_dc_iout;
static double g_dc_vin, g_dc_iin;
static double g_dc_power;
static double g_bank_vocc(0.0), g_bank_racc(0.166);

static function<double(const double, const double, const double)> power_calculator;

typedef void (*GuessFunction)(N_Vector, UserData); 

static GuessFunction PrimaryGuess;
static GuessFunction SecondaryGuess;

/*
 *--------------------------------------------------------------------
 * MAIN PROGRAM
 *--------------------------------------------------------------------
 */

#ifdef _DC_SOLVER_MAIN_
int main() {
	DCSolver dc_solver;

	double dc_vout, dc_iout;
	double dc_vin(1.0), dc_iin(0.113714);
	double dc_power;

	dc_solver.SolveItGivenDCInput(dc_vin, dc_iin, dc_vout, dc_iout, dc_power, (ees_bank *)NULL);
	dc_solver.SolveItGivenDCInput(dc_vin, dc_iin, dc_vout, dc_iout, dc_power, (ees_bank *)NULL);

	cout << dc_vout << "\t" << dc_iout << endl;

	return 0;
}
#endif

DCSolver::DCSolver() :
    m_Rsw(4, 25e-3),
    m_Qsw(4, 60e-9),
    m_RL(39e-3),
    m_RC(100e-3),
    m_Ictrl(4e-3),
    m_fs(500e3),
    m_Lf(6e-6) {

  /* User data */

  data = (UserData)malloc(sizeof *data);
  data->lb[0] = RCONST(ABSTOL);       
  data->ub[0] = RCONST(10.0);

  /* Create serial vectors of length NEQ */
  u1 = N_VNew_Serial(NEQ);
  check_flag((void *)u1, "N_VNew_Serial", 0);

  u = N_VNew_Serial(NEQ);
  check_flag((void *)u, "N_VNew_Serial", 0);

  s = N_VNew_Serial(NEQ);
  check_flag((void *)s, "N_VNew_Serial", 0);

  c = N_VNew_Serial(NEQ);
  check_flag((void *)c, "N_VNew_Serial", 0);

  N_VConst_Serial(ONE,s); /* no scaling */

  Ith(c,1) =  ZERO;   /* no constraint on x1 */
  Ith(c,2) =  ONE;    /* l1 = x1 - x1_min >= 0 */
  Ith(c,3) = -ONE;    /* L1 = x1 - x1_max <= 0 */
  
  fnormtol=FTOL; scsteptol=STOL;

  glstr = KIN_NONE;
  mset = 1;

  kmem = KINCreate();
  check_flag((void *)kmem, "KINCreate", 0);

  flag = KINSetConstraints(kmem, c);
  check_flag(&flag, "KINSetConstraints", 1);
  flag = KINSetFuncNormTol(kmem, fnormtol);
  check_flag(&flag, "KINSetFuncNormTol", 1);
  flag = KINSetScaledStepTol(kmem, scsteptol);
  check_flag(&flag, "KINSetScaledStepTol", 1);

  flag = KINSetUserData(kmem, data);
  check_flag(&flag, "KINSetUserData", 1);

  flag = KINInit(kmem, func_given_input, u);
  check_flag(&flag, "KINInit", 1);

  /* Call KINDense to specify the linear solver */

  flag = KINDense(kmem, NEQ);
  check_flag(&flag, "KINDense", 1);

  flag = KINSetMaxSetupCalls(kmem, mset);
  check_flag(&flag, "KINSetMaxSetupCalls", 1);

}

DCSolver::~DCSolver() {
  /* Free memory */

  N_VDestroy_Serial(u);
  N_VDestroy_Serial(u1);
  N_VDestroy_Serial(s);
  N_VDestroy_Serial(c);
  KINFree(&kmem);
  free(data);
}

int DCSolver::SolveItGivenDCInput(double dc_vin, double dc_iin, double &dc_vout, double &dc_iout, double &dc_power, ees_bank *bank) {

	int ret = 0;
	// Initialize the static global variables here
	g_dc_vin = dc_vin;
	g_dc_iin = dc_iin;
	g_bank_racc = bank->EESBankGetRacc();
	g_bank_vocc = bank->EESBankGetVoc();

	PrimaryGuess = SetInitialGuess1;
	SecondaryGuess = SetInitialGuess2;

	// Bind the function here
	flag = KINSetSysFunc(kmem, func_given_input);
    if (check_flag(&flag, "KINSetSysFunc", 1)) return(1);

	// Set the lower bound and initial guess
	data->lb[0] = max(RCONST(g_bank_vocc - TOL), ABSTOL);
	PrimaryGuess(u1,data);

	KINSolverWapper();

	if (buck_failed && boost_failed) {
		// In case we can not solve the problem, 
		// assume there is no current between bank and DC-DC
		g_dc_vout = g_bank_vocc;
		g_dc_iout = 0.0;
		if (g_dc_vin >= g_dc_vout) {
			g_dc_power = ComputeDCPowerBuck(g_dc_vin, g_dc_vout, g_dc_iout);
		} else {
			g_dc_power = ComputeDCPowerBoost(g_dc_vin, g_dc_vout, g_dc_iout);
		}
		ret = -2;
	}
	// Assign the computed value back
	dc_vout = g_dc_vout;
	dc_iout = g_dc_iout;
	dc_power = g_dc_power;

	return (ret);
}

int DCSolver::SolveItGivenDCOutput(double dc_vout, double dc_iout, double &dc_vin, double &dc_iin, double &dc_power, ees_bank *bank) {

	int ret = 0;
	// Initialize the static global variables here
	g_dc_vout = dc_vout;
	g_dc_iout = dc_iout;
	g_bank_racc = bank->EESBankGetRacc();
	g_bank_vocc = bank->EESBankGetVoc();

	PrimaryGuess = SetInitialGuess2;
	SecondaryGuess = SetInitialGuess1;

	// Bind the function here
	flag = KINSetSysFunc(kmem, func_given_output);
    if (check_flag(&flag, "KINSetSysFunc", 1)) return(1);

	// Set the upper bound and initial guess
	data->ub[0] = RCONST(g_bank_vocc + TOL);
	PrimaryGuess(u1,data);

	KINSolverWapper();

	if (buck_failed && boost_failed) {
		// In case we can not solve the problem, 
		// assume there is no current between bank and DC-DC
		g_dc_vin = g_bank_vocc;
		g_dc_iin = 0.0;
		if (g_dc_vin >= g_dc_vout) {
			g_dc_power = ComputeDCPowerBuck(g_dc_vin, g_dc_vout, g_dc_iout);
		} else {
			g_dc_power = ComputeDCPowerBoost(g_dc_vin, g_dc_vout, g_dc_iout);
		}
		ret = -2;
	}
	// Assign the computed value back
	dc_vin = g_dc_vin;
	dc_iin = g_dc_iin;
	dc_power = g_dc_power;

	return (ret);
}

int DCSolver::KINSolverWapper()
{

  /* --------------------------- */

  printf("\n------------------------------------------\n");
  printf("\nInitial guess on lower bounds\n");
  printf("  [x1,x2] = ");
  PrintOutput(u1);

  N_VScale_Serial(ONE,u1,u);

  if (mset==1)
    printf("Exact Newton");
  else
    printf("Modified Newton");

  if (glstr == KIN_NONE)
    printf("\n");
  else
    printf(" with line search\n");

  // Assume buck mode first
  power_calculator = bind(&DCSolver::ComputeDCPowerBuck, this, placeholders::_1, placeholders::_2, placeholders::_3);
  buck_failed = KINSol(kmem, u, glstr, s, s);
  if (buck_failed != KIN_SUCCESS) {
    check_flag(&buck_failed, "KINSol", 1);

	// Start from another bound
	SecondaryGuess(u1,data);
    N_VScale_Serial(ONE,u1,u);
	buck_failed = KINSol(kmem, u, glstr, s, s);
  }

  // If it is in boost mode
  if ((buck_failed != KIN_SUCCESS) || (g_dc_vin < g_dc_vout)) {
	power_calculator = bind(&DCSolver::ComputeDCPowerBoost, this, placeholders::_1, placeholders::_2, placeholders::_3);
	boost_failed = KINSol(kmem, u, glstr, s, s);
	if ((boost_failed != KIN_SUCCESS) || (g_dc_vin > g_dc_vout)){
	  check_flag(&boost_failed, "KINSol", 1);

	  // Start from another bound
	  SecondaryGuess(u1,data);
      N_VScale_Serial(ONE,u1,u);
	  boost_failed = KINSol(kmem, u, glstr, s, s);
	  if (g_dc_vin > g_dc_vout) {
	    boost_failed = -1;
	  }
	}
  }

  printf("Solution:\n  [x1,x2] = ");
  PrintOutput(u);

  PrintFinalStats(kmem);

  return(0);
}

/*
 *--------------------------------------------------------------------
 * FUNCTIONS CALLED BY KINSOL
 *--------------------------------------------------------------------
 */

/* 
 * System function for predator-prey system 
 */

static int func_given_input(N_Vector u, N_Vector f, void *user_data)
{
  realtype *udata, *fdata;
  realtype x1, l1, L1;
  realtype *lb, *ub;
  UserData data;
  
  data = (UserData)user_data;
  lb = data->lb;
  ub = data->ub;

  udata = NV_DATA_S(u);
  fdata = NV_DATA_S(f);

  x1 = udata[0];
  l1 = udata[1];
  L1 = udata[2];

  // x1 is the dc_vout, y1 is the dc_iout
  g_dc_vout = x1;
  g_dc_iout = (g_dc_vout - g_bank_vocc)/g_bank_racc;
  g_dc_power = power_calculator(g_dc_vin, g_dc_vout, g_dc_iout);

  // fdata[0] = x1*x1 - 1; 
  fdata[0] = g_dc_vout*g_dc_iout + g_dc_power - g_dc_vin*g_dc_iin; 
  fdata[1] = l1 - x1 + lb[0];
  fdata[2] = L1 - x1 + ub[0];

  return(0);
}

static int func_given_output(N_Vector u, N_Vector f, void *user_data)
{
  realtype *udata, *fdata;
  realtype x1, l1, L1;
  realtype *lb, *ub;
  UserData data;
  
  data = (UserData)user_data;
  lb = data->lb;
  ub = data->ub;

  udata = NV_DATA_S(u);
  fdata = NV_DATA_S(f);

  x1 = udata[0];
  l1 = udata[1];
  L1 = udata[2];

  // x1 is the dc_vout, y1 is the dc_iout
  g_dc_vin = x1;
  g_dc_iin = (g_bank_vocc - g_dc_vin)/g_bank_racc;
  g_dc_power = power_calculator(g_dc_vin, g_dc_vout, g_dc_iout);

  // fdata[0] = x1*x1 - 1; 
  fdata[0] = g_dc_vout*g_dc_iout + g_dc_power - g_dc_vin*g_dc_iin; 
  fdata[1] = l1 - x1 + lb[0];
  fdata[2] = L1 - x1 + ub[0];

  return(0);
}

/*
 *--------------------------------------------------------------------
 * PRIVATE FUNCTIONS
 *--------------------------------------------------------------------
 */

/*
 * Initial guesses
 */

static void SetInitialGuess1(N_Vector u, UserData data)
{
  realtype x1;
  realtype *udata;
  realtype *lb, *ub;

  udata = NV_DATA_S(u);

  lb = data->lb;
  ub = data->ub;

  /* this init. guess should take us to (0.29945; 2.83693) */
  x1 = lb[0];

  udata[0] = x1;
  udata[1] = x1 - lb[0];
  udata[2] = x1 - ub[0];
}

static void SetInitialGuess2(N_Vector u, UserData data)
{
  realtype x1;
  realtype *udata;
  realtype *lb, *ub;

  udata = NV_DATA_S(u);

  lb = data->lb;
  ub = data->ub;

  /* this init. guess should take us to (0.29945; 2.83693) */
  x1 = ub[0];

  udata[0] = x1;
  udata[1] = x1 - lb[0];
  udata[2] = x1 - ub[0];
}

double DCSolver::ComputeDCPowerBuck(const double &Vin, const double &Vout, const double &Iout) {
    double m_D = Vout / Vin;
    double m_delI = (Vout * (1 - m_D)) / (m_Lf * m_fs); 
    double power = Iout * Iout * (m_RL + m_D * m_Rsw[0] + (1 - m_D) * m_Rsw[1] + m_Rsw[3]) + ((m_delI * m_delI) / 12) * (m_RL + m_D * m_Rsw[0] + (1 - m_D) * m_Rsw[1] + m_Rsw[3] + m_RC) + Vin * m_fs * (m_Qsw[0] + m_Qsw[1]) + Vin * m_Ictrl;
    return power;
}

double DCSolver::ComputeDCPowerBoost(const double &Vin, const double &Vout, const double &Iout) {
    double m_D = 1 - (Vin / Vout); 
    double m_delI = (Vin * m_D) / (m_Lf * m_fs);
    double power = ((Iout * Iout) / ((1 - m_D) * (1 - m_D))) * (m_RL + m_D * m_Rsw[2] + (1 - m_D) * m_Rsw[3] + m_Rsw[0] + m_D * (1 - m_D) * m_RC) + ((m_delI * m_delI) / 12) * (m_RL + m_D * m_Rsw[2] + (1 - m_D) * m_Rsw[3] + m_Rsw[0] + (1 - m_D) * m_RC) + Vout * m_fs * (m_Qsw[2] + m_Qsw[3]) + Vin * m_Ictrl;
    
    return power;
}

/* 
 * Print solution
 */

static void PrintOutput(N_Vector u)
{
#if defined(SUNDIALS_EXTENDED_PRECISION)
    printf(" %8.6Lg  %8.6Lg\n", Ith(u,1), Ith(u,2));
#elif defined(SUNDIALS_DOUBLE_PRECISION)
    printf(" %8.6lg  %8.6lg\n", Ith(u,1), Ith(u,2));
#else
    printf(" %8.6g  %8.6g\n", Ith(u,1), Ith(u,2));
#endif
}

/* 
 * Print final statistics contained in iopt 
 */

static void PrintFinalStats(void *kmem)
{
  long int nni, nfe, nje, nfeD;
  int flag;
  
  flag = KINGetNumNonlinSolvIters(kmem, &nni);
  check_flag(&flag, "KINGetNumNonlinSolvIters", 1);
  flag = KINGetNumFuncEvals(kmem, &nfe);
  check_flag(&flag, "KINGetNumFuncEvals", 1);

  flag = KINDlsGetNumJacEvals(kmem, &nje);
  check_flag(&flag, "KINDlsGetNumJacEvals", 1);
  flag = KINDlsGetNumFuncEvals(kmem, &nfeD);
  check_flag(&flag, "KINDlsGetNumFuncEvals", 1);

  printf("Final Statistics:\n");
  printf("  nni = %5ld    nfe  = %5ld \n", nni, nfe);
  printf("  nje = %5ld    nfeD = %5ld \n", nje, nfeD);
}

/*
 * Check function return value...
 *    opt == 0 means SUNDIALS function allocates memory so check if
 *             returned NULL pointer
 *    opt == 1 means SUNDIALS function returns a flag so check if
 *             flag >= 0
 *    opt == 2 means function allocates memory so check if returned
 *             NULL pointer 
 */

static int check_flag(void *flagvalue, char *funcname, int opt)
{
  int *errflag;

  /* Check if SUNDIALS function returned NULL pointer - no memory allocated */
  if (opt == 0 && flagvalue == NULL) {
    fprintf(stderr, 
            "\nSUNDIALS_ERROR: %s() failed - returned NULL pointer\n\n",
	    funcname);
    return(1);
  }

  /* Check if flag < 0 */
  else if (opt == 1) {
    errflag = (int *) flagvalue;
    if (*errflag < 0) {
      fprintf(stderr,
              "\nSUNDIALS_ERROR: %s() failed with flag = %d\n\n",
	      funcname, *errflag);
      return(1); 
    }
  }

  /* Check if function returned NULL pointer - no memory allocated */
  else if (opt == 2 && flagvalue == NULL) {
    fprintf(stderr,
            "\nMEMORY_ERROR: %s() failed - returned NULL pointer\n\n",
	    funcname);
    return(1);
  }

  return(0);
}
