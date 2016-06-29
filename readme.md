# OSASK implementation in Linux enviorment

## Acknowledgement

Special thanks to [yourtion](https://github.com/yourtion) for providing osask Linux toolchains.

Special thanks to [zchrissirhcz](https://github.com/zchrissirhcz) for Makefile hint and qemu execution code.

## Purpose

* Create a friendly Linux environment for OSASK developers.
* Retain author code and variable names for better comprehension.
* Add English comments.

## Prerequist
- qemu: if you are using Debian Package Manager, use `sudo apt-get install qemu` (or fedora `su -c "dnf install qemu"` or `su -c "yum install qemu"`)
- virtualbox: vbox can be downloaded [here](https://www.virtualbox.org/wiki/Downloads)

## Configuration
- qemu: you are ready to go
- virtualbox: run `./bin/creatVM.sh` script to setup _osask_ VM for your virutalbox

## How to use
- Clone the repository and add ./bin/ folder under your .bashrc PATH variable.
- Each day() folder contains project source code and Makefile. Feel free to use them.
- Build OS image: `make`
- Emulate image with qemu: `make run`
- Emulate image with virtualbox: `make run_vbox`

## About me
You can contact me through github or zenghanzhang46@gmail.com
