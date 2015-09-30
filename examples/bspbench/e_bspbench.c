// This program is based on bspbench from BSPedupack by Rob Bisseling, copyrighted in 2004
// BSPedupack is under the GNU General Public License

#include <e_bsp.h>
#include "e-lib.h"
 
/*  This program measures p, r, g, and l of a BSP computer
    using bsp_put for communication.
*/

/* This program needs order 6*MAXH+3*MAXN memory */
#define NITERSN 500000 /* number of iterations. Default: 100 */
#define NITERSH 8000   /* number of iterations. Default: 100 */
#define MAXN 256        /* maximum length of DAXPY computation. Default: 1024 */
#define MAXH 64         /* maximum h in h-relation. Default: 256 */
#define MEGA 1000000.0
#define KILO    1000.0


/* A subset of bspedupack starts here */

#define SZDBL (sizeof(float))
#define SZINT (sizeof(int))
#define TRUE (1)
#define FALSE (0)
#define MAX(a,b) ((a)>(b) ? (a) : (b))
#define MIN(a,b) ((a)<(b) ? (a) : (b))
#define fabs(a) ((a)>0 ? (a) : -1.0f*(a))
#define SZULL (sizeof( long long))
#define ulong long long

float *vecallocd(int n) { 
    return (float*)ebsp_malloc(n * sizeof(float));
}
/* end bspedupack */


void leastsquares(int h0, int h1, float *t, float *g, float *l) {
    /* This function computes the parameters g and l of the 
       linear function T(h)= g*h+l that best fits
       the data points (h,t[h]) with h0 <= h <= h1. */

    float nh, sumt, sumth, sumh, sumhh, a;
    int h;

    nh = h1-h0+1;
    /* Compute sums:
        sumt  =  sum of t[h] over h0 <= h <= h1
        sumth =         t[h]*h
        sumh  =         h
        sumhh =         h*h     */
    sumt = sumth = 0.0;
    for (h=h0; h<=h1; h++) {
        sumt  += t[h];
        sumth += t[h]*h;
    }
    sumh  = (h1*h1-h0*h0+h1+h0)/2;  
    sumhh = ( h1*(h1+1)*(2*h1+1) - (h0-1)*h0*(2*h0-1))/6;

    /* Solve      nh*l +  sumh*g =  sumt 
                sumh*l + sumhh*g = sumth */
    if(fabs(nh)>fabs(sumh)) {
        a = sumh/nh;
        /* subtract a times first eqn from second eqn */
        *g = (sumth-a*sumt)/(sumhh-a*sumh);
        *l = (sumt-sumh* *g)/nh;
    } else {
        a = nh/sumh;
        /* subtract a times second eqn from first eqn */
        *g = (sumt-a*sumth)/(sumh-a*sumhh);
        *l = (sumth-sumhh* *g)/sumh;
    }

} /* end leastsquares */

