INCPATH  = ../z_tools/haribote/

MAKE     = make -r
NASK     = nask
CC1      = cc1 -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = gas2nask -a
OBJ2BIM  = obj2bim
BIM2HRB  = bim2hrb
RULEFILE = ../haribote/haribote.rul
EDIMG    = edimg
COPY     = cat
DEL      = rm

default :
	$(MAKE) img

ipl10.bin : ipl10.asm Makefile
	$(NASK) ipl10.asm ipl10.bin ipl10.lst

asmhead.bin : asmhead.asm Makefile
	$(NASK) asmhead.asm asmhead.bin asmhead.lst

bootpack.gas : bootpack.c Makefile
	$(CC1) -o bootpack.gas bootpack.c

bootpack.nas : bootpack.gas Makefile
	$(GAS2NASK) bootpack.gas bootpack.nas

bootpack.obj : bootpack.nas Makefile
	$(NASK) bootpack.nas bootpack.obj bootpack.lst

naskfunc.obj : naskfunc.asm Makefile
	$(NASK) naskfunc.asm naskfunc.obj naskfunc.lst

bootpack.bim : bootpack.obj naskfunc.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		bootpack.obj naskfunc.obj
# 3MB+64KB=3136KB

bootpack.hrb : bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0

haribote.sys : asmhead.bin bootpack.hrb Makefile
	$(COPY) asmhead.bin bootpack.hrb > haribote.sys

haribote.img : ipl10.bin haribote.sys Makefile
	$(EDIMG) imgin:../fdimg0at.tek \
		wbinimg src:ipl10.bin len:512 from:0 to:0 \
		copy from:haribote.sys to:@: \
		imgout:haribote.img

img :
	$(MAKE) haribote.img

run_vbox:
	VBoxManage storageattach osask --storagectl "Floppy" --port 0 --device 0 --medium haribote.img
	virtualbox --startvm osask

run :
	qemu-system-i386 -drive file=haribote.img,if=floppy

clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.gas
	-$(DEL) *.obj
	-$(DEL) bootpack.nas
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) haribote.sys

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img
