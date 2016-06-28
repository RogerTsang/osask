default:
	$(MAKE) img

# Copy with dd command instead of edimg
# Thanks github.com/zchrissirchcz for providing the code
osask.img : ipl10.bin haribote.bin
	dd if=ipl10.bin of=osask.img bs=512 count=1
	# Fill the rest of floppy disk image with zero
	dd if=/dev/zero of=osask.img bs=512 skip=1 seek=1 count=2879
	# Virtual mount osask.img and write haribote.bin into osask.img
	mkdir -p /tmp/floppy
	sudo mount -o loop osask.img /tmp/floppy -o fat=12
	sudo cp haribote.bin /tmp/floppy
	sudo umount /tmp/floppy

ipl10.bin : ipl10.asm
	nasm -o $@ $^

haribote.bin : haribote.asm
	nasm -o $@ $^

.PHONY:
img: osask.img

install:
	cp osask.img ../osask.img

run:
	virtualbox --startvm osask

clean:
	-rm *.img *.bin
