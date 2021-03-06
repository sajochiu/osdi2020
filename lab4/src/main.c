#include <float.h>
#include <iso646.h>
#include <limits.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdnoreturn.h>

#include "tools.h"
#include "uart.h"
#include "mbox.h"
#include "peripherals.h"
#include "gpio.h"
#include "utils.h"
#include "lfb.h"
#include "printf.h"
#include "irq.h"
#include "timer.h"
#include "entry.h"

// for reboot
#define PM_PASSWORD (0x5a000000)
#define PM_RSTC ((int*)0x3F10001c)
#define PM_WDOG ((int*)0x3F100024)

extern unsigned char __bss_start;
extern unsigned char __bss_end;

// framebuffer extern settings
extern unsigned int width, height, pitch, isrgb;
extern unsigned char *lfb;

void sys_core_timer_enable(){
	
	// core timer init using system call
	asm volatile("sub sp, sp, 8");
	asm volatile("str x8, [sp, #8]");
	asm volatile("mov x8, #1");	// #1
	asm volatile("svc #0");	// arm system call
	asm volatile("ldr x8, [sp, #8]");
	asm volatile("add sp, sp, 8");
}

void shell(){
	
	char buffer[1000], buf1[1000], buf2[1000];
    int i1 = 0, i2 = 0, el = 0, sizeof_current_line = 0;
    _Bool command_not_found = 1;
	
	// Print the peripheral information
	get_serial_number();
	get_board_revision();
	get_VC_memory();

	uart_puts("\nWelcome to raspberry pi 3!\n# ");

	while(1){
		//sizeof_current_line = uart_get_string(buffer);
		sizeof_current_line = uart_get_string_with_echo(buffer);
		command_not_found = 1;
		if(!strcmp(buffer, "hello")) {
			uart_puts("Hello World!\n");
			command_not_found = 0;
		}else if(!strcmp(buffer, "help")) {
			uart_puts("hello: print Hello World!\n");
			uart_puts("help: help\n");
			uart_puts("reboot: reboot rpi3\n");
			uart_puts("timestamp: get current timestamp\n");
			uart_puts("framebuffer: show the framebuffer\n");
			uart_puts("loadimg: load kernel at specified address\n");
			command_not_found = 0;
		}else if(!strcmp(buffer, "timestamp")) {
			timestamp(&i1, &i2);
			// turn the number into string
			// char *buf = "", *buf1 = "";
			itoa(i1, buf1, 10);
			itoa(i2, buf2, 10);
			uart_puts("["); uart_puts(buf1); uart_puts(".");
			uart_puts(buf2); uart_puts("]"); uart_puts("\n");
			command_not_found = 0;
		}else if(!strcmp(buffer, "reboot")) {
			/* reboot rpi3 */
			*PM_RSTC = ((PM_PASSWORD) | 0x20); // full reset
  			*PM_WDOG = ((PM_PASSWORD) | 10); // number of watchdog tick
			
			command_not_found = 0;
			continue;
		}else if(!strcmp(buffer, "framebuffer")) {
			uart_puts("Show framebuffer\n");
			lfb_init();
			lfb_showpicture();
			command_not_found = 0;
		}else if(!strcmp(buffer, "loadimg")) {
			uart_puts("cannot load kernel image:(\n");
			command_not_found = 0;
		}else if(!strcmp(buffer, "exc")){
			asm volatile("svc #1");
			command_not_found = 0;
		}else if(!strcmp(buffer, "irq")){
			sys_core_timer_enable();
			command_not_found = 0;
		}else if(!strcmp(buffer, "brk")){
			asm volatile("brk #1");
			command_not_found = 0;
		}
		if(command_not_found == 1 && sizeof_current_line>0) {
			uart_puts("Error: command \"");
			uart_puts(buffer);
			uart_puts("\" not found. Try <help>\n");
		}
		uart_puts("# ");
		sizeof_current_line = 0;
    }
}

void main()
{

	//sys_core_timer_enable();
	shell();

	

}
