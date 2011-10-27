#!/bin/env python

from CMGTools.RootTools.cmgInit import *

from optparse import OptionParser
import sys

parser = OptionParser()

parser.usage = '''
cmgInit.py <root rile pattern>.
'''

(options,args) = parser.parse_args()

if len(args) != 1:
    print 'ERROR: please provide a pattern for your root files'
    sys.exit(1)

pattern = args[0]

(events,lumi) = cmgInit( pattern )
