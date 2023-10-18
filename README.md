# Custom Memory Allocator

## Overview

This project is a custom memory allocator implementation. It includes various components such as allocation logic and utility scripts to facilitate the development and testing process.

## Components

### Allocation Logic

- **alloc.c**: Contains the core allocation logic of the custom memory allocator.

### Utility Scripts and Files

- **Makefile**: A makefile to manage the build process of the project.
- **Dockerfile**: A Dockerfile to containerize the application.

### OS X Specific

- **lib/osx-sbrk-mmap-wrapper.c**: A wrapper for sbrk and mmap functionalities specific to OS X.

## Building the Project

You can build the project using the provided `Makefile`. Run the following command in the project directory:

```bash
make
```

## Running the Project

After building the project, you can execute it as follows:

./alloc

## Docker Support

A Dockerfile is provided to containerize the application. You can build and run the Docker container using the following commands:

docker build -t custom-memory-allocator .
docker run custom-memory-allocator