#!/usr/bin/env python

"""Setup script for the CDDB module distribution."""

__revision__ = "$Id$"

from distutils.core import setup

setup (# Distribution meta-data
       name = "CDDB",
       version = "1.1",
       description = "Module for retrieving track information about audio CDs from CDDB",
       author = "Ben Gertzfield",
       author_email = "che@debian.org",
       url = "http://csl.cse.ucsc.edu/~ben/python/",

       # Description of the modules and packages in the distribution
       py_modules = ['CDDB', 'DiscID'],
       ext_modules = 
           [('cdrom',
             { 'sources': ['unix/cdrommodule.c'] }
           )]
      )
