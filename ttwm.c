/**************************************************************************\
* TTWM - a tiny tabbed tagging tiling wm
*
* Author: Jesse McClure, copyright 2012-2013
* License: GPLv3
*
* Based on code from TinyWM by Nick Welch <mack@incise.org>, 2005,
* with coode influence from DWM by the Suckless team (suckless.org),
* and concepts inspired by i3 (i3wm.org).
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*
\**************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/extensions/Xrandr.h>

#define MAX(a,b)	((a)>(b) ? (a) : (b))

#define TTWM_FLOATING	0x0010
#define TTWM_TRANSIENT	0x0030
#define TTWM_FULLSCREEN	0x0040
#define TTWM_TOPSTACK	0x0080
#define TTWM_URG_HINT	0x0100
#define TTWM_FOC_HINT	0x0200
#define TTWM_ANY		0xFFFF

#define GET_MON(x)		(x->flags & 0x000F)
#define SET_MON(x,n)	{ x->flags &= ~0x000F; x->flags += n; }
#define INC_MON(x)		(x->flags += ((x->flags & 0x000F) == nscr ? 0 : 1))
#define DEC_MON(x)		(x->flags -= ((x->flags & 0x000F) == 0 ? 0 : 1))

enum { Background, Default, Occupied, Selected, Urgent, Title, LASTColor };
enum { MOff, MMove, MResize };

typedef struct {
	unsigned int mod;
	KeySym keysym;
	void (*func)(const char *);
	const char *arg;
} Key;

typedef struct {
	unsigned int mod, button;
	void (*func)(const char *);
	const char *arg;
} Button;


typedef struct Client Client;
struct Client {
	Window win, parent;
	Client *next;
	int x,y,w,h;
	int tags, flags;
	char *title;
};

/* not used yet:
typedef stuct {
	int x, y, w, h;
	Pixmap buf;
	Window bar;
} Monitor;
*/

/*********************** [1] PROTOTYPES & VARIABLES ***********************/
/* 1.0 EVENT HANDLER PROTOTYPES */
static void buttonpress(XEvent *);
static void buttonrelease(XEvent *);
static void configurerequest(XEvent *);
static void enternotify(XEvent *);
static void expose(XEvent *);
static void keypress(XEvent *);
static void maprequest(XEvent *);
static void motionnotify(XEvent *);
static void propertynotify(XEvent *);
static void unmapnotify(XEvent *);

/* 1.1 BINDABLE FUNCTION PROTOTYPES */
static void fullscreen(const char *);
static void killclient(const char *);
static void monitor(const char *);
static void mouse(const char *);
static void quit(const char *);
static void spawn(const char *);
static void tag(const char *);
static void tile_conf(const char *);
static void tile(const char *);
static void toggle(const char *);
static void window(const char *);

/* 1.2 TTWM INTERNAL PROTOTYPES */
static inline Bool tile_check(Client *, int);
static int draw();
static int get_hints(Client *);
static int neighbors(Client *);
static GC setcolor(int);
static int swap(Client *, Client *);
static int tile_mode(int (*)(int,int,int,int,int,int),int, int);
static int tile_bstack(int,int,int,int,int,int);
static int tile_monocle(int,int,int,int,int,int);
static int tile_rstack(int,int,int,int,int,int);
static Client *wintoclient(Window);
static int xerror(Display *,XErrorEvent *);

/* 1.3 GLOBAL VARIABLES */
#include "icons.h"
#include "config.h"
static Display *dpy;
static Window root, bar[2];
//static Monitor *mon;
static int scr, sw, sh, ex_sw, ex_sh, nscr = 1;
static Colormap cmap;
static XColor color;
static GC gc,bgc;
static XFontStruct *fontstruct;
static int fontheight, barheight, statuswidth = 0;
static Pixmap buf[2], sbar, iconbuf;
static Bool running = True;
static XButtonEvent mouseEvent;
static int mouseMode = MOff, ntilemode = 0;
static Client *clients = NULL, *focused = NULL, *nextwin, *prevwin, *altwin;
static FILE *inpipe;
static const char *noname_window = "(UNNAMED)";
static int tagsUrg = 0, tagsSel = 1, maxTiled = 1;
static void (*handler[LASTEvent]) (XEvent *) = {
	[ButtonPress]		= buttonpress,
	[ButtonRelease]		= buttonrelease,
	[ConfigureRequest]	= configurerequest,
	[EnterNotify]		= enternotify,
	[Expose]			= expose,
	[KeyPress]			= keypress,
	[MapRequest]		= maprequest,
	[PropertyNotify]	= propertynotify,
	[MotionNotify]		= motionnotify,
	[UnmapNotify]		= unmapnotify,
};

/************************ [2] FUNCTION DEFINITIONS ************************/
/* 2.0 EVENT HANDLERS */

void buttonpress(XEvent *ev) {
	Client *c;
	XButtonEvent *e = &ev->xbutton;
	if (!( (c=wintoclient(e->subwindow)) && e->state) ) return;
	if (c && e->button < 4) { focused = c; XRaiseWindow(dpy,c->win); }
	int i;
	mouseEvent = *e;
	for (i = 0; i < sizeof(buttons)/sizeof(buttons[0]); i++)
		if ( (e->button == buttons[i].button) && buttons[i].func &&
				buttons[i].mod == ((e->state&~Mod2Mask)&~LockMask) )
			buttons[i].func(buttons[i].arg);
	if (mouseMode != MOff)
		XGrabPointer(dpy,root,True,PointerMotionMask | ButtonReleaseMask,
				GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
	draw();
}

void buttonrelease(XEvent *ev) {
	XUngrabPointer(dpy, CurrentTime);
	mouseMode = MOff;
}

void configurerequest(XEvent *ev) {
	XConfigureRequestEvent *e = &ev->xconfigurerequest;
	Client *c;
	if ( (c=wintoclient(e->window)) && (e->width==sw) && (e->height==sh) ) {
		c->flags |= TTWM_FULLSCREEN;
		draw();
		return;
	}
	XWindowChanges wc;
	wc.x = e->x; wc.y = e->y; 
	wc.width = e->width; wc.height = e->height;
	wc.border_width = borderwidth;
	wc.sibling = e->above; wc.stack_mode = e->detail;
	XConfigureWindow(dpy,e->window,e->value_mask,&wc);
	XFlush(dpy);
}

void enternotify(XEvent *ev) {
	if (!focusfollowmouse) return;
	Client *c = wintoclient(ev->xcrossing.window);
	if (c) {
		focused = c;
		draw();
	}
}

void expose(XEvent *ev) {
	draw();
}

void keypress(XEvent *ev) {
	unsigned int i;
	XKeyEvent *e = &ev->xkey;
	KeySym keysym = XkbKeycodeToKeysym(dpy,(KeyCode)e->keycode,0,0);
	for (i = 0; i < sizeof(keys)/sizeof(keys[0]); i++) {
		if ( (keysym == keys[i].keysym) && keys[i].func &&
				keys[i].mod == ((e->state&~Mod2Mask)&~LockMask) )
			keys[i].func(keys[i].arg);
	}
}

void maprequest(XEvent *ev) {
int mon=0; //TODO <--
	Client *c, *p;
	XWindowAttributes wa;
	XMapRequestEvent *e = &ev->xmaprequest;
	if (!XGetWindowAttributes(dpy,e->window,&wa) || wa.override_redirect) return;
	if (wintoclient(e->window)) return;
	if (!(c=calloc(1,sizeof(Client)))) exit(1);
	c->win = e->window;
	c->w = wa.width; c->h = wa.height;
	c->x = (sw-c->w)/2; c->y = (sh-c->h)/2;
	c->tags = (	(tagsSel & 0xFFFF) ? tagsSel : (tagsSel |= 1) ) & 0xFFFF;
	if (c->tags == 0) c->tags = 1;
	if (c->x < 0) c->x = 0; if (c->y < 0) c->y = 0;
	if ( (c->w==sw) && (c->h==sh) ) c->flags |= TTWM_FULLSCREEN;
	if (XGetTransientForHint(dpy,c->win,&c->parent))
		c->flags |= TTWM_TRANSIENT;
	else
		c->parent = e->parent;
	if (!XFetchName(dpy,c->win,&c->title) || c->title == NULL) {
		if ( (p=wintoclient(c->parent)) ) c->title = strdup(p->title);
		else c->title = strdup(noname_window);
	}
	get_hints(c);
	XSelectInput(dpy,c->win,PropertyChangeMask | EnterWindowMask);
	if (clients && attachmode == 1) {
		for (p = clients; p && p->next && !tile_check(p,mon); p = p->next);
		c->next = p->next; p->next = c;
	}
	else if (clients && attachmode == 2) {
		for (p = clients; p->next; p = p->next); p->next = c;
	}
	else {
		c->next = clients; clients = c;
	}
	XSetWindowBorderWidth(dpy,c->win,borderwidth);
	XMapWindow(dpy,c->win);
	XRaiseWindow(dpy,c->win);
	focused = c;
	if (!(c->flags & TTWM_FLOATING)) tile(tile_modes[ntilemode]);
	draw();
}

void motionnotify(XEvent *ev) {
	int xdiff, ydiff;
	while (XCheckTypedEvent(dpy,MotionNotify,ev));
	XButtonEvent *e = &ev->xbutton;
	xdiff = e->x_root - mouseEvent.x_root;
	ydiff = e->y_root - mouseEvent.y_root;
	if (mouseMode == MMove) {
		focused->x+=xdiff; focused->y+=ydiff; draw();
	}
	else if (mouseMode == MResize) {
		focused->w+=xdiff; focused->h+=ydiff; draw();
	}
	focused->flags |= TTWM_FLOATING;
	mouseEvent.x_root+=xdiff; mouseEvent.y_root+=ydiff;
}

void propertynotify(XEvent *ev) {
	XPropertyEvent *e = &ev->xproperty;
	Client *c, *p;
	if (!(c=wintoclient(e->window)) ) return;
	if (e->atom == XA_WM_NAME) {
		XFree(c->title); c->title = NULL;
		if (!XFetchName(dpy,c->win,&c->title) || c->title == NULL) {
			if ( (p=wintoclient(c->parent)) ) c->title = strdup(p->title);
			else c->title = strdup(noname_window);
		}
		draw();
	}
	else if (e->atom == XA_WM_HINTS) {
		get_hints(c);


		draw();
	}
	// TODO Size hints for fullscreen?
}

void unmapnotify(XEvent *ev) {
	Client *c,*t;
	if (!(c=wintoclient(ev->xunmap.window)) || ev->xunmap.send_event) return;
	if (c == focused) {
		neighbors(c);
		if (nextwin) focused = nextwin;
		else focused = prevwin;
	}
	if (c == clients) clients = c->next;
	else {
		for (t = clients; t && t->next != c; t = t->next);
		t->next = c->next;
	}
	XFree(c->title);
	free(c); c = NULL;
	tile(tile_modes[ntilemode]);
	draw();
}

/* 2.1 BINDABLE FUNCTIONS */

void fullscreen(const char *arg) {
	if (!focused) return;
	focused->flags ^= TTWM_FULLSCREEN;
	XRaiseWindow(dpy,focused->win);
	draw();
}

void killclient(const char *arg) {
	if (!focused) return;
	XEvent ev;
	ev.type = ClientMessage; ev.xclient.window = focused->win;
	ev.xclient.message_type = XInternAtom(dpy,"WM_PROTOCOLS",True);
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = XInternAtom(dpy,"WM_DELETE_WINDOW",True);
	ev.xclient.data.l[1] = CurrentTime;
	XSendEvent(dpy,focused->win,False,NoEventMask,&ev);
}

void monitor(const char *arg) {
/* CURRENTLY only good for two monitors
   eventually will work with any arbitrary number */
	char cmd[128];
	if (arg[0] == 'a') {
		nscr = 2;
		sprintf(cmd,"xrandr --output %s --auto --output %s --auto --%s %s",
			video1, video2, video_location, video1);
		system(cmd);
		int num_sizes;
		XRRScreenSize *xrrs = XRRSizes(dpy,scr,&num_sizes);
		XRRScreenConfiguration *config = XRRGetScreenInfo(dpy,root);
		Rotation rotation;
		int sizeID = XRRConfigCurrentConfiguration(config,&rotation);
		XRRFreeScreenConfigInfo(config);
		ex_sw = xrrs[sizeID].width; ex_sh = xrrs[sizeID].height;
		buf[1] = XCreatePixmap(dpy,root,ex_sw,barheight,DefaultDepth(dpy,scr));
		bar[1] = XCreateSimpleWindow(dpy,root,0,(topbar ? 0:ex_sh - barheight)+sh,
			ex_sw,barheight,0,0,0);
		XSetWindowAttributes wa;
		wa.override_redirect = True;
		wa.event_mask = ExposureMask;
		XChangeWindowAttributes(dpy,bar[0],CWOverrideRedirect|CWEventMask,&wa);
		XMapWindow(dpy,bar[1]);
	}
	else if (arg[0] == 'd') {
		nscr = 1;
		sprintf(cmd,"xrandr --output %s --auto --output %s --off", video1, video2);
		system(cmd);
		ex_sw = 0; ex_sh = 0;
		XFreePixmap(dpy,buf[1]);
		XDestroyWindow(dpy,bar[1]);
	}
	else if (arg[0] == 's' && focused) INC_MON(focused);
	else if (arg[0] == 'r' && focused) DEC_MON(focused);
	tile(tile_modes[ntilemode]);
}

void mouse(const char *arg) {
	if (arg[0] == 'm') mouseMode = MMove;
	else if (arg[0] == 'r') mouseMode = MResize;
}

void quit(const char *arg) {
	running = False;
}

void spawn(const char *arg) {
	system(arg);
}

void tag(const char *arg) {
	int i = arg[2] - 49;
	if (arg[0] == 's') tagsSel = ((tagsSel & 0xFFFF0000) | (1<<i));
	else if (arg[0] == 'f') tagsSel = ( tagsSel<<16 | tagsSel >>16);
	else if (arg[0] == 't') tagsSel ^= (1<<i);
	else if (arg[0] == 'a' && focused ) focused->tags ^= (1<<i);
	else if (arg[0] == 'm' && focused ) focused->tags = (1<<i);
	tile(tile_modes[ntilemode]);
	draw();
}

void tile_conf(const char *arg) {
	if (arg[0] == 'i') tilebias += 4;
	else if (arg[0] == 'd') tilebias -= 4;
	else if (arg[0] == '+') stackcount++;
	else if (arg[0] == '-') stackcount--;
	else if (arg[0] == 'a') stackcount = maxTiled;
	else if (arg[0] == 'o') stackcount = 1;
	if (stackcount < 1) stackcount = 1;
	else if (stackcount > maxTiled) stackcount = maxTiled;
	if (tilebias > sh/2 - 2*win_min) tilebias = sh/2 - 2*win_min;
	else if (tilebias < 2*win_min - sh/2) tilebias = 2*win_min - sh/2;
	tile(tile_modes[ntilemode]);
}

void tile(const char *arg) {
	int i,j;
	Client *c,*t = NULL;
	for (i = 0; tile_modes[i]; i++) 
		if (arg[0] == tile_modes[i][0]) ntilemode = i;
	if (ex_sw & ex_sh) for (i = 0, j = 0, c = clients; c; c = c->next) {
		if (c->tags & tagsSel && !(c->flags & TTWM_FLOATING) &&
			(GET_MON(c) == 0)) i++;
		if (c->tags & tagsSel && !(c->flags & TTWM_FLOATING) &&
			(GET_MON(c) == 1)) j++;
	}
	else for (i = 0, j = 0, c = clients; c; c = c->next) {
		if (c->tags & tagsSel) {
			if (!t && !(focused && (focused->tags & tagsSel))) t = c;
			if (t && (c->tags & TTWM_FULLSCREEN)) t = c;
			if (!(c->flags & TTWM_FLOATING)) i++;
		}
	}
	if (t) focused = t;
	maxTiled = MAX(i,j) - 1;
	if (i > stackcount + 1) i = stackcount + 1;
	if (j > stackcount + 1) j = stackcount + 1;
	if (i == 0 && j == 0) return;
	else if (arg[0] == 'b') tile_mode(&tile_bstack,i,j);
	else if (arg[0] == 'm') tile_mode(&tile_monocle,i,j);
	else if (arg[0] == 'r') tile_mode(&tile_rstack,i,j);
	else if (arg[0] == 'c') {
		if (!tile_modes[++ntilemode]) ntilemode = 0;
		tile(tile_modes[ntilemode]);
	}
	if (focused) {
		XRaiseWindow(dpy,focused->win);
		for (c = clients; c; c = c->next)
			if ( (c->tags & tagsSel) && (c->flags & TTWM_FLOATING))
				XRaiseWindow(dpy,c->win);
	}
	draw();
}

void toggle(const char *arg) {
	if (arg[0] == 'p') topbar = !topbar;
	else if (arg[0] == 'v') showbar = !showbar;
	else if (arg[0] == 'f' && focused) focused->flags ^= TTWM_FLOATING;
	XMoveWindow(dpy,bar[0],(showbar ? 0 : -2*sw),(topbar ? 0 : sh-barheight));
	// TODO bar[1]
	tile(tile_modes[ntilemode]);
}

void window(const char *arg) {
	if (!focused) return;
	if (focused->flags & TTWM_FLOATING) return;
	neighbors(focused);
	Client *t = NULL;
	if (arg[2] == 'p') t = prevwin;
	else if (arg[2] == 'n') t = nextwin;
	else if (arg[2] == 'a') t = altwin;
	if (!t) return;
	if (arg[0] == 'f') focused = t;
	else if (arg[0] == 's') { swap(focused, t); focused = t; }
	XRaiseWindow(dpy,focused->win);
	draw();
}

/* 2.2 WM INTERNAL FUNCTIONS */

static inline void draw_tab(Pixmap buf, int x, int w, int col) {
	XPoint top_pts[6] = { {x,barheight}, {0,2-barheight}, {2,-2},
		{w-4,0}, {2,2}, {0,barheight-2} };
	XPoint bot_pts[6] = { {x,0}, {0,barheight-3}, {2,2}, {w-4,0},
			{2,-2}, {0,3-barheight} };
	XDrawLines(dpy,buf,setcolor(col),(topbar ? top_pts : bot_pts),6,CoordModePrevious);
}

inline Bool tile_check(Client *c, int mon) {
	if (GET_MON(c) < nscr) return (c && (c->tags & tagsSel) && !(c->flags & TTWM_FLOATING) && (GET_MON(c) == mon) );
	else return (c && (c->tags & tagsSel) && !(c->flags & TTWM_FLOATING) && (nscr == mon + 1) );
}

static void draw_tabs(Pixmap buf, int x, int w, int mid, int mon) {
	Client *c;
	int count, tab1w, tabw;
	/* get count - skip if zero */
	for (count = 0, c = clients; c; c = c->next)
		if (tile_check(c,mon)) count++;
	if (!count) return;
	/* set tab widths */
	if (tile_modes[ntilemode][0] == 'm' || count == 1) tab1w = tabw = w/count - 8;
	else { tab1w = mid - x; tabw = (x + w - mid)/(count - 1) - 8; }
	if (tabw < 20) tabw = 20;
	/* draw master title and tab */
	for (c = clients; !(tile_check(c,mon)); c = c->next);
	setcolor(c == focused ? Title : Default);
	if (c->flags & TTWM_URG_HINT) setcolor(Urgent);
	XDrawString(dpy,buf,gc,x,fontheight,c->title,strlen(c->title));
	XFillRectangle(dpy,buf,setcolor(Background),x + tab1w - 4,0,w-x-tab1w,barheight);
	draw_tab(buf,x-8,tab1w+5,Occupied);
	if (count == 1) return;
	x += tab1w+8;
	/* draw stack titles and tabs */
	for (c = c->next; c; c = c->next) {
		if (!tile_check(c,mon)) continue;
		setcolor( (c->flags & TTWM_URG_HINT ? Urgent :
			(c == focused ? Title : Default)) );
		XDrawString(dpy,buf,gc,x,fontheight,c->title,strlen(c->title));
		XFillRectangle(dpy,buf,setcolor(Background),x+tabw-4,0,sw-x-tabw,barheight);
		draw_tab(buf,x-8,tabw+5,(c->flags & TTWM_TOPSTACK ? Occupied : Title));
		x += tabw+8;
	}
}

int draw() {
	tagsUrg &= ~tagsSel;
	int tagsOcc = 0, nstack = -1;
	/* windows */
	Client *stack = clients, *master = NULL, *slave = NULL;
	XSetWindowAttributes wa;
	while (stack) {
		tagsOcc |= stack->tags;
		if (!(stack->tags & tagsSel)) { /* not on selected tag(s) */
			XMoveWindow(dpy,stack->win,-2*sw,0);
			stack = stack->next; continue;
		}
		else if (!(stack->flags & TTWM_FLOATING)) { /* tiled */
			if (!master) master = stack;
			else if (!slave) slave = stack;
			else if (stack->flags & TTWM_TOPSTACK) slave = stack;
			stack->flags &= ~TTWM_TOPSTACK; nstack++;
		}
		else { /* floating */
			stack->flags &= ~TTWM_TOPSTACK;
		}
		if (stack->flags & TTWM_FULLSCREEN )
			XMoveResizeWindow(dpy,stack->win,-borderwidth,-borderwidth,sw,sh);
		else
			XMoveResizeWindow(dpy,stack->win,stack->x,stack->y,stack->w,stack->h);
		if (stack == focused) setcolor(Selected);
		else setcolor(Occupied);
		wa.border_pixel = color.pixel;
		XChangeWindowAttributes(dpy,stack->win,CWBorderPixel,&wa);
		stack = stack->next;
	}
	if (focused) {
		if ( (focused != master) && !(focused->flags & TTWM_FLOATING) )
			slave = focused;
		if ( !(focused->flags & TTWM_FOC_HINT) )
			XSetInputFocus(dpy,focused->win,RevertToPointerRoot,CurrentTime);
		focused->flags &= ~TTWM_URG_HINT;
	}
	if (slave) slave->flags |= TTWM_TOPSTACK;
	/* tags */
	XFillRectangle(dpy,buf[0],setcolor(Background),0,0,sw,barheight);
	if (nscr > 1)
		XFillRectangle(dpy,buf[1],setcolor(Background),0,0,ex_sw,barheight);
	int i,x=10,w=0,col, tagsAlt = (tagsSel>>16);
	for (i = 0; tag_name[i]; i++) {
		if (!((tagsOcc|tagsSel) & (1<<i))) continue;
		col = (tagsUrg & (1<<i) ? Urgent :
			(tagsOcc & (1<<i) ? Occupied : Default));
		if (focused && focused->tags & (1<<i)) col = Selected;
		XDrawString(dpy,buf[0],setcolor(col),x,fontheight,
			tag_name[i],strlen(tag_name[i]));
		w = XTextWidth(fontstruct,tag_name[i],strlen(tag_name[i]));
		if (tagsSel & (1<<i))
			XFillRectangle(dpy,buf[0],gc,x-2,fontheight+1,w+4,barheight-fontheight);
		if (tagsAlt & (1<<i))
			XFillRectangle(dpy,buf[0],gc,x-2,0,w+4,2);
		x+=w+10;
	}
	if ( (x=x+20) < sw/10 ) x = sw/10; /* add padding */
	/* titles / tabs */
	//for (i = 0; i < nscr; i++)
	//	draw_tabs(buf[i],x,sw - x - statuswidth - 10,sw/2+tilebias,i);
draw_tabs(buf[0],x,sw - x - statuswidth - 10,sw/2+tilebias,0);
if (nscr > 1)
draw_tabs(buf[1],10,ex_sw - 10,ex_sw/2+tilebias,1);
	/* status */
	if (statuswidth)
		XCopyArea(dpy,sbar,buf[0],gc,0,0,statuswidth,barheight,sw-statuswidth,0);
	XCopyArea(dpy,buf[0],bar[0],gc,0,0,sw,barheight,0,0);
	if (nscr > 1)
		XCopyArea(dpy,buf[1],bar[1],gc,0,0,ex_sw,barheight,0,0);
	XFlush(dpy);
	return 0;
}

int get_hints(Client *c) {
	XWMHints *hint;
	if ( (hint=XGetWMHints(dpy,c->win)) ) {
		if (hint->flags & XUrgencyHint) {
			tagsUrg |= c->tags;
			c->flags |= TTWM_URG_HINT;
		}
		else if (hint->flags & InputHint && !hint->input) {
			c->flags |= TTWM_FOC_HINT;
		}
		XFree(hint);
	}
	return 0;
}

int neighbors(Client *c) {
	prevwin = NULL; nextwin = NULL; altwin = NULL;
	if (!(c->tags & tagsSel)) return -1;
	Client *stack, *t;
	for (stack = clients; stack && stack != c; stack = stack->next)
		if (stack->tags & tagsSel && !(stack->flags & TTWM_FLOATING) &&
			(GET_MON(stack) == GET_MON(c)) ) prevwin = stack;
	for (nextwin = stack->next; nextwin; nextwin = nextwin->next)
		if ( (nextwin->tags & tagsSel) && !(nextwin->flags & TTWM_FLOATING) &&
			(GET_MON(stack) == GET_MON(c)) ) break;
	for (t = clients; t &&
		( !((t->tags & tagsSel) && (GET_MON(t) == GET_MON(c))) ||
			(t->flags & TTWM_FLOATING) );
		t = t->next);
	if (!t) return -1;
	for (stack = t->next; stack && 
			!( (GET_MON(stack) == GET_MON(c)) &&
			!(stack->flags & TTWM_FLOATING) &&
			(stack->flags & TTWM_TOPSTACK) );
		stack = stack->next);
	if (!stack) return -1;
	if (stack == focused) altwin = t;
	else altwin = stack;
	return 0;
}

GC setcolor(int col) {
	XAllocNamedColor(dpy,cmap,colors[col],&color,&color);
	XSetForeground(dpy,gc,color.pixel);
	return gc;
}

int status(char *msg) {
	char col[8] = "#000000";
	char *t,*c = msg;
	int l, arg;
	int lastwidth = statuswidth;
	statuswidth = 0;
	XFillRectangle(dpy,sbar,setcolor(Background),0,0,sw/2,barheight);
	setcolor(Default);
	while (*c != '\n') {
		if (*c == '{') {
			if (*(++c) == '#') {
				strncpy(col,c,7);
				XAllocNamedColor(dpy,cmap,col,&color,&color);
				XSetForeground(dpy,gc,color.pixel);
			}
			else if (*c == 'i' && sscanf(c,"i %d",&arg) == 1) {
				XFillRectangle(dpy,iconbuf,bgc,0,0,iconwidth,iconheight);
				XDrawPoints(dpy,iconbuf,gc,icons[arg].pts,icons[arg].n,
					CoordModeOrigin);
				XCopyArea(dpy,iconbuf,sbar,gc,0,0,iconwidth,iconheight,statuswidth,
						(barheight-iconheight)/2);
				statuswidth+=iconwidth+1;
			}
			c = strchr(c,'}')+1;
		}
		else {
			if ( (t=strchr(c,'{'))==NULL) t = strchr(c,'\n');
			if ( (l = (t == NULL ? 0 : t - c) ) ) {
				XDrawString(dpy,sbar,gc,statuswidth,fontheight,c,l);
				statuswidth += XTextWidth(fontstruct,c,l);
			}
			c+=l;
		}
	}
	if (statuswidth == lastwidth) {
		XCopyArea(dpy,sbar,bar[0],gc,0,0,statuswidth,barheight,sw-statuswidth,0);
		XFlush(dpy);
	}
	else draw();
	return 0;
}

int swap(Client *a, Client *b) {
	if (!a || !b) return -1;
	Client t;
	t.title = a->title; a->title=b->title; b->title = t.title;
	t.tags = a->tags; a->tags = b->tags; b->tags = t.tags;
	t.flags = a->flags; a->flags = b->flags; b->flags = t.flags;
	t.win = a->win; a->win = b->win; b->win = t.win;
	if (a->flags & TTWM_TOPSTACK) {
		a->flags &= ~TTWM_TOPSTACK; b->flags |= TTWM_TOPSTACK;
	}
	else if (b->flags & TTWM_TOPSTACK) {
		b->flags &= ~TTWM_TOPSTACK; a->flags |= TTWM_TOPSTACK;
	}
	return 0;
}

int tile_mode(int (*func)(int,int,int,int,int,int),int count, int excount) {
	int (*in)(int,int,int,int,int,int) = func;
	int (*ex)(int,int,int,int,int,int) = func;
	if (count == 1) in = &tile_monocle;
	if (excount == 1) ex = &tile_monocle;
	if (count)
		in(count, 0,
			tilegap,
			(showbar ? (topbar ? barheight : 0) : 0) + tilegap,
			sw-2*(tilegap+borderwidth),
			sh - (showbar ? barheight : 0) - 2*(tilegap+borderwidth));
	if (excount)
		ex(excount, 1,
			tilegap,
			(showbar ? (topbar ? barheight : 0) : 0) + tilegap + sh,
			ex_sw-2*(tilegap+borderwidth),
			ex_sh - (showbar ? barheight : 0) - 2*(tilegap+borderwidth));
	return 0;
}	

int tile_bstack(int count,int flag,int x, int y, int w, int h) {
	Client *c, *t = NULL;
	for (c = clients; !tile_check(c,flag); c = c->next);
	int wh = (h - tilegap)/2 - borderwidth;
	int ww = w/(count - 1) - tilegap/2;
	c->x = x; c->y = y;	c->w = w; c->h = wh + tilebias;
	int i = 0;
	while ( (c=c->next) ) {
		if (!tile_check(c,flag)) continue;
		c->x = x + i*(ww + tilegap + borderwidth);
		c->y = y + wh + tilegap + 2*borderwidth + tilebias;
		c->w = MAX(ww - borderwidth, win_min);
		c->h = wh - tilebias;
		if (!t && i == count - 2) t = c;
		i = ( i == count - 2 ? count - 2 : i + 1);
	}
	while (t) {
		if (tile_check(t,flag)) t->w = MAX(x + w - t->x,win_min);
		t = t->next;
	}
	return 0;
}

int tile_monocle(int count,int flag,int x, int y, int w, int h) {
	Client *c = clients;
	for (c = clients; !tile_check(c,flag); c = c->next);
	if (c) do {
		if (!tile_check(c,flag)) continue;
		c->x = x; c->y = y;	c->w = w; c->h = h;
	} while ( (c=c->next) );
	return 0;
}

int tile_rstack(int count,int flag,int x, int y, int w, int h) {
	Client *c, *t = NULL;
	for (c = clients; !tile_check(c,flag); c = c->next);
	int ww = (w - tilegap)/2 - borderwidth;
	int wh = h/(count - 1) - tilegap/2;
	c->x = x; c->y = y; c->w = ww + tilebias; c->h = h;
	int i = 0;
	while ( (c=c->next) ) {
		if (!tile_check(c,flag)) continue;
		c->x = x + ww + tilegap + 2*borderwidth + tilebias;
		c->y = y + i*(wh + tilegap + borderwidth);
		c->w = ww - tilebias;
		c->h = MAX(wh - borderwidth,win_min);
		if (!t && i == count - 2) t = c;
		i = ( i == count - 2 ? count - 2 : i + 1);
	}
	while (t) {
		if (tile_check(t,flag)) t->h = MAX(y + h - t->y, win_min);
		t = t->next;
	}
	return 0;
}

Client *wintoclient(Window w) {
	Client *c;
	for (c = clients; c && c->win != w; c = c->next);
	return c;
}

int xerror(Display *d, XErrorEvent *ev) {
	char msg[1024];
	XGetErrorText(dpy,ev->error_code,msg,sizeof(msg));
	fprintf(stderr,"[TTWM] (%d:%d) %s\n",ev->request_code,ev->error_code,msg);
	return 0;
}

int main(int argc, const char **argv) {
	if (argc > 1) inpipe = popen(argv[1] ,"r");
	else inpipe = popen("while :; do date \"+%I:%M %p\"; sleep 10; done","r");
	/* init X */
	if (!(dpy=XOpenDisplay(0x0))) return 1;
	scr = DefaultScreen(dpy);
	sw = DisplayWidth(dpy,scr);
	sh = DisplayHeight(dpy,scr);
	root = DefaultRootWindow(dpy);
	XSetErrorHandler(xerror);
	XDefineCursor(dpy,root,XCreateFontCursor(dpy,ttwm_cursor));
	/* gc init */
	cmap = DefaultColormap(dpy,scr);
	XColor color;
	XGCValues val;
	val.font = XLoadFont(dpy,font);
	fontstruct = XQueryFont(dpy,val.font);
	fontheight = fontstruct->ascent+1;
	barheight = fontstruct->ascent+fontstruct->descent+2;
	gc = XCreateGC(dpy,root,GCFont,&val);
	bgc = DefaultGC(dpy,scr);
	XAllocNamedColor(dpy,cmap,colors[Background],&color,&color);
	XSetForeground(dpy,bgc,color.pixel);
	/* buffers and windows */
	bar[0] = XCreateSimpleWindow(dpy,root,0,(topbar ? 0 : sh - barheight),
			sw,barheight,0,0,0);
	buf[0] = XCreatePixmap(dpy,root,sw,barheight,DefaultDepth(dpy,scr));
	sbar = XCreatePixmap(dpy,root,sw/2,barheight,DefaultDepth(dpy,scr));
	iconbuf = XCreatePixmap(dpy,root,iconwidth,iconheight,DefaultDepth(dpy,scr));
	XSetWindowAttributes wa;
	wa.override_redirect = True;
	wa.event_mask = ExposureMask;
	XChangeWindowAttributes(dpy,bar[0],CWOverrideRedirect|CWEventMask,&wa);
	XMapWindow(dpy,bar[0]);
	wa.event_mask = ExposureMask |FocusChangeMask | SubstructureNotifyMask |
			ButtonReleaseMask | PropertyChangeMask | SubstructureRedirectMask |
			StructureNotifyMask;
	XChangeWindowAttributes(dpy,root,CWEventMask,&wa);
	XSelectInput(dpy,root,wa.event_mask);
	/* key and mouse bindings */
	unsigned int mods[] = {0, LockMask, Mod2Mask, LockMask|Mod2Mask};
	KeyCode code;
	XUngrabKey(dpy,AnyKey,AnyModifier,root);
	int i,j;
	for (i = 0; i < sizeof(keys)/sizeof(keys[0]); i++)
		if ( (code=XKeysymToKeycode(dpy,keys[i].keysym)) ) for (j = 0; j < 4; j++)
			XGrabKey(dpy,code,keys[i].mod|mods[j],root,True,
					GrabModeAsync,GrabModeAsync);
	for (i = 0; i < sizeof(buttons)/sizeof(buttons[0]); i++)
		if (buttons[i].mod) for (j = 0; j < 4; j++)
			XGrabButton(dpy,buttons[i].button,buttons[i].mod|mods[j],root,True,
					ButtonPressMask,GrabModeAsync,GrabModeAsync,None,None);
	/* main loop */
	draw();
	XEvent ev;
	int xfd, sfd;
	fd_set fds;
	sfd = fileno(inpipe);
	xfd = ConnectionNumber(dpy);
	char *line = (char *) calloc(max_status_line+1,sizeof(char));
	while (running) {
		FD_ZERO(&fds);
		FD_SET(sfd,&fds);
		FD_SET(xfd,&fds);
		select(xfd+1,&fds,0,0,NULL);
		if (FD_ISSET(xfd,&fds)) while (XPending(dpy)) {
			XNextEvent(dpy,&ev);
			if (handler[ev.type]) handler[ev.type](&ev);
		}
		if (FD_ISSET(sfd,&fds)) {
			if (fgets(line,max_status_line,inpipe))
				status(line);
		}
	}
	/* clean up */
	XFreePixmap(dpy,buf[0]);
	XDestroyWindow(dpy,bar[0]);
	free(line);
	XFreeFontInfo(NULL,fontstruct,1);
	XUnloadFont(dpy,val.font);
	XCloseDisplay(dpy);
	return 0;
}

// vim: ts=4	

