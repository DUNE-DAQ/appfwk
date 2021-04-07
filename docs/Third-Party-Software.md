# Dependencies

* Note that all dependency management is currently handled by quick-start.sh. These notes are purely informational; please follow the Software Coordination team's instructions for obtaining the software.

The application framework relies on third party software. The *complete* list of the required packages is:
* *Compiler* - There are no specific requirements for the compiler right now, yet the selected C++ standard is C++17. This puts some constraints on the minimum version of the compiler you might want to use. In case of gcc, that is 7.0. Please note that on the target system where the DAQ applications are supposed to run (CentOS) the default gcc version is usually around 4.8. 
* *Boost libraries* - minimum required version is ???
* *ERS* - Error Reporting Service. This is a package developed by ATLAS. The code can be found in the DUNE-DAQ organisation at this [link](https://github.com/DUNE-DAQ/ers). The package has a dedicated branch to be used fort the application framework installation: ```dune/devel```, make sure to check it out before attempting any compilation. At the moment the compilation of ERS has to be done together with the application framework as shared object libraries are not available. 
* *Trace* - Minimum required version is v3_15_09  Source [here](https://cdcvs.fnal.gov/projects/trace)
* [*cetlib*](https://cdcvs.fnal.gov/redmine/projects/cetlib/wiki) - Tested version is v3_10_00
* *nlohmann_json* - If not available to find_package, the CMake scripts will download the single-header version
* *folly* - Folly has been packaged as a UPS product and is automatically set up by quick-start.sh
```
$ git clone http://cdcvs.fnal.gov/projects/trace-git
$ make OUT=$PWD XTRA_CFLAGS=-std=c11 XTRA_CXXFLAGS=-std=c++11  src_utility src_example_user modules
$ cp -a include/* /path/to/install/include/
#(still not enough: cmake can't find it.  what makes TRACEConfig.cmake???)
```

These are the only strict requirements on the depencies. 
We understand that managing different version of libraries is a nuisance and therefore we provide instructions here so that the required software is managed via Spack. 
Anyone is welcome to contribute with other solutions if any are necessary. 

## Using UPS

As usual when using UPS, retrieve the necessary products (including UPS itself) from https://scisoft.fnal.gov/scisoft

```bash
setup gcc v8_2_0
setup TRACE v3_15_09
setup boost v1_70_0 -qe19:prof
setup cetlib v3_10_00 -qe19:prof
```

Once setup, `cmake;make` will build the software using the setup products.

## Using Spack

Dependencies for the app-framework packages are managed via Spack. The instructions below describe how to install Spack and relevant software packages. It is probably best to do the spack installation outside of MyTopDir.  That way, if you create multiple MyTopDir(s), the same spack installation can be used for all of them.  Also, there seems to be some modification to `$HOME/.spack/linux/compilers.yaml` when we run `spack compiler find`.  With a single spack installation, the modifications to this file don't keep multiplying.
[Another caveat: when these instructions were tried on an AFS disk on the lxplus cluster, the `spack bootstrap` failed because of a failed `chmod 02755` call.  We could look into this problem, or maybe we just avoid AFS.]

<details><summary>Spack Install Instructions</summary>

```bash
# Install Spack
git clone https://github.com/spack/spack.git
cd spack
echo "export SPACK_ROOT=$PWD;. \$SPACK_ROOT/share/spack/setup-env.sh" >setup-env.sh
source setup-env.sh
spack bootstrap

# OPTIONAL: Install FNAL Spack buildcache
spack mirror add  --scope site fnal https://spack-cache-1.fnal.gov/binaries/
spack buildcache keys --install --force
spack buildcache keys --trust

# Install spack_art repository for FNAL-developed products (TRACE & cetlib)
cd var/spack/repos
git clone https://cdcvs.fnal.gov/projects/spack-planning-spack_art spack_art
cd spack_art
git checkout feature/for_dune
cd ../../../etc/spack
cp defaults/repos.yaml .
echo '  - $spack/var/spack/repos/spack_art' >>repos.yaml
cd ../..

# Install dependencies
spack install gcc@8.2.0
spack load gcc@8.2.0
spack compiler find
spack install boost@1.70.0 %gcc@8.2.0
spack install --no-checksum trace@3.15.09 %gcc@8.2.0
spack install cetlib@MVP1a %gcc@8.2.0 # Make sure to check out feature/for_dune in spack_art (see above)
spack install nlohmann-json %gcc@8.2.0
```

</details>

<details><summary>Some details</summary>

### Note 2:

Currently, the fermilab spack cache (https://spack-cache-1.fnal.gov/binaries/build_cache/index.html) doesn't have all of the packages that we want for the flavors that we want.  As these get populated, this cache will become more useful.

### Note 3:

If you lose your connection before completing the spack installation instructions above, you can simply `cd` to the spack installation directory, re-run `source setup-env.sh`, and then continue with the steps that you haven't done already.

### Note 4:

For reference, 

* the spack bootstrap installs zlib, tcl, and environment-modules
* the installation of gcc includes libsigsegv, pkgconf, zlib, m4, nurses, libtool, readline, gdbm, perl, autoconf, automake, gmp, mpfr, isl, and mpc
* the installation of boost includes libiconv, zlib, diffutils, and bzip2
* the installation of trace includes pkgconf, ncurses, readline, gdbm, perl, openssl, cmake, and cetmodules
* the installed products are placed into `$SPACK_ROOT/opt/spack/<arch>/<compiler>/<package>`

</details>

### Before compiling
Before compiling the software you have to make sure that Spack has loaded the correct libraries. You can do it with these commands.

```bash
[MyTopDir]$ export SPACK_ROOT=<your spack root here>
[MyTopDir]$ source $SPACK_ROOT/setup-env.sh
[MyTopDir]$ spack load gcc
[MyTopDir]$ spack load cmake
[MyTopDir]$ spack load boost
[MyTopDir]$ spack load trace
[MyTopDir]$ spack load cetlib
[MyTopDir]$ spack load nlohmann-json
```