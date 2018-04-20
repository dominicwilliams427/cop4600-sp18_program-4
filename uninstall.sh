#!/bin/bash
sudo rm /dev/charkmod-out
sudo rm /dev/charkmod-in
sudo rmmod charkmod-out
sudo rmmod charkmod-in
make clean
