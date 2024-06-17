# CUDAatScaleForTheEnterpriseCourseProjectTemplate
This is a template for the course project for the CUDA at Scale for the Enterprise

## Project Description

This project is just a basic experiment with ImageNPP lib and few filters. In the course lab box npp filter was used, I refactored the code a bit and added gauss filter that is working with the same image type pgm where image is grayscale and 8 bits per point is used.

I used coursera visual studio code sandbox to run a code, binaries are compiled for x86_64 arc.

## Code Organization

```bin/```
This folder should hold all binary/executable code that is built automatically or manually. Executable code should have use the .exe extension or programming language-specific extension.

```data/```
This folder should hold all example data in any format. If the original data is rather large or can be brought in via scripts, this can be left blank in the respository, so that it doesn't require major downloads when all that is desired is the code/structure.

```lib/```
Any libraries that are not installed via the Operating System-specific package manager should be placed here, so that it is easier for inclusion/linking.

```src/```
The source code should be placed here in a hierarchical fashion, as appropriate.

```README.md```
This file should hold the description of the project so that anyone cloning or deciding if they want to clone this repository can understand its purpose to help with their decision.

```INSTALL```
check this file if you want to build and run code your self

```Makefile or CMAkeLists.txt or build.sh```
There should be some rudimentary scripts for building your project's code in an automatic fashion.

```run.sh```
Is an entry point to run the code, it has predefined arguments that will take necessary inputs. You can override them in case you want to use another image or filter.


