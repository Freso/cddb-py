#include "mci.h"
#include <windows.h>
#include <mmsystem.h>

#include <C:\Programme\Python\include\Python.h>

/* 
 * mci.c 
 * micSendString implementation
 * Purpose:
 * Python extension module for reading in audio CD-ROM data in wi
 *
 * Written 6 Dez 1999 by Frank David <f.david@digitecgmbh.de>
 */

static PyObject *mci_error;

#define MCI_STRING_LEN  1000

static PyObject *mci_mciSendString(PyObject *self, PyObject *args)
{
    char resultStr[MCI_STRING_LEN+1];
    PyObject *pyStr = 0;
    if (!PyArg_ParseTuple(args, "O!", &PyString_Type, &pyStr))
	   return NULL;
    // winows mciSendString see cdrom.py for Samples (or MSDN for complete doc)
    mciSendString( PyString_AsString(pyStr), resultStr,MCI_STRING_LEN,0);
    return Py_BuildValue("s", resultStr);
}


static PyMethodDef mci_methods[] = {
    { "mciSendString", mci_mciSendString, METH_VARARGS },
    { NULL, NULL }
};

void initmci(void)
{
    PyObject *module, *dict;

    module = Py_InitModule("mci", mci_methods);
    dict = PyModule_GetDict(module);
    mci_error = PyErr_NewException("mci.error", NULL, NULL);
    PyDict_SetItemString(dict, "error", mci_error);
}

