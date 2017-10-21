.. _build:

Block Installation
******************

Source code and binary
======================

* Version 1.5

  - Source code `block-1.5.0.tar.gz <http://www.sunqm.net/pyscf/files/src/block-1.5.0.tar.gz>`_,
  - Source code `block-1.5.0-serial.tar.gz <http://www.sunqm.net/pyscf/files/src/block-1.5.0-serial.tar.gz>`_,
  - Binary `block.spin_adapted-1.5.0.gz <http://www.sunqm.net/pyscf/files/bin/block.spin_adapted-1.5.0.gz>`_,
    1.8 MB, compiled with GCC-4.8, Boost-1.55, OpenMPI-1.10.3, MKL-11
  - Binary `block.spin_adapted-1.5.0-serial.gz <http://www.sunqm.net/pyscf/files/bin/block.spin_adapted-1.5.0-serial.gz>`_
    1.7 MB, compiled with GCC-4.8, Boost-1.55, MKL-11

* Version 1.1.1

  - Binary `block.spin_adapted-1.1.1.gz <http://www.sunqm.net/pyscf/files/bin/block.spin_adapted-1.1.1.gz>`_,
    2.4 MB, compiled with GCC-4.8, Boost-1.55, OpenMPI-1.10.3, MKL-11
  - Binary `block.spin_adapted-1.1.1-serial.gz <http://www.sunqm.net/pyscf/files/bin/block.spin_adapted-1.1.1-serial.gz>`_
    2.2 MB, compiled with GCC-4.8, Boost-1.55, MKL-11

* Version 1.1.0

  - Binary `block.spin_adapted-1.1.0.gz <http://www.sunqm.net/pyscf/files/bin/block.spin_adapted-1.1.0.gz>`_,
    4.3 MB, compiled with GCC-4.8, Boost-1.55, OpenMPI-1.6.5, MKL-11
  - Binary `block.spin_adapted-1.1.0-serial.gz <http://www.sunqm.net/pyscf/files/bin/block.spin_adapted-1.1.0-serial.gz>`_
    2.2 MB, compiled with GCC-4.8, Boost-1.55, MKL-11

Compile
=======

Building Block code requires C++11 compiler,  BLAS, LAPACK and Boost libraries.
For distributed-memory parallel compilation, MPI and multi-threading
Boost-MPI libraries are needed.  When you compile Boost-MPI, you can use
the following flags for ``b2`` the Boost built-in compilation tool to
generate the multi-threading ``-mt`` libraries.::

 ./b2 --layout=tagged link=static,shared threading=multi install

Note Boost and MPI libraries must be compiled using the same compiler as for compiling Block.
See also `boost documents <http://www.boost.org/doc/libs/1_60_0/doc/html/mpi/getting_started.html>`_
for details of the installation of Boost with the MPI components.

To compile Block code, the following customizations need to be made to
the makefile placed in the main directory.::

    CXX = g++
    BOOSTINCLUDE = /lib64/boost_1_55_0/include/
    BOOSTLIB = /lib64/boost_1_55_0/lib/
    OPENMP = yes

Please note that when choosing your compiler, either GNU or Intel, C++0x/C++11 standards must be appropriately supported,
as Block requires new features for some of the modules (eg, :mod:`npdm`, :mod:`nevpt2`, etc).
Here are our suggested `minimum` GNU/Intel compiler versions in order for the compiling process to be successful: 

* GNU ``g++``: 4.8 or newer,
* or Intel ``icpc``: at least 14.0.1 (2013 SP1 Update 1) or newer.

Turn on the MPI support for Block code::

    USE_MPI = yes
    MPICXX = mpicxx

And supply MKL libraries::

    USE_MKL = yes
    MKLLIB = /opt/intel/composer_xe_2013_sp1.0.080/mkl/lib/intel64/`` 
    MKLFLAGS = /opt/intel/composer_xe_2013_sp1.0.080/mkl/include``

For certain compilers, you may have error message::

        error: ‘auto_ptr’ is deprecated (declared at /usr/include/c++/4.8.2/backward/auto_ptr.h:87) [-Werror=deprecated-declarations]

It is caused by the flag ``-Werror``.  It is safe to remove this flag
from ``OPT`` variable.  Some compiler/linker may issue errors if
``OPENMP = yes`` was specified in Makefile::

        /usr/bin/ld: dmrg.o: undefined reference to symbol 'shm_openn@@GLIBC_2.2.5'

Appending ``-lpthread -lrt`` at the end of ``LIBS`` can solve this problem.

When the makefile is configured, run in the directory ``./Block``::

        $ make

The successful compilation generates the executable ``block.spin_adapted``, static and shared DMRG libraries ``libqcdmrg.a`` and ``libqcdmrg.so``.


.. _pyscf-itrf:

Interface to PySCF package
--------------------------

The electronic structure Python module `PySCF <http://chemists.princeton.edu/chan/software/pyscf/>`_
provided an interface to run Block code.  You need create a pyscf
config file ``/path/to/pyscf/future/dmrgscf/settings.py`` and add the
following settings in it::

    BLOCKEXE = "/path/to/Block/block.spin_adapted"
    BLOCKEXE_COMPRESS_NEVPT = "/path/to/serially/compiled/Block/block.spin_adapted"
    BLOCKSCRATCHDIR = "/path/to/scratch"
    MPIPREFIX = "mpirun"

``BLOCKEXE`` is the parallel Block program. Most DMRG calculations (DMRG-CASCI,
DMRG-CASSCF etc) will call this parallel executable through ``mpirun``
interface.  ``BLOCKEXE_COMPRESS_NEVPT`` points to the **serially
compiled** Block executable.  It is only needed by the compressed perturber
NEVPT2 method.  Note the serial version of Block is slightly different to the
parallel Block with compiling flag ``USE_MPI=yes``.  Specifically, NEVPT2
implementation in the serial code is not available in the parallelized code.
Although this Block executable file is not MPI-parallelized, the
DMRG-NEVPT2 program are efficiently parallelized in a different manner.
Note the parameter ``MPIPREFIX`` should be adjusted according to your
job scheduler, eg::

    # For OpenPBS/Torque 
    MPIPREFIX = "mpirun"
    # For SLURM
    MPIPREFIX = "srun"

If calculation is carried out on interactive node, eg with 4 processors,
the setting looks like::

    MPIPREFIX = "mpirun -n 4"

With the Block-PySCF interface, a simple DMRG-SCF calculation can be
input in Python interpereter:: 

    >>> from pyscf import gto, scf, dmrgscf
    >>> mf = gto.M(atom='C 0 0 0; C 0 0 1', basis='ccpvdz').apply(scf.RHF).run()
    >>> mc = dmrgscf.dmrgci.DMRGSCF(mf, 6, 6)
    >>> mc.run()

DMRG-NEVPT2 calculation can be applied::

    >>> from pyscf import mrpt
    >>> mrpt.NEVPT(mc).compress_approx().run()


Run Block in cmdline
====================

The standalone serial code can be executed running::

        $ block.spin_adapted input.dat > output.dat

``input.dat`` is the input file and the output of the program is piped into the output file ``output.dat``.

The MPI parallel mode can be called running::

        $ mpirun -np 4 block.spin_adapted input.dat > output.dat

Testjobs
=========

Tests are placed in the directory ``./Block/dmrg_tests``::

        $ cd dmrg_tests
        $ ./runtest

The tests require Python to be installed on the system.


