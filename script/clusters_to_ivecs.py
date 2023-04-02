import numpy as np
import struct

# clusters is only a single row, with 10 mill data

clusters_load_loc = './data/clusters_10k_sift10m.npy'
clusters_save_loc = './data/clusters_10k_sift10m.ivecs'

clusters = np.load(clusters_load_loc)
print(clusters.shape, "dim=", clusters.shape[1])
ndims = clusters.shape[1]

print(clusters[:5])
out_file = open(clusters_save_loc, 'wb')
for x in clusters:
    out_file.write(struct.pack('<I', ndims))
    for y in x:
        out_file.write(struct.pack('<I', y))
