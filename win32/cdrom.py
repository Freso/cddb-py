#!/usr/bin/env python

# Supporting Ben Gertzfield's <che@debian.org> cdrom-Interface using mciSendString on WIN32
# Frank David <F.David@digitecgmbh.de>

# This work is released under the GNU GPL, version 2 or later.

import mci
import string

def toc_header(device):
   return (1, string.atoi(mci.mciSendString("status CDAudio number of tracks")))

def toc_entry(device, track):
   mci.mciSendString("set CDAudio time format msf")
   mciString = "status CDAudio position track " +  `track`
   resultStr = mci.mciSendString(mciString)
   result = string.split(resultStr, ':', 2)
   r = []
   for i in result: r.append(string.atoi(i))
   return r

def toc_entry_pos(device, track): 
   return toc_entry(device, track)
   

def toc_entry_len(device, track): 
   mci.mciSendString("set CDAudio time format msf")
   mciString = "status CDAudio length track " +  `track`
   resultStr = mci.mciSendString(mciString)
   result = string.split(resultStr, ':', 2)
   r = []
   for i in result: r.append(string.atoi(i))
   return r

def leadout(device):
   firstTrack, lastTrack = toc_header(device)
   trackPosMin, trackPosSecond, trackPosFrame = toc_entry_pos(device, lastTrack)
   trackLenMin, trackLenSecond, trackLenFrame = toc_entry_len(device, lastTrack)
   # calculate raw leadout
   leadoutMin, leadoutSecond, leadoutFrame = ( trackPosMin +  trackLenMin,  trackPosSecond  + trackLenSecond, trackPosFrame + trackLenFrame)   
   # add windows spezific correction
   leadoutFrame = leadoutFrame + leadoutFrame
   # convert to minute, second, frame
   if leadoutFrame >= 75:
      leadoutFrame  = leadoutFrame - 75;
      leadoutSecond = leadoutSecond +1           
   if leadoutSecond >= 60:
      leadoutSecond  = leadoutSecond - 60;
      leadoutMin = leadoutMin +1           
   # return leadout as tuple
   return (leadoutMin, leadoutSecond, leadoutFrame)
