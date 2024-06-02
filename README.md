# ALPS
Created by: Jamie Prendiville, Calixt Charlebois

## What Is ALPS?
ALPS (Accelerated Linked Parallel System) is a high-performance remote desktop application written in C++, leveraging NVIDIA's CUDA toolkit for rapid, parallel processing to ensure seamless and efficient remote desktop experiences.

## Dependencies
* **A NVIDIA GPU CAPABLE OF CUDA PROCESSING**
* [NVIDIA CUDA Toolkit](https://developer.nvidia.com/cuda-toolkit) (>= 12.2)
* ZRTPCPP
* openssl

## How to build
Currently, settings within the Visual Studio project are absolute rather than relative paths. This results in a complicated setup, please open the solution settings and follow the steps below:

>### VC++ Directories
>* Locate your installation of the NVIDIA CUDA Toolkit and add the directory to ```External Include Directories``` as well as the interface of the NVIDIA folder included in ```lib```
>* Add the ```x64``` library of the NVIDIA CUDA Toolkit as well as the Win32 library of the NIVIDA folder included to ```Library Directories```
>### Linker -> General
>* Add the ```x64``` library of the NVIDIA CUDA Toolkit to ```Addtional Library Directories```
>### Linker -> Input
>* Add ```cuda.lib``` to ```Addtional Dependencies```
>

## Contributions
Contributions are always welcome! When making a pull request, please give a larger summary of all changes as well as how they were made. This allows us to make better judgement!