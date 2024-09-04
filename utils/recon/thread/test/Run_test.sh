#!/bin/bash
"""
cd utils/recon/thread/test/
bash Run_test.sh
"""

rm -rf test_thread
gcc test_thread.c -o test_thread -lpthread -g
./test_thread

rm -rf test_omp
gcc test_omp.c -o test_omp -fopenmp -lpthread -g
./test_omp