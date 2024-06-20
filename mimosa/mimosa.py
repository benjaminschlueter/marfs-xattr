#!/usr/bin/python3

# The main executable for the Metadata Ingetsion for MarFS Object Storage Adaptation tool. This is 
# a wrapper around the PowerWalk program by Bob Sneed, which has been slightly modified to suit the
# needs of this program. 
#
# Arguments: source directory for root of POSIX scan, root directory of MarFS mapping

import subprocess
import sys 
import os

THREAD_COUNT = 1

def print_usage():
    print("Usage: ./mimosa.py -source <source list> -dest <dest path> <flags>")
    sys.exit()    

# input validation

if (len(sys.argv) < 5) or (sys.argv[1] != "-source") or ("-dest" not in sys.argv):
    print_usage()

dest_flag_index = sys.argv.index("-dest")

# verify source directories exist

for dir in sys.argv[2:dest_flag_index]:
    if os.path.exists(dir) and os.path.isdir(dir):
        continue
    else:
        print(f"ERROR: source path \"{dir}\" does not exist")
        sys.exit()

print('---------------------------------------------')
print(f'Starting MIMOSA with:\n')
print(f'\tnumber of threads: {THREAD_COUNT}\n')
print(f'\tsource location(s): {sys.argv[2:dest_flag_index]}\n')
print(f'\troot of destination tree: {sys.argv[dest_flag_index+1]}\n')
print(f'\toutput/log location: testing/pwalk-output')
print('---------------------------------------------')

for dir in sys.argv[2:dest_flag_index]:
    dest_path = f"{sys.argv[dest_flag_index + 1]}/{os.path.basename(dir)}"
    
    if os.path.exists(dest_path):
        print(f"Warning: {dest_path} already exists ... using {dest_path}_mimosa instead")
        dest_path = f"{dest_path}_mimosa"

    print(f"Mapping source {dir} to dest {dest_path}")

    # subprocess.run(['pwalk-mfs', '-mimosa', f'-dop={THREAD_COUNT}', f'-source={dir}', f'-dest={dest_path}', '-output=testing/pwalk-output'])


# Run summary with data from pwalk log file


