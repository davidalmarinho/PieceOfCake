import os
import sys
import re

# Global variables
WORKING_DIR = os.getcwd()
OUTPUT_DIR  = WORKING_DIR + '/out'
CONFIG_FILE_NAME = 'starter.cfg'

HELP = '''
    --build
    --run

    config --prj_name 'ProjectNameExample'
    config --release ON
    config --mem_check ON
'''
PRJ_NAME  = 'Default project name'
MEM_CHECK = False
DEBUG     = True

def is_linux():
    return sys.platform.__eq__('linux') or sys.platform.__eq__('linux2')

def is_windows():
    return sys.platform.__eq__('win32')

def is_mac():
    return sys.platform.__eq__('darwin')

def exists_file(path_file):
    return os.path.exists(path_file)

def move_file(file_path, new_path):
    os.rename(file_path, new_path)

def save_config():
    file_config = open(CONFIG_FILE_NAME, 'w')
    file_config.write('Project name:' + PRJ_NAME + '/\n')
    release = not DEBUG
    file_config.write('Release:' + str(release) + '/\n')
    file_config.write('Memory checking:' + str(MEM_CHECK) + '/\n')
    file_config.close()

def boolean(text):
    return text == 'True'

def load_config_file():
    if not exists_file(WORKING_DIR + '/' + CONFIG_FILE_NAME):
        return

    with open(CONFIG_FILE_NAME, encoding='utf8') as file_config:
        i = 0
        for line in file_config:
            #print(line)
            navigator = re.split('/', re.split(':', line)[1])
            
            if i == 0:
                global PRJ_NAME
                PRJ_NAME = str(navigator[0])
            elif i == 1:
                global DEBUG
                DEBUG = not boolean(str(navigator[0]))
            elif i == 2:
                global MEM_CHECK
                MEM_CHECK = boolean(str(navigator[0]))
            
            i += 1

def build_program():
    # Create out directory if it doesn't exist
    if not exists_file(OUTPUT_DIR):
        os.mkdir(OUTPUT_DIR)

    os.chdir(OUTPUT_DIR)
    if DEBUG:
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
   # TODO: This error checking isn't working 
   #  if not exists_file(OUTPUT_DIR + '/bin/' + PRJ_NAME):
   #      print(OUTPUT_DIR + '/bin/' + PRJ_NAME)
   #      print('ERROR: Please specify the project name.')
   #      print(HELP)
   #      return

    if not MEM_CHECK:
        os.system('./' + PRJ_NAME)
    else:
        # 	#valgrind --leak-check=full \
        # --show-leak-kinds=all \
        # --track-origins=yes \
        # --verbose \
        # --log-file=valgrind-out.txt \
        # ./PieceOfCake

        os.system('valgrind --leak-check=yes ./' + PRJ_NAME)
        # Warning: This script ends here if we reach at this point, because
        #          when closing the window, we also terminate this script.

def main():
    # Load configuration file if it exists
    load_config_file()

    # Change configuration file
    num_args = len(sys.argv)
    if num_args == 4:
        arg1 = sys.argv[1]
        arg2 = sys.argv[2]
        arg3 = sys.argv[3]
        if arg1.__eq__('config'):
            if arg2.__eq__('--mem_check'):
                if arg3.__eq__('ON'):
                    global MEM_CHECK
                    MEM_CHECK = True
                elif arg3.__eq__('OFF'):
                    MEM_CHECK = False
            elif arg2.__eq__('--prj_name'):
                    global PRJ_NAME
                    PRJ_NAME = arg3
            elif arg2.__eq__('--release'):
                if arg3.__eq__('ON'):
                    global DEBUG
                    DEBUG = False
                elif arg3.__eq__('OFF'):
                    DEBUG = True

        save_config()
    elif num_args == 2 and sys.argv[1] == '--gen_conf':
        save_config()

    # Load possible configuration change
    load_config_file()   

    # Handle your C++ program :)
    if num_args == 1:
        # TODO: Print help
        print(HELP)
    elif num_args == 2:
        arg = sys.argv[1]
        if arg.__eq__('--run'):
            build_program()
            run_program()
        elif arg.__eq__('--build'):
            build_program()
        else:
            print(HELP)

    else:
        print(HELP)

main()
