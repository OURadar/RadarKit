#include <Python.h>
#include <stdint.h>

// Wrapper
static PyObject *test(PyObject *self, PyObject *args, PyObject *keywords) {
    printf("Test.\n");
    PyObject *result = NULL;
    return result;
}

// Standard boiler plates
static struct PyMethodDef radarkitMethods = {
    PyModuleDef_HEAD_INIT,
	"radarkit", "Test module"}, -1,

PyMODINIT_FUNC

initradarkit(void) {
	(void) Py_InitModule("radarkit", radarkitMethods);
}
