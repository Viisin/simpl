# SIMPL (SIMPL Is Memory Pool Library)
* Modified from Two Level Segregated Fit(TLSF: http://www.gii.upv.es/tlsf/main/docs)
* Distributed under the BSD License.

Features
--------
* Same strategy for all block sizes: O(1) cost for malloc, free, realloc, memalign.
* Extremely low overhead per allocation (4 Bytes) on x86 and x32, (8 Bytes) on x86-64.
* Dynamic overhead per SIMP, (0.13kB, 1 minimal 12B chunk) to (0.74kB, 1GB memory buffer) on x86 and x32, (0.32kB, 1 minimal 24B chunk) to (1.44kB, 1GB memory buffer) on x86-64.
* Low fragmentation: Immediate coalescing, Good-fit strategy.

Caveats
--------
* No lock implementation.
