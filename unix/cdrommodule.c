/*
 * cdrommodule.c 
 * Python extension module for reading in audio CD-ROM data
 *
 * Please port me to other OSes besides Linux, Solaris, OpenBSD,
 * and FreeBSD! 
 *
 * See the README for info.
 *
 * Written 17 Nov 1999 by Ben Gertzfield <che@debian.org>
 * This work is released under the GNU GPL, version 2 or later.
 *
 * FreeBSD support by Michael Yoon <michael@yoon.org>
 * OpenBSD support added by Alexander Guy <alex@andern.org>
 * Darwin/MacOS X support added by Andre Beckedorf <andre@beckedorf.net>
 *
 * Thanks to Viktor Fougstedt <viktor@dtek.chalmers.se> for info
 * on the <sys/cdio.h> include file to make this work on Solaris!
 *
 * Release version 1.4
 */

#include "Python.h"
#include <fcntl.h>
#include <sys/ioctl.h>

#ifdef __linux__
#include <linux/cdrom.h>
#endif

#if defined(sun) || defined(__FreeBSD__) || defined(__OpenBSD__)
#include <sys/cdio.h>
#endif

#if defined(__APPLE__)
#include <sys/types.h>
#include <IOKit/storage/IOCDTypes.h>
#include <IOKit/storage/IOCDMediaBSDClient.h>
#endif

/* 
 * Since FreeBSD has identical support but different names for lots
 * of these structs and constants, we'll just #define CDDB_WHATEVER
 * so that we don't have to repeat the code.
 */

#ifdef __FreeBSD__

#define CDDB_TOC_HEADER_STRUCT ioc_toc_header 
#define CDDB_STARTING_TRACK_FIELD starting_track 
#define CDDB_ENDING_TRACK_FIELD ending_track
#define CDDB_READ_TOC_HEADER_FLAG CDIOREADTOCHEADER 
#define CDDB_TOC_ENTRY_STRUCT ioc_read_toc_single_entry 
#define CDDB_TRACK_FIELD track 
#define CDDB_FORMAT_FIELD address_format 
#define CDDB_MSF_FORMAT CD_MSF_FORMAT 
#define CDDB_ADDR_FIELD entry.addr 
#define CDDB_READ_TOC_ENTRY_FLAG CDIOREADTOCENTRY 
#define CDDB_CDROM_LEADOUT 0xaa 
#define CDDB_DEFAULT_CDROM_DEVICE "/dev/cdrom"
#define CDDB_DEFAULT_CDROM_FLAGS 0

#elif defined(__OpenBSD__)

#define CDDB_TOC_HEADER_STRUCT ioc_toc_header
#define CDDB_STARTING_TRACK_FIELD starting_track
#define CDDB_ENDING_TRACK_FIELD ending_track
#define CDDB_READ_TOC_HEADER_FLAG CDIOREADTOCHEADER
#define CDDB_TOC_ENTRY_STRUCT ioc_read_toc_entry
#define CDDB_TRACK_FIELD starting_track
#define CDDB_FORMAT_FIELD address_format
#define CDDB_MSF_FORMAT CD_MSF_FORMAT
#define CDDB_ADDR_FIELD data->addr
#define CDDB_READ_TOC_ENTRY_FLAG CDIOREADTOCENTRIES
#define CDDB_CDROM_LEADOUT 0xaa
#define CDDB_DEFAULT_CDROM_DEVICE "/dev/cdrom"
#define CDDB_DEFAULT_CDROM_FLAGS 0

#elif defined (__APPLE__) /* Darwin and MacOS X */

#define CDDB_TOC_HEADER_STRUCT CDDiscInfo
#define CDDB_STARTING_TRACK_FIELD numberOfFirstTrack
#define CDDB_ENDING_TRACK_FIELD lastTrackNumberInLastSessionLSB
#define CDDB_TOC_ENTRY_STRUCT CDTrackInfo
#define CDDB_DEFAULT_CDROM_DEVICE "/dev/disk1"
#define CDDB_DEFAULT_CDROM_FLAGS O_RDONLY | O_NONBLOCK

#else /* Linux and Solaris */

