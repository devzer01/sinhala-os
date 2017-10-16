#!/bin/bash

for pixel in `cat txt`; 
do 
   printf 0x%X,  "$((2#$pixel))"
done
