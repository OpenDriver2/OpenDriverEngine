#include "game/pch.h"
#include <SDL.h>
#include <sol/sol.hpp>

#include "input.h"

struct CommonEnum
{
	const char* name;
	int number;
};

// SDL.Scancode
const CommonEnum KeyboardScancodes[] = {
	{ "Unknown",		SDL_SCANCODE_UNKNOWN	},
	{ "A",				SDL_SCANCODE_A			},
	{ "B",				SDL_SCANCODE_B			},
	{ "C",				SDL_SCANCODE_C			},
	{ "D",				SDL_SCANCODE_D			},
	{ "E",				SDL_SCANCODE_E			},
	{ "F",				SDL_SCANCODE_F			},
	{ "G",				SDL_SCANCODE_G			},
	{ "H",				SDL_SCANCODE_H			},
	{ "I",				SDL_SCANCODE_I			},
	{ "J",				SDL_SCANCODE_J			},
	{ "K",				SDL_SCANCODE_K			},
	{ "L",				SDL_SCANCODE_L			},
	{ "M",				SDL_SCANCODE_M			},
	{ "N",				SDL_SCANCODE_N			},
	{ "O",				SDL_SCANCODE_O			},
	{ "P",				SDL_SCANCODE_P			},
	{ "Q",				SDL_SCANCODE_Q			},
	{ "R",				SDL_SCANCODE_R			},
	{ "S",				SDL_SCANCODE_S			},
	{ "T",				SDL_SCANCODE_T			},
	{ "U",				SDL_SCANCODE_U			},
	{ "V",				SDL_SCANCODE_V			},
	{ "W",				SDL_SCANCODE_W			},
	{ "X",				SDL_SCANCODE_X			},
	{ "Y",				SDL_SCANCODE_Y			},
	{ "Z",				SDL_SCANCODE_Z			},
	{ "1",				SDL_SCANCODE_1			},
	{ "2",				SDL_SCANCODE_2			},
	{ "3",				SDL_SCANCODE_3			},
	{ "4",				SDL_SCANCODE_4			},
	{ "5",				SDL_SCANCODE_5			},
	{ "6",				SDL_SCANCODE_6			},
	{ "7",				SDL_SCANCODE_7			},
	{ "8",				SDL_SCANCODE_8			},
	{ "9",				SDL_SCANCODE_9			},
	{ "0",				SDL_SCANCODE_0			},
	{ "Return",			SDL_SCANCODE_RETURN		},
	{ "Escape",			SDL_SCANCODE_ESCAPE		},
	{ "Backspace",			SDL_SCANCODE_BACKSPACE		},
	{ "Tab",			SDL_SCANCODE_TAB		},
	{ "Space",			SDL_SCANCODE_SPACE		},
	{ "Minus",			SDL_SCANCODE_MINUS		},
	{ "Equals",			SDL_SCANCODE_EQUALS		},
	{ "LeftBracket",		SDL_SCANCODE_LEFTBRACKET	},
	{ "RightBracked",		SDL_SCANCODE_RIGHTBRACKET	},
	{ "Backslash",			SDL_SCANCODE_BACKSLASH		},
	{ "NonUShash",			SDL_SCANCODE_NONUSHASH		},
	{ "SemiColon",			SDL_SCANCODE_SEMICOLON		},
	{ "Apostrophe",			SDL_SCANCODE_APOSTROPHE		},
	{ "Grave",			SDL_SCANCODE_GRAVE		},
	{ "Comma",			SDL_SCANCODE_COMMA		},
	{ "Period",			SDL_SCANCODE_PERIOD		},
	{ "Slash",			SDL_SCANCODE_SLASH		},
	{ "CapsLock",			SDL_SCANCODE_CAPSLOCK		},
	{ "F1",				SDL_SCANCODE_F1			},
	{ "F2",				SDL_SCANCODE_F2			},
	{ "F3",				SDL_SCANCODE_F3			},
	{ "F4",				SDL_SCANCODE_F4			},
	{ "F5",				SDL_SCANCODE_F5			},
	{ "F6",				SDL_SCANCODE_F6			},
	{ "F7",				SDL_SCANCODE_F7			},
	{ "F8",				SDL_SCANCODE_F8			},
	{ "F9",				SDL_SCANCODE_F9			},
	{ "F10",			SDL_SCANCODE_F10		},
	{ "F11",			SDL_SCANCODE_F11		},
	{ "F12",			SDL_SCANCODE_F12		},
	{ "PrintScreen",		SDL_SCANCODE_PRINTSCREEN	},
	{ "ScrollLock",			SDL_SCANCODE_SCROLLLOCK		},
	{ "Pause",			SDL_SCANCODE_PAUSE		},
	{ "Insert",			SDL_SCANCODE_INSERT		},
	{ "Home",			SDL_SCANCODE_HOME		},
	{ "PageUp",			SDL_SCANCODE_PAGEUP		},
	{ "Delete",			SDL_SCANCODE_DELETE		},
	{ "End",			SDL_SCANCODE_END		},
	{ "PageDown",			SDL_SCANCODE_PAGEDOWN		},
	{ "Right",			SDL_SCANCODE_RIGHT		},
	{ "Left",			SDL_SCANCODE_LEFT		},
	{ "Down",			SDL_SCANCODE_DOWN		},
	{ "Up",				SDL_SCANCODE_UP			},
	{ "NumlockClear",		SDL_SCANCODE_NUMLOCKCLEAR	},
	{ "KPDivide",			SDL_SCANCODE_KP_DIVIDE		},
	{ "KPMultiply",			SDL_SCANCODE_KP_MULTIPLY	},
	{ "KPMinus",			SDL_SCANCODE_KP_MINUS		},
	{ "KPPlus",			SDL_SCANCODE_KP_PLUS		},
	{ "KPEnter",			SDL_SCANCODE_KP_ENTER		},
	{ "KP1",			SDL_SCANCODE_KP_1		},
	{ "KP2",			SDL_SCANCODE_KP_2		},
	{ "KP3",			SDL_SCANCODE_KP_3		},
	{ "KP4",			SDL_SCANCODE_KP_4		},
	{ "KP5",			SDL_SCANCODE_KP_5		},
	{ "KP6",			SDL_SCANCODE_KP_6		},
	{ "KP7",			SDL_SCANCODE_KP_7		},
	{ "KP8",			SDL_SCANCODE_KP_8		},
	{ "KP9",			SDL_SCANCODE_KP_9		},
	{ "KP0",			SDL_SCANCODE_KP_0		},
	{ "KPPeriod",			SDL_SCANCODE_KP_PERIOD		},
	{ "NonUSBackslash",		SDL_SCANCODE_NONUSBACKSLASH	},
	{ "Application",		SDL_SCANCODE_APPLICATION	},
	{ "Power",			SDL_SCANCODE_POWER		},
	{ "KPEquals",			SDL_SCANCODE_KP_EQUALS		},
	{ "F13",			SDL_SCANCODE_F13		},
	{ "F14",			SDL_SCANCODE_F14		},
	{ "F15",			SDL_SCANCODE_F15		},
	{ "F16",			SDL_SCANCODE_F16		},
	{ "F17",			SDL_SCANCODE_F17		},
	{ "F18",			SDL_SCANCODE_F18		},
	{ "F19",			SDL_SCANCODE_F19		},
	{ "F20",			SDL_SCANCODE_F20		},
	{ "F21",			SDL_SCANCODE_F21		},
	{ "F22",			SDL_SCANCODE_F22		},
	{ "F23",			SDL_SCANCODE_F23		},
	{ "F24",			SDL_SCANCODE_F24		},
	{ "Execute",			SDL_SCANCODE_EXECUTE		},
	{ "Help",			SDL_SCANCODE_HELP		},
	{ "Menu",			SDL_SCANCODE_MENU		},
	{ "Select",			SDL_SCANCODE_SELECT		},
	{ "Stop",			SDL_SCANCODE_STOP		},
	{ "Again",			SDL_SCANCODE_AGAIN		},
	{ "Undo",			SDL_SCANCODE_UNDO		},
	{ "Cut",			SDL_SCANCODE_CUT		},
	{ "Copy",			SDL_SCANCODE_COPY		},
	{ "Paste",			SDL_SCANCODE_PASTE		},
	{ "Find",			SDL_SCANCODE_FIND		},
	{ "Mute",			SDL_SCANCODE_MUTE		},
	{ "VolumeUp",			SDL_SCANCODE_VOLUMEUP		},
	{ "VolumeDown",			SDL_SCANCODE_VOLUMEDOWN		},
	{ "Comma",			SDL_SCANCODE_KP_COMMA		},
	{ "KPEqualsAS400",		SDL_SCANCODE_KP_EQUALSAS400	},
	{ "International1",		SDL_SCANCODE_INTERNATIONAL1	},
	{ "International2",		SDL_SCANCODE_INTERNATIONAL2	},
	{ "International3",		SDL_SCANCODE_INTERNATIONAL3	},
	{ "International4",		SDL_SCANCODE_INTERNATIONAL4	},
	{ "International5",		SDL_SCANCODE_INTERNATIONAL5	},
	{ "International6",		SDL_SCANCODE_INTERNATIONAL6	},
	{ "International7",		SDL_SCANCODE_INTERNATIONAL7	},
	{ "International8",		SDL_SCANCODE_INTERNATIONAL8	},
	{ "International9",		SDL_SCANCODE_INTERNATIONAL9	},
	{ "Lang1",			SDL_SCANCODE_LANG1		},
	{ "Lang2",			SDL_SCANCODE_LANG2		},
	{ "Lang3",			SDL_SCANCODE_LANG3		},
	{ "Lang4",			SDL_SCANCODE_LANG4		},
	{ "Lang5",			SDL_SCANCODE_LANG5		},
	{ "Lang6",			SDL_SCANCODE_LANG6		},
	{ "Lang7",			SDL_SCANCODE_LANG7		},
	{ "Lang8",			SDL_SCANCODE_LANG8		},
	{ "Lang9",			SDL_SCANCODE_LANG9		},
	{ "Alterase",			SDL_SCANCODE_ALTERASE		},
	{ "Sysreq",			SDL_SCANCODE_SYSREQ		},
	{ "Cancel",			SDL_SCANCODE_CANCEL		},
	{ "Clear",			SDL_SCANCODE_CLEAR		},
	{ "Prior",			SDL_SCANCODE_PRIOR		},
	{ "Return2",			SDL_SCANCODE_RETURN2		},
	{ "Separator",			SDL_SCANCODE_SEPARATOR		},
	{ "Out",			SDL_SCANCODE_OUT		},
	{ "Oper",			SDL_SCANCODE_OPER		},
	{ "Clearagain",			SDL_SCANCODE_CLEARAGAIN		},
	{ "CrSel",			SDL_SCANCODE_CRSEL		},
	{ "Exsel",			SDL_SCANCODE_EXSEL		},
	{ "KP00",			SDL_SCANCODE_KP_00		},
	{ "KP000",			SDL_SCANCODE_KP_000		},
	{ "ThousandsSeparator",		SDL_SCANCODE_THOUSANDSSEPARATOR	},
	{ "DecimalSeparator",		SDL_SCANCODE_DECIMALSEPARATOR	},
	{ "CurrencyUnit",		SDL_SCANCODE_CURRENCYUNIT	},
	{ "CurrencySubUnit",		SDL_SCANCODE_CURRENCYSUBUNIT	},
	{ "LeftParen",			SDL_SCANCODE_KP_LEFTPAREN	},
	{ "RightParen",			SDL_SCANCODE_KP_RIGHTPAREN	},
	{ "LeftBrace",			SDL_SCANCODE_KP_LEFTBRACE	},
	{ "RightBrace",			SDL_SCANCODE_KP_RIGHTBRACE	},
	{ "KPTab",			SDL_SCANCODE_KP_TAB		},
	{ "KPBackspace",		SDL_SCANCODE_KP_BACKSPACE	},
	{ "KPA",			SDL_SCANCODE_KP_A		},
	{ "KPB",			SDL_SCANCODE_KP_B		},
	{ "KPC",			SDL_SCANCODE_KP_C		},
	{ "KPD",			SDL_SCANCODE_KP_D		},
	{ "KPE",			SDL_SCANCODE_KP_E		},
	{ "KPF",			SDL_SCANCODE_KP_F		},
	{ "KPXor",			SDL_SCANCODE_KP_XOR		},
	{ "KPPower",			SDL_SCANCODE_KP_POWER		},
	{ "KPPercent",			SDL_SCANCODE_KP_PERCENT		},
	{ "KPLess",			SDL_SCANCODE_KP_LESS		},
	{ "KPGreater",			SDL_SCANCODE_KP_GREATER		},
	{ "KPAmpersand",		SDL_SCANCODE_KP_AMPERSAND	},
	{ "KPDblAmpersand",		SDL_SCANCODE_KP_DBLAMPERSAND	},
	{ "KPVerticalBar",		SDL_SCANCODE_KP_VERTICALBAR	},
	{ "KPDblVerticalBar",		SDL_SCANCODE_KP_DBLVERTICALBAR	},
	{ "KPColon",			SDL_SCANCODE_KP_COLON		},
	{ "KPHash",			SDL_SCANCODE_KP_HASH		},
	{ "KPSpace",			SDL_SCANCODE_KP_SPACE		},
	{ "KPAt",			SDL_SCANCODE_KP_AT		},
	{ "KPExclam",			SDL_SCANCODE_KP_EXCLAM		},
	{ "KPMemStore",			SDL_SCANCODE_KP_MEMSTORE	},
	{ "KPMemRecall",		SDL_SCANCODE_KP_MEMRECALL	},
	{ "KPMemClear",			SDL_SCANCODE_KP_MEMCLEAR	},
	{ "KPMemAdd",			SDL_SCANCODE_KP_MEMADD		},
	{ "KPMemSubstract",		SDL_SCANCODE_KP_MEMSUBTRACT	},
	{ "KPMemMultiply",		SDL_SCANCODE_KP_MEMMULTIPLY	},
	{ "KPMemDivide",		SDL_SCANCODE_KP_MEMDIVIDE	},
	{ "KPPlusMinus",		SDL_SCANCODE_KP_PLUSMINUS	},
	{ "KPClear",			SDL_SCANCODE_KP_CLEAR		},
	{ "KPClearEntry",		SDL_SCANCODE_KP_CLEARENTRY	},
	{ "KPBinary",			SDL_SCANCODE_KP_BINARY		},
	{ "KPOctal",			SDL_SCANCODE_KP_OCTAL		},
	{ "KPDecimal",			SDL_SCANCODE_KP_DECIMAL		},
	{ "KPHexadecimal",		SDL_SCANCODE_KP_HEXADECIMAL	},
	{ "LeftControl",		SDL_SCANCODE_LCTRL		},
	{ "LeftShift",			SDL_SCANCODE_LSHIFT		},
	{ "LeftAlt",			SDL_SCANCODE_LALT		},
	{ "LeftGUI",			SDL_SCANCODE_LGUI		},
	{ "RightControl",		SDL_SCANCODE_RCTRL		},
	{ "RightShift",			SDL_SCANCODE_RSHIFT		},
	{ "RightAlt",			SDL_SCANCODE_RALT		},
	{ "RGUI",			SDL_SCANCODE_RGUI		},
	{ "Mode",			SDL_SCANCODE_MODE		},
	{ "AudioNext",			SDL_SCANCODE_AUDIONEXT		},
	{ "AudioPrev",			SDL_SCANCODE_AUDIOPREV		},
	{ "AudioStop",			SDL_SCANCODE_AUDIOSTOP		},
	{ "AudioPlay",			SDL_SCANCODE_AUDIOPLAY		},
	{ "AudioMute",			SDL_SCANCODE_AUDIOMUTE		},
	{ "MediaSelect",		SDL_SCANCODE_MEDIASELECT	},
	{ "WWW",			SDL_SCANCODE_WWW		},
	{ "Mail",			SDL_SCANCODE_MAIL		},
	{ "Calculator",			SDL_SCANCODE_CALCULATOR		},
	{ "Computer",			SDL_SCANCODE_COMPUTER		},
	{ "ACSearch",			SDL_SCANCODE_AC_SEARCH		},
	{ "ACHome",			SDL_SCANCODE_AC_HOME		},
	{ "ACBack",			SDL_SCANCODE_AC_BACK		},
	{ "ACForward",			SDL_SCANCODE_AC_FORWARD		},
	{ "ACStop",			SDL_SCANCODE_AC_STOP		},
	{ "ACRefresh",			SDL_SCANCODE_AC_REFRESH		},
	{ "ACBookmarks",		SDL_SCANCODE_AC_BOOKMARKS	},
	{ "BrightnessDown",		SDL_SCANCODE_BRIGHTNESSDOWN	},
	{ "BrightnessUp",		SDL_SCANCODE_BRIGHTNESSUP	},
	{ "DisplaySwitch",		SDL_SCANCODE_DISPLAYSWITCH	},
	{ "KBDIllumToggle",		SDL_SCANCODE_KBDILLUMTOGGLE	},
	{ "KBDIllumDown",		SDL_SCANCODE_KBDILLUMDOWN	},
	{ "KBDIllumUp",			SDL_SCANCODE_KBDILLUMUP		},
	{ "Eject",			SDL_SCANCODE_EJECT		},
	{ "Sleep",			SDL_SCANCODE_SLEEP		},
	{ "App1",			SDL_SCANCODE_APP1		},
	{ "App2",			SDL_SCANCODE_APP2		},
	{ NULL,				-1				}
};

