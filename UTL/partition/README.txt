
PARTITION
=========

Author: Wenqing Wang
Licence: None yet
Copyright:  Wenqing Wang

#TODO:

  o  Add Windows install details
  o  Default destination of output files should be the current working
     directory, not the one where the .msh file starts from
  o  Pipes for input and output


DESCRIPTION:

A tool for converting a GeoSys mesh into a METIS format mesh.

INSTALL:

On Linux type:

$: make

On Windows: # TODO


USAGE:

    partion [filename without extension] [-][integer]

Assuming you have a file called foo.msh :

$: ./partition foo 2  # to write METIS format mesh file (foo.mesh)

...then use METIS to (see documents in SEE ALSO)...

$ ./partition foo -[n] # where n is the number of domains


Descriptions of the usage are given in the documents mentioned below
under SEE ALSO.


SEE ALSO:

This directory contains the source code for the mesh partitioning tool
referred to in the attachment on the Wiki page (linked to from the
front page by the text "How to compile source code and run simulation
on Linux/UNIX", titled "A short description of how to compile GeoSys
and run benchmarks under UNIX/Linux").

The URL to the attachment "Parallel Simulation by Rockflow/GeoSys:Howto" is:
https://geosys.ufz.de/trac/attachment/wiki/LinuxPage/Parallel%20simulation%20by%20GeoSys.pdf

Also referred to by the page at
https://geosys.ufz.de/trac/wiki/DomainDecomposition



Myles English, 2008-10-23
