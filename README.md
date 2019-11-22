# k_means
[![Build status](https://ci.appveyor.com/api/projects/status/405uogcy90af30ex?svg=true)](https://ci.appveyor.com/project/Defvyb/k-means)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=Defvyb_k_means&metric=alert_status)](https://sonarcloud.io/dashboard?id=Defvyb_k_means)


**Implementation of k_means algorithm;**

Input dataset format:

    X1 Y2 ... N1, Where N - dimensions number <=1000
    X2 Y2 ... N2,
    ...
    XM YM ... NM, Where M <= 1.000.000.000

example:

Input File(input.file):

    1 2
    3 4
    5 6
    7 8
    9 10
    11 12
    13 14
    15 16
    17 18
    19 20
    21 22
    23 24
    25 26
    27 28
    29 30
    
Input params:
test_task2 -f=input.file -t=1 -k=2

Output File(output.file):

    8 9
    23 24

Usage: 

    -f=<filename>(mandatory parameter)
    -t=<thread pool size>(Default: 1) 
       <thread pool size> <= <kluster centroids count> && <thread pool size> < 31 
    -k=<kluster centroids count>(Default: 10)
    -m=<max iterations>(Default: 1000000)
    -o=<filename>(Default: output.file)
    -n not to check file. Will slightly increase speed(Default: is false)
    -h help