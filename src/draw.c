/********************************************************************\
* DRAW.C - part of alopex, see alopex.c for license information
*
* Author: Jesse McClure, copyright 2012-2013
\********************************************************************/

/********************************************************************/
/*  LOCAL DATA                                                      */
/********************************************************************/

#include "alopex.h"

/********************************************************************/
/*  GLOBAL FUNCTIONS                                                */
/********************************************************************/

int draw() {
	tile();
	if (!m->focus) m->focus = m->container;
	if (m->focus->top)
		XSetInputFocus(dpy,m->focus->top->win,
				RevertToPointerRoot,CurrentTime);
	XFlush(dpy);
}

int draw_background(Container *C) {
	cairo_set_source_rgba(C->bar.ctx,0.5,0.5,0.5,1);
	cairo_rectangle(C->bar.ctx,0,0,C->w,BAR_HEIGHT(C->bar.opts));
	cairo_fill(C->bar.ctx);
}

int draw_tab(Container *C, int con, Client *c, int n, int count) {
	//if (con < 2) {
		//status bar
	//}
	int tw = C->w/count;
	int tx = n * tw;
	int th = BAR_HEIGHT(C->bar.opts);
	if (m->focus == C && C->top == c)
		cairo_set_source_rgba(C->bar.ctx,1,1,0,1);
	else if (C->top == c)
		cairo_set_source_rgba(C->bar.ctx,0,1,1,1);
	else
		cairo_set_source_rgba(C->bar.ctx,0,0,0.5,1);
	cairo_rectangle(C->bar.ctx,tx+2,2,tw-4,th-4);
	cairo_fill(C->bar.ctx);
cairo_set_source_rgba(C->bar.ctx,0,0,0,1);
cairo_move_to(C->bar.ctx,tx+4,th-4);
cairo_show_text(C->bar.ctx,c->title);
cairo_stroke(C->bar.ctx);
}

/********************************************************************/
/*  LOCAL FUNCTIONS                                                 */
/********************************************************************/

