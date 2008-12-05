#!/usr/bin/env python

import sys, os
from chm.chm import CHMFile
from chm.chmlib import (chm_enumerate,
        chm_retrieve_object, CHM_ENUMERATE_NORMAL)

def _extract_callback(py_h, py_ui, py_c):
    print "%10d %8d %4d %s" % (py_ui.start, py_ui.length, py_ui.space, py_ui.path)

    # for security reason
    if py_ui.path.startswith("/../"):
        return

    # tranlate to relative path
    if py_ui.path.startswith("/"):
        py_ui.path = py_ui.path[1:]

    dirtarget = py_c
    path = os.path.join(dirtarget, py_ui.path)

    if py_ui.length > 0:
        # length>0 is a regular file
        size, text = chm_retrieve_object(py_h, py_ui, 0l, py_ui.length)
        open(path, "wb").write(text)
    elif py_ui.length == 0:
        # length=0, this is a directory
        if not os.path.exists(path):
            os.mkdir(path)

def usage(exitflag=1):
    print >> sys.stderr, "usage: extract_chmLib <chmfile> <outdir>"
    sys.exit(exitflag-1)

filename, dirtarget = "", "."

if len(sys.argv) != 3:
    usage(2)
else:
    filename  = sys.argv[1]
    dirtarget = sys.argv[2]

chmfile = CHMFile()
if chmfile.LoadCHM(sys.argv[1]):
    chm_enumerate(chmfile.file,
            CHM_ENUMERATE_NORMAL,
            _extract_callback,
            dirtarget)
