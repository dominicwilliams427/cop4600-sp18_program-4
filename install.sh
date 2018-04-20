#!/bin/bash
make
sudo insmod charkmod-in.ko
sudo insmod charkmod-out.ko
sudo mknod /dev/charkmod-in c 246 0
sudo mknod /dev/charkmod-out c 245 0
sudo chmod 777 /dev/charkmod-in
sudo chmod 777 /dev/charkmod-out 
