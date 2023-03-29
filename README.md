# SeDANN: Scalable Disk Approximate Nearest Neighbors Search

Project structure:
```
|- build        # place for makefile and the compiled binary
|- data         # contains datasets, e.g sift1m
|- include      # included libary
|- src          # the main source code
```

Working machine:
```
OS: Linux
Arch: AMD64 with AVX2 (256 bit) support
```
Note: ARM with NEON (e.g Apple M1 chip) is not supported for now.

Dependencies:
- Boost

Compiling the project:
```
cd build
cmake ..
make
```
The compilation produces a single binary file named `sedann`.