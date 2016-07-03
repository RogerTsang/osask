INCPATH  = ../haribote/

MAKE     = make -r
NASK     = nask
CC1      = cc1 -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = gas2nask -a
OBJ2BIM  = obj2bim
BIN2OBJ  = bin2obj
BIM2HRB  = bim2hrb
MAKEFONT = makefont
RULEFILE = ../haribote/haribote.rul
EDIMG    = edimg
COPY     = cat
DEL      = rm

LIBRARIES = bootpack.obj naskfunc.obj hankaku.obj graphic.obj dsctbl.obj int.obj fifo.obj mouse.obj keyboard.obj memory.obj

default :
	$(MAKE) img

ipl10.bin : ipl10.asm Makefile
	$(NASK) ipl10.asm ipl10.bin ipl10.lst

asmhead.bin : asmhead.asm Makefile
	$(NASK) asmhead.asm asmhead.bin asmhead.lst

hankaku.bin : hankaku.txt Makefile
	$(MAKEFONT) hankaku.txt hankaku.bin

%.gas : %.c Makefile
	$(CC1) -o $*.gas $*.c

%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas

%.obj : %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst

# Piority: Normal Rules -> General Rules
naskfunc.obj : naskfunc.asm Makefile
	$(NASK) naskfunc.asm naskfunc.obj

hankaku.obj : hankaku.bin Makefile
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku

bootpack.bim : $(LIBRARIES) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		$(LIBRARIES)	
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
	$(MAKE) img
	VBoxManage storageattach osask --storagectl "Floppy" --port 0 --device 0 --medium haribote.img
	virtualbox --startvm osask

run :
	$(MAKE) img
	qemu-system-i386 -drive file=haribote.img,if=floppy,format=raw

clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.gas
	-$(DEL) *.obj
	-$(DEL) *.nas
	-$(DEL) bootpack.nas
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) haribote.sys

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img
