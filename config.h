/* See LICENSE file for copyright and license details. */

static const char font[] = "-*-terminus2-medium-r-*-*-*-*-*-*-*-*-*-*";

#define NUMCOLORS	19
#define BARHEIGHT	13
#define FONTHEIGHT	11
#define FONTWIDTH	6
#define WORKSPACES	5

#define CPU_FILE	"/proc/stat"
#define AUD_FILE	"/home/USERNAME/.audio_volume"
#define BATT_NOW	"/sys/class/power_supply/BAT1/charge_now"
#define BATT_FULL	"/sys/class/power_supply/BAT1/charge_full"
#define BATT_STAT	"/sys/class/power_supply/BAT1/status"

#define VIDEO1	"LVDS1"
#define VIDEO2	"VGA1"

static const char colors[NUMCOLORS][9] = {
	[Background]	=	"#101010",
	[Clock]			=	"#484862",
	[SpacesNorm]	=	"#484862",
	[SpacesActive]	=	"#6284FF",
	[SpacesSel]		=	"#FFDD0E",
	[SpacesUrg]		=	"#FF0000",  /* "Urgent" not yet implemented */
	[BarsNorm]		=	"#303030",
	[BarsFull]		=	"#122864",
	[BarsCharge]	=	"#308030",
	[BarsWarn]		=	"#997700",
	[BarsAlarm]		=	"#990000",
	[TitleNorm]		=	"#696969",
	[TitleSel]		=	"#4466FF",
	[StackNorm]		=	"#484862",
	[StackAct]		=	"#4466FF",
	[StackSel]		=	"#FFDD0E",
	[StackNormBG]	=	"#000000",
	[StackActBG]	=	"#080808",
	/* set StackSelBG to same as term background for good tab look */
	[StackSelBG]	=	"#080808",
};

/* minimum percentage of screen that a window can take */
/*  and granularity in adjustment of master window size */
#define FACT_MIN	0.2
#define FACT_ADJUST	0.02
static uint8_t bstack	= False;	/* start with bottom stack? */
static float fact		= 0.50;		/* portion of screen for master window */

#define DMENU		"dmenu_run -fn \"-*-terminus2-medium-r-*-*-*-*-*-*-*-*-*-*\" -nb \"#101010\" -nf \"#484862\" -sb \"#080808\" -sf \"#FFDD0E\""
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
	{ 0,				0x1008ff11,	spawn,		CMD("vol down")	},
	{ 0,				0x1008ff13,	spawn,		CMD("vol up")	},
	{ 0,				0x1008ff12,	spawn,		CMD("vol toggle")},
	{ 0,				0x1008ff2f,	spawn,		CMD("sleep 1; xset dpms force off")},
	/* misc: (swap swaps master with top stack client) */
	{ MODKEY|ShiftMask,	XK_q,		quit,		NULL			},
	{ MODKEY,			XK_space,	swap,		NULL			},
	{ MODKEY,			XK_f,		fullscreen,	NULL			},
	{ Mod1Mask,			XK_F4,		killclient,	NULL			},
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
	/* increase or decrease master size.  */
	/* set bottom stack, right stack, or toggle between the two */
	{ MODKEY,			XK_i,		stackmode,	"increase"		},
	{ MODKEY,			XK_d,		stackmode,	"decrease"		},
	{ MODKEY,			XK_b,		stackmode,	"bottom"		},
	{ MODKEY,			XK_r,		stackmode,	"right"			},
	{ MODKEY,			XK_t,		stackmode,	"toggle"		},
	/* change workspaces: */
	{ MODKEY,			XK_1,		workspace,	"1"	},
	{ MODKEY,			XK_2,		workspace,	"2"	},
	{ MODKEY,			XK_3,		workspace,	"3"	},
	{ MODKEY,			XK_4,		workspace,	"4"	},
	{ MODKEY,			XK_5,		workspace,	"5"	},
	/* move client to workspace: */
	{ MODKEY|ShiftMask,	XK_1,		putclient,	"1"	},
	{ MODKEY|ShiftMask,	XK_2,		putclient,	"2"	},
	{ MODKEY|ShiftMask,	XK_3,		putclient,	"3"	},
	{ MODKEY|ShiftMask,	XK_4,		putclient,	"4"	},
	{ MODKEY|ShiftMask,	XK_5,		putclient,	"5"	},
	/* external monitor support: HIGHLY EXPERIMENTAL, will likely break! */
	{ MODKEY|ShiftMask,	XK_a,		exscreen,	"activate"		},
	{ MODKEY|ShiftMask,	XK_d,		exscreen,	"deactivate"	},
	{ MODKEY|ShiftMask,	XK_s,		exscreen,	"send"			},
	{ MODKEY|ShiftMask,	XK_r,		exscreen,	"return"		},
};

// vim: ts=4
