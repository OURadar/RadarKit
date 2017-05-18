#include <Python.h>
#include <RadarKit.h>

// Wrapper
static PyObject *rkstruct_test(PyObject *self, PyObject *args, PyObject *keywords) {
    printf("rkstruct_test()\n");

    RKSetWantScreenOutput(true);
    RKShowTypeSizes();

    Py_INCREF(Py_None);
    return Py_None;
}

// Standard boiler plates
static PyMethodDef rkstructMethods[] = {
	{"test", (PyCFunction)rkstruct_test, METH_VARARGS | METH_KEYWORDS, "Test module"},
	{NULL, NULL, 0, NULL}
};

static struct PyModuleDef rkstructmodule = {
    PyModuleDef_HEAD_INIT,
    "rkstruct",
    NULL,
    -1,
    rkstructMethods
};

PyMODINIT_FUNC

PyInit_rkstruct(void) {
    return PyModule_Create(&rkstructmodule);
}
