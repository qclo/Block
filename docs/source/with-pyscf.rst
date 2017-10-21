.. _dmrg_pyscf:

DMRG for electronic structure calculations
******************************************

Block program supports two executing modes: running standalone through command
line or as a plugin of other quantum chemistry package.  The Python-based
quantum chemistry program package `PySCF <http://www.pyscf.org>`_ provides a
simple solution to run Block program.  It is the recommended way to use
Block program in most scenario.

In PySCF, DMRG program is mainly used as a replacement of Full CI solver for
large active space CASCI or CASSCF problem.  On top of DMRG-CASCI and
DMRG-CASSCF, MPS-PT can be called through Block-PySCF interface.  Using Block
with PySCF,  systems around 50-active-orbital DMRG-CASSCF or 30-active-orbital
MPSPT can be studied in a regular basis.


CASCI/CASSCF in PySCF
=====================
PySCF is a collection Python modules for electronic structure simulation
and theory developing.  In this section, we briefly review the usage of
PySCF.  More usage details of PySCF package can be found in PySCF online
documents http://www.pyscf.org.  If you have PySCF installed and setup
correctly (see http://www.pyscf.org/install.html), you can create a
Python script for CASCI and CASSCF calculation::

    $ cat c2_cas.py
    from pyscf import gto, scf, mcscf
    mol = gto.M(atom="N 0 0 0; N 0 0 1", basis="ccpvdz")
    mf = scf.RHF(mol).run()
    ncas = 6
    nelec_cas = 6
    mc = mcscf.CASCI(mf, ncas, nelec_cas)
    mc.kernel()
    mc = mcscf.CASSCF(mf, ncas, nelec_cas)
    mc.kernel()

The Python script :file:`c2_cas.py` can be executed by Python interpreter in
command line::

    $ python c2_cas.py
    converged SCF energy = -108.929838385609
    CASCI E = -108.980200822148  E(CI) = -11.9360559617961  S^2 = 0.0000000
    CASSCF energy = -109.044401900068

Based on the given active space size and number of correlated electrons, the
CASCI/CASSCF solver by default takes the highest occupied and lowest unoccupied
orbitals to form the active space.  To change the active space, you need prepare
a set of orbitals and reorder the orbitals to place the required active orbitals
in the HOMO and LUMO space.  You can feed the reordered orbitals to function
``mc.kernel(orbs)`` as the initial guess.  CASCI/CASSCF solver will take the
"HOMO/LUMO" orbitals from ``orbs`` as the active space.  It is inconvenient to
prepare the active space through this selecting-then-reordering procedure.
To simplify this procedure, PySCF package provides some helper functions, such
as :meth:`sort_mo`, :meth:`sort_mo_by_irrep`, :meth:`dmet_cas.guess_cas`,
:meth:`atomic_valence` [#]_.
In the following example, we selected 4 :math:`\pi` orbitals and 1 :math:`\sigma`
orbital and 1 :math:`\sigma^*` orbital from mean-field molecular orbitals to
form the active space using the helper function :meth:`sort_mo_by_irrep`.

.. code-block:: python
   :linenos:

    from pyscf import gto, scf, mcscf
    mol = gto.M(atom="N 0 0 0; N 0 0 1", basis="ccpvdz", symmetry=True)
    mf = scf.RHF(mol).run()

    mc = mcscf.CASSCF(mf, 6, 6)
    ncas_by_irreps = {'E1ux':2, 'E1uy':2, 'A1g':1, 'A1u':1}
    orbs = mc.sort_mo_by_irrep(mf.mo_coeff, ncas_by_irreps)
    mc.kernel(orbs)

In the above example, you should read the input as a Python program.  Line 2
creates a molecule and applied mean-field calculation on the molecule.  The
mean-field results are saved in ``mf`` object so that you can access them later.
For example, in line 5, HF MOs ``mf.mo_coeff`` are passed to function
:meth:`mc.sort_mo_by_irrep`.  :meth:`mc.sort_mo_by_irrep` read the configs from
``ncas_by_irreps`` and return the reordered orbitals ``orbs`` which is then fed
to :meth:`mc.kernel` function as the initial guess.  ``mc`` is the CASSCF object
created by ``mcscf.CASSCF`` function.  More options can be specified for ``mc``
object to control the calculation.  For example, you can set the convergence
tolerance ``mc.conv_tol = 1e-6``; require more computation details to be printed
in the output with ``mc.verbose=5``; call :func:`mc.analyze` to print out the
population analysis of the CASCI/CASSCF results.  The FCI solver of CASCI/CASSCF
object is handled by the attribute :attr:`mc.fcisolver`.  You can control the
number of roots to compute by setting ``mc.fcisolver.nroots = 3``, or change the
symmetry of the correlated wave function with ``mc.fcisolver.wfnsym = 'A1u'``::

    from pyscf import gto, scf, mcscf
    mol = gto.M(atom="N 0 0 0; N 0 0 1", basis="ccpvdz", symmetry=True)
    mf = scf.RHF(mol).run()

    mc = mcscf.CASCI(mf, 6, 6)
    ncas_by_irreps = {'E1ux':2, 'E1uy':2, 'A1g':1, 'A1u':1}
    orbs = mcscf.sort_mo_by_irrep(mc, mf.mo_coeff, ncas_by_irreps)
    mc.fcisolver.nroots = 3
    mc.fcisolver.wfnsym = 'a1u'
    mc.kernel(orbs)
    mc.analyze()

Replacing :attr:`mc.fcisolver` with DMRG solver leads to the DMRG-CASCI and
DMRG-CASSCF methods.  But the rest of the input code should be the same to the
regular CASCI/CASSCF calculation.  You need the molecule and the mean-field
objects to create the DMRG-CASCI/DMRG-CASSCF object ``mc``.  You can adjust the
parameters in ``mc`` object to control the DMRG-CASCI/DMRG-CASSCF calculation
and adjust DMRG configs through the :attr:`mc.fcisolver` object.
More CASCI/CASSCF parameters are documented in http://www.pyscf.org/mcscf.html


Setup Block in PySCF package
============================

First is to :ref:`prepare the Block executable binary <build>`.
Next, you need setup the Block runtime environment in PySCF.  In the config file
:file:`/path/to/pyscf/future/dmrgscf/settings.py` (see also the template
:file:`/path/to/pyscf/future/dmrgscf/settings.py.template`), you need specify::

    BLOCKEXE = "/path/to/Block/block.spin_adapted"
    BLOCKEXE_COMPRESS_NEVPT = "/path/to/serially/compiled/Block/block.spin_adapted"
    BLOCKSCRATCHDIR = "/path/to/scratch"
    MPIPREFIX = "mpirun"  # or srun for SLURM system

You need at least set ``BLOCKEXE`` for DMRG-CASCI and DMRG-CASSCF methods.
``BLOCKSCRATCHDIR`` is the directory where to store temporary data and the DMRG
wave function.

.. note::

  Usually, the size of DMRG wave function is very large.  Be sure that the disk
  which ``BLOCKSCRATCHDIR`` pointed to has enough space.

In the input script, you can replace the :attr:`mc.fcisolver` by
`DMRGCI <http://www.pyscf.org/dmrgscf.html#id1>`_ object to call Block program
in CASCI/CASSCF calculation::

    from pyscf import gto, scf, mcscf
    from pyscf import dmrgscf
    mol = gto.M(atom="N 0 0 0; N 0 0 1", basis="ccpvdz", symmetry=True)
    mf = scf.RHF(mol).run()

    mc = mcscf.CASCI(mf, 6, 6)
    ncas_by_irreps = {'E1ux':2, 'E1uy':2, 'A1g':1, 'A1u':1}
    orbs = mcscf.sort_mo_by_irrep(mc, mf.mo_coeff, ncas_by_irreps)
    mc.fcisolver = dmrgscf.DMRGCI(mol)
    mc.fcisolver.nroots = 3
    mc.fcisolver.wfnsym = 'a1u'
    mc.kernel(orbs)
    mc.analyze()

Generally speaking, this simple replacement of :attr:`mc.fcisolver` is enough to
call the DMRG-CASCI and DMRG-CASSCF methods in your calculation.  The rest
settings of the ``mc`` object are all the same to the regular CASCI/CASSCF.
When :func:`mc.kernel` is finished, the CASCI/CASSCF results such as orbital
coefficients, natural occupancy etc. are held in ``mc`` object.  But the DMRG
wave-function is not.  It is stored in the directory specified by the attribute
``DMRGCI.scratchDirectory`` or ``BLOCKSCRATCHDIR`` (the default value) in
the config :file:`pyscf/future/dmrgscf/settings.py`.

To make the embedded DMRG solver work more efficiently in CASSCF optimization,
one needs carefully tune the DMRG parameters and dynamically update the
parameters during the CASSCF optimization.  It requires more codes in the
interface to let CASSCF and DMRG talk to each other.  We provided a shortcut
function :func:`DMRGSCF` in the :mod:`dmrgscf` module to handle this
functionality::

    from pyscf import gto, scf, mcscf
    from pyscf import dmrgscf
    mol = gto.M(atom="N 0 0 0; N 0 0 1", basis="ccpvdz", symmetry=True)
    mf = scf.RHF(mol).run()

    mc = dmrgscf.DMRGSCF(mf, 6, 6)
    ncas_by_irreps = {'E1ux':2, 'E1uy':2, 'A1g':1, 'A1u':1}
    orbs = mcscf.sort_mo_by_irrep(mc, mf.mo_coeff, ncas_by_irreps)
    mc.fcisolver.wfnsym = 'a1u'
    mc.kernel(orbs)

We recommend to use :func:`dmrgscf.DMRGSCF` as the entry of DMRG-CASSCF method
whenever is possible.


Control Block program through PySCF wrapper
===========================================

Parallelism
-----------

MPI parallelization parameters for Block are controlled by the variable
``MPIPREFIX`` in :file:`pyscf/future/dmrgscf/settings.py` or the attribute
:attr:`mpiprefix` of :class:`DMRGCI` object.  For example, if you want to run
Block using 4 processors on 2 nodes with Infiniband as the communication layer,
you can specify in the input script::

    mc = dmrgscf.DMRGSCF(mf, 6, 6)
    mc.fcisolver.mpiprefix = 'mpirun -np 4 -npernode --mca btl self,openib'
    mc.kernel()

If you are using `SLURM <https://www.open-mpi.org/faq/?category=slurm>`_ system
for job manager,  you can put ``MPIPREFIX = 'srun'`` in the :file:`settings.py`

To efficiently use memory, starting from Block-1.5, Block code introduces
threading level parallelism, more specifically, the OpenMP threading.  To enable
the multi-threading feature in Block, you need specify the attribute
:attr:`num_thrds` in :class:`DMRGCI` object to indicate the maximum number of
threads to be used by each MPI process::

    mc = dmrgscf.DMRGSCF(mf, 6, 6)
    mc.fcisolver.num_thrds = 4
    mc.kernel()

By default, Block code uses 1 thread in each process.  Using the multi-threading
with the multi-processing model (mpirun -np) potentially offers higher
performance and better scaling for DMRG parallelism.  It is recommended to
enable the multi-threading feature if your block program is newer than version
1.5.

On SLURM job system, the hybrid parallelism settings are controlled by
`SLURM runtime environment variables <http://slurm.schedmd.com/sbatch.html#lbAG>`_.
You can control the parallel model by either configuring the resources through
the ``#SBATCH`` flags or setting the ``$SLURM_XXX`` variables in the SLURM
script.  For example, the following slurm script allocated in total 32 CPUs
which are distributed in 8 processes on 2 nodes::

    #SBATCH --nodes=2
    #SBATCH --ntasks-per-node=4
    #SBATCH --cpus-per-task=4

    python c2_cas.py

Specifying ``mc.fcisolver.mpiprefix = 'srun'`` will use SLURM to lanuch the
Block program which will be executed on 2 nodes with 4 processes on each node.
Note Block program does not detect the environment and setup the multi-threading
automatically.  You still need explicitly set ``mc.fcisolver.num_thrds = 4`` in
the PySCF input script to turn on the multi-threading for Block program.


Bond dimension and sweep scheduler
----------------------------------

Depending on the system, you may need change the DMRG bond dimension to improve
the accuracy or balance the accuracy and efficiency.  The default bond dimension
is 1000.  You can change the bond dimension by setting :attr:`fcisolver.maxM`::

    from pyscf import gto, scf, mcscf
    from pyscf import dmrgscf
    mol = gto.M(atom="N 0 0 0; N 0 0 1", basis="ccpvdz", symmetry=True)
    mf = scf.RHF(mol).run()

    mc = dmrgscf.DMRGSCF(mf, 6, 6)
    mc.fcisolver.maxM = 50
    mc.kernel()

Generally, other default scheduler implemented in the PySCF wrapper should work
fine in most systems.  You can adjust the sweep schedule through the
:class:`DMRGCI` object::

    dmrgsolver.scheduleSweeps = [0, 4, 8, 12, 16, 20, 24, 30]
    dmrgsolver.scheduleMaxMs  = [200, 400, 800, 1200, 2000, 2000, 2000, 2000]
    dmrgsolver.scheduleTols   = [0.0001, 0.0001, 0.0001, 0.0001, 1e-5, 1e-7, 1e-7, 1e-7]
    dmrgsolver.scheduleNoises = [0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0, 0.0]
    dmrgsolver.twodot_to_onedot = 34
    dmrgsolver.maxIter = 50

The first four attributes which prefixed with ``schedule`` will be converted to
the ``schedule`` section in the Block config file::

    schedule 
      0  200   0.0001  0.0001
      4  400   0.0001  0.0001
      8  800   0.0001  0.0001
      12 1200  0.0001  0.0001
      16 2000  1e-5    0.0001
      20 2000  1e-7    0.0001
      24 2000  1e-7    0.0
      30 2000  1e-7    0.0
    end

In the early stage of Block sweep, the wave function is easy to stuck at local
minimum.  Although less efficient and accurate, applying the two-dot algorithm
can effectively help DMRG solver moving out of the local minimum.  Attribute
:attr:`twodot_to_onedot` indicates when to switch to the one-dot algorithm which
is efficient and stable to converge.


DMRGCI functions and attributes
-------------------------------

.. class:: DMRGCI

  The interface of Block and PySCF.  The class exposes the Block keywords to
  PySCF so that the Block code can be run and controlled in Python script.

  .. attribute:: approx_maxIter

    In 1-step DMRG-CASSCF algorithm, the number of sweeps during the approximate
    FCI/DMRG updating.  Default is 4

  .. attribute:: block_extra_keyword

    It allows you to input Block keywords which were not exposed in :class:`DMRGCI`
    class.  Some commonly used keywords include
    
      | warmup local_2site
      | nonspinadapted
      | fiedler

    See :ref:`keywords_list` for the details of Block code keywords

  .. attribute:: configFile
  
    By default, keywords are written to file dmrg.conf

  .. attribute:: dmrg_switch_tol

    In 1-step DMRG-CASSCF, when the orbital gradients is smaller than this
    value, the DMRG calculation starts to read the solution from previous step
    as the initial guess (to reduce the computational cost).  Default is 1e-3.

  .. attribute:: executable

    Default is settings.BLOCKEXE

  .. attribute:: integralFile

    The file to store FCIDUMP.  Default is FCIDUMP

  .. attribute:: maxIter

    Max number of sweeps

  .. attribute:: memory

    The maximum memory (in GB) to use.  Default is 2 GB.  When you enabled multi
    threading and had large bond dimensions :attr:`maxM`, you might need
    more memory to hold the intermediates.  Generally, large memory is helpful
    to improve efficiency.

    .. versionadded:: Block-1.5 (stackblock)

  .. attribute:: mpiprefix

    Default is settings.MPIPREFIX

  .. attribute:: nroots

    Number of states to solver simultaneously.

  .. attribute:: num_thrds

    Number of OpenMP threads to be used in each MPI process.  Default is 1.

  .. attribute:: outputFile

    Block output.  Default is dmrg.out

  .. attribute:: outputlevel

    0 (less output) to 3 (very noise).  Default is 2.

  .. attribute:: restart

    Whether to read the wave function from the temporary directory (specified by
    :attr:`scratchDirectory`) as the initial guess.

    .. note::

      Block code does not check whether the system of the existed wave funciton
      matches the one in study.  A mismatched DMRG wave function (from wrong
      :attr:`DMRGCI.scratchDirectory`) may lead to wrong solution or cause
      DMRG program crash.

  .. attribute:: runtimeDir

    Where to put files dmrg.conf, dmrg.out etc temporarily.  Default is current
    directory (where you execute python).

  .. attribute:: scratchDirectory

    The directory where to store the intermediates and wave functions.  Default
    is settings.BLOCKSCRATCHDIR.

    .. note::
   
      Be sure ``mc.fcisolver.scratchDirectory`` is properly assigned.
      Since all DMRGCI object by default uses the same ``BLOCKSCRATCHDIR`` settings,
      it's easy to cause name conflicts on the scratch directory, especially when
      two DMRG-CASSCF calculations are run on the same node.

  .. attribute:: spin

    2S (= nelec_alpha - nelec_beta).  If the argument ``nelec`` of
    :meth:`DMRGCI.kernel` function is a two-item list to represent the number of
    alpha and beta electrons, the Block program will use the given alpha and
    beta electron numbers to determine the spin.  Otherwise, Block program
    takes this value as the spin of the system.

  .. attribute:: twodot_to_onedot
  
    When to switch to one-dot algorithm.

  .. attribute:: weights

    In state average calculation, the weight assocated to each state.

  .. attribute:: wfnsym

    In the DMRGCI interface, the wave function symmetry ID follows the PySCF
    convention (see http://www.pyscf.org/symm.html).  But Block code follows
    Molpro convention.  A mapping between two symmetry ID is invoked in the
    DMRGCI initialization function.  It is recommended to put the label of wave
    function  (such as 'A1g', 'B2u') here to avoid the ambiguity.

  .. method:: make_rdm1(state, norb, nelec)

    Given state ID, read its 1-particle density matrix from the directory
    indicated by :attr:`scratchDirectory`.

  .. method:: make_rdm12(state, norb, nelec)

    Given state ID, read its 1-particle and 2-particle density matrices from the
    directory indicated by :attr:`scratchDirectory`.  Note the 2-particle
    density matrix is reordered to match the 2e integrals of chemists' notation,
    dm2[p,q,r,s] :math:`= \langle p^\dagger r^\dagger s q\rangle`.

  .. method:: make_rdm123(state, norb, nelec)

    Given state ID, read 1, 2 and 3-particle density matrices from the
    directory indicated by :attr:`scratchDirectory`.  Note the 2-particle
    density matrix is reordered to match the 2e integrals of chemists' notation.
    dm2[p,q,r,s] = :math:`\langle p^\dagger r^\dagger s q\rangle`;  The
    3-particle density matrix takes the similar convention,
    dm3[p,q,r,s,t,u] :math:`= \langle p^\dagger r^\dagger t^\dagger u s q\rangle`.

  .. method:: trans_rdm1(statebra, stateket, norb, nelec)

    Given the state ID of bra and ket, read the 1-particle density matrix from
    the directory indicated by :attr:`scratchDirectory`.

  .. method:: trans_rdm12(statebra, stateket, norb, nelec)

    Given the state ID of bra and ket, read the 1-particle and 2-particle
    density matrices from the directory indicated by :attr:`scratchDirectory`.
    Note the 2-particle density matrix is reordered to match the 2e integrals of
    chemists' notation, dm2[p,q,r,s] :math:`= \langle p^\dagger r^\dagger s q\rangle`.

  .. method:: kernel(h1e, eri, norb, nelec, fciRestart=None, ecore=0)

    The kernel function to call Block program.  "eri" is the array of 2-electron
    integrals (ij|kl).  8-fold permutation symmetry is required.
    The function returns the total energy and the state ID which is
    corresponding to the wave-function files in :attr:`scratchDirectory`.
    If multiple roots were required,  the function returns two lists.  The first
    list is the energy of each state.  The second is a list of state ID.

.. function:: DMRGSCF(mf, norb, nelec)

  Shortcut function to setup CASSCF with the DMRG solver.  The DMRG
  solver is properly initialized in this function so that the 1-step
  algorithm can applied efficiently in DMRG-CASSCF method.

  Examples:

  >>> mol = gto.M(atom='N 0 0 0; N 0 0 1')
  >>> mf = scf.RHF(mol).run()
  >>> mc = DMRGSCF(mf, 4, 4)
  >>> mc.kernel()
  -74.414908818611522


State-average and state-specific DMRG-CASCI/DMRG-CASSCF
=======================================================

State-average and state-specific calculations were also supported in the
DMRG-CASCI/DMRG-CASSCF through the Block-PySCF interface.  The usage is
the same to that in regular CASCI/CASSCF calculation.
:func:`mc.state-average_` function provides the average over the
multiple solutions over a single :attr:`fcisolver`::

    mc = dmrgscf.DMRGSCF(mf, 6, 6)
    # half-half average over ground state and first excited state
    mc.state_average_([0.5, 0.5])
    mc.kernel()

In this example, :func:`DMRGSCF` replaced the :attr:`fcisolver` with the
:class:`DMRGCI` object.  Two DMRG states with the same spin and spatial
(point group) symmetry are computed and half-half averaged.  The two states are
saved on the disk indicated by :attr:`mc.fcisolver.scratchDirectory`.
In many calculations, one would require the state-average for states with
different spin or spatial symmetry.  Multiple FCI/DMRG solvers need to be
created and each solver should handle one particular symmetry.  Function
:func:`mcscf.state_average_mix_` offers this functionality to mix different
solvers in a single :attr:`fcisolver` object::

    from pyscf import gto, scf, mcscf, dmrgscf
    mol = gto.M(atom="N 0 0 0; N 0 0 1", basis="ccpvdz", symmetry=1, verbose=4)
    mf = scf.RHF(mol).run()

    mc = dmrgscf.DMRGSCF(mf, 6, 6)
    weights = [.5, .25, .25]
    solver1 = dmrgscf.DMRGCI(mol)
    solver1.scratchDirectory = '/scratch/solver1'
    solver1.nroots = 1
    solver1.wfnsym = 'a1g'
    solver1.spin = 2  # nelec_alpha - nelec_beta
    solver2 = dmrgscf.DMRGCI(mol)
    solver2.scratchDirectory = '/scratch/solver2'
    solver2.nroots = 2
    solver2.wfnsym = 'a1u'
    mcscf.state_average_mix_(mc, [solver1, solver2], weights)
    mc.kernel()

In this example, one solver for a triplet state of A1g symmetry and another
solver for two singlet states of A1u symmetry are combined into one faked
solver and assigned to :attr:`fcisolver` by :func:`state_average_mix_`.
If the fake solver needs to handle solvers of different spin symmetry, you
need explicitly assign the spin attribute to the solver.  For first solver
``solver1``, ``solver1.spin = 2`` indciates that the number of
alpha electrons is 2 more than the number of beta electrons.
The :meth:`kernel` function of fake solver :attr:`mc.fcisolver` will return 3
states in a list ``[0, 0, 1]``.  The number in the list represents the state
ID in each solver.  The first state (the first 0 in the list) is obtained from
``solver1``.  Its wave-function and density matrices can be found in
``/scratch/solver1``.  The second and third elements of ``[0, 0, 1]`` are the
states obtained from ``solver2``.  The relevant wave functions and density
matrices are all stored in ``/scratch/solver2``.

.. note::

  Block program stores the wave function in :attr:`scratchDirectory`.
  You must assign different :attr:`scratchDirectory` for different DMRG solvers.
  If two Block wave function are put in the same :attr:`scratchDirectory`, the
  solver may crash or produce wrong solution.

State-specific DMRG-CASSCF is the other common calculation one would take.
Setting up state-specific DMRG-CASSCF object is the same to the regular CASSCF
code.  By calling :meth:`mc.state_specific_` function with state ID: 0 for
ground state, 1 for first excited state ..., you can optimize the target state
with DMRG-CASSCF::

    # Optimize the first excited state
    mc = dmrgscf.DMRGSCF(mf, 6, 6)
    mc.state_specific_(state=1)
    mc.kernel()

The :meth:`mc.state_specific_` function can be applied with DMRG-CASCI object as
well.  However, a straightforward solution for DMRG-CASCI is to compute multiple
states simultaneously with attribute :attr:`nroots`::

    mc = mcscf.CASCI(mf, 6, 6)
    mc.fcisolver = dmrgscf.DMRGCI(mol)
    mc.fcisolver.nroots = 5
    mc.kernel()

In PySCF `source code <https://github.com/sunqm/pyscf/blob/master/examples/dmrg>`_,
you can find more examples of state-average and state-specific calculations.


DMRG-NEVPT2
===========

DMRG-NEVPT2 [#]_ calculations are available since Block-1.1.  In Block-1.1, we
implemented the standard DMRG-NEVPT2 method which requires the 4-particle
density matrix.  Computing and storing the 4-particle density matrix is
extremely demanding.  It limits the system size to at most 26 orbitals.
To handle systems with larger active space, we implemented an effective
approximation based on compressed MPS-perturber
technique which can significantly reduce the computation cost.  The MPS-perturber
NEVPT2 implementation requires the `MPI4Py <http://mpi4py.scipy.org>`_ library
and the **serial version** of Block program.  You need set in the config file
:file:`/path/to/pyscf/future/dmrgscf/settings.py` the variable
``BLOCKEXE_COMPRESS_NEVPT``::

    BLOCKEXE_COMPRESS_NEVPT = "/path/to/serially/compiled/Block/block.spin_adapted-serial"

.. note::

  The wavefunction structure from different Block versions are incompatible.  If
  BLOCKEXE for zeroth order wavefunction is set to Block-1.1, the variable
  BLOCKEXE_COMPRESS_NEVPT should also be Block-1.1.  Similarly, Block-1.5
  (stackblock) PT code only compatible with the zeroth order wavefunction of
  Block-1.5 (stackblock).

Now you can use :func:`compress_approx` function to initialize a compressed
pertuber NEVPT2 method.  In the :func:`compress_approx` function, we precomputed
the most demanding intermediates and stored them on disk::

    from pyscf import gto, scf, dmrgscf, mrpt
    mol = gto.M(atom="N 0 0 0; N 0 0 1", basis="ccpvdz")
    mf = scf.RHF(mol).run()
    mc = dmrgscf.dmrgci.DMRGSCF(mf, 6, 6).run()

    mrpt.NEVPT(mc).compress_approx().run()

.. note::

  The compressed NEVPT2 algorithm is also very demanding, especially on the memory
  usage.  It can support up to about 35 orbitals in Block-1.5.  Please refer to
  the :ref:`benchmark` for approximate costs.

If the excitation energy is of interest, we can use DMRG-NEVPT2 to compute the
energy of excited state based on the multiple-root CASCI calculations::

    mc = mcscf.CASCI(mf, 6, 6)
    mc.fcisolver = dmrgscf.DMRGCI(mol)
    mc.fcisolver.nroots = 2
    mc.kernel()
    mrpt.NEVPT(mc, root=0).compress_approx(maxM=100).run()
    mrpt.NEVPT(mc, root=1).compress_approx(maxM=100).run()

In the above example, two NEVPT2 calculations are called separately for
two states which are indicated by the argument ``root=*``.  If the DMRG-NEVPT2
calculations are called based on the state-average DMRG-CASSCF calculation, you
should be very careful with :attr:`scratchDirectory` for the DMRG wave function
that NEVPT2 perturbation is applied on.  In the multiple-solver state-average
DMRG-CASSCF calculation,  you need assign the right :attr:`fcisolver` and state
ID to the ``mc`` object before passing it to :func:`mrpt.NEVPT` method.::

    mc = dmrgscf.DMRGSCF(mf, 6, 6)
    weights = [.5, .25, .25]
    solver1 = dmrgscf.DMRGCI(mol)
    solver1.scratchDirectory = '/scratch/solver1'
    solver1.nroots = 1
    solver1.wfnsym = 'a1g'
    solver1.spin = 2  # nelec_alpha - nelec_beta
    solver2 = dmrgscf.DMRGCI(mol)
    solver2.scratchDirectory = '/scratch/solver2'
    solver2.nroots = 2
    solver2.wfnsym = 'a1u'
    mcscf.state_average_mix_(mc, [solver1, solver2], weights)
    mc.kernel()

    mc.fcisolver = solver1
    mrpt.NEVPT(mc, root=0).compress_approx(maxM=100).run()

    mc.fcisolver = solver2
    mrpt.NEVPT(mc, root=1).compress_approx(maxM=100).run()


Case study
==========

In this example, we computed the lowest excited states with DMRG-CASSCF and
DMRG-NEVPT2 methods.

.. literalinclude::  vocl4.py


Run Block standalone
====================
``Block`` program can be run standalone without the PySCF environment.
In PySCF-1.3,  the DMRG interface provides dry run mode to generate the Block
input config :file:`dmrg.conf` and the integral file
:file:`FCIDUMP`.::

    from pyscf import gto, scf, dmrgscf
    mf = gto.M(atom="N 0 0 0; N 0 0 1", basis="ccpvdz").apply(scf.RHF).run()
    mc = dmrgscf.DMRGSCF(mf, 6, 6)
    dmrgscf.dryrun(mc)

You can execute Block program in command line::

    mpirun -n 2 block.spin_adapted dmrg.conf > dmrg.out

See more examples in Chapter :ref:`standalone`.



.. rubric:: Footnotes

.. [#] E. R. Sayfutyarova, Q. Sun, G. K.-L. Chan, G. Knizia, arXiv:1701.07862 [physics.chem-ph]

.. [#] S. Guo, M. A. Watson, W. Hu, Q. Sun, G. K.-L. Chan, J. Chem.  Theory Comput. 12, 1583 (2016)
