#!/usr/bin/python3

# The main executable for the Metadata Ingetsion for MarFS Object Storage Adaptation tool. This is 
# a wrapper around the PowerWalk program by Bob Sneed, which has been slightly modified to suit the
# needs of this program. 
#
# Arguments: source directory for root of POSIX scan, root directory of MarFS mapping

import subprocess
import sys 

THREAD_COUNT = 1

# input validation

# if sys.argv[1] is not "-sources" or "-dest" not in sys.argv:
    

print('---------------------------------------------')
print(f'Starting MIMOSA with:\n')
print(f'\tnumber of threads: {THREAD_COUNT}\n')
print(f'\tsource location: {sys.argv[2:]}\n')
print(f'\troot of destination tree: {sys.argv[2]}\n')
print(f'\toutput/log location: testing/pwalk-output')
print('---------------------------------------------')

# subprocess.run(['pwalk-mfs', '-mimosa', f'-dop={THREAD_COUNT}', f'-source={sys.argv[1]}', f'-dest={sys.argv[2]}', '-output=testing/pwalk-output'])


# Run summary with data from pwalk log file


print_usage()
