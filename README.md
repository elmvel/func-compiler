# A Functional Compiler

A brief implementation of a unique functional language compiler to study backend optimization. 

> [!WARNING]
> The current source code lies in the `g-machine-library` branch.
> The code is not in the master branch as the G-Machine part was to be handwritten,
> but to complete the thesis I had to use a library instead.

## Building

CMake is used to build the project. From the repo directory, use the following commands.

```console
$ mkdir build
$ cd build
$ cmake ..
```

This will build the project. For ease of use when developing, the script `run.sh`
is used to both *compile* the compiler and *run* the compiler.
The usage for doing so can be seen below.

First, return to the working directory.

```console
$ cd ..
```

Then, use the `run.sh` script to build the compiler and run it.

```console
# Basic usage
$ ./run.sh fc        # This will both compile 'fc' and run it

# Run the functional compiler with arguments
$ ./run.sh fc ./examples/ex01_basic.fc -d --ast

# Alternatively, the binary will be within the ./build/ dirrectory
$  ./build/fc        # This will run the binary 'fc' directly
```

If the compiler was given a valid source file, then there will be an executable `f.out` in the working directory.

---

This compiler will produce executables that output a final result to the standard output.
One possible output may be the one below.

```console
$ ./f.out
  GM Output: 1337
```

This is expected when using the compiler normally. The usage of `GM` indicates that the compiler
is using the G-Machine optimized backend. An unoptimized executable, which can be created when using the `--O0` flag, would use `O0` in the output instead.

```console
$ ./run.sh fc ./examples/ex01_basic.fc --O0
$ ./f.out
  O0 Output: 1337
```

