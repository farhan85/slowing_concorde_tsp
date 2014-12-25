/* Modifications.
    January 8, 2008 (Farhan Ahammed)
      - Implemented the structs so that multiple PRNGs can be used.
        Create a MT-PRNG like this:
            struct mt_prng prng;
            init_genrand(&prng, seed);
            randomNumber = genrand_intXX(&prng);
            randomNumber = genrand_realX(&prng);
*/

/*
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.
   Copyright (C) 2005, Mutsuo Saito,
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote
        products derived from this software without specific prior written
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/ep->mt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/

#include <stdio.h>
#include "t_mt19937ar.h"


/* initializes p->mt->mt[N] with a seed */
void init_genrand(mt_prng* p, unsigned long s)
{
	/* mag01[x] = x * MATRIX_A  for x=0,1 */
	p->mag01[0] = 0x0UL;
	p->mag01[1] = MATRIX_A;

    p->mt[0]= s & 0xffffffffUL;
    for (p->mti=1; p->mti<N; p->mti++) {
        p->mt[p->mti] = (1812433253UL * (p->mt[p->mti-1] ^ (p->mt[p->mti-1] >> 30)) + p->mti);
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array p->mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        p->mt[p->mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
void init_by_array(mt_prng* p, unsigned long init_key[], int key_length)
{
    int i, j, k;
    init_genrand(p, 19650218UL);
    i=1; j=0;
    k = (N>key_length ? N : key_length);
    for (; k; k--) {
        p->mt[i] = (p->mt[i] ^ ((p->mt[i-1] ^ (p->mt[i-1] >> 30)) * 1664525UL))
          + init_key[j] + j; /* non linear */
        p->mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=N) { p->mt[0] = p->mt[N-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=N-1; k; k--) {
        p->mt[i] = (p->mt[i] ^ ((p->mt[i-1] ^ (p->mt[i-1] >> 30)) * 1566083941UL))
          - i; /* non linear */
        p->mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=N) { p->mt[0] = p->mt[N-1]; i=1; }
    }

    p->mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */
}

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(mt_prng* p)
{
    unsigned long y;

    if (p->mti >= N) { /* generate N words at one time */
        int kk;

        if (p->mti == N+1)   /* if init_genrand() has not been called, */
            init_genrand(p, 5489UL); /* a default initial seed is used */

        for (kk=0;kk<N-M;kk++) {
            y = (p->mt[kk]&UPPER_MASK)|(p->mt[kk+1]&LOWER_MASK);
            p->mt[kk] = p->mt[kk+M] ^ (y >> 1) ^ p->mag01[y & 0x1UL];
        }
        for (;kk<N-1;kk++) {
            y = (p->mt[kk]&UPPER_MASK)|(p->mt[kk+1]&LOWER_MASK);
            p->mt[kk] = p->mt[kk+(M-N)] ^ (y >> 1) ^ p->mag01[y & 0x1UL];
        }
        y = (p->mt[N-1]&UPPER_MASK)|(p->mt[0]&LOWER_MASK);
        p->mt[N-1] = p->mt[M-1] ^ (y >> 1) ^ p->mag01[y & 0x1UL];

        p->mti = 0;
    }

    y = p->mt[p->mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

/* generates a random number on [0,0x7fffffff]-interval */
long genrand_int31(mt_prng* p)
{
    return (long)(genrand_int32(p)>>1);
}

/* generates a random number on [0,1]-real-interval */
double genrand_real1(mt_prng* p)
{
    return genrand_int32(p)*(1.0/4294967295.0);
    /* divided by 2^32-1 */
}

/* generates a random number on [0,1)-real-interval */
double genrand_real2(mt_prng* p)
{
    return genrand_int32(p)*(1.0/4294967296.0);
    /* divided by 2^32 */
}

/* generates a random number on (0,1)-real-interval */
double genrand_real3(mt_prng* p)
{
    return (((double)genrand_int32(p)) + 0.5)*(1.0/4294967296.0);
    /* divided by 2^32 */
}

/* generates a random number on [0,1) with 53-bit resolution*/
double genrand_res53(mt_prng* p)
{
    unsigned long a=genrand_int32(p)>>5, b=genrand_int32(p)>>6;
    return(a*67108864.0+b)*(1.0/9007199254740992.0);
}
/* These real versions are due to Isaku Wada, 2002/01/09 added */
