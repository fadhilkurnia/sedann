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

______________
# Getting Started

1. Install CMake
    ```
    wget https://github.com/Kitware/CMake/releases/download/v3.26.2/cmake-3.26.2-linux-x86_64.sh
    sudo chmod +x cmake-3.26.2-linux-x86_64.sh
    sudo sh cmake-3.26.2-linux-x86_64.sh --prefix=/opt/cmake
    sudo ln -s /opt/cmake/cmake-3.26.2-linux-x86_64/bin/cmake /usr/local/bin/cmake
    ```

2. Install Boost
    ```
    sudo apt-get update
    sudo apt-get install -y libboost-all-dev
    ```

3. Install BLAS and LAPACK (For FAISS)
    ```
    sudo apt-get install -y libblas-dev liblapack-dev
    ```

4. Compiling the project
    ```
    cd build
    cmake ..
    make
    ```
    This results in binary file of `sedann` and `test_bplustree`.

______________
# Handle Search Operation

1. Get the dataset (SIFT10M)
   First, download the dataset of 100 millions vectors.
   ```
    cd data
    wget ftp://ftp.irisa.fr/local/texmex/corpus/bigann_learn.bvecs.gz
   ```
   Then run a script to get the first 10 millions vectors from the downloaded dataset for our measurement. The script also 
   generates `c=10000` centroids and assigns each of the 10M vectors into one of the centroid. Thus, a cluster consists of a
   centroid with multiple vectors. The script requires `faiss` installed via `conda`.
   ```
    python3 script/cluster_dataset.py
   ```
    In the end, the script produces `centroids_10k_sift10m.npy` and `clusters_10k_sift10m.npy` which contain the 10,000 
    centroids and the cluster ID of each vector in the SIFT10M dataset.

    > We potentially change this SIFT10M (extracted from bigann_learn.bvecs) with the first 10M vectors in `bigann_base.bvecs.gz` since it already provide the ground truth.

2. Build the Graph Index for the Centroids
   
   
3. Build the B+Tree Index while Calculating the Precomputed Distance (PCD)


4. Receiving Search Requests