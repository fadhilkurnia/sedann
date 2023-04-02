import numpy as np
import struct

centroids_load_loc = './data/centroids_10k_sift10m.npy'
centroids_save_loc = './data/centroids_10k_sift10m.fvecs'

centroids = np.load(centroids_load_loc)
print(centroids.shape, "dim=", centroids.shape[1])
ndims = centroids.shape[1]

print(centroids[0])

out_file = open(centroids_save_loc, 'wb')
for x in centroids:
    out_file.write(struct.pack('<I', ndims))
    for y in x:
        out_file.write(struct.pack('<f', y))