void CInput::Lua_Init(sol::state& lua)
{
	auto SDL = lua["SDL"].get_or_create<sol::table>();
	auto Scancode = SDL["Scancode"].get_or_create<sol::table>();

	const CommonEnum* val = KeyboardScancodes;

	while (val->name)
	{
		Scancode[val->name] = val->number;
		val++;
	}
}

void CInput::UpdateEvents(SDL_Event& event, sol::table& engineHostTable, bool imguiFocused)
{
	switch (event.type)
	{
		case SDL_MOUSEMOTION:
		{
			if (imguiFocused)
				break;

			if (engineHostTable.valid())
			{
				try {
					sol::function mouseMoveFunc = engineHostTable["MouseMove"];

					mouseMoveFunc(
						event.motion.x,
						event.motion.y,
						event.motion.xrel,
						event.motion.yrel
					);
				}
				catch (const sol::error& e)
				{
				}
			}

			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		{
			bool down = (event.type == SDL_MOUSEBUTTONDOWN);

			if (imguiFocused)
				down = false;

			if (engineHostTable.valid())
			{
				try {
					sol::function mouseFunc = engineHostTable["MouseButton"];

					mouseFunc(event.button.button, down);
				}
				catch (const sol::error& e)
				{
				}
			}

			break;
		}
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		{
			int nKey = event.key.keysym.scancode;
			bool down = (event.type == SDL_KEYDOWN);

			// lshift/right shift
			if (nKey == SDL_SCANCODE_RSHIFT)
				nKey = SDL_SCANCODE_LSHIFT;
			else if (nKey == SDL_SCANCODE_RCTRL)
				nKey = SDL_SCANCODE_LCTRL;
			else if (nKey == SDL_SCANCODE_RALT)
				nKey = SDL_SCANCODE_LALT;

			if (engineHostTable.valid())
			{
				try {
					sol::function keyFunc = engineHostTable["KeyPress"];

					keyFunc(nKey, down);
				}
				catch (const sol::error& e)
				{
				}
			}

			break;
		}
	}
}