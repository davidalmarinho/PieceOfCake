import os, sys, re, shutil

# Global variables
WORKING_DIR = os.getcwd().replace('\\', '/')
OUTPUT_DIR  = WORKING_DIR + '/out'
CONFIG_FILE_NAME = 'starter.cfg'
SHADERS_NAME_DIR = 'shaders'
SHADERS_COMPILED_FOLDER = OUTPUT_DIR + '/' + SHADERS_NAME_DIR
SHADERS_SRC_DIR = WORKING_DIR + '/' + SHADERS_NAME_DIR

HELP = '''
    Commands:
    --build
    --run
    --help

    config --prj_name 'ProjectNameExample'
    config --release ON
    config --mem_check OFF
    config --glslc_path '/usr/bin/glslc'
    config --vcpkg_path 'D:/Softwares/vcpackage/vcpkg/scripts/buildsystems/vcpkg.cmake'
'''
PRJ_NAME   = 'Default project name'
MEM_CHECK  = False
DEBUG      = True
GLSLC_PATH = 'No Path'
VCPKG_PATH = 'No Path'

class Colors:
    GREEN     = '\033[92m'
    BLUE      = '\033[94m'
    YELLOW    = '\033[93m'
    RED       = '\033[91m'
    END_COLOR = '\033[0m'
    BOLD      = '\033[1m'
    UNDERLINE = '\033[4m'

def compile_shaders_project():
    # Firstly, let's clean others already compilled shaders.
    clean_shaders_compilation()

    print('-=-=-=-=- Compiling shaders -=-=-=-=-' + Colors.RED)
    os.chdir(SHADERS_SRC_DIR)

    look_for_shaders(SHADERS_SRC_DIR)
    os.chdir(OUTPUT_DIR)
    print(Colors.END_COLOR + '-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-')

def clean_shaders_compilation():
    compile_shaders_path = SHADERS_COMPILED_FOLDER
    for files in os.listdir(compile_shaders_path):
        path = os.path.join(compile_shaders_path, files)
        try:
            shutil.rmtree(path)
        except OSError:
            os.remove(path)

def compile_shader(filepath):
    """
    compile_shader function compiles a shader file.
    The file must have the extension '.vert' or '.frag'.

    :param filename: File's path.
    """

    # Get the folder of where the current shader is located
    compiled_shader_folder = OUTPUT_DIR + '/' + SHADERS_NAME_DIR + filepath.split(SHADERS_NAME_DIR)[1].rsplit('/', 1)[0]
    # Generate compilled shader name --just changing its extension.
    compiled_shader_path = filepath.split('.')[0] + '.spv'

    # Compile shader
    if (os.system(GLSLC_PATH + ' ' + filepath + ' -o ' + compiled_shader_path) == 0):
        # Successful message
        print(f'{Colors.END_COLOR}-- {Colors.GREEN} > Compiled {Colors.BLUE}{filepath}{Colors.GREEN} shader. {Colors.RED}')
        compiled_shader_path_dest = compiled_shader_folder + '/' + compiled_shader_path.rsplit('/', 1)[1]

        # Duplicate the same shaders' folder system into the output directory
        if (not os.path.exists(compiled_shader_folder)):
            os.mkdir(compiled_shader_folder)

        # Move compiled shader file to the output directory
        os.rename(compiled_shader_path, compiled_shader_path_dest)

def look_for_shaders(path):
    for dir in os.listdir():
        next_file = os.path.join(path, dir).replace('\\', '/')

        # Compile shader
        if os.path.isfile(next_file):
            compile_shader(next_file)
        elif os.path.isdir(next_file):
            os.chdir(next_file)
            look_for_shaders(next_file)

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
    
def replace_file(file_path, new_path):
    os.replace(file_path, new_path)

def save_config():
    file_config = open(CONFIG_FILE_NAME, 'w')
    file_config.write('Project name=' + PRJ_NAME + ';\n')
    release = not DEBUG
    file_config.write('Release=' + str(release) + ';\n')
    file_config.write('Memory checking=' + str(MEM_CHECK) + ';\n')
    file_config.write('GLSLC path application=' + GLSLC_PATH + ';\n')
    file_config.write('Cmake vcpkg path=' + VCPKG_PATH + ';\n')
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
            navigator = re.split(';', re.split('=', line)[1])
            
            if i == 0:
                global PRJ_NAME
                PRJ_NAME = str(navigator[0])
            elif i == 1:
                global DEBUG
                DEBUG = not boolean(str(navigator[0]))
            elif i == 2:
                global MEM_CHECK
                MEM_CHECK = boolean(str(navigator[0]))
            elif i == 3:
                global GLSLC_PATH
                GLSLC_PATH = str(navigator[0])
            elif i == 4:
                global VCPKG_PATH
                VCPKG_PATH = str(navigator[0])
            
            i += 1

