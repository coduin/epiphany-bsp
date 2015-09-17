BSP Model
=========

The Bulk Synchronous Parallel (BSP) model was developed by Leslie Valiant in the 1980s. The BSP model is intended as a bridging model between parallel hardware and software. It is an elegant and simple model that has a small and easy to understand interface.

The BSP model is defined on an abstract computer called a BSP computer. This computer has three important requirements.

1. It has \f$n\f$ processors capable of computation and communication, i.e. it allows for local memory transactions.
2. It has a network in place that allows the different processors to send and receive data.
3. It has a mechanism that allows for the synchronisation of these processors, e.g. by means of a blocking barrier.

A BSP program consists of a number of distinct blocks of computation and communication called _supersteps_. These steps are separated by a barrier synchronisation, and consist of a computation and a communication step.

An important part of a BSP algorithm is the associated cost function. To this end we introduce two important concepts: namely an \f$h\f$-relation, and a notion of the _work_ done by a processor. Furthermore we introduce two parameters that define a BSP computer: \f$g\f$ and \f$l\f$.

An \f$h\f$-relation is a superstep in which each processor sends or receives a maximum of \f$h\f$ words of data. We commonly denote with \f$p\f$ the id of a processor such that we can write for the \f$h\f$-relation:

\f[h = \max_p \left\{ \max \{ (h_p)_\text{sent}, (h_p)_\text{received} \}~|~\text{processors } p \right\} \f]

Where \f$h_p\f$ denotes the number of words received or sent by processor \f$p\f$. Similarly we define the work \f$w\f$ done in a superstep as the maximum number of flops, floating point operations, performed by all processors. Finally we define the latency \f$l\f$ of a superstep as the fixed constant overhead, used primarily to account for the barrier synchronisation. The values for \f$g\f$ and \f$l\f$ are platform-specific constants that are found emperically. The values for \f$w\f$ and \f$h\f$ are superstep specific and commonly obtained analytically. The total BSP cost associated to a BSP algorithm is:

.. :math:: T = \sum_{\text{supersteps } i} (w_i + g \cdot h_i + l)

The BSP model has gained significant interest in the last couple of years. Most notably because Google has adopted the model and has developed some technologies based on BSP such as MapReduce and Pregel. The standard for BSP implementations is [BSPlib][2]. Modern implementations of the BSP model include BSPonMPI, which simulates the BSP model on top of MPI, and MulticoreBSP, which provides a BSP implementation for shared-memory multi-core computers.

For a more detailed introduction on the BSP model, as well as a large number of examples of BSP programs we refer to the [introductory textbook on BSP and MPI][1] by Rob Bisseling.

A large number of algorithms have already been implemented using the BSP model. Some of them with their associated cost function are listed below:

<table>
<tr>
<th>Problem</th>
<th>BSP complexity</th>
</tr>
<tr>
<td>Matrix multiplication</td>
<td>:math:`n^3/p + (n^2/p^{2/3}) \cdot g + l`</td>
</tr>
<tr>
<td>Sorting</td>
<td>:math:`(n \log n)/p + (n/p)\cdot g + l`</td>
</tr>
<tr>
<td>Fast Fourier Transform</td>
<td>:math:`(n \log n)/p + (n/p)\cdot g + l`</td>
</tr>
<tr>
<td>LU Decomposition</td>
<td>:math:`n^3/p + (n^2/p^{1/2})\cdot g + p^{1/2}\cdot l`</td>
</tr>
<tr>
<td>Cholesky Factorisation</td>
<td>:math:`n^3/p + (n^2/p^{1/2})\cdot g + p^{1/2}\cdot l`</td>
</tr>
<tr>
<td>Algebraic Path Problem (Shortest Paths)</td>
<td>:math:`n^3/p + (n^2/p^{1/2})\cdot g + p^{1/2}\cdot l`</td>
</tr>
<tr>
<td>Triangular Solver</td>
<td>:math:`n^2/p + n\cdot g + p\cdot l`</td>
</tr>
<tr>
<td>String Edit Problem</td>
<td>:math:`n^2/p + n\cdot g + p\cdot l`</td>
</tr>
<tr>
<td>Dense Matrix-Vector Multiplication</td>
<td>:math:`n^2/p + (n/p^{1/2})\cdot g+l`</td>
</tr>
<tr>
<td>Sparse Matrix-Vector Multiplication (2D grid)</td>
<td>:math:`n/p + (n/p)^{1/2}\cdot g+l`</td>
</tr>
<tr>
<td>Sparse Matrix-Vector Multiplication (3D grid)</td>
<td>:math:`n/p + (n/p)^{2/3}\cdot g+l`</td>
</tr>
<tr>
<td>Sparse Matrix-Vector Multiplication (random)</td>
<td>:math:`n/p + (n/p)\cdot g+l`</td>
</tr>
<tr>
<td>List Ranking</td>
<td>:math:`n/p + (n/p)\cdot g+(\log p)\cdot l`</td>
</tr>
</table>
(McColl 1998 "Foundations of Time-Critical Scalable Computing")

[1]: http://ukcatalogue.oup.com/product/9780198529392.do
[2]: http://www.bsp-worldwide.org/
