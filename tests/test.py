import os
import platform
import sys
import subprocess
import locale

ENCODING = locale.getpreferredencoding()
print(f"Default encoding: {ENCODING}")

BUILD_DIR = sys.argv[1]
EXAMPLES_DIR = sys.argv[2]
PRECOMP_DIR = sys.argv[3]

PRECOMPUTE: bool = len(sys.argv) > 4

special_cases = [
    {
        'example_name': "input",
        'text': True,
        'shell': True,
        'input': "test message!"
    }
]

if platform.system() == "Windows":
    BUILD_DIR = BUILD_DIR.replace("/", "\\")
    EXAMPLES_DIR = EXAMPLES_DIR.replace("/", "\\")
    PRECOMP_DIR = PRECOMP_DIR.replace("/", "\\")
    PATH_SEP: str = "\\"
    VPU = BUILD_DIR + PATH_SEP + "virtual"

else:
    PATH_SEP: str = "/"
    VPU = BUILD_DIR + PATH_SEP + "virtual"

ASSEMBLE    = VPU + " -assemble"
DISASSEMBLE = VPU + " -disassemble"
RUN         = VPU + " -execute"
DEBUG       = VPU + " -debug -0"

def run_process(*command, text=True, shell=False, _input=None):
    cmd = ""
    for c in command:
        cmd += c + " "
    print(f"CMD: '{cmd}'")
    return subprocess.run(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        shell=True,
        input=_input
    )

def cmpf(f1, f2, mode):
    file1 = open(f1, mode)
    file2 = open(f2, mode)

    contents1 = file1.read()
    contents2 = file2.read()

    file1.close()
    file2.close()

    rng = min(len(contents1), len(contents2))

    first_differ = 0
    line = 1
    column = 1
    for i in range(rng):
        if contents1[i] != contents2[i]:
            first_differ = i
            rng = min(rng, i + 20)
            i = max(i - 20, 0)
            print(f"files differ at byte {first_differ}, in line {line} column {column}")
            print(f"view of first file from {i} to {rng}: '{contents1[i:rng]}'")
            print(f"view of second file from {i} to {rng}: '{contents2[i:rng]}'")
            return False
        if ('b' not in mode and contents1[i] == '\n') or (contents1[i] == b'\n'):
            line += 1
            column = 0
        else:
            column += 1
    
    return len(contents1) == len(contents2)

def precompute(example_path: str):
    EXAMPLE_NAME = example_path.removesuffix(".txt").split(PATH_SEP)
    EXAMPLE_NAME = EXAMPLE_NAME[len(EXAMPLE_NAME) - 1]

    PRECOMPILED = PRECOMP_DIR + PATH_SEP + EXAMPLE_NAME + ".out"
 
    PREDECOMPILED = PRECOMP_DIR + PATH_SEP + EXAMPLE_NAME + ".txt"
    
    process = run_process(ASSEMBLE, example_path, "-o", PRECOMPILED)
    if process.returncode != 0:
        print("Compilation Failed For " + EXAMPLE_NAME)
        print("stderr: " + process.stderr.decode(ENCODING))
        return 1

    process = run_process(DISASSEMBLE, PRECOMPILED, "-o", PREDECOMPILED)
    if process.returncode != 0:
        print("Decompilation Failed For " + EXAMPLE_NAME)
        print("stderr: " + process.stderr.decode(ENCODING))
        return 1

    return 0


def test_example(example_path: str) -> int:
    EXAMPLE_NAME = example_path.removesuffix(".txt").split(PATH_SEP)
    EXAMPLE_NAME = EXAMPLE_NAME[len(EXAMPLE_NAME) - 1]
    print("Beggining Test " + EXAMPLE_NAME)

    COMPILED = BUILD_DIR + PATH_SEP + "assembled" + PATH_SEP + EXAMPLE_NAME + ".out"
    DECOMPILED = BUILD_DIR + PATH_SEP + "tmp.txt"
    
    PRECOMPILED = PRECOMP_DIR + PATH_SEP + EXAMPLE_NAME + ".out"

    process = run_process(ASSEMBLE, example_path, "-o", COMPILED)
    err_status = 0
    if process.returncode != 0:
        print("Compilation Failed For " + EXAMPLE_NAME)
        print("stderr: " + process.stderr.decode(ENCODING))
        return 1
    elif cmpf(COMPILED, PRECOMPILED, "rb") == 0:
        print("Compiled " + EXAMPLE_NAME + " Does Not Match Expected")
        err_status = 1
 
    process = run_process(DISASSEMBLE, COMPILED, "-o", DECOMPILED)
    if process.returncode != 0:
        print("Decompilation Failed For " + EXAMPLE_NAME)
        print("stderr: " + process.stderr.decode(ENCODING))
        err_status = 1
    else:
        process = run_process(ASSEMBLE, DECOMPILED, "-o", BUILD_DIR + PATH_SEP + "tmp.out")
        if process.returncode != 0:
            print("Could Not Compile " + EXAMPLE_NAME + " Decompiled File")
            print("stderr: " + process.stderr.decode(ENCODING))
            err_status = 1
        elif cmpf(BUILD_DIR + PATH_SEP + "tmp.out", COMPILED, "rb") == 0:
            print("Decompiled " + EXAMPLE_NAME + " Does Not Compile Back To Original Executable")
            err_status = 1

    special_case = None
    for i in special_cases:
        if i['example_name'] == EXAMPLE_NAME:
            special_case = i
            break
    
    if special_case is not None and 'input' in special_case:
        process = run_process(f"echo \"{special_case['input']}\" |", RUN, COMPILED)
    else:
        process = run_process(RUN, COMPILED)
    if process.returncode != 0:
        print("Run Failed For " + EXAMPLE_NAME)
        print("stderr: " + process.stderr.decode(ENCODING))
        err_status = 1

    if err_status == 0:
        print("Test " + EXAMPLE_NAME + " was successfull")
        print("stdout: " + process.stdout.decode(ENCODING))
    else:
        print("*Test " + EXAMPLE_NAME + " failed ***")
    return err_status
    

examples = [f for f in os.listdir(EXAMPLES_DIR) if os.path.isfile(os.path.join(EXAMPLES_DIR, f))]
test_count = len(examples)
failed_tests = 0

if not PRECOMPUTE:
    os.makedirs(BUILD_DIR + PATH_SEP + "assembled", exist_ok=True)
    for example in examples:
        failed_tests += test_example(EXAMPLES_DIR + PATH_SEP + example)
    
    print(str(failed_tests) + " tests failed out of " + str(test_count) + " (" + str(100 * (float(failed_tests) / float(test_count))) + "%)" if failed_tests > 0 else "all tests were successfull")
else:
    for example in examples:
        failed_tests += precompute(EXAMPLES_DIR + PATH_SEP + example)
    
    print(str(failed_tests) + " precomputes failed out of " + str(test_count) + " (" + str(100 * (float(failed_tests) / float(test_count))) + "%)"  if failed_tests > 0 else "all precomputes were successfull")


if PRECOMPUTE:
    print("\n\nstarting precompute of debug test...")
else:
    print("\n\nstarting debug test...")

dummy = sys.argv[0].replace('/', PATH_SEP).split(PATH_SEP)

test_dir = ""

if len(dummy) > 0:
    for i in range(len(dummy) - 1):
        test_dir += dummy[i] + PATH_SEP
else:
    test_dir = "." + PATH_SEP



DEBUG_EXAMPLE_PATH = test_dir + "debug_example.out"


if PRECOMPUTE:
    DEBUG_EXAMPLESRC_PATH = test_dir + "debug_example.txt"
    print(f"quering debug example source in {DEBUG_EXAMPLESRC_PATH}")
    try:
        debug_example_file = open(DEBUG_EXAMPLESRC_PATH, "r")
        debug_example_file.close()
        print(f"compiling {DEBUG_EXAMPLESRC_PATH} to {DEBUG_EXAMPLE_PATH}")
        process = run_process(ASSEMBLE, DEBUG_EXAMPLESRC_PATH, "-o", DEBUG_EXAMPLE_PATH)
        if process.returncode:
            print(f"*Precompute Debug Failed trying to compile {DEBUG_EXAMPLESRC_PATH} to {DEBUG_EXAMPLE_PATH} ***")
            print("stderr: " + process.stderr.decode(ENCODING))
            exit(1)
    except FileNotFoundError:
        print(\
            "no debug example src file",\
            "will proceed to query already existing program"
        )
    except Exception as e:
        print(f"Exception {e} ocurred while trying to compile the debug test example source file in {DEBUG_EXAMPLESRC_PATH}")
        exit(1)

print(f"quering debug example in {DEBUG_EXAMPLE_PATH}")

try:
    debug_example_file = open(DEBUG_EXAMPLE_PATH, "rb")
    debug_example_file.close()
except FileNotFoundError:
    print(\
        "no debug example, ",\
        "will take this to mean no debug test is intended and thus this does not count as a failure on its own, "\
        "the exit status will depend on the previous tests only"\
    )
    exit(failed_tests != 0)
except Exception as e:
    print(f"Exception {e} ocurred while trying to find the debug test example in {DEBUG_EXAMPLE_PATH}")
    exit(1)

DEBUG_INPUT_PATH = test_dir + "debug_input.txt"

print(f"quering debug input file in {DEBUG_EXAMPLE_PATH}")

try:
    debug_input_file = open(DEBUG_INPUT_PATH, "r")
    debug_input_file.close()
except FileNotFoundError:
    print(f"*Could not find debug input file in {DEBUG_INPUT_PATH} ***")
    exit(1)
except Exception as e:
    print(f"*Exception {e} ocurred while trying to find the expected debug test output in {DEBUG_INPUT_PATH} ***")
    exit(1)

DEBUG_OUTPUT_DUMP_NAME = "tmp.txt"
DEBUG_EXPECTED_OUTPUT_PATH = test_dir + "debug_expected_output.txt"

try: #clean dump file, if it exists
    dummy = open(DEBUG_OUTPUT_DUMP_NAME, "r")
    dummy.close()
    dummy = open(DEBUG_OUTPUT_DUMP_NAME, "w")
    dummy.close()
except:
    dummy = 0

if PRECOMPUTE:
    process = run_process(DEBUG, DEBUG_EXAMPLE_PATH, "<", DEBUG_INPUT_PATH)
    if process.returncode:
        print("*Precompute Debug Failed For " + DEBUG_EXAMPLE_PATH + " ***")
        print("stderr: " + process.stderr.decode(ENCODING))
        exit(1)
    else:
        try:
            dummy = open(DEBUG_OUTPUT_DUMP_NAME, "r")
            dummy.close()
        except FileNotFoundError:
            print(f"*Could not find {DEBUG_OUTPUT_DUMP_NAME} that should be generated by debug example ***")
            exit(1)
        except Exception as e:
            print(f"*Exception {e} ocurred while trying to find {DEBUG_OUTPUT_DUMP_NAME} that should be generated by debug example ***")
            exit(1)
        process = run_process("mv", DEBUG_OUTPUT_DUMP_NAME, DEBUG_EXPECTED_OUTPUT_PATH)
        if process.returncode:
            print("*When trying to rename the dump file ***")
            print("stderr: " + process.stderr.decode(ENCODING))
            exit(1)
        
        print("Precompute of debug test was successfull")
        exit(failed_tests != 0)


print(f"quering debug expected output in {DEBUG_EXPECTED_OUTPUT_PATH}")

try:
    debug_output_file = open(DEBUG_EXPECTED_OUTPUT_PATH, "r")
    debug_output_file.close()
except FileNotFoundError:
    print(f"Could not find expected debug test output in {DEBUG_EXPECTED_OUTPUT_PATH}")
    exit(1)
except Exception as e:
    print(f"Exception {e} ocurred while trying to find the expected debug test output in {DEBUG_EXPECTED_OUTPUT_PATH}")
    exit(1)


debug_test_failed = 0

process = run_process(DEBUG, DEBUG_EXAMPLE_PATH, "<", DEBUG_INPUT_PATH)

if process.returncode != 0:
    print("Debug Failed For " + DEBUG_EXAMPLE_PATH)
    print("stderr: " + process.stderr.decode(ENCODING))
    debug_test_failed = 1
else:
    try:
        dummy = open(DEBUG_OUTPUT_DUMP_NAME, "r")
        dummy.close()
    except FileNotFoundError:
        print(f"*Could not find {DEBUG_OUTPUT_DUMP_NAME} that should be generated by debug example ***")
        exit(1)
    except Exception as e:
        print(f"*Exception {e} ocurred while trying to {DEBUG_OUTPUT_DUMP_NAME} that should be generated by debug example ***")
        exit(1)

    if not cmpf(DEBUG_OUTPUT_DUMP_NAME, DEBUG_EXPECTED_OUTPUT_PATH, "r"):
        print("Debug test does generate expected output")
        debug_test_failed = 1


if debug_test_failed:
    print("*Debug test failed ***")
else:
    print("Degub test was successfull")

exit(failed_tests != 0 or debug_test_failed)