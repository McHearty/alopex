
#ifndef __ALOPEX_H__
#define __ALOPEX_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <cairo.h>

#define BAR_TOP		0x0100
#define BAR_BOTTOM	0x0200
#define BAR_VISIBLE	0x0400

#define BAR_HEIGHT(x)		(x & 0x00FF)
#define SET_BAR_HEIGHT(x)	x

#define RSTACK			0x1000
#define BSTACK			0x2000

#define ATTACH_TOP		0x0001
#define ATTACH_BOTTOM	0x0002
#define ATTACH_ABOVE		0x0003
#define ATTACH_BELOW		0x0004

typedef struct Key {
	unsigned short int mod;
	KeySym keysym;
	const char *arg;
} Key;

typedef struct Client Client;
struct Client {
	Client *next;
	Window win;
	char *title;
	// icon
	int tags;
};

typedef struct Bar {
	Window win;
	Pixmap buf;
	cairo_t *ctx;
	int opts;
} Bar;

typedef struct Container Container;
struct Container {
	Container *next;
	int n, w;
	Bar bar;
	Client *top;
};

typedef struct Monitor Monitor;
struct Monitor {
	Monitor *next;
	int x, y, w, h, gap;
	int tags, mode, split;
	Container *container;
	Container *focus;
};

Monitor *mons;
Monitor *m;
Client *clients;
Display *dpy;
int scr;
Window root;
Bool running;
GC gc;

extern const char *status_fmt;
extern const char *string[];

#endif /* __ALOPEX_H__ */

