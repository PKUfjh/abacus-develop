#include "charge_mixing.h"
#include "global.h"
#include "../src_pw/inverse_matrix.h"

Charge_Mixing::Charge_Mixing(){}
Charge_Mixing::~Charge_Mixing(){}

void Charge_Mixing::set_mixing
(
    const string &mixing_mode_in,
    const double &mixing_beta_in,
    const int &mixing_ndim_in,
	const double &mixing_gg0_in
)
{
    this->mixing_mode = mixing_mode_in;
    this->mixing_beta = mixing_beta_in;
    this->mixing_ndim = mixing_ndim_in;
	this->mixing_gg0 = mixing_gg0_in; //mohan add 2014-09-27

//    if (mixing_mode != "plain") // "TF","local-TF","potential"
//    {
//        WARNING_QUIT("set_mixing","only plain mixing availabel.");
//    }

    return;
}

    
// Fourier transform of rho(g) to rho(r) in real space.
void Charge_Mixing::set_rhor(complex<double> *rhog, double *rho)const
{
    if (test_charge)TITLE("Charge_Mixing","set_rhor");
    for (int is=0; is < NSPIN; is++)
    {
		UFFT.ToRealSpace(rhog, rho);
    }
    return;
}


void Charge_Mixing::set_rhog(const double *rho_in, complex<double> *rhog_in)const
{
    if (test_charge)TITLE("Charge_Mixing","set_rhog");
	UFFT.ToReciSpace(rho_in, rhog_in);
    return;
}


void Charge_Mixing::plain_mixing( double *rho, double *rho_save_in ) const
{
    // if mixing_beta == 1, each electron iteration,
    // use all new charge density,
    // on the contrary, if mixing_beta == 0,
    // no new charge will be generated!
    const double mix_old = 1 - mixing_beta;
	
//	this->check_ne(rho);
//	this->check_ne(rho_save_in);

    // in real space
	// mohan modify 2010-02-05
	// after mixing, the charge density become 
	// the input charge density of next iteration.
    //double* rho_tmp = new double[pw.nrxx];
    //DCOPY( rho, rho_tmp, pw.nrxx);

//xiaohui add 2014-12-09
	if(this->mixing_gg0 > 0.0)
	{
		double* Rrho = new double[pw.nrxx];
		complex<double> *kerpulay = new complex<double>[pw.ngmc];
		double* kerpulayR = new double[pw.nrxx];

		for(int ir=0; ir<pw.nrxx; ir++)
		{
			Rrho[ir] = rho[ir] - rho_save_in[ir];
		}
		set_rhog(Rrho, kerpulay);

		const double fac = this->mixing_gg0;
		const double gg0 = std::pow(fac * 0.529177 / ucell.tpiba, 2);
		double* filter_g = new double[pw.ngmc];
		for(int ig=0; ig<pw.ngmc; ig++)
		{
			double gg = pw.gcar[ig].norm2();
			filter_g[ig] = max(gg / (gg + gg0), 0.1);

			kerpulay[ig] = (1 - filter_g[ig]) * kerpulay[ig];
		}
		set_rhor(kerpulay, kerpulayR);

		for(int ir=0; ir<pw.nrxx; ir++)
		{
			Rrho[ir] = Rrho[ir] - kerpulayR[ir];
			rho[ir] = Rrho[ir] * mixing_beta + rho_save_in[ir];
		}

		delete[] Rrho;
		delete[] kerpulay;
		delete[] kerpulayR;
		delete[] filter_g;
	}
	else
	{
		for (int ir=0; ir<pw.nrxx; ir++)
		{
			rho[ir] = rho[ir]*mixing_beta + mix_old*rho_save_in[ir];
		}
	}

	DCOPY( rho, rho_save_in, pw.nrxx);
//    delete[] rho_tmp;

    return;
}


