#!/usr/bin/env python

import DiscID, CDDB, sys

if __name__ == '__main__':
    dev = '/dev/cdrom'

    if len(sys.argv) >= 2:
	dev = sys.argv[1]

    print "Getting disc id in CDDB format... ",

    disc_id = DiscID.disc_id(dev)

    print "got it.\nQuerying CDDB for disc info... ",

    (query_stat, query_info) = CDDB.query(disc_id)

    if query_stat == 200:
	print ("success!\nQuerying CDDB for track info for `%s'... " % query_info['title'])
	(read_stat, read_info) = CDDB.read(query_info['category'], query_info['disc_id'])
	if read_stat == 210:
	    print "success!"
	    keys = read_info.keys()
	    keys.sort()
	    for i in keys:
		print "%s: %s" % (i, read_info[i])
	else:
	    print "failure getting track info, status: %i" % read_stat
    else:
	print "failure getting disc info, status %i" % query_stat


	