#define CDDB_TOC_HEADER_STRUCT cdrom_tochdr 
#define CDDB_STARTING_TRACK_FIELD cdth_trk0 
#define CDDB_ENDING_TRACK_FIELD cdth_trk1 
#define CDDB_READ_TOC_HEADER_FLAG CDROMREADTOCHDR 
#define CDDB_TOC_ENTRY_STRUCT cdrom_tocentry
#define CDDB_TRACK_FIELD cdte_track 
#define CDDB_FORMAT_FIELD cdte_format 
#define CDDB_MSF_FORMAT CDROM_MSF 
#define CDDB_ADDR_FIELD cdte_addr 
#define CDDB_READ_TOC_ENTRY_FLAG CDROMREADTOCENTRY 
#define CDDB_CDROM_LEADOUT CDROM_LEADOUT

#ifdef sun
#define CDDB_DEFAULT_CDROM_DEVICE "/dev/vol/aliases/cdrom0"
#else
#define CDDB_DEFAULT_CDROM_DEVICE "/dev/cdrom"
#endif /* sun */

#define CDDB_DEFAULT_CDROM_FLAGS O_RDONLY | O_NONBLOCK

#endif /* __FreeBSD__ */

static PyObject *cdrom_error;

static PyObject *cdrom_toc_header(PyObject *self, PyObject *args)
{
    struct CDDB_TOC_HEADER_STRUCT hdr;
#if defined(__APPLE__)
    dk_cd_read_disc_info_t discInfoParams;	
#endif
	
    PyObject *cdrom_fileobj;
    int cdrom_fd;

    if (!PyArg_ParseTuple(args, "O!", &PyFile_Type, &cdrom_fileobj))
	return NULL;

    cdrom_fd = fileno(PyFile_AsFile(cdrom_fileobj));

#if defined(__APPLE__)
    memset(&discInfoParams, 0, sizeof(discInfoParams));
    discInfoParams.buffer = &hdr;
    discInfoParams.bufferLength = sizeof(hdr);
	
    if (ioctl(cdrom_fd, DKIOCCDREADDISCINFO, &discInfoParams) < 0) {
#else /* not defined(__APPLE__) */
    if (ioctl(cdrom_fd, CDDB_READ_TOC_HEADER_FLAG, &hdr) < 0) {
#endif
	PyErr_SetFromErrno(cdrom_error);
	return NULL;
    }
    
    return Py_BuildValue("bb", hdr.CDDB_STARTING_TRACK_FIELD, 
			 hdr.CDDB_ENDING_TRACK_FIELD);
}

static PyObject *cdrom_toc_entry(PyObject *self, PyObject *args)
{
    struct CDDB_TOC_ENTRY_STRUCT entry;
    PyObject *cdrom_fileobj;
    int cdrom_fd;
    unsigned char track;

#if defined(__OpenBSD__)
    struct cd_toc_entry data;
#elif defined(__APPLE__)
    dk_cd_read_track_info_t trackInfoParams;
    CDMSF trackMSF;
#endif

    if (!PyArg_ParseTuple(args, "O!b", &PyFile_Type, &cdrom_fileobj, &track))
	return  NULL;

    cdrom_fd = fileno(PyFile_AsFile(cdrom_fileobj));

#if defined(__APPLE__)
    memset( &trackInfoParams, 0, sizeof(trackInfoParams));
    trackInfoParams.addressType = kCDTrackInfoAddressTypeTrackNumber;
    trackInfoParams.address = track;
    trackInfoParams.bufferLength = sizeof(entry);
    trackInfoParams.buffer = &entry;
	
    if (ioctl(cdrom_fd, DKIOCCDREADTRACKINFO, &trackInfoParams) < 0) {
        PyErr_SetFromErrno(cdrom_error);
        return NULL;
    }

    trackMSF = CDConvertLBAToMSF(entry.trackStartAddress);
	
    return Py_BuildValue("bbb", trackMSF.minute, 
			 trackMSF.second, 
			 trackMSF.frame);
#else /* not defined(__APPLE__) */
    entry.CDDB_TRACK_FIELD = track;
    entry.CDDB_FORMAT_FIELD = CDDB_MSF_FORMAT;
	
#if defined(__OpenBSD__)
    entry.data = &data;
    entry.data_len = sizeof(data);
#endif
    
    if (ioctl(cdrom_fd, CDDB_READ_TOC_ENTRY_FLAG, &entry) < 0) {
	PyErr_SetFromErrno(cdrom_error);
	return NULL;
    }

    return Py_BuildValue("bbb", entry.CDDB_ADDR_FIELD.msf.minute, 
			 entry.CDDB_ADDR_FIELD.msf.second, 
			 entry.CDDB_ADDR_FIELD.msf.frame);
#endif
}

static PyObject *cdrom_leadout(PyObject *self, PyObject *args)
{
    struct CDDB_TOC_ENTRY_STRUCT entry;
    PyObject *cdrom_fileobj;
    int cdrom_fd;

#if defined(__OpenBSD__)
    struct cd_toc_entry data;
#elif defined(__APPLE__)
    struct CDDB_TOC_HEADER_STRUCT hdr;	
    dk_cd_read_disc_info_t discInfoParams;	
    dk_cd_read_track_info_t trackInfoParams;
    CDMSF trackMSF;
#endif

    if (!PyArg_ParseTuple(args, "O!", &PyFile_Type, &cdrom_fileobj))
	return  NULL;

    cdrom_fd = fileno(PyFile_AsFile(cdrom_fileobj));

#if defined(__APPLE__)
    memset(&discInfoParams, 0, sizeof(discInfoParams));
    discInfoParams.buffer = &hdr;
    discInfoParams.bufferLength = sizeof(hdr);

    if (ioctl(cdrom_fd, DKIOCCDREADDISCINFO, &discInfoParams) < 0) {
	PyErr_SetFromErrno(cdrom_error);
	return NULL;
    }

    memset(&trackInfoParams, 0, sizeof(trackInfoParams));
    trackInfoParams.addressType = kCDTrackInfoAddressTypeTrackNumber;
    trackInfoParams.address = hdr.CDDB_ENDING_TRACK_FIELD;
    trackInfoParams.bufferLength = sizeof(entry);
    trackInfoParams.buffer = &entry;
	
    if (ioctl(cdrom_fd, DKIOCCDREADTRACKINFO, &trackInfoParams) < 0) {
	PyErr_SetFromErrno(cdrom_error);
	return NULL;
    }

    trackMSF = CDConvertLBAToMSF(entry.trackStartAddress + entry.trackSize + 1);
	
    return Py_BuildValue("bbb", trackMSF.minute, 
			 trackMSF.second, 
			 trackMSF.frame);
#else /* not defined(__APPLE__) */
    entry.CDDB_TRACK_FIELD = CDDB_CDROM_LEADOUT;
    entry.CDDB_FORMAT_FIELD = CDDB_MSF_FORMAT;

#if defined(__OpenBSD__)
    entry.data = &data;
    entry.data_len = sizeof(data);
#endif

    if (ioctl(cdrom_fd, CDDB_READ_TOC_ENTRY_FLAG, &entry) < 0) {
	PyErr_SetFromErrno(cdrom_error);
	return NULL;
    }

    return Py_BuildValue("bbb", entry.CDDB_ADDR_FIELD.msf.minute, 
			 entry.CDDB_ADDR_FIELD.msf.second, 
			 entry.CDDB_ADDR_FIELD.msf.frame);
#endif
}

int cdrom_close(FILE *cdrom_file) 
{
    return fclose(cdrom_file);
}

static PyObject* cdrom_open(PyObject *self, PyObject *args)
{
    int cdrom_fd;
    FILE *cdrom_file;
    char *cdrom_device = CDDB_DEFAULT_CDROM_DEVICE;
    int cdrom_open_flags = CDDB_DEFAULT_CDROM_FLAGS;

    PyObject *cdrom_file_object;

    if (!PyArg_ParseTuple(args, "|si", &cdrom_device, &cdrom_open_flags))
	return NULL;

    cdrom_fd = open(cdrom_device, cdrom_open_flags);

    if (cdrom_fd == -1) {
	PyErr_SetFromErrno(cdrom_error);
	return NULL;
    }

    cdrom_file = fdopen(cdrom_fd, "r");

    if (cdrom_file == NULL) {
	PyErr_SetFromErrno(cdrom_error);
	return NULL;
    }

    cdrom_file_object = PyFile_FromFile(cdrom_file, cdrom_device, "r", cdrom_close);

    if (cdrom_file_object == NULL) {
	PyErr_SetString(cdrom_error, "Internal conversion from file pointer to Python object failed");
	fclose(cdrom_file);
	return NULL;
    }

    return Py_BuildValue("O", cdrom_file_object);
}

static PyMethodDef cdrom_methods[] = {
    { "toc_header", cdrom_toc_header, METH_VARARGS },
    { "toc_entry", cdrom_toc_entry, METH_VARARGS },
    { "leadout", cdrom_leadout, METH_VARARGS},
    { "open", cdrom_open, METH_VARARGS},
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
