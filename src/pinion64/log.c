/*
 * Copyright (C) 2012 Nick Johnson <nickbjohnson4224 at gmail.com>
 * 
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include "port.h"
#include "log.h"

int z;

static void logputc(char c);
static void logputs(const char *str);
static void logputd(int n);
static void logputx(uint32_t x);
static void logputp(uint64_t p);

// abstract logging interface

void log(int level, const char *fmt, ...) {
	va_list ap;

	switch (level) {
	case INFO:  logputs("[info]  "); break;
	case INIT:  logputs("[init]  "); break;
	case DEBUG: logputs("[debug] "); break;
	case ERROR: logputs("[error] "); break;
	}

	va_start(ap, fmt);
	while (*fmt) {
		if (*fmt == '%' && fmt[1] != '\0') {
			switch (fmt[1]) {
				case 's': logputs(va_arg(ap, const char*)); break;
				case 'c': logputc(va_arg(ap, int)); break;
				case 'd': logputd(va_arg(ap, int)); break;
				case 'x': logputx(va_arg(ap, uint32_t)); break;
				case 'p': logputp(va_arg(ap, uint64_t)); break;
				case '%': logputc('%'); break;
			}
			fmt++;
		}
		else {
			logputc(*fmt);
		}
		fmt++;
	}

	va_end(ap);
	logputc('\n');
}

// device-independent formatting functions

static const char *digit = "0123456789ABCDEF";

static void logputs(const char *str) {
	
	if (!str) {
		logputs("(null)");
		return;
	}

	for (size_t i = 0; str[i]; i++) {
		logputc(str[i]);
	}
}

static void logputd(int n) {
	char buffer[10];
	int i;

	if (n < 0) {
		logputc('-');
		n = -n;
	}

	if (n == 0) {
		logputc('0');
		return;
	}

	for (i = 0; i < 10 && n; i++) {
		buffer[i] = digit[n % 10];
		n /= 10;
	}
	i--;

	for (; i >= 0; i--) {
		logputc(buffer[i]);
	}
}

static void logputx(uint32_t x) {
	
	for (int i = 0; i < 8; i++) {
		logputc(digit[(x >> (28 - i * 4)) & 0xF]);
	}
}

static void logputp(uint64_t p) {
	
	for (int i = 0; i < 16; i++) {
		logputc(digit[(p >> (60 - i * 4)) & 0xF]);
	}
}

#ifdef LOGDEV_VGA

// VGA character device

static uint16_t * const vgabuf = (void*) 0xB8000;
static uint16_t vgacursor = 0;

static void vgaclear(void) {
	
	for (size_t i = 0; i < 1920; i++) vgabuf[i] = 0x0720;
}

static void vgaputc(char c) {
	static int cleared = 0;

	if (!cleared) {
		vgaclear();
		cleared = 1;
	}

	switch (c) {
	case '\n':
		vgacursor = vgacursor - (vgacursor % 80) + 80;
		break;
	case '\t':
		vgacursor = vgacursor - (vgacursor % 8) + 8;
		break;
	case '\r':
		vgacursor = vgacursor - (vgacursor % 80);
		break;
	default:
		vgabuf[vgacursor] = 0x0700 | c;
		vgacursor++;
	}
}

#endif

#ifdef LOGDEV_SERIAL

// serial character device

static void serialputc(char c) {
	static int serialinit = 0;

	if (!serialinit) {
		outb(0x3F9, 0x00);
		outb(0x3FB, 0x80);
		outb(0x3F8, 0x03);
		outb(0x3F9, 0x00);
		outb(0x3FB, 0x03);
		outb(0x3FA, 0xC7);
		outb(0x3FC, 0x0B);
		serialinit = 1;

		serialputc('\n');
	}

	while ((inb(0x3FD) & 0x20) == 0);

	if (c == '\n') outb(0x3F8, '\r');
	outb(0x3F8, c);
}

#endif

static void logputc(char c) {

	#ifdef LOGDEV_VGA
	vgaputc(c);
	#endif

	#ifdef LOGDEV_SERIAL
	serialputc(c);
	#endif
}
