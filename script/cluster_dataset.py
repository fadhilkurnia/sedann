import faiss
import numpy as np

dataset_source = './data/sift10m_base.bvecs'
centroids_save_loc = './data/centroids_10k_sift10m.npy'
clusters_save_loc = './data/clusters_10k_sift10m.npy'

def bvecs_read(fname):
    a = np.fromfile(fname, dtype=np.int32, count=1)
    b = np.fromfile(fname, dtype=np.uint8)
    d = a[0]
    return b.reshape(-1, d + 4)[:, 4:].copy()

def ivecs_read(fname):
    a = np.fromfile(fname, dtype='int32')
    d = a[0]
    return a.reshape(-1, d + 1)[:, 1:].copy()

def fvecs_read(fname):
    return ivecs_read(fname).view('float32')


# reading bvecs file
filename = dataset_source
print(">>> start reading bvecs file (", filename, ") ...")
dataset = bvecs_read(filename).astype(np.float32)
print(">>> dataset is loaded")

print(dataset[:10])

print(">>> get the first 10M of vectors")
D = 128
N = 100000010
C = 10000
dataset = dataset[:N]

kmeans = faiss.Kmeans(d=D, k=C, niter=50, nredo=3, verbose=True, seed=354)
print(">>> running kmeans clustering ...")
kmeans.train(dataset)
print(">>> kmeans is done :)")
print("     ", kmeans.obj)
print("     ", kmeans.centroids.shape)

print(">>> saving the centroids")
np.save(centroids_save_loc, kmeans.centroids)

print(">>> assigning all vectors to a cluster")
dists, ids = kmeans.index.search(dataset, 1)
print("     ", ids.shape)
print(">>> saving the clusters")
np.save(clusters_save_loc, ids)

print("D:", kmeans.d)
print("C:", kmeans.k)
print("niter:", kmeans.cp.niter)