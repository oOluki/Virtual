import os
import platform
import sys
import subprocess

BUILD_DIR = sys.argv[1]
EXAMPLES_DIR = sys.argv[2]
PRECOMP_DIR = sys.argv[3]

PATH_SEP = '\\' if platform.system() == "Windows" else '/'

COMPILE = BUILD_DIR + PATH_SEP + "compile"
DECOMPILE = BUILD_DIR + PATH_SEP + "decompile"
RUN = BUILD_DIR + PATH_SEP + "VPU"

os.makedirs(BUILD_DIR + PATH_SEP + "compiled", exist_ok=True)
os.makedirs(BUILD_DIR + PATH_SEP + "decompiled", exist_ok=True)

def run_process(*command):
    return subprocess.run(
        command,
        stdout=subprocess.PIPE,                                    stderr=subprocess.PIPE,                                    text=True
    )


def test_example(example_path) -> int:
    EXAMPLE_NAME = example_path.removesuffix(".txt").split(PATH_SEP)
    EXAMPLE_NAME = EXAMPLE_NAME[len(EXAMPLE_NAME) - 1]

    COMPILED_PATH = BUILD_DIR + PATH_SEP + "compiled" + PATH_SEP + EXAMPLE_NAME + ".out"
    DECOMPILED_PATH = BUILD_DIR + PATH_SEP + "decompiled" + PATH_SEP + EXAMPLE_NAME + ".txt"
    
    PRECOPMPILED_PATH = PRECOMP_DIR + PATH_SEP + EXAMPLE_NAME + ".out"
    PREDECOMPILED_PATH = PRECOMP_DIR + PATH_SEP + EXAMPLE_NAME + ".txt"

    process = run_process(COMPILE, example_path, "-o", COMPILED_PATH)
    if process.returncode != 0:
        print("Compilation Failed For " + EXAMPLE_NAME)
        print("stderr: " + process.stderr)
        return 1

    process = run_process(DECOMPILE, COMPILED_PATH, DECOMPILED_PATH)
    err_status = 0
    if process.returncode != 0:
        print("Decompilation Failed For " + EXAMPLE_NAME)
        print("stderr: " + process.stderr)
        err_status = 1
    
    process = run_process(RUN, COMPILED_PATH)
    if process.returncode != 0:
        print("Run Failed For " + EXAMPLE_NAME)
        print("stderr: " + process.stderr)
        err_status = 1

    if err_status == 0:
        print("Test " + EXAMPLE_NAME + " was successfull")
        print("stdout: " + process.stdout)
    else:
        print("*Test " + EXAMPLE_NAME + " failed ***")
    return err_status
    

examples = [f for f in os.listdir(EXAMPLES_DIR) if os.path.isfile(os.path.join(EXAMPLES_DIR, f))]
test_count = len(examples)
failed_tests = 0

for example in examples:
    failed_tests += test_example(EXAMPLES_DIR + PATH_SEP + example)

print(str(failed_tests) + " (" + str(100 * (float(failed_tests) / float(test_count))) + "%) tests failed out of " + str(test_count) if failed_tests > 0 else "all tests were successfull")
    

