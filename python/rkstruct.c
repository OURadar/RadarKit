#include <Python.h>
#include <RadarKit.h>

// Wrappers
static PyObject *PyRKInit(PyObject *self, PyObject *args, PyObject *keywords) {
    RKSetWantScreenOutput(true);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *PyRKTest(PyObject *self, PyObject *args, PyObject *keywords) {
    RKShowTypeSizes();
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *PyRKTestShowColors(PyObject *self, PyObject *args, PyObject *keywords) {
    RKTestShowColors();
    Py_INCREF(Py_None);
    return Py_None;
}

// Standard boiler plates
static PyMethodDef PyRKMethods[] = {
    {"init",       (PyCFunction)PyRKInit,           METH_VARARGS | METH_KEYWORDS, "Init module"},
	{"test",       (PyCFunction)PyRKTest,           METH_VARARGS | METH_KEYWORDS, "Test module"},
    {"showColors", (PyCFunction)PyRKTestShowColors, METH_VARARGS | METH_KEYWORDS, "Color module"},
	{NULL, NULL, 0, NULL}
};

static struct PyModuleDef PyRKModule = {
    PyModuleDef_HEAD_INIT,
    "rk",
    NULL,
    -1,
    PyRKMethods
};

PyMODINIT_FUNC

PyInit_rkstruct(void) {
    return PyModule_Create(&PyRKModule);
}
