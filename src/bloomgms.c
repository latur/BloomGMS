#include "Python.h"

#define BYTE 8
#define num unsigned long long int

#include "_reader.c"
#include "_filter.c"

static PyObject *
run_bloomgms(PyObject *self, PyObject *args, PyObject *keywds)
{
    char * src;

    unsigned read_length = 100;
    unsigned quality = 3;

    static char * kwlist[] = {"src", "read", "quality", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "si|i", kwlist,
                                     &src, &read_length, &quality)) return NULL;
    
    if (quality < 3) quality = 3;

    /* --------------------------------------------------------------------- */

    Genome * seq = genome_read(src, read_length);
    Filter * bloom = bloom_init(seq->reads, quality);
    
    for (num i = 0; i < seq->length - read_length; i++) {
        bloom_insert(bloom, seq, i);
    }

    free(seq->sequence);
    free(bloom->first);

    PyObject *l = PyList_New(seq->reads);
    for (num k = 0; k != seq->reads; ++k) {
        PyList_SetItem(l, k, Py_BuildValue("h", bloom_exist(bloom, seq, k) ? 0 : 1));
    }

    free(seq);

    free(bloom->hashes);
    free(bloom->second);
    free(bloom);

    return l;
}

static PyMethodDef bloomgms_methods[] = {
    {"make", (PyCFunction) run_bloomgms, METH_VARARGS | METH_KEYWORDS, "Make raw GMS track"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef bloomgms_module = {
    PyModuleDef_HEAD_INIT, "bloomgms", NULL, -1, bloomgms_methods
};

PyMODINIT_FUNC PyInit_bloomgms(void)
{
    return PyModule_Create(&bloomgms_module);
}