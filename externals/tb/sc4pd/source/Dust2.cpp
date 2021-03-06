/* sc4pd: 
   Dust2, Dust2~ 
   
   Copyright (c) 2004 Tim Blechmann.                                       

   This code is derived from:
	SuperCollider real time audio synthesis system
    Copyright (c) 2002 James McCartney. All rights reserved.
	http://www.audiosynth.com


   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,             
   but WITHOUT ANY WARRANTY; without even the implied warranty of         
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

   Based on:
     PureData by Miller Puckette and others.
         http://www.crca.ucsd.edu/~msp/software.html
     FLEXT by Thomas Grill
         http://www.parasitaere-kapazitaeten.net/ext
     SuperCollider by James McCartney
         http://www.audiosynth.com
     
   Coded while listening to: Herbert Eimert: Epitaph f�r Aikichi Kuboyama

*/

#include "sc4pd.hpp"

/* ------------------------ Dust2~ -------------------------------------*/

class Dust2_ar:
    public sc4pd_dsp
{
    FLEXT_HEADER(Dust2_ar,sc4pd_dsp);

public:
    Dust2_ar(int argc, t_atom *argv);
    
protected:
    virtual void m_signal(int n, t_sample *const *in, t_sample *const *out);
    virtual void m_dsp (int n,t_signalvec const * insigs,
			t_signalvec const * outsigs);
    
    void m_set(float f)
    {
	m_density = f;
	m_thresh = m_density/Samplerate();
	m_scale = m_thresh > 0.f ? 1.f / m_thresh : 0.f;
    }

    void m_seed(int i)
    {
	rgen.init(i);
    }
    
private:
    FLEXT_CALLBACK_F(m_set);
    FLEXT_CALLBACK_I(m_seed);
    float m_density, m_thresh, m_scale;
    RGen rgen;

};

FLEXT_LIB_DSP_V("Dust2~",Dust2_ar);

Dust2_ar::Dust2_ar(int argc, t_atom *argv)
{
    FLEXT_ADDMETHOD_(0,"set",m_set);
    FLEXT_ADDMETHOD_(0,"seed",m_seed);

    //parse arguments
    AtomList Args(argc,argv);
    m_density=sc_getfloatarg(Args,0);

    rgen.init(timeseed());

    AddOutSignal();
}    

void Dust2_ar::m_dsp(int n,t_signalvec const * insigs,
		t_signalvec const * outsigs)
{
    m_thresh = m_density/Samplerate();
    m_scale = m_thresh > 0.f ? 2.f / m_thresh : 0.f;
}

void Dust2_ar::m_signal(int n, t_sample *const *in, 
			       t_sample *const *out)
{
    t_sample *nout = *out;

    RGET;
    float thresh = m_thresh;
    float scale = m_scale;

    for (int i = 0; i!= n;++i)
    {
	float z = frand(s1,s2,s3);
	if (z < thresh) 
	    (*(nout)++)  = z * scale - 1.f;
	else 
	    (*(nout)++)  = 0.f;
    }

    RPUT;
}


/* ------------------------ Dust2 ---------------------------------------*/

class Dust2_kr:
    public flext_base
{
    FLEXT_HEADER(Dust2_kr,flext_base);

public:
    Dust2_kr(int argc, t_atom *argv);
    
protected:
    void m_set(float f);
    Timer Dust2_timer;
    void m_doit(void*);

    void m_seed(int i)
    {
	rgen.init(i);
    }
    
private:
    float m_density, m_scale;
    float mtbt; //medium time between trigger
    RGen rgen;
    FLEXT_CALLBACK_1(m_set,float);
    FLEXT_CALLBACK_I(m_seed);
    FLEXT_CALLBACK_T(m_doit);
};

FLEXT_LIB_V("Dust2",Dust2_kr);

Dust2_kr::Dust2_kr(int argc, t_atom *argv)
    : Dust2_timer(false)
{
    FLEXT_ADDMETHOD(0,m_set);
    FLEXT_ADDMETHOD_(0,"seed",m_seed);

    //parse arguments
    AtomList Args(argc,argv);
    m_density=sc_getfloatarg(Args,0);
    
    rgen.init(timeseed());
    AddOutFloat();
    
    FLEXT_ADDTIMER(Dust2_timer,m_doit);

    m_set(m_density);
}

void Dust2_kr::m_set(float f)
{
    Dust2_timer.Reset();
    
    if(f==0)
    {
	return;
    }
    m_density = f;
    mtbt = 1/f*1000;

    Dust2_timer.Delay(mtbt * 0.002 * rgen.frand());
}

void Dust2_kr::m_doit(void*)
{
    ToOutFloat(0,2*rgen.frand() - 1 );

    Dust2_timer.Delay(mtbt * 0.002 * rgen.frand());
}
