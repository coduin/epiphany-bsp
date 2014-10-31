# Introduction

The [Bulk Synchronous Parallel (BSP)](http://en.wikipedia.org/wiki/Bulk_synchronous_parallel)
computing model is a model for designing parallel algorithms. It provides
a description of how parallel computation should be 
carried out. Programs written with this model consist of
a number of supersteps, which in turn consist of a local 
computations and non-blocking communication. A (barrier)
synchronisation at the end of such a step is used to guarentee
occurance of all communications within a step.

This library provides an implementation of the model on top of the Epiphany SDK. 
This allows the BSP computing model to be used with the Epiphany
architecture developed by [Adapteva](http://www.adapteva.com).
In particular this library has been tested on the 
[Parallela](http://www.paralla.org) board.

# Building

Currently this is still a very early work in progress, and
build methods are still undergoing heavy changes. Feel free
to try and get things running yourself.
