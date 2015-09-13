/*
File: host_streaming_dot_product.c

This file is part of the Epiphany BSP library.

Copyright (C) 2014 Buurlage Wits
Support e-mail: <info@buurlagewits.nl>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License (LGPL)
as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
and the GNU Lesser General Public License along with this program,
see the files COPYING and COPYING.LESSER. If not, see
<http://www.gnu.org/licenses/>.
*/

#include <host_bsp.h>
#include <host_bsp_inspector.h>
#include <stdlib.h>
#include <stdio.h> 
#include <stdint.h> 
#include <math.h> 

int main(int argc, char **argv)
{
    bsp_init("e_primes.srec", argc, argv);
    bsp_begin(bsp_nprocs());

    // create a table of primes
    int n_pre_primes = 100;
    int* pre_primes = (int*)malloc(sizeof(int) * n_pre_primes);

    int current_n_primes = 0;
    // calculate primes up to sqrt(max_prime)
    // this is a small number of primes, so a slow algorithm is acceptable
    int kaas=0;
    for (int i = 2; current_n_primes < n_pre_primes; ++i)
    {
        int is_prime = 1;
        for (int d = 2; d < (int)(sqrt(i)+1.0); d++)
        {
            if (i%d == 0)
            {
                is_prime = 0;
                break;
            }
        }
        if( !is_prime )
            continue;

        kaas += i;
        pre_primes[current_n_primes] = i;
        current_n_primes++;
    }
    printf("pr %d\n", kaas);

    int max_prime = pre_primes[n_pre_primes-1]*pre_primes[n_pre_primes-1];

    // write prime table to processors

    int range_per_core = (max_prime + bsp_nprocs() - 1) / bsp_nprocs();
    int prime_begin = 0;

    int tag;
    int tagsize = sizeof(int);
    ebsp_set_tagsize(&tagsize);

    for (int pid = 0; pid < bsp_nprocs(); pid++)
    {
        ebsp_send_buffered((void*) pre_primes, pid, n_pre_primes*sizeof(int), 100*sizeof(int));

        int prime_end = prime_begin + range_per_core;
        if (prime_end >= max_prime)
            prime_end = max_prime;

        //this core will look for primes in the interval [prime_begin, prime_end[
        tag = 1;
        ebsp_send_down(pid, &tag, &prime_begin, sizeof(int));
        tag = 2;
        ebsp_send_down(pid, &tag, &prime_end,   sizeof(int));

        prime_begin += range_per_core;
    }

    void** outputs = (void**)malloc(sizeof(void*)*bsp_nprocs()); //TODO handle output & test current code
    for (int pid = 0; pid < bsp_nprocs(); pid++)
    {
        outputs[pid] = ebsp_get_buffered(pid, sizeof(int)*(max_prime/bsp_nprocs()+1), 100*sizeof(int));
    }

    ebsp_spmd();

    for (int pid = 0; pid < bsp_nprocs(); pid++)
    {
        int teller = 0;
        printf("outstream %d; %p\n", pid, outputs[pid]);
        for (int i=0; i<20; i++)
        {
            teller = ((int*) outputs[pid])[i];
            printf("core %02d: output[%02d] = %d\n", pid, i, teller);
        }
    }
    
    // read output
    int packets, accum_bytes;
    ebsp_qsize(&packets, &accum_bytes);

    int status;
    int result;
    int sum = 0;
    printf("proc \t partial_sum\n");
    printf("---- \t -----------\n");
    for (int i = 0; i < packets; i++)
    {
        ebsp_get_tag(&status, &tag);
        ebsp_move(&result, sizeof(int));
        printf("%i: \t %i\n", tag, result);
        sum += result;
    }

    printf("SUM: %i\n", sum);

    free((void*) pre_primes);

    // finalize
    bsp_end();

    return 0;
}

