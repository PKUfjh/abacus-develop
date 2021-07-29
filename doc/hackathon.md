# Introduction to ABACUS hackathon environment

**Notice**: Navigate to our [hackathon github page](https://github.com/deepmodeling/abacus-develop/tree/hackathon2021) for hackathon target examples, latest information and full documentation.

## Preface

In this environment, we have prepared all requirements to build ABACUS. We also built an original version of ABACUS at `~/abacus-develop/build/abacus`. 

## Too Long; Don't Read

### Configure

```(bash)
cd ~/abacus-develop
mkdir build
cmake -B build
```

### Build

```(bash)
cmake --build build -j16
```

### Install

```(bash)
cmake --install build
```

**Note**: this step may require `sudo` priviledge for installing `abacus` at default location `/usr/local/bin`.

### Test

```(bash
ctest --test-dir build --verbose
```

### Compute:

```bash
cd ~/abacus-develop/examples/01a_Si_diamond_pw_scf # the directory with INPUT file
mpirun -np 8 abacus
```



## Configure, Build and Install

We recommend building ABACUS with `cmake` to avoid dependency issues.  For furthur information, see [Install with CMake](https://github.com/deepmodeling/abacus-develop/blob/reconstruction/doc/install.md).

### Configure

Specify the path of ABACUS binary install destination by passing `CMAKE_INSTALL_PREFIX` to `cmake`:

```bash
ABACUS_BIN_PATH=~/abacus-develop
cmake -B build -DCMAKE_INSTALL_PREFIX=${ABACUS_BIN_PATH}
```

The binary will be installed to `/usr/local/bin/abacus` by default.

Specify path to requirements if you wish to use your own library. Options`LAPACK_DIR`, `SCALAPACK_DIR`, `ELPA_DIR`, `FFTW3_DIR`, `CEREAL_INCLUDEDIR`, `BOOST_INCLUDEDIR`, `MPI_CXX_COMPILER` and `MKL_DIR`  are available. 

```
cmake -B build -DFFTW3_ROOT=/opt/fftw3 -DBOOST_INCLUDEDIR=/usr/include/boost
```

### Build

After configuring, start building by the commands below.

```
cmake --build build
```

This operation is equivalant to perform `make` in `build/`.

```bash
cd build/
make -j
```

`make` will generate `abacus` executable if no error occurs.

### Install

This step installs `abacus` from `build/` to `ABACUS_BIN_PATH` set in [configuration part](#Configure, Build and Install).

```(bash)
cmake --install build
```

It is optional if `abacus` was invoked as `./build/abacus` .

To test whether the installation is correct, run:

```bash
ctest --test-dir build --verbose
```

This process will invoke `tests/Autotest.sh` . The shell script will compute some examples, and compare to canonical result. It may take minutes to complete the whole process. 

## Develop

After changing source file, perform **build** and **install** steps, and test on modified `abacus`. 

Changing library requires to perform **configure** step to generate new `Makefile` for the project.