void Charge_Mixing::Kerker_mixing( double *rho, const complex<double> *residual_g, double *rho_save)
{
//  TITLE("Charge_Mixing","Kerker");
    timer::tick("Charge_Mixing","Kerker");

//	cout << " here is Kerker" << endl;
//	this->check_ne(rho);
//	this->check_ne(rho_save);

    // (1) do kerker mixing in reciprocal space.
    complex<double> *rhog = new complex<double>[pw.ngmc];
    ZEROS(rhog, pw.ngmc);

	// mohan modify 2010-02-03, rhog should store the old
	// charge density. " rhog = FFT^{-1}(rho_save) "
    this->set_rhog(rho_save, rhog);

    // (2) set up filter
    const double a = 0.8; // suggested by VASP.

	// mohan fixed bug 2010/03/25
	// suggested by VASP, 1.5(angstrom^-1) is always satisfied.
    const double gg0 = std::pow(1.5 * 0.529177 / ucell.tpiba, 2);
    double *filter_g = new double[pw.ngmc];
    for (int ig=0; ig<pw.ngmc; ig++)
    {
        double gg = pw.gcar[ig].norm2();
//      filter_g[ig] = a * gg / (gg+gg0);
		filter_g[ig] = mixing_beta * gg / (gg+gg0);//mohan modify 2010/03/25
    }

    // (3)
    for (int ig=0; ig<pw.ngmc; ig++)
    {
        rhog[ig] += filter_g[ig] * residual_g[ig];
    }

    // (4) transform rhog from G-space to real space.
	// get the new charge. " FFT(rhog) = rho "
    this->set_rhor( rhog, rho);

    // (5)
	// mohan change the order of (4) (5), 2010-02-05
    DCOPY(rho, rho_save, pw.nrxx);

    //this->renormalize_rho();


    delete[] filter_g;
    delete[] rhog;
    timer::tick("Charge_Mixing","Kerker");
    return;
}


double Charge_Mixing::rhog_dot_product(
    const complex<double> *rhog11,
    const complex<double> *rhog12,
    const complex<double> *rhog21,
    const complex<double> *rhog22) const
{
    TITLE("Charge_Mixing","rhog_dot_product");
	timer::tick("Charge_Mixing","rhog_dot_product");
    static const double fac = e2 * FOUR_PI / ucell.tpiba2;
    static const double fac2 = e2 * FOUR_PI / (TWO_PI * TWO_PI);

    double sum = 0.0;

    switch ( NSPIN )
    {
    case 1:
        for (int ig=pw.gstart; ig<pw.ngmc; ig++)
        {
            sum += ( conj( rhog11[ig] )* rhog21[ig] ).real() / pw.gg[ig];
        }
        sum *= fac;
        break;

    case 2:
		
		// (1) First part of density error.
        for (int ig=pw.gstart; ig<pw.ngmc; ig++)
        {
            sum += ( conj( rhog11[ig]+rhog12[ig] ) * (rhog21[ig]+rhog22[ig]) ).real() / pw.gg[ig];
        }
        sum *= fac;

		if(GAMMA_ONLY_PW)
		{
			sum *= 2.0;
		}

		// (2) Second part of density error.
        // including |G|=0 term.
		double sum2 = 0.0;
		
		sum2 += fac2 * ( conj( rhog11[0]-rhog12[0] ) * ( rhog21[0]-rhog22[0] ) ).real();

		double mag = 0.0;
        for (int ig=0; ig<pw.ngmc; ig++)
        {
            mag += ( conj( rhog11[ig]-rhog12[ig] ) * ( rhog21[ig]-rhog22[ig] ) ).real();
        }
		mag *= fac2;

		if(GAMMA_ONLY_PW);
		{
			mag *= 2.0;
		}

		//cout << " sum=" << sum << " mag=" << mag << endl;
		sum2 += mag;
		sum += sum2;


	//case 4:
		// non-collinear spin	

    }

    Parallel_Reduce::reduce_double_pool( sum );

	timer::tick("Charge_Mixing","rhog_dot_product");

	sum *= ucell.omega * 0.5;

	bool dft_is_meta = false;
	bool lda_plus_u = false;
	bool okpaw = false;
	bool dipfield = false;

//	if(dft_is_meta) sum += tauk_ddot();
//	if(lda_pluse_u) sum += ns_ddot();
//	if(okpaw) sum += paw_ddot();
//	if(dipfield) sum += ...;

    return sum;
}
