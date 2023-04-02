import os
from pickle import FALSE
import sys

# Global variables
WORKING_DIR = os.getcwd()
OUTPUT_DIR  = WORKING_DIR + '/out'
PRJ_NAME = 'PieceOfCake'

MEM_CHECK = False
DEBUG = True

def is_linux():
    return sys.platform.__eq__('linux') or sys.platform.__eq__('linux2')

def is_windows():
    return sys.platform.__eq__('win32')

def is_mac():
    return sys.platform.__eq__('darwin')

def exists_file(path):
    return os.path.exists(path)

def move_file(file_path, new_path):
    os.rename(file_path, new_path)

def build_program():
    # Create out directory if it doesn't exist
    if not exists_file(OUTPUT_DIR):
        os.mkdir(OUTPUT_DIR)

    os.chdir(OUTPUT_DIR)
    if not DEBUG:
        os.system('cmake -DCMAKE_BUILD_TYPE=Debug ..')
    else:
        os.system('cmake -DCMAKE_BUILD_TYPE=Release ..')

    # Check for compile_commands.json file and move it to sources directory
    compile_commands = OUTPUT_DIR + '/compile_commands.json'
    if exists_file(compile_commands):
        move_file(compile_commands, WORKING_DIR + '/compile_commands.json')

    if is_linux():
        os.system('make')


def run_program():
    os.chdir(OUTPUT_DIR + '/bin')
    if not MEM_CHECK:
        os.system('./' + PRJ_NAME)
    else:
        os.system('valgrind --leak-check=yes ./' + PRJ_NAME)

def main():
    num_args = len(sys.argv)
    if num_args == 1:
        # TODO: Print help
        print("No args")
    elif num_args == 2:
        arg = sys.argv[1]
        if arg.__eq__('--mem_check'):
            print('Mem check running...\n')
        elif arg.__eq__('--run'):
            build_program()
            run_program()
    elif num_args == 4:
        arg1 = sys.argv[1]
        arg2 = sys.argv[2]
        arg3 = sys.argv[3]
        if arg1.__eq__('config'):
            if arg2.__eq__('--mem_check'):
                if arg3.__eq__('ON'):
                    # Write file for MEM_CHECK
                    MEM_CHECK = True
                elif arg3.__eq__('OFF'):
                    MEM_CHECK = False
            elif arg2.__eq__('--prj_name'):
                    PRJ_NAME = arg3
            elif arg2.__eq__('--release'):
                if arg3.__eq__('ON'):
                    DEBUG = False
                elif arg3.__eq__('OFF'):
                    DEBUG = True

