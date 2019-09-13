#!/usr/bin/env python3
"""Setup script for the CDDB module distribution."""
# Standard libraries.
import sys
from setuptools import setup
from distutils.core import Extension

MODULES = ['CDDB', 'DiscID']
if sys.platform == 'win32':
    EXTENSION = Extension('mci', ['win32/mci.c'])
    MODULES += ['win32/cdrom']
else:
    EXTENSION = Extension('cdrom', ['unix/cdrommodule.c'])


setup(  # Distribution meta-data
    name="CDDB",
    version="1.5",
    use_2to3=True,
    description="""Module for retrieving track information about audio CDs
from CDDB, improved Python 3 port.""",
    author="Ben Gertzfield",
    author_email="che@debian.org",
    url="http://cddb-py.sourceforge.net/",

    # Description of the modules and packages in the distribution
    py_modules=MODULES,
    ext_modules=[EXTENSION],

    setup_requires=['setuptools'],
)