def build_program():
    success = 0
    
    # Create out and shaders directories if they doesn't exist
    if not exists_file(OUTPUT_DIR):
        os.mkdir(OUTPUT_DIR)
    
    if not exists_file(SHADERS_COMPILED_FOLDER):
        os.mkdir(SHADERS_COMPILED_FOLDER)

    # Compile shaders
    compile_shaders_project()

    if is_windows():
        if DEBUG:
            # With msbuild
            # os.system('cmake .. -DCMAKE_TOOLCHAIN_FILE=' + VCPKG_PATH + ' -DCMAKE_BUILD_TYPE=Debug')
            # With nmake
            os.system('cmake -G "NMake Makefiles" .. -DCMAKE_TOOLCHAIN_FILE=' + VCPKG_PATH + ' -DCMAKE_BUILD_TYPE=Debug')
        else:
            # With msbuild
            # os.system('cmake .. -DCMAKE_TOOLCHAIN_FILE=' + VCPKG_PATH + ' -DCMAKE_BUILD_TYPE=Release')
            # With nmake
            os.system('cmake -G "NMake Makefiles" .. -DCMAKE_TOOLCHAIN_FILE=' + VCPKG_PATH + ' -DCMAKE_BUILD_TYPE=Release')
    else:
        if DEBUG:
            os.system('cmake -DCMAKE_BUILD_TYPE=Debug ..')
        else:
            os.system('cmake -DCMAKE_BUILD_TYPE=Release ..')

    # Check for compile_commands.json file and move it to sources directory
    compile_commands = OUTPUT_DIR + '/compile_commands.json'
    if exists_file(compile_commands):
        target_replace_compile_commands_path = WORKING_DIR + '/compile_commands.json'
        if exists_file(target_replace_compile_commands_path):
            replace_file(compile_commands, target_replace_compile_commands_path)
        else:
            move_file(compile_commands, target_replace_compile_commands_path)
    else:
        print(Colors.RED + "Error: Couldn't find " + compile_commands + " file." + Colors.END_COLOR)

    if is_windows():
        # With nmake
        success = os.system('nmake')
        
        # With msbuild
        #if DEBUG:
            # With msbuild
            # os.system('msbuild ' + PRJ_NAME + '.sln /property:Configuration=Debug')
        #else:
            # With msbuild
            # os.system('msbuild ' + PRJ_NAME + '.sln /property:Configuration=Release')
    else:
        success = os.system('make')

    return success == 0
    


def run_program():
    os.chdir(OUTPUT_DIR + '/bin')
   # TODO: This error checking isn't working 
   #  if not exists_file(OUTPUT_DIR + '/bin/' + PRJ_NAME):
   #      print(OUTPUT_DIR + '/bin/' + PRJ_NAME)
   #      print('ERROR: Please specify the project name.')
   #      print(HELP)
   #      return

    if not MEM_CHECK:
        if is_windows():
            os.system(PRJ_NAME)
        #    if DEBUG:
        #        os.chdir('Debug')
        #    else:
        #        os.chdir('Release')
        #    os.system(PRJ_NAME)
        else:
            os.system('./' + PRJ_NAME)
    else:
        # Valgrind options:
        # --leak-check=full \
        # --show-leak-kinds=all \
        # --track-origins=yes \
        # --verbose \
        # --log-file=valgrind-out.txt \
        # ./PieceOfCake

        os.system('valgrind --leak-check=yes --tool=memcheck --leak-check=full ./' + PRJ_NAME)
        # Warning: This script ends here if we reach at this point, because
        #          when closing the window, we also terminate this script.

def handle_user_option(args):
    num_args = len(args)
    if num_args == 4:
        arg1 = args[1]
        arg2 = args[2]
        arg3 = args[3]
        if arg1 == 'config':
            if arg2 == '--mem_check':
                if arg3 == 'ON':
                    global MEM_CHECK
                    MEM_CHECK = True
                elif arg3 == 'OFF':
                    MEM_CHECK = False
            elif arg2 == '--prj_name':
                global PRJ_NAME
                PRJ_NAME = arg3
            elif arg2 == '--release':
                if arg3 == 'ON':
                    global DEBUG
                    DEBUG = False
                elif arg3 == 'OFF':
                    DEBUG = True
            elif arg2 == '--glslc_path':
                global GLSLC_PATH
                GLSLC_PATH = arg3
            elif arg2 == '--vcpkg_path':
                global VCPKG_PATH
                VCPKG_PATH = arg3

        save_config()
    elif num_args == 2 and sys.argv[1] == '--gen_conf':
        save_config()

    # Load possible configuration change
    load_config_file()   

    # Handle your C++ program :)
    if num_args == 1:
        print(HELP)
    elif num_args == 2:
        arg = sys.argv[1]
        if arg.__eq__('--run'):
            if build_program():
                run_program()
        elif arg.__eq__('--build'):
            build_program()
        else:
            print(HELP)

    else:
        print(HELP)

def main():
    # Load configuration file if it exists
    load_config_file()

    # Change configuration file
    handle_user_option(sys.argv)
main()
