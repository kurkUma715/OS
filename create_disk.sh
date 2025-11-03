#!/bin/bash

# Создаем образ диска
dd if=iso/huesos.iso of=disk.img bs=4M conv=fsync oflag=direct status=progress
