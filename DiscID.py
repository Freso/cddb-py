#!/usr/bin/env python

import cdrom, CDROM, sys

def cddb_sum(n):
    ret = 0
    
    while n > 0:
	ret = ret + (n % 10)
	n = n / 10

    return ret

def disc_id(device_name):
    cdrom_file = open(device_name)

    (first, last) = cdrom.toc_header(cdrom_file)

    track_lbas = []

    for i in range(first, last + 1):
	track_lbas.append(cdrom.toc_entry_lba(cdrom_file, i))

    track_lbas.append(cdrom.toc_entry_lba(cdrom_file, CDROM.CDROM_LEADOUT))
	
    cdrom_file.close()

    checksum = 0

    for i in track_lbas[:-1]:
	checksum = checksum + cddb_sum((i + CDROM.CD_MSF_OFFSET) / 
				       CDROM.CD_FRAMES)

    totaltime = ((track_lbas[-1] + CDROM.CD_MSF_OFFSET) / CDROM.CD_FRAMES -
		 ((track_lbas[0] + CDROM.CD_MSF_OFFSET) / CDROM.CD_FRAMES))

    result = '%08lx' % (((checksum % 0xff) << 24) | totaltime << 8 | last)
    result = result + ' %d' % last

    for i in track_lbas[:-1]:
	result = result + ' %d' % (i + CDROM.CD_MSF_OFFSET)

    result = result + ' %d' % ((track_lbas[-1] + CDROM.CD_MSF_OFFSET) 
			       / CDROM.CD_FRAMES)

    return result

if __name__ == '__main__':
    dev = '/dev/cdrom'

    if len(sys.argv) >= 2:
	dev = sys.argv[1]

    print disc_id(dev)

