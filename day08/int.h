#ifndef INT_H
#define INT_H

#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

#define PORT_KEYDAT     0x0060
#define PORT_KEYSTA     0x0064
#define PORT_KEYCMD     0x0064
#define KEYSTA_SEND_NOTREADY  0x02
#define KEYCMD_WRITE_MODE     0x60
#define KBC_MODE              0x47

#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE     0xf4

#define KB_BUFSIZE     32
#define MO_BUFSIZE     128

struct _mousedec {
    unsigned char phase;
    unsigned char buf[3];
    int x;
    int y;
    int btn;
};

void init_pic(void);

/* Keyboard Handler */
void inthandler21(int *esp);
void wait_KBC_sendready(void);
void init_keyboard(void);

/* Init Compulsory Interrupt */
void inthandler27(int *esp);

/* Mouse Handler */
void inthandler2c(int *esp);
void enable_mouse(struct _mousedec *mdec);
int mouse_decode(struct _mousedec *mdec, unsigned char dat);

#endif
