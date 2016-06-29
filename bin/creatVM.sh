#!/bin/sh

# By https://github.com/RogerTsang

type VBoxManage >/dev/null 2>&1 || { echo "Virtual Box is not install"; exit 1;}

VM='osask'
VBoxManage createvm -name $VM --ostype "Other" --register
VBoxManage modifyvm $VM --memory 64 --vram 64
VBoxManage modifyvm $VM --boot1 floppy --boot2 none --boot3 none --boot4 none
VBoxManage storagectl $VM --name "Floppy" --add floppy --controller I82078 --bootable on --hostiocache on
