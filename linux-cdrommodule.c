#include "Python.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>

static PyObject *cdrom_error;

static PyObject *cdrom_toc_header(PyObject *self, PyObject *args)
{
    struct cdrom_tochdr hdr;
    PyObject *cdrom_fileobj;
    int cdrom_fd;

    if (!PyArg_ParseTuple(args, "O!", &PyFile_Type, &cdrom_fileobj))
	return NULL;

    cdrom_fd = fileno(PyFile_AsFile(cdrom_fileobj));

    if (ioctl(cdrom_fd, CDROMREADTOCHDR, &hdr) < 0) {
	PyErr_SetFromErrno(cdrom_error);
	return NULL;
    }
    
    return Py_BuildValue("bb", hdr.cdth_trk0, hdr.cdth_trk1);
}

static PyObject *cdrom_toc_entry(PyObject *self, PyObject *args)
{
    struct cdrom_tocentry entry;
    PyObject *cdrom_fileobj;
    int cdrom_fd;
    unsigned char track;

    if (!PyArg_ParseTuple(args, "O!b", &PyFile_Type, &cdrom_fileobj, &track))
	return  NULL;

    cdrom_fd = fileno(PyFile_AsFile(cdrom_fileobj));

    entry.cdte_track = track;
    entry.cdte_format = CDROM_MSF;

    if (ioctl(cdrom_fd, CDROMREADTOCENTRY, &entry) < 0) {
	PyErr_SetFromErrno(cdrom_error);
	return NULL;
    }

    return Py_BuildValue("bbb", entry.cdte_addr.msf.minute, 
			 entry.cdte_addr.msf.second, entry.cdte_addr.msf.frame);
}

static PyObject *cdrom_leadout(PyObject *self, PyObject *args)
{
    struct cdrom_tocentry entry;
    PyObject *cdrom_fileobj;
    int cdrom_fd;

    if (!PyArg_ParseTuple(args, "O!", &PyFile_Type, &cdrom_fileobj))
	return  NULL;

    cdrom_fd = fileno(PyFile_AsFile(cdrom_fileobj));

    entry.cdte_track = CDROM_LEADOUT;
    entry.cdte_format = CDROM_MSF;

    if (ioctl(cdrom_fd, CDROMREADTOCENTRY, &entry) < 0) {
	PyErr_SetFromErrno(cdrom_error);
	return NULL;
    }

    return Py_BuildValue("bbb", entry.cdte_addr.msf.minute, 
			 entry.cdte_addr.msf.second, entry.cdte_addr.msf.frame);
}

static PyMethodDef cdrom_methods[] = {
    { "toc_header", cdrom_toc_header, METH_VARARGS },
    { "toc_entry", cdrom_toc_entry, METH_VARARGS },
    { "leadout", cdrom_leadout, METH_VARARGS},
    { NULL, NULL }
};

void initcdrom(void)
{
    PyObject *module, *dict;

    module = Py_InitModule("cdrom", cdrom_methods);
    dict = PyModule_GetDict(module);
    cdrom_error = PyErr_NewException("cdrom.error", NULL, NULL);
    PyDict_SetItemString(dict, "error", cdrom_error);
}

