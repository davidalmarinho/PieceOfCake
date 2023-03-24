import os

WORKING_DIR = os.getcwd()
OUTPUT_DIR  = WORKING_DIR + '/out'

# Create out directory if it doesn't exist
if not os.path.exists(OUTPUT_DIR):
    os.mkdir(OUTPUT_DIR)


