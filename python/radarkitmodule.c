#include <Python.h>

// Wrapper
static PyObject *radarkit_test(PyObject *self, PyObject *args, PyObject *keywords) {
    printf("Test.\n");
    //PyObject *result = NULL;
    Py_INCREF(Py_None);
    return Py_None;
}

// Standard boiler plates
static PyMethodDef radarkitMethods[] = {
	{"test", (PyCFunction)radarkit_test, METH_VARARGS | METH_KEYWORDS, "Test module"},
	{NULL, NULL, 0, NULL}
};

static struct PyModuleDef radarkitmodule = {
    PyModuleDef_HEAD_INIT,
    "radarkit",
    NULL,
    -1,
    radarkitMethods
};

PyMODINIT_FUNC

//initradarkit(void) {
//	(void) Py_InitModule("radarkit", radarkitMethods);
//}
PyInit_radarkit(void) {
    return PyModule_Create(&radarkitmodule);
}
