"""
=========================================================
Comparing different clustering algorithms on toy datasets
=========================================================

This example shows characteristics of different
clustering algorithms on datasets that are "interesting"
but still in 2D. With the exception of the last dataset,
the parameters of each of these dataset-algorithm pairs
has been tuned to produce good clustering results. Some
algorithms are more sensitive to parameter values than
others.

The last dataset is an example of a 'null' situation for
clustering: the data is homogeneous, and there is no good
clustering. For this example, the null dataset uses the
same parameters as the dataset in the row above it, which
represents a mismatch in the parameter values and the
data structure.

While these examples give some intuition about the
algorithms, this intuition might not apply to very high
dimensional data.
"""

"""
This is just a naive example for clustering bitmaps.
It MUST be refactored later!!!
"""
import os
import time
import warnings

import numpy as np
import matplotlib.pyplot as plt

from sklearn import cluster, datasets, mixture
from sklearn.neighbors import kneighbors_graph
from sklearn.preprocessing import StandardScaler
from itertools import cycle, islice

##
# Values need to be set:
#  n_samples



# ===================
# Read bitmap files in byte
# Return [{id: []}]
# ===================
def read_bitmaps(bitmap_path, normalize=False, use_mini=False):
    dataset = {}
    if normalize:
        factor = 255.0
    else:
        factor = 1

    all_files = [f for f in os.listdir(bitmap_path) if os.path.isfile(os.path.join(bitmap_path, f))]
    for bitmap in all_files:
        print "Reading %s ... " % bitmap
        seed_id, ext = os.path.splitext(bitmap)
        if ext != ".fuzz_bitmap":
            continue

        data = list()
        with open(os.path.join(bitmap_path, bitmap), "rb") as binary_file:
            data_string = binary_file.read()
            if use_mini:
                data = [int((ord(ch) != 0)) for ch in data_string]
            else:
                data = [ord(ch)/factor for ch in data_string]

        dataset[seed_id] = data

    return dataset 


def clustering(bitmaps, n_clusters):
    from sklearn import cluster, datasets, mixture
    from sklearn.neighbors import kneighbors_graph
    from sklearn.preprocessing import StandardScaler
    from itertools import cycle, islice

    np.random.seed(0)

    # ============
    # Generate datasets. We choose the size big enough to see the scalability
    # of the algorithms, but not too big to avoid too long running times
    # ============
    n_samples = 1500
    noisy_circles = datasets.make_circles(n_samples=n_samples, factor=.5,
                                          noise=.05)

    bmp_list = list()
    for seed_id in bitmaps.keys():
        bmp = bitmaps[seed_id]
        bmp_list.append(bmp)
    
    bitmap_array = np.array(bmp_list)

    belongs_to = np.zeros(len(bitmap_array))

    bitmap_dataset = (bitmap_array, belongs_to)

    # ============
    # Set up cluster parameters
    # ============
    default_base = {'quantile': .3,
                    'eps': .3,
                    'damping': .9,
                    'preference': -200,
                    'n_neighbors': 10,
                    'n_clusters': 3}

    datasets = [
        (bitmap_dataset, {'damping': .77, 'preference': -240,
                         'quantile': .2, 'n_clusters': n_clusters})
    ]

    for i_dataset, (dataset, algo_params) in enumerate(datasets):
        # update parameters with dataset-specific values
        params = default_base.copy()
        params.update(algo_params)

        X, y = dataset
        # ============
        # Create cluster objects
        # ============
        spectral = cluster.SpectralClustering(
            n_clusters=params['n_clusters'], eigen_solver='arpack',
            affinity="nearest_neighbors",
            n_jobs=-1)

        clustering_algorithms = (
            ('SpectralClustering', spectral),
        )

        for name, algorithm in clustering_algorithms:
            t0 = time.time()

            # catch warnings related to kneighbors_graph
            with warnings.catch_warnings():
                warnings.filterwarnings(
                    "ignore",
                    message="the number of connected components of the " +
                    "connectivity matrix is [0-9]{1,2}" +
                    " > 1. Completing it to avoid stopping the tree early.",
                    category=UserWarning)
                warnings.filterwarnings(
                    "ignore",
                    message="Graph is not fully connected, spectral embedding" +
                    " may not work as expected.",
                    category=UserWarning)
                algorithm.fit(X)

            if hasattr(algorithm, 'labels_'):
                y_pred = algorithm.labels_.astype(np.int)
            else:
                y_pred = algorithm.predict(X)

            # Collect to buckets
            result = {}
            index=0
            for seed_id in bitmaps.keys():
                belongs = y_pred[index]
                result[seed_id] = belongs
                index += 1


            print result
            return result

def read_and_clustering(bitmap_dir, n_clusters):
    start = time.time()
    bitmaps = read_bitmaps(bitmap_dir, use_mini=True)
    result = clustering(bitmaps, n_clusters)
    end = time.time()
    elapsed = end - start
    msg = ("[INFO] Clustering into %d sets consumes %d seconds.\n" % (n_clusters, elapsed))
    with open("./clustering.log", "a") as f_log:
        f_log.write(msg)

    return result


if __name__ == "__main__":
    from sklearn import cluster, datasets, mixture
    from sklearn.neighbors import kneighbors_graph
    from sklearn.preprocessing import StandardScaler
    from itertools import cycle, islice

    print read_and_clustering("/home/binzhang/fuzz/test/libpng/bitmaps", 5)
