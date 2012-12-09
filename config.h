/* See LICENSE file for copyright and license details. */

static const char font[] = "-misc-fixed-medium-r-normal--13-120-75-75-c-70-*-*";
//static const char font[] = "-*-terminus-bold-r-*--12-120-72-72-c-60-*-*";
static int barheight = 0;	/* zero: ttwm calculates height based on font */

/* STATUSBARSPACE: horizontal pixel space for rectabar input */
#define STATUSBARSPACE	50
#define WORKSPACES		5
#define TTWM_CURSOR		XC_left_ptr
/* VIDEO1 = local monitor VIDEO2 = any external monitor connection */
#define VIDEO1	"LVDS1"
#define VIDEO2	"VGA1"

static const char colors[LASTColor][9] = {
    [Background]    =   "#101010",
    [Clock]         =   "#484862",
    [SpacesNorm]    =   "#484862",
    [SpacesActive]  =   "#6284FF",
    [SpacesSel]     =   "#FFDD0E",
    [SpacesUrg]     =   "#FF0000",
    [TitleNorm]     =   "#646464",
    [TitleSel]      =   "#8888AA",
    [StackNorm]     =   "#282828",
    [StackAct]      =   "#424242",
    [StackSel]      =   "#848484",
    [StackNormBG]   =   "#101010",
    [StackActBG]    =   "#181818",
    /* set StackSelBG to same as term background for good tab look */
    [StackSelBG]    =   "#181818",
};

/* minimum portion of screen that a window can take */
/* and granularity in adjustment of master window size */
#define FACT_MIN	0.2
#define FACT_ADJUST	0.02
static Bool bstack	= False;	/* start with bottom stack? */
static Bool columns = True;		/* start with two columns - or monocle-like mode */
static float fact	= 0.50;		/* portion of screen for master window */
static Bool topbar	= True;		/* bar on top (or bottom) of screen */

#define DMENU		"dmenu_run -fn \"-*-terminus-bold-r-*--12-120-72-72-c-60-*-*\" -nb \"#101010\" -nf \"#484862\" -sb \"#080808\" -sf \"#FFDD0E\""
#define TERM		"urxvt" 		/* or "urxvtc","xterm","terminator",etc */
#define CMD(app)	app "&"

/* key definitions */
#define MODKEY Mod4Mask
static Key keys[] = {
	/* modifier			key			function	argument */
	/* launchers: */
	{ MODKEY,			XK_Return,	spawn,		CMD(TERM)		},
	{ MODKEY,			XK_p,		spawn,		CMD(DMENU)		},
	{ MODKEY,			XK_w,		spawn,		CMD("luakit")	},
	/* misc: (swap swaps master with top stack client) */
	{ MODKEY|ShiftMask,	XK_q,		quit,		NULL			},
	{ MODKEY,			XK_space,	swap,		NULL			},
	{ MODKEY,			XK_f,		fullscreen,	NULL			},
	{ Mod1Mask,			XK_F4,		killclient,	NULL			},
	{ MODKEY,			XK_Print,	stack,		NULL			},
	{ MODKEY,			XK_a,		stackmode,	"hide bar"		},
	{ MODKEY,			XK_x,		stackmode,	"flip bar"		},
	/* switch focus / rearrange client windows: */
	{ MODKEY,			XK_j,		focus,		"left"			},
	{ MODKEY,			XK_k,		focus,		"right"			},
	{ MODKEY,			XK_h,		move,		"left"			},
	{ MODKEY,			XK_l,		move,		"right"			},
	{ MODKEY,			XK_Left,	focus,		"left"			},
	{ MODKEY,			XK_Right,	focus,		"right"			},
	{ MODKEY|ShiftMask,	XK_Left,	move,		"left"			},
	{ MODKEY|ShiftMask,	XK_Right,	move,		"right"			},
	{ MODKEY,			XK_Tab,		focus,		"other"			},
	{ MODKEY|ShiftMask,	XK_Tab,		focus,		"toggle"		},
	/* increase or decrease master size.  */
	{ MODKEY,			XK_i,		stackmode,	"increase"		},
	{ MODKEY,			XK_d,		stackmode,	"decrease"		},
	/* set bottom stack, right stack, or toggle between the two */
	{ MODKEY,			XK_b,		stackmode,	"bottom"		},
	{ MODKEY,			XK_r,		stackmode,	"right"			},
	{ MODKEY,			XK_t,		stackmode,	"toggle"		},
	/* single window (monocle-like) or master+stack */
	{ MODKEY,			XK_Up,		stackmode,	"single"		},
	{ MODKEY,			XK_Down,	stackmode,	"multiple"		},
	/* change workspaces: */
	{ MODKEY,			XK_1,		workspace,	"1"				},
	{ MODKEY,			XK_2,		workspace,	"2"				},
	{ MODKEY,			XK_3,		workspace,	"3"				},
	{ MODKEY,			XK_4,		workspace,	"4"				},
	{ MODKEY,			XK_5,		workspace,	"5"				},
	/* move client to workspace: */
	{ MODKEY|ShiftMask,	XK_1,		putclient,	"1"				},
	{ MODKEY|ShiftMask,	XK_2,		putclient,	"2"				},
	{ MODKEY|ShiftMask,	XK_3,		putclient,	"3"				},
	{ MODKEY|ShiftMask,	XK_4,		putclient,	"4"				},
	{ MODKEY|ShiftMask,	XK_5,		putclient,	"5"				},
	/* external monitor support: */
	{ MODKEY|ShiftMask,	XK_a,		exscreen,	"activate"		},
	{ MODKEY|ShiftMask,	XK_d,		exscreen,	"deactivate"	},
	{ MODKEY|ShiftMask,	XK_s,		exscreen,	"send"			},
	{ MODKEY|ShiftMask,	XK_r,		exscreen,	"return"		},
};

// vim: ts=4
