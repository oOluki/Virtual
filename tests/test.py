import os
import platform
import sys
import subprocess
import locale

print(f"Default encoding: {locale.getpreferredencoding()}")

BUILD_DIR = sys.argv[1]
EXAMPLES_DIR = sys.argv[2]
PRECOMP_DIR = sys.argv[3]

if platform.system() == "Windows":
    BUILD_DIR = BUILD_DIR.replace("/", "\\")
    EXAMPLES_DIR = EXAMPLES_DIR.replace("/", "\\")
    PRECOMP_DIR = PRECOMP_DIR.replace("/", "\\")
    PATH_SEP = '\\'
    COMPILE = BUILD_DIR + PATH_SEP + "compile"
    DECOMPILE = BUILD_DIR + PATH_SEP + "decompile"
    RUN = BUILD_DIR + PATH_SEP + "VPU"

else:
    PATH_SEP = '/'
    COMPILE = BUILD_DIR + PATH_SEP + "compile"
    DECOMPILE = BUILD_DIR + PATH_SEP + "decompile"
    RUN = BUILD_DIR + PATH_SEP + "VPU"

os.makedirs(BUILD_DIR + PATH_SEP + "compiled", exist_ok=True)

def run_process(*command):
    return subprocess.run(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=False,
    )

def cmpf(f1, f2, mode):
    file1 = open(f1, mode)
    file2 = open(f2, mode)
    status = file1.read() == file2.read()
    file1.close()
    file2.close()
    return status

def precompute(example_path):
    EXAMPLE_NAME = example_path.removesuffix(".txt").split(PATH_SEP)
    EXAMPLE_NAME = EXAMPLE_NAME[len(EXAMPLE_NAME) - 1]

    PRECOMPILED = PRECOMP_DIR + PATH_SEP + EXAMPLE_NAME + ".out"
 
    PREDECOMPILED = PRECOMP_DIR + PATH_SEP + EXAMPLE_NAME + ".txt"
    
    process = run_process(COMPILE, example_path, "-o", PRECOMPILED)
    if process.returncode != 0:
        print("Compilation Failed For " + EXAMPLE_NAME)
        print("stderr: " + str(process.stderr))
        return 1

    process = run_process(DECOMPILE, PRECOMPILED, PREDECOMPILED)
    if process.returncode != 0:
        print("Decompilation Failed For " + EXAMPLE_NAME)
        print("stderr: " + str(process.stderr))
        return 1

    return 0


def test_example(example_path) -> int:
    EXAMPLE_NAME = example_path.removesuffix(".txt").split(PATH_SEP)
    EXAMPLE_NAME = EXAMPLE_NAME[len(EXAMPLE_NAME) - 1]
    print("Beggining Test " + EXAMPLE_NAME)

    COMPILED = BUILD_DIR + PATH_SEP + "compiled" + PATH_SEP + EXAMPLE_NAME + ".out"
    DECOMPILED = BUILD_DIR + PATH_SEP + "tmp.txt"
    
    PRECOMPILED = PRECOMP_DIR + PATH_SEP + EXAMPLE_NAME + ".out"
    #PREDECOMPILED = PRECOMP_DIR + PATH_SEP + EXAMPLE_NAME + ".txt"

    #print(COMPILE, example_path, "-o" , COMPILED)
    process = run_process(COMPILE, example_path, "-o", COMPILED)
    err_status = 0
    if process.returncode != 0:
        print("Compilation Failed For " + EXAMPLE_NAME)
        print("stderr: " + str(process.stderr))
        return 1
    elif cmpf(COMPILED, PRECOMPILED, "rb") == 0:
        print("Compiled " + EXAMPLE_NAME + " Does Not Match Expected")
        err_status = 1
 
    process = run_process(DECOMPILE, COMPILED, DECOMPILED)
    if process.returncode != 0:
        print("Decompilation Failed For " + EXAMPLE_NAME)
        print("stderr: " + str(process.stderr))
        err_status = 1
    else:
        process = run_process(COMPILE, DECOMPILED, "-o", BUILD_DIR + PATH_SEP + "tmp.out")
        if process.returncode != 0:
            print("Could Not Compile " + EXAMPLE_NAME + " Decompiled File")
            print("stderr: " + str(process.stderr))
            err_status = 1
        elif cmpf(BUILD_DIR + PATH_SEP + "tmp.out", COMPILED, "rb") == 0:
            print("Decompiled " + EXAMPLE_NAME + " Does Not Compile Back To Original Executable")
            err_status = 1
    
    process = run_process(RUN, COMPILED)
    if process.returncode != 0:
        print("Run Failed For " + EXAMPLE_NAME)
        print("stderr: " + str(process.stderr))
        err_status = 1

    if err_status == 0:
        print("Test " + EXAMPLE_NAME + " was successfull")
        print("stdout: " + str(process.stdout))
    else:
        print("*Test " + EXAMPLE_NAME + " failed ***")
    return err_status
    

examples = [f for f in os.listdir(EXAMPLES_DIR) if os.path.isfile(os.path.join(EXAMPLES_DIR, f))]
test_count = len(examples)
failed_tests = 0

if len(sys.argv) < 5:
    for example in examples:
        failed_tests += test_example(EXAMPLES_DIR + PATH_SEP + example)
    
    print(str(failed_tests) + " tests failed out of " + str(test_count) + " (" + str(100 * (float(failed_tests) / float(test_count))) + "%)" if failed_tests > 0 else "all tests were successfull")
else:
    for example in examples:
        failed_tests += precompute(EXAMPLES_DIR + PATH_SEP + example)
    
    print(str(failed_tests) + " precomputes failed out of " + str(test_count) + " (" + str(100 * (float(failed_tests) / float(test_count))) + "%)"  if failed_tests > 0 else "all precomputes were successfull")

exit(failed_tests != 0)
