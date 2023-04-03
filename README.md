# SeDANN: Streaming Disk Approximate Nearest Neighbors Search

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

0. Getting the Source Code
    ```
    git clone --recursive https://github.com/fadhilkurnia/sedann.git
    ```

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

3. Install BLAS and LAPACK (For FAISS), also conda (for FAISS in python)
    ```
    sudo apt-get install -y libblas-dev liblapack-dev
    wget https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
    sudo bash Miniconda3-latest-Linux-x86_64.sh
    # make sure conda is installed, you might need to reload your terminal
    # you can also run: source ~/.bashrc
    conda install -c pytorch faiss-cpu
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
    First, download the dataset of a billion vectors.
    ```
    cd data
    wget ftp://ftp.irisa.fr/local/texmex/corpus/bigann_base.bvecs.gz
    wget ftp://ftp.irisa.fr/local/texmex/corpus/bigann_query.bvecs.gz
    gzip -d bigann_base.bvecs.gz
    gzip -d bigann_query.bvecs.gz
    ```
    > Note, downloading the 1B vectors and unziping it will take some times.

    Then run the script to get the first 10M of the 1B vectors in `bigann_base.bvecs`, we call this prefix dataset as SIFT10M:
    ```
    ./build/tools_get_bvecs_prefix ./data/bigann_base.bvecs ./data/sift10m_base.bvecs 10000000
    ```

    Next, generate `c=10000` centroids and assigns each of the 10M vectors into one of the centroid. Thus, a cluster consists of a
    centroid with multiple vectors. The script requires `faiss` installed via `conda`.
    ```
    python3 script/cluster_dataset.py
    ```
    > The clustering script potentially run for hours, depend on how many CPU cores are available. Alternatively, we 
    provided the final centroid and cluster file in [data/centroids_10k_sift10m.fvecs](data/centroids_10k_sift10m.fvecs) and 
    [data/clusters_10k_sift10m.ivecs](data/clusters_10k_sift10m.ivecs).

    In the end, the script produces `centroids_10k_sift10m.npy` and `clusters_10k_sift10m.npy` in `./data` directory. They contain the 10,000 
    centroids and the cluster ID of each vector in the SIFT10M dataset.

    Finally, convert the stored centroids and clusters (`.npy` files) into `.fvecs` and `.ivecs` file so they can be processed in our C++ code.
    ```
    python3 script/centroids_to_fvecs.py
    python3 script/clusters_to_fvecs.py
    ```

    The final results from this step are the centroid and cluster files: `data/centroids_10k_sift10m.fvecs` and `clusters_10k_sift10m.ivecs`.

2. Build the Graph Index for the Centroids

    ```
    cd build
    ./test_faiss_graph
    ```
   
3. Build the B+Tree Index while Calculating the Precomputed Distance (PCD)


4. Receiving Search Requests