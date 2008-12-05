#!/usr/bin/python
# -*- coding: UTF-8 -*-

# Simply speaking, GPL licensed

"""
Extract CHM for web browsing.

Usage: cs2w [-C dir] -T tpl chmfile
Example: cs2w -C /var/www/freebsd_handbook -T ./mytempl FreeBSD_Handbook.chm

Options:
  -C, --dir                 extract to this target directory
  -T, --tpl                 web template directory
  -h, --help                display this help and exit
  -V, --version             print version information and exit

Report bugs to crquan@gmail.com
"""

__version__ = "0.0.1"
verbose = False

def display_usage(exitflag):
    print __doc__
    if exitflag:
        sys.exit(exitflag-1)

def display_version(exitflag):
    print """cs2w %(__version__)s

Copyright (C) 2007  rankle_ <crquan@gmail.com>
This is free software; see the source for copying conditions. There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
""" % globals()

    if exitflag:
        sys.exit(exitflag-1)

import os, sys
import getopt
import shutil

from chm.chm import CHMFile
from chm.chmlib import (chm_enumerate,
        chm_retrieve_object, CHM_ENUMERATE_NORMAL)
from sgmllib import SGMLParser, SGMLParseError

class TopicObject(dict):
    def __repr__(self):
        return '<Topic - "%(name)s": "%(local)s">' % self

class TopicsParser(SGMLParser):

    def reset(self):
        """Reset this instance. Loses all unprocessed data."""
        SGMLParser.reset(self)
        self.__depth = 0
        self.__inobject = False
        self.__param = {}

        # this a critical data structure,
        self.__nodeTree = [[], ]

    def feed(self, data):
        SGMLParser.feed(self, data)
        return self.__nodeTree.pop()

    def start_object(self, attrs):
        if self.__inobject:
            self.error("object nested")
        attrs = dict(attrs)
        if attrs["type"] == "text/sitemap":
            self.__inobject = True
            self.__param = {}
    def end_object(self):
        if self.__inobject:
            topic = TopicObject(**self.__param)
            self.__nodeTree[self.__depth-1].append(topic)
            self.__inobject = False

    def start_ul(self, attrs):
        self.__depth += 1
        if self.__depth > 1:
            self.__nodeTree.append([])
            nextNode = self.__nodeTree[self.__depth-1]
            self.__nodeTree[self.__depth-2].append(nextNode)
    def end_ul(self):
        if self.__depth > 1:
            self.__nodeTree.pop()
        self.__depth -= 1

        # some chm file is very strange,
        # to generate a correct ouput,
        # we should merge two adjacent Topic list
        uplayer = self.__nodeTree[-1]
        if isinstance(uplayer[-1], list) and \
                isinstance(uplayer[-2], list):
            last = uplayer.pop()
            uplayer[-1].extend(last)

    def do_param(self, attrs):
        if self.__inobject:
            attrs = dict(attrs)
            name, value = attrs["name"], attrs["value"]
            self.__param[str(name).lower()] = value

def _extract_callback(py_h, py_ui, py_c):
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

    print ".",

def extract_chm(chmfile, dirname):
    print "Extracting chm ...",
    chm_enumerate(chmfile.file,
            CHM_ENUMERATE_NORMAL,
            _extract_callback,
            dirname)
    print "done."

def printTree(tree, depth=0, out=sys.stdout):
    # this is a debug function, you can use it to dump the nodeTree

    for t in tree:
        if isinstance(t, TopicObject):
            print >> out, "%s%s" % (
                    "\t" * depth, t)
        elif isinstance(t, list):
            print >> out, "%slist length: %d" % (
                    "\t" * depth, len(t))
            printTree(t, depth+1, out)
        else:
            print >> out, "%sunknown: %s %r" % (
                    "\t" * depth, type(t), t)

def gen_book_tree(target, nodeTree):
    if verbose:
        printTree(nodeTree)

    # this function is to generate a "tree_items.js" file

    book_tree_file = os.path.join(target, "tree_items.js")
    book_tree_file = open(book_tree_file, "w+")

    def _printTree(nodeTree, depth=1):
        node_len = len(nodeTree)
        for i in range(node_len):
            t = nodeTree[i]
            if isinstance(t, TopicObject):
                if i < node_len-1 and isinstance(nodeTree[i+1], list):
                    templ = "%s['%s', '%s',"
                else:
                    templ = "%s['%s', '%s'],"
                print >> book_tree_file, templ % (
                        "\t" * depth,
                        t["name"].replace("\\", r"\\").replace("\'", r"\'"),
                        t["local"])
            elif isinstance(t, list):
                _printTree(t, depth+1)
                print >> book_tree_file, "%s],\n" % ("\t" * depth)

    print >> book_tree_file, \
            "\nvar TREE_ITEMS = [\n['chmsee2web', 'cs2w.html',"
    _printTree(nodeTree)
    print >> book_tree_file, "]];"
    book_tree_file.close()

def main():
    try:
        opts, args = getopt.gnu_getopt(sys.argv[1:],
                "C:T:hvV",
                ["dir=", "tpl=", "help", "version", "verbose"])
    except getopt.GetoptError:
        display_usage(2)

    tpldir, target = "", ""
    global verbose

    for o, a in opts:
        if o in ("-h", "--help"):
            display_usage(1)
        elif o in ("-V", "--version"):
            display_version(1)
        elif o in ("-C", "--dir"):
            target = a
        elif o in ("-T", "--tpl"):
            tpldir = a
        elif o in ("-v", "--verbose"):
            verbose = True

    if not target:
        target = os.getcwd()
    if not tpldir:
        display_usage(2)

    if len(args) < 1:
        display_usage(2)

    filename = args[0]
    try:
        shutil.copytree(tpldir, target)
    except OSError:
        pass

    chmfile = CHMFile()
    if not chmfile.LoadCHM(filename):
        print >> sys.stderr, """\
it seems that the file '%(filename)s' corrupted, or python-chm has a bug. """ % locals()
        sys.exit(3)

    extract_chm(chmfile, target)

    topics = chmfile.GetTopicsTree()

    # GetLCID return this:
    #
    # ('iso8859_1', 'English_United_States', 'Western Europe & US')
    # ('cp936', 'Chinese_PRC', 'Simplified Chinese')
    # ('iso8859_1', 'English_United_States', 'Western Europe & US')
    enc, country, lang = chmfile.GetLCID()

    topics = unicode(topics, enc)
    if verbose:
        print topics

    print "Parsing chm hhc ...",
    parser = TopicsParser()
    nodeTree = parser.feed(topics)
    gen_book_tree(target, nodeTree)
    print "finished."

    print """\
Convert "%(filename)s" to web finished.
You can browse "%(target)s/cs2w_index.html" to visit.""" % locals()

if __name__ == "__main__":
    main()
