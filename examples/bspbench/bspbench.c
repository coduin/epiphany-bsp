#include "bspedupack.h"
 
/*  This program measures p, r, g, and l of a BSP computer
    using bsp_put for communication.
*/

#define NITERS 100     /* number of iterations */
#define MAXN 1024      /* maximum length of DAXPY computation */
#define MAXH 256       /* maximum h in h-relation */
#define MEGA 1000000.0

int P; /* number of processors requested */

void leastsquares(int h0, int h1, double *t, double *g, double *l){
    /* This function computes the parameters g and l of the 
       linear function T(h)= g*h+l that best fits
       the data points (h,t[h]) with h0 <= h <= h1. */

    double nh, sumt, sumth, sumh, sumhh, a;
    int h;

    nh= h1-h0+1;
    /* Compute sums:
        sumt  =  sum of t[h] over h0 <= h <= h1
        sumth =         t[h]*h
        sumh  =         h
        sumhh =         h*h     */
    sumt= sumth= 0.0;
    for (h=h0; h<=h1; h++){
        sumt  += t[h];
        sumth += t[h]*h;
    }
    sumh= (h1*h1-h0*h0+h1+h0)/2;  
    sumhh= ( h1*(h1+1)*(2*h1+1) - (h0-1)*h0*(2*h0-1))/6;

    /* Solve      nh*l +  sumh*g =  sumt 
                sumh*l + sumhh*g = sumth */
    if(fabs(nh)>fabs(sumh)){
        a= sumh/nh;
        /* subtract a times first eqn from second eqn */
        *g= (sumth-a*sumt)/(sumhh-a*sumh);
        *l= (sumt-sumh* *g)/nh;
    } else {
        a= nh/sumh;
        /* subtract a times second eqn from first eqn */
        *g= (sumt-a*sumth)/(sumh-a*sumhh);
        *l= (sumth-sumhh* *g)/sumh;
    }

} /* end leastsquares */

void bspbench(){
    void leastsquares(int h0, int h1, double *t, double *g, double *l);
    int p, s, s1, iter, i, n, h, destproc[MAXH], destindex[MAXH];
    double alpha, beta, x[MAXN], y[MAXN], z[MAXN], src[MAXH], *dest,
           time0, time1, time, *Time, mintime, maxtime,
           nflops, r, g0, l0, g, l, t[MAXH+1]; 
  
    /**** Determine p ****/
    bsp_begin(P);
    p= bsp_nprocs(); /* p = number of processors obtained */
    s= bsp_pid();    /* s = processor number */
  
    Time= vecallocd(p); bsp_push_reg(Time,p*SZDBL);
    dest= vecallocd(2*MAXH+p); bsp_push_reg(dest,(2*MAXH+p)*SZDBL);
    bsp_sync();

    /**** Determine r ****/

    /* Set default rate of 0 */
    r = 0;
    for (n=1; n <= MAXN; n *= 2){
        /* Initialize scalars and vectors */
        alpha= 1.0/3.0;
        beta= 4.0/9.0;
        for (i=0; i<n; i++){
            z[i]= y[i]= x[i]= (double)i;
        }
        /* Measure time of 2*NITERS DAXPY operations of length n */
        time0=bsp_time();
        for (iter=0; iter<NITERS; iter++){
            for (i=0; i<n; i++)
                y[i] += alpha*x[i];
            for (i=0; i<n; i++)
                z[i] -= beta*x[i];
        }
        time1= bsp_time(); 
        time= time1-time0; 
        bsp_put(0,&time,Time,s*SZDBL,SZDBL);
        bsp_sync();
    
        /* Processor 0 determines minimum, maximum, average computing rate */
        if (s==0){
            mintime= maxtime= Time[0];
            for(s1=1; s1<p; s1++){
                mintime= MIN(mintime,Time[s1]);
                maxtime= MAX(maxtime,Time[s1]);
            }
            if (mintime>0.0){
                /* Compute r = average computing rate in flop/s */
                nflops= 4*NITERS*n;
                r= 0.0;
                for(s1=0; s1<p; s1++)
                    r += nflops/Time[s1];
                r /= p; 
                printf("n= %5d min= %7.3lf max= %7.3lf av= %7.3lf Mflop/s ",
                       n, nflops/(maxtime*MEGA),nflops/(mintime*MEGA), r/MEGA);
                fflush(stdout);
                /* Output for fooling benchmark-detecting compilers */
                printf(" fool=%7.1lf\n",y[n-1]+z[n-1]);
            } else
                printf("minimum time is 0\n"); fflush(stdout);
        }
    }

    /**** Determine g and l ****/
    for (h=0; h<=MAXH; h++){
        /* Initialize communication pattern */
        for (i=0; i<h; i++){
            src[i]= (double)i;
            if (p==1){
                destproc[i]=0;
                destindex[i]=i;
            } else {
                /* destination processor is one of the p-1 others */
                destproc[i]= (s+1 + i%(p-1)) %p;
                /* destination index is in my own part of dest */
                destindex[i]= s + (i/(p-1))*p;
            }
        }

        /* Measure time of NITERS h-relations */
        bsp_sync(); 
        time0= bsp_time(); 
        for (iter=0; iter<NITERS; iter++){
            for (i=0; i<h; i++)
                bsp_put(destproc[i],&src[i],dest,destindex[i]*SZDBL,SZDBL);
            bsp_sync(); 
        }
        time1= bsp_time();
        time= time1-time0;
 
        /* Compute time of one h-relation */
        if (s==0){
            t[h]= (time*r)/NITERS;
            printf("Time of %5d-relation= %lf sec= %8.0lf flops\n",
                   h, time/NITERS, t[h]); fflush(stdout);
        }
    }

    if (s==0){
        printf("size of double = %d bytes\n",(int)SZDBL);
        leastsquares(0,p,t,&g0,&l0); 
        printf("Range h=0 to p   : g= %.1lf, l= %.1lf\n",g0,l0);
        leastsquares(p,MAXH,t,&g,&l);
        printf("Range h=p to HMAX: g= %.1lf, l= %.1lf\n",g,l);

        printf("The bottom line for this BSP computer is:\n");
        printf("p= %d, r= %.3lf Mflop/s, g= %.1lf, l= %.1lf\n",
               p,r/MEGA,g,l);
        fflush(stdout);
    }
    bsp_pop_reg(dest); vecfreed(dest);
    bsp_pop_reg(Time); vecfreed(Time);
 
    bsp_end();
} /* end bspbench */

int main(int argc, char **argv){

    bsp_init(bspbench, argc, argv);
    printf("How many processors do you want to use?\n"); fflush(stdout);
    scanf("%d",&P);
    if (P > bsp_nprocs()){
        printf("Sorry, not enough processors available.\n");
        exit(1);
    }
    bspbench();
    exit(0);

} /* end main */
