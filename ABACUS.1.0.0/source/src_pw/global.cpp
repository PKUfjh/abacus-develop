//==========================================================
// Author: Lixin He,mohan
// DATE : 2008-11-6
//==========================================================

#include "global.h"
//----------------------------------------------------------
// init "GLOBAL CLASS" object
//----------------------------------------------------------

kvect kv; // mem check in in here.
Use_FFT UFFT; // mohan add 2010-07-22
FFT fftwan;
output out;

PW_Basis pw;
energy en;
wavefunc wf;
Hamilt hm;
Exx_pw exxpw;

#ifdef __EPM

UnitCell_epm ucell;
Potential_EPM epm;
FFT fftsb;
so_pw sopw;
so_smallbox sobox ;
zeeman zm;

#else


#ifdef __FP
//wannier wan;
#endif

UnitCell_pseudo ucell;
pseudopot_cell_vnl ppcell;
xcfunc xcf;
Charge_Broyden chr;
Magnetism mag;

potential pot;
electrons elec;
Symmetry symm;
Parallel_Grid Pgrid; //mohan add 2010-06-06 
Parallel_Kpoints Pkpoints; // mohan add 2010-06-07

#endif