int main() { /*  bsp_bench */
    /* void leastsquares(int h0, int h1, float *t, float *g, float *l); */
    int p, s, s1, iter, i, n, h, *destproc, *destindex;
    float alpha, beta, *x, *y, *z, *src, *dest,
           time0, time1, time, *Time, mintime, maxtime,
           nflops, r, g0, l0, g, l, *t; 
  
    /**** Determine p ****/
    bsp_begin();
    p = bsp_nprocs(); /* p = number of processors obtained */
    s = bsp_pid();    /* s = processor number */
    
    bsp_sync();

    Time = vecallocd(p); 
    x = vecallocd(MAXN);
    y = vecallocd(MAXN);
    z = vecallocd(MAXN);
    src = vecallocd(MAXH);
    destproc = (int*) vecallocd(MAXH); 
    destindex = (int*) vecallocd(MAXH); 
    
    t = vecallocd(MAXH+1);

    bsp_push_reg(Time,p*SZDBL);
    bsp_sync();
    dest = vecallocd(2*MAXH+p); 

    bsp_push_reg(dest,(2*MAXH+p)*SZDBL);
    bsp_sync();

    /**** Determine r ****/

    /* Set default rate of 0 */
    r = 0;
    for (n=1; n <= MAXN; n *= 2) {
        /* Initialize scalars and vectors */
        alpha = 1.0f/3.0f;
        beta = 4.0f/9.0f;
        for (i=0; i<n; i++) {
            z[i] = y[i] = x[i] = (float)i;
        }

        /* Measure time of 2*NITERS DAXPY operations of length n */
        time0 = ebsp_host_time();
        for (iter=0; iter<NITERSN; iter++) {
            for (i=0; i<n; i++)
                y[i] += alpha*x[i];
            for (i=0; i<n; i++)        
                z[i] -= beta*x[i];
        }
        time1 = ebsp_host_time();
        time = time1-time0; 
        bsp_hpput(0,&time,Time,s*SZDBL,SZDBL);
        bsp_sync();
    
        /* Processor 0 determines minimum, maximum, average computing rate */
        if (s == 0) {
            mintime = maxtime = Time[0];
            for(s1=1; s1<p; s1++) {
                mintime = MIN(mintime,Time[s1]);
                maxtime = MAX(maxtime,Time[s1]);
            }
            if (mintime > 0.0f) {
                /* Compute r = average computing rate in flop/s */
                nflops = 4*NITERSN*n;
                r = 0.0f;
                for(s1=0; s1<p; s1++)
                    r += nflops/Time[s1];
                r /= (float) p; 

                ebsp_message("n = %5d min = %7.3lf max = %7.3lf av = %7.3lf Mflop/s",
                       n, nflops/(maxtime*MEGA),nflops/(mintime*MEGA), r/MEGA);
            } else {
                ebsp_message("n = %5d unable to compute, at least one core took 0 time", n);
            }
        }
    }

    /**** Determine g and l ****/
    float unbufferedTime;
    for (h=0; h<=MAXH; h++) {
        /* Initialize communication pattern */
        for (i=0; i<h; i++) {
            src[i] = (float)i;
            if (p == 1) {
                destproc[i] = 0;
                destindex[i] = i;
            } else {
                /* destination processor is one of the p-1 others */
                destproc[i] = (s+1 + i%(p-1)) % p;
                /* destination index is in my own part of dest */
                destindex[i] = s + (i/(p-1))*p;
            }
        }

        /* Measure time of NITERS h-relations */
        int total_iters = (int)(NITERSH/(h+1));
        bsp_sync(); 
        time0 = ebsp_host_time(); 
        for (iter=0; iter<total_iters; iter++) {
            for (i=0; i<h; i++)
                bsp_hpput(destproc[i], &src[i], dest, destindex[i]*SZDBL, SZDBL);
            bsp_sync(); 
        }
        time1 = ebsp_host_time();
        time = time1 - time0;
 
        /* Compute time of one h-relation */
        if (s == 0) {
            t[h] = (time*r)/(float)total_iters;
            ebsp_message("Time of %5d-relation = %lf sec = %8.0lf flops (bsp_hpput)",
                   h, time/NITERSH, t[h]);
        }

        unbufferedTime = time;
        /* Measure time of NITERS h-relations using buffered put */
        bsp_sync(); 
        time0 = ebsp_host_time(); 
        for (iter=0; iter<total_iters; iter++) {
            for (i=0; i<h; i++)
                bsp_put(destproc[i], &src[i], dest, destindex[i]*SZDBL, SZDBL);
            bsp_sync(); 
        }
        time1 = ebsp_host_time();
        time = time1 - time0;
 
        /* Compute time of one h-relation */
        if (s == 0) {
            t[h] = (time*r)/(float)total_iters;
            ebsp_message("Time of %5d-relation = %lf sec = %8.0lf flops (bsp_put  ) put/hpput = %f",
                   h, time/NITERSH, t[h], time/unbufferedTime);
        }

    }

    if (s == 0) {
        ebsp_message("size of float = %d bytes",(int)SZDBL);
        leastsquares(0, p, t, &g0, &l0); 
        ebsp_message("Range h=0 to p   : g = %.1lf, l = %.1lf",g0,l0);
        leastsquares(p, MAXH, t, &g, &l);
        ebsp_message("Range h=p to HMAX: g = %.1lf, l = %.1lf",g,l);

        int tag;
        tag = 'r';
        ebsp_send_up(&tag, &r, sizeof(float));
        tag = 'g';
        ebsp_send_up(&tag, &g, sizeof(float));
        tag = 'l';
        ebsp_send_up(&tag, &l, sizeof(float));
        tag = 'h';
        ebsp_send_up(&tag, t, MAXH * sizeof(float));
    }
    /* No need tot pop register/free vectors in our implementation... */
    bsp_end();
    
    return 0;
} /* end bspbench */

