CC=g++
# framework path
INC=../include/

CFLAGS=-I $(INC)

frame_files := $(INC)ps_framework.cpp
msort_files := $(INC)ps_merge_sort.cpp

h_frame_files := $(INC)ps_framework.hpp
h_msort_files := $(INC)ps_merge_sort.hpp

median_of_lines: $(h_frame_files) $(h_msort_files)
	$(CC) $(CFLAGS) -o median_of_lines median_of_lines.cpp $(frame_files) $(msort_files)


