import subprocess as sp
import shutil
import os
import glob

SRC = "src"
DIST = "dist"
BIN = "avx-4x8-filter.exe"

CC_PATH = r"C:\mingw-w64\i686-8.1.0-release-win32-sjlj-rt_v6-rev0\mingw32\bin"

def run_cmd(cmd):
    print(cmd)
    step = sp.run(cmd, check=True)
    
def clear_dir(dir):
    try:
        shutil.rmtree(dir, ignore_errors=True)
    except FileNotFoundError:
        pass
    os.mkdir(dir)

def build_bin():
    run_cmd(f'{CC_PATH}/g++ -O3 -g3 -fno-exceptions -march=i686 -m32 -mavx -mavx2 -masm=intel {SRC}/*.c {SRC}/*.cpp -static-libgcc -static-libstdc++ -o {DIST}/{BIN}')

if __name__ == "__main__":
    clear_dir(DIST)
    build_bin()
