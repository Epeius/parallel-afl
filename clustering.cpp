#include <iostream>
#include <map>
#include <Python.h>

extern "C" {
#include "config.h"
#include "clustering.h"
}

typedef std::map<u32, u32> ClusterResult;
using namespace std;

u8 clustering_worker(string, u32, ClusterResult &);
PyObject *moduleName = NULL;
PyObject *pModule = NULL;
PyObject *pv = NULL;

void clustering_queue(u32 n_clusters)
{
    ClusterResult cr;
    u8 succeed = clustering_worker("/home/binzhang/fuzz/test/libpng/bitmaps", n_clusters, cr);
    if (!succeed) {
        printf("Clustering failed!\n");
        return;
    }

    FILE *f_cluster = fopen("/tmp/clustering.res", "w");
    if (!f_cluster) {
        printf("Cannot open clutering file!\n");
        return;
    }

    auto it = cr.begin(), end = cr.end();
    for (; it != end; it++) {
        char map[128];
        memset(map, 0, 128);
        sprintf(map, "%u:%d\n", it->first, it->second);
        fwrite(map, sizeof(u8), strlen(map), f_cluster);
    }

    fclose(f_cluster);
}

void init_python_env()
{
    u8 verbose = 0;
#if 0 
    verbose = 1;
#endif

    Py_Initialize();
    string path = "./bitmap_clustering";
    string chdir_cmd = string("sys.path.append(\"") + path + "\")";
    const char* cstr_cmd = chdir_cmd.c_str();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("import os");
    PyRun_SimpleString(cstr_cmd);

    // Disable python's output
    PyRun_SimpleString("fnull = open('/dev/null','a')");
    PyRun_SimpleString("sys.stdout = fnull");
    PyRun_SimpleString("sys.stderr = fnull");

    moduleName = PyString_FromString("clustering_worker");

    pModule = PyImport_Import(moduleName);

    if (!pModule)
    {
        cout << "[ERROR] Python get module failed." << endl;
        exit(0);
    }

    if (verbose)
        cout << "[INFO] Python get module succeed." << endl;

    pv = PyObject_GetAttrString(pModule, "read_and_clustering");

    if (!(pv) || !PyCallable_Check(pv))
    {
        cout << "[ERROR] Can't find funftion (read_and_clustering)" << endl;
        exit(0);
    }

    cout << "[INFO] Get function (read_and_clustering) succeed." << endl;
}

void fini_python_env()
{
    Py_INCREF(pv);
    Py_INCREF(moduleName);
    Py_INCREF(pModule);
    Py_Finalize();
}

/* Real worker, return 1 if succeed. */
u8 clustering_worker(string bitmapDir, u32 nClusters, ClusterResult & cr)
{
    u8 verbose = 0;
#if 1 
    verbose = 1;
#endif
    static u8 init_flag = 0;
    if (!init_flag) {
        init_flag += 1;
        init_python_env();
        atexit(fini_python_env);
    }

    assert(pv);

    PyObject* args = PyTuple_New(2);
    PyObject* arg1 = PyString_FromString(bitmapDir.c_str());
    PyObject* arg2 = PyInt_FromLong(nClusters);
    PyTuple_SetItem(args, 0, arg1);
    PyTuple_SetItem(args, 1, arg2);

    PyObject* pRet = PyObject_CallObject(pv, args);

    /*
     * Python function `clustering` returns a map {cksum: belongs_to},
     * then we can collect it to a map in c++.
     */
    if (pRet) 
    {
        PyObject *pCksum, *pBelongsTo;
        Py_ssize_t pos = 0;
        while (PyDict_Next(pRet, &pos, &pCksum, &pBelongsTo)) {
            string str_cksum = PyString_AsString(pCksum);
            u32 cksum = atoi(str_cksum.c_str());
            u32 belongs_to = PyInt_AsLong(pBelongsTo);
            cr.insert(std::make_pair(cksum, belongs_to));

            Py_DECREF(pCksum);
            Py_DECREF(pBelongsTo);
        }
    }

    return 1;
}
