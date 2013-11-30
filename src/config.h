

/* STATUS FORMAT STRING
	%T	tags (icon + name)
	%t tags (name + icon)
	%n tags (name)
	%i tags (icon)
	%C clock (24 hour)
	%c clock (12 hour)
STATUS INPUT NOT IMPLEMENTED JUST YET
	%1 first line of status input
	%2 second (or previous) line of status input
	%3 ...
	...
*/

const char *status_fmt = "%T\n %C \n";
//const char *status_fmt = "%T\n %1 %C \n";
const char *font_path = "/usr/share/fonts/TTF/DroidSansMono.ttf";
//const char *font_path = "/usr/share/fonts/TTF/comic.ttf";
const int font_size = 14;

const Bool focusfollowmouse = False;
const int containers[]	= { 1, 3, 3, -1 };
//const int container_pad	= 4;
const int container_pad	= 2;
const int client_opts	= ATTACH_TOP;
//const int bar_opts		= BAR_VISIBLE | BAR_TOP | SET_BAR_HEIGHT(18);
const int bar_opts		= BAR_VISIBLE | BAR_TOP;
const char *tag_names[] = { "one", "two", "three", "four", NULL };
const int tag_pad			= 5;

const Theme theme[] = {
						/*  X    Y     W    H     Radius */
	[tabOffset]			= { 2.0, 2.0, -4.0, 10.0, 5.0 },
						/*  R    G    B    A    Ignore */
	[tabRGBAFocus]		= { 1.0, 1.0, 0.0, 0.8, 0.0 },
	[tabRGBATop]		= { 0.0, 1.0, 1.0, 0.7, 0.0 },
	[tabRGBAOther]		= { 0.0, 0.0, 1.0, 0.5, 0.0 },
						/*  R    G    B    A    Alignment */
	[tabRGBAFocusText]	= { 0.0, 0.0, 0.0, 1.0, 0.0 },
	[tabRGBATopText]	= { 0.0, 0.0, 0.0, 1.0, 0.0 },
	[tabRGBAOtherText]	= { 0.0, 0.0, 0.0, 1.0, 0.0 },
};


const char *string[127] = {
	[ 0 ] = "mm",
	[ 1 ] = "mm",
	['m'] = "interrobang",
	['t'] = "urxvt",
	['w'] = "firefox",
};


/* clients:
	0	currently focused
	1	last focused
	2	user marks
	3	''
	...
*/

#define KEY1	Mod4Mask
#define KEY2	Mod1Mask
#define KEY3	ControlMask
#define KEY4	ShiftMask

/* tag-like: */
#define TagKey(KEY,TAG)	\
	{ KEY1,			KEY,	"t" TAG }, \
	{ KEY1|KEY2,	KEY,	"x" TAG }, \
	{ KEY1|KEY3,	KEY,	"T" TAG }, \
	{ KEY1|KEY4,	KEY,	"m" TAG }

/* more "desktop-like": */
/*
#define TagKey(KEY,TAG)	\
	{ KEY1,			KEY,	"x" TAG }, \
	{ KEY1|KEY2,	KEY,	"t" TAG }, \
	{ KEY1|KEY3,	KEY,	"m" TAG }, \
	{ KEY1|KEY4,	KEY,	"T" TAG }
*/

const Key key[] = {
	{ KEY1,	XK_Return,	"ct"},
	{ KEY1,	XK_p,			"cm"},
	{ KEY1,	XK_w,			"cw"},
	{ KEY1,	XK_q,			"Q"},
	{ KEY2,	XK_F4,		"q0"},
	{ KEY1,	XK_Tab,		"o"},
	{ KEY2,	XK_Tab,		"v"},
	TagKey(	XK_1,			"1"),
	TagKey(	XK_2,			"2"),
	TagKey(	XK_3,			"3"),
	TagKey(	XK_4,			"4"),
	TagKey(	XK_5,			"5"),
	TagKey(	XK_6,			"6"),
};





/* keys  
    e g      n pqr  u w yz
 BCDE G I    NOPQR  UVW YZ

COMMAND MODE
	b[#][sSxXtT]
		# = bar number (default = 1)
		s|S = show/hide
		x|X = show exclusively (hide others) (default)
		t|T = top/bottom
	t#
		toggle visibility of tag # 
	T#[WIN]
		toggle window's presence on tag # (default = current win)
	(s|S|x|X)#
		show/hide exlusive/all-but tag #
	(m|a|A)#
		move/add/remove focused window to tag #
	v
		toggle view
	c[a-z]
		launch command associated with a-z
			w = firefox
			t = urxvt
			m = interrobang
	[#]j|k
		move focus up/down in stack # times
	h|l
		move focus to top/bottom of stack
	o
		foccus "other"
	[#]J|K
		move window up/down in stack # times
	H|L
		move window to top/bottom of stack
	[#]i|d
		increase/decrease master size by # steps
	[#]+|-
		increase/decrease nclients by #
	>|<
		nclients = 1/all
	f
		toggle fullscreen
	F
		toggle floating
	M[rbm]
		Tile mode: rstack, bstack, monocle (default = cycle)
	ENTER | ESC
		return to normal mode (execute string on ENTER, ignore on ESC)
*/
