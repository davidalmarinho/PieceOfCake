import os
import sys

WORKING_DIR = os.getcwd()
OUTPUT_DIR  = WORKING_DIR + '/out'

# Create out directory if it doesn't exist
if not os.path.exists(OUTPUT_DIR):
    os.mkdir(OUTPUT_DIR)

if len(sys.argv) == 1:
    print("No args")
elif sys.argv[1].__eq__('--mem_check'):
    print('Mem check running...\n')

