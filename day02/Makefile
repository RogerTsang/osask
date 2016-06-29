osask.img : osask.asm
	nasm -o $@ $<

.PHONY:
run_vbox:
	VBoxManage storageattach osask --storagectl "Floppy" --port 0 --device 0 --medium osask.img
	virtualbox --startvm osask

run:
	qemu-system-i386 -drive file=osask.img,if=floppy

clean:
	rm osask.img 
