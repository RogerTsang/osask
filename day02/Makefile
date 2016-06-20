osask.img : osask.asm
	nasm -o $@ $<

.PHONY:
install:
	cp osask.img ../

run:
	virtualbox --startvm osask

clean:
	rm osask.img ../osask.img
