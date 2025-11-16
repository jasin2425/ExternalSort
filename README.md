External Sort: "Merging with Large Buffers" Project

This is a C++ implementation of the external merge sort algorithm, specifically using the "Merging with Large Buffers" method. The program is designed to sort binary files that are too large to fit into main memory, simulating disk I/O operations (pages) to minimize disk access.

This project was completed for the Database Structures course.

🚀 Features

Stage 1 (Create Runs): Reads the input file in $n$-page chunks, sorts them in-memory, and writes them to disk as sorted "runs".

Stage 2 (Merge Runs): Implements a multi-phase merge process to combine all runs into a single sorted file.

Min-Heap: Uses a custom MinHeap to efficiently merge $k = n-1$ runs simultaneously.

Disk Simulation: A DiskManager class abstracts all file I/O into page-based operations (readPage, writePage) and counts them.

Flexible Input: Supports data input from randomly generated records, keyboard, or a .txt file.

Statistics: Reports the total number of page reads, page writes, and merge phases at the end of the process.

⚙️ How to Build and Run

1. Build

The project is written in C++ and uses features like <memory> (C++11 or newer).

# Compile using g++
g++ -std=c++11 main.cpp -o external_sort


2. Run

# On Linux/macOS
./external_sort

# On Windows
external_sort.exe


The program will then prompt you to enter:

n (number of buffers): The number of pages available in RAM.

b (blocking factor): The number of records per page.

Data Source: Choose how to generate the initial data (random, keyboard, or file).

The final sorted result will be saved as output.bin.
