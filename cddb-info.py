#!/usr/bin/env python

# Example of how to use CDDB.py and DiscID.py

# Written 17 Nov 1999 by Ben Gertzfield <che@debian.org
# This work is released under the GNU GPL, version 2 or later.

import DiscID, CDDB, sys

dev = '/dev/cdrom'

if len(sys.argv) >= 2:
    dev = sys.argv[1]

fd = open(dev)

print "Getting disc id in CDDB format...",

disc_id = DiscID.disc_id(fd)

print "Disc ID: %08lx Num tracks: %d" % (disc_id[0], disc_id[1])
print "Querying CDDB for info on disc...",

(query_stat, query_info) = CDDB.query(disc_id)

if query_stat == 200:
    print ("success!\nQuerying CDDB for track info of `%s'... " % 
	   query_info['title']),
    (read_stat, read_info) = CDDB.read(query_info['category'], 
				       query_info['disc_id'])
    if read_stat == 210:
	print "success!"
	for i in range(1, disc_id[1]):
	    print "Track %.02d: %s" % (i, read_info['TTITLE' + `i`])
    else:
	print "failure getting track info, status: %i" % read_stat
elif query_stat == 210 or query_stat == 211:
    print "multiple matches found! Matches are:"
    for i in query_info:
	print "ID: %08lx Category: %s Title: %s" % \
	      (i['disc_id'], i['category'], i['title'])
else:
    print "failure getting disc info, status %i" % query_stat


	
