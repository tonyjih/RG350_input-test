/* GCW Zero input tester, code file for SDL 1.2 and 2.0
 *
 * Copyright (C) 2014 Nebuleon Fumika <nebuleon@gcw-zero.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdbool.h>
#include <string.h>

#if defined SDL_1
#  define SDL_VER_STR "1.2"
#  include "SDL.h"
#  define SDL_SCREEN_TYPE SDL_Surface*
#  define SDL_RASTER_TYPE SDL_Surface*
#elif defined SDL_2
#  define SDL_VER_STR "2.0"
#  include "SDL2/SDL.h"
#  define SDL_SCREEN_TYPE SDL_Window*
#  define SDL_RASTER_TYPE SDL_Texture*
#else
#  error "neither SDL_1 nor SDL_2 is defined"
#endif
#include "SDL_ttf.h"

/* - - - DATA DEFINITIONS - - - */

/* Binary elements (pressed or not) that are shown on the display. */
enum Element {
	ELEMENT_DPAD_UP,
	ELEMENT_DPAD_DOWN,
	ELEMENT_DPAD_LEFT,
	ELEMENT_DPAD_RIGHT,
	ELEMENT_Y,
	ELEMENT_B,
	ELEMENT_X,
	ELEMENT_A,
	ELEMENT_SELECT,
	ELEMENT_START,
	ELEMENT_L,
	ELEMENT_R,
	ELEMENT_POWER,
	ELEMENT_HOLD,
};
#define ELEMENT_COUNT   14

/* These define the keys that are not covered by JS 0, or used when the
 * GCW Zero's buttons are not being mapped to JS 0. */
#ifdef SDL_1
SDLKey KeysHavingElements[ELEMENT_COUNT] = {
	SDLK_LEFT,
	SDLK_RIGHT,
	SDLK_UP,
	SDLK_DOWN,
	SDLK_LCTRL,
	SDLK_LALT,
	SDLK_LSHIFT,
	SDLK_SPACE,
	SDLK_TAB,
	SDLK_BACKSPACE,
	SDLK_ESCAPE,
	SDLK_RETURN,
	SDLK_HOME,
	SDLK_PAUSE,
};
#else
SDL_Scancode KeysHavingElements[ELEMENT_COUNT] = {
	SDL_SCANCODE_LEFT,
	SDL_SCANCODE_RIGHT,
	SDL_SCANCODE_UP,
	SDL_SCANCODE_DOWN,
	SDL_SCANCODE_LCTRL,
	SDL_SCANCODE_LALT,
	SDL_SCANCODE_LSHIFT,
	SDL_SCANCODE_SPACE,
	SDL_SCANCODE_TAB,
	SDL_SCANCODE_BACKSPACE,
	SDL_SCANCODE_ESCAPE,
	SDL_SCANCODE_RETURN,
	SDL_SCANCODE_HOME,
	SDL_SCANCODE_PAUSE,
};
#endif

/* These define the elements that should be lit up by pressing the above keys,
 * ordered in the same way as in KeysHavingElements. */
enum Element KeysToElements[ELEMENT_COUNT] = {
	ELEMENT_DPAD_LEFT,
	ELEMENT_DPAD_RIGHT,
	ELEMENT_DPAD_UP,
	ELEMENT_DPAD_DOWN,
	ELEMENT_A,
	ELEMENT_B,
	ELEMENT_X,
	ELEMENT_Y,
	ELEMENT_L,
	ELEMENT_R,
	ELEMENT_SELECT,
	ELEMENT_START,
	ELEMENT_POWER,
	ELEMENT_HOLD,
};

/* These define the elements that should be lit up by pressing JS 0's
 * buttons. */
enum Element JoyButtonsToElements[8] = {
	ELEMENT_B,
	ELEMENT_A,
	ELEMENT_Y,
	ELEMENT_X,
	ELEMENT_SELECT,
	ELEMENT_START,
	ELEMENT_L,
	ELEMENT_R,
};

const char* ElementNames[ELEMENT_COUNT] = {
	"D-pad Up",
	"D-pad Down",
	"D-pad Left",
	"D-pad Right",
	"Y",
	"B",
	"X",
	"A",
	"Select",
	"Start",
	"L",
	"R",
	"Power",
	"Hold",
};

/* Last readings for all the elements and axes. */
bool ElementPressed[ELEMENT_COUNT];
bool ElementEverPressed[ELEMENT_COUNT];

bool DPadOppositeEverPressed = false;

int16_t BuiltInJS_X = 0;
int16_t BuiltInJS_Y = 0;
int16_t BuiltInJS_X2 = 0;
int16_t BuiltInJS_Y2 = 0;

struct DrawnElement {
	      SDL_Rect   Rect;
	const SDL_Color* ColorPressed;
	const SDL_Color* ColorEverPressed;
};

SDL_SCREEN_TYPE Screen;
#ifndef SDL_1
SDL_Renderer* Renderer;
SDL_Haptic* HapticDevice;
bool HapticActive = false;
SDL_RASTER_TYPE TextRumble;
#endif

SDL_RASTER_TYPE TextCross;
SDL_RASTER_TYPE TextAnalog;
SDL_RASTER_TYPE TextAnalog2;
SDL_RASTER_TYPE TextFace;
SDL_RASTER_TYPE TextOthers;

SDL_RASTER_TYPE TextCrossError;
SDL_RASTER_TYPE TextExit;

TTF_Font* Font = NULL;

/* - - - CUSTOMISATION - - - */

#define FONT_FILE        "/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed.ttf"
#define FONT_SIZE         12

#define SCREEN_WIDTH     320
#define SCREEN_HEIGHT    240

#define INNER_SCREEN_X    84
#define INNER_SCREEN_Y    18
#define INNER_SCREEN_W   154
#define INNER_SCREEN_H   117

#define GCW_ZERO_PIC_Y    43

#define TEXT_CROSS_LX      4
#define TEXT_CROSS_Y      20

#define TEXT_ANALOG_CX   160
#define TEXT_ANALOG_Y      4

#define TEXT_GRAVITY_CX  160
#define TEXT_GRAVITY_Y    20

#define TEXT_OTHERS_RX   316
#define TEXT_OTHERS_Y      4

#define TEXT_FACE_RX     316
#define TEXT_FACE_Y       20

#define TEXT_CROSS_ERR_LX  4
#define TEXT_CROSS_ERR_Y 188

#define TEXT_EXIT_RX     316
#define TEXT_EXIT_Y      220

#define TEXT_RUMBLE_RX   316
#define TEXT_RUMBLE_Y    204

                                   //  R    G    B    A
const SDL_Color ColorBackground   = {   0,   0,   0, 255 };
const SDL_Color ColorBorder       = { 255, 255, 255, 255 };
const SDL_Color ColorInnerBorder  = { 128, 128, 128, 255 };
const SDL_Color ColorError        = { 255,  32,  32, 255 };
const SDL_Color ColorPrompt       = { 255, 255, 255, 255 };
const SDL_Color ColorNeverPressed = {  32,  32,  32, 255 };

const SDL_Color ColorCross        = { 255, 255,  32, 255 };
const SDL_Color ColorAnalog       = {  32, 255,  32, 255 };
const SDL_Color ColorAnalog2      = { 255, 128,  32, 255 };
const SDL_Color ColorFace         = {  32,  32, 255, 255 };
const SDL_Color ColorOthers       = { 255,  32, 255, 255 };

const SDL_Color ColorEverCross    = {  64,  64,  32, 255 };
const SDL_Color ColorEverAnalog   = {  32,  64,  32, 255 };
const SDL_Color ColorEverAnalog2  = {  64,  48,  32, 255 };
const SDL_Color ColorEverFace     = {  32,  32,  64, 255 };
const SDL_Color ColorEverOthers   = {  64,  32,  64, 255 };

struct DrawnElement DrawnElements[ELEMENT_COUNT] = {
	{ .Rect = { .x = 32, .y = 38 + GCW_ZERO_PIC_Y, .w = 14, .h = 16 }, .ColorPressed = &ColorCross, .ColorEverPressed = &ColorEverCross },
	{ .Rect = { .x = 32, .y = 68 + GCW_ZERO_PIC_Y, .w = 14, .h = 16 }, .ColorPressed = &ColorCross, .ColorEverPressed = &ColorEverCross },
	{ .Rect = { .x = 16, .y = 54 + GCW_ZERO_PIC_Y, .w = 16, .h = 14 }, .ColorPressed = &ColorCross, .ColorEverPressed = &ColorEverCross },
	{ .Rect = { .x = 46, .y = 54 + GCW_ZERO_PIC_Y, .w = 16, .h = 14 }, .ColorPressed = &ColorCross, .ColorEverPressed = &ColorEverCross },
	{ .Rect = { .x = 272, .y = 33 + GCW_ZERO_PIC_Y, .w = 19, .h = 19 }, .ColorPressed = &ColorFace, .ColorEverPressed = &ColorEverFace },
	{ .Rect = { .x = 272, .y = 71 + GCW_ZERO_PIC_Y, .w = 19, .h = 19 }, .ColorPressed = &ColorFace, .ColorEverPressed = &ColorEverFace },
	{ .Rect = { .x = 253, .y = 52 + GCW_ZERO_PIC_Y, .w = 19, .h = 19 }, .ColorPressed = &ColorFace, .ColorEverPressed = &ColorEverFace },
	{ .Rect = { .x = 291, .y = 52 + GCW_ZERO_PIC_Y, .w = 19, .h = 19 }, .ColorPressed = &ColorFace, .ColorEverPressed = &ColorEverFace },
	{ .Rect = { .x = 266, .y = 96 + GCW_ZERO_PIC_Y, .w = 31, .h = 14 }, .ColorPressed = &ColorOthers, .ColorEverPressed = &ColorEverOthers },
	{ .Rect = { .x = 266, .y = 118 + GCW_ZERO_PIC_Y, .w = 31, .h = 14 }, .ColorPressed = &ColorOthers, .ColorEverPressed = &ColorEverOthers },
	{ .Rect = { .x = 9, .y = 3 + GCW_ZERO_PIC_Y, .w = 47, .h = 19 }, .ColorPressed = &ColorOthers, .ColorEverPressed = &ColorEverOthers },
	{ .Rect = { .x = 264, .y = 3 + GCW_ZERO_PIC_Y, .w = 47, .h = 19 }, .ColorPressed = &ColorOthers, .ColorEverPressed = &ColorEverOthers },
	{ .Rect = { .x = 312, .y = 52 + GCW_ZERO_PIC_Y, .w = 6, .h = 21 }, .ColorPressed = &ColorOthers, .ColorEverPressed = &ColorEverOthers },
	{ .Rect = { .x = 312, .y = 73 + GCW_ZERO_PIC_Y, .w = 6, .h = 21 }, .ColorPressed = &ColorOthers, .ColorEverPressed = &ColorEverOthers },
};

/* - - - HELPER FUNCTIONS - - - */

bool MustExit(void)
{
	// Start+Select allows exiting this application.
	return ElementPressed[ELEMENT_SELECT] && ElementPressed[ELEMENT_START];
}

#ifndef SDL_1
void UpdateHaptic(void)
{
	bool NewHapticActive = ElementPressed[ELEMENT_L] && ElementPressed[ELEMENT_R];

	if (!HapticActive && NewHapticActive)
	{
		printf("Starting force feedback as requested by the user\n");
		if (SDL_HapticRumblePlay(HapticDevice, 0.33f /* Strength */, 15000 /* Time */) < 0)
		{
			printf("SDL_HapticRumblePlay failed: %s\n", SDL_GetError());
		}
	}
	else if (HapticActive && !NewHapticActive)
	{
		printf("Stopping force feedback as requested by the user\n");
		if (SDL_HapticRumbleStop(HapticDevice) < 0)
		{
			printf("SDL_HapticRumbleStop failed: %s\n", SDL_GetError());
		}
	}

	HapticActive = NewHapticActive;
}
#endif

static int WIDTH(SDL_RASTER_TYPE Raster)
{
#ifdef SDL_1
	return Raster->w;
#else
	int Result;
	SDL_QueryTexture(Raster, NULL, NULL, &Result, NULL);
	return Result;
#endif
}

static int HEIGHT(SDL_RASTER_TYPE Raster)
{
#ifdef SDL_1
	return Raster->h;
#else
	int Result;
	SDL_QueryTexture(Raster, NULL, NULL, NULL, &Result);
	return Result;
#endif
}

#ifdef SDL_1
#  define JOYSTICK_NAME(Index) SDL_JoystickName(Index)
#  define JOYSTICK_INDEX(Joystick) SDL_JoystickIndex(Joystick)
#  define SDL_COLOR(Source) SDL_MapRGB(Screen->format, (Source).r, (Source).g, (Source).b)
#  define MAKE_RASTER(Surface) Surface
#  define FREE_RASTER(Raster) SDL_FreeSurface(Raster)
#  define PRESENT() SDL_Flip(Screen)
#else
#  define JOYSTICK_NAME(Index) SDL_JoystickNameForIndex(Index)
#  define JOYSTICK_INDEX(Joystick) SDL_JoystickInstanceID(Joystick)
#  define SDL_COLOR(Source) (Source).r, (Source).g, (Source).b, (Source).a

static SDL_RASTER_TYPE MAKE_RASTER(SDL_Surface* Surface)
{
	SDL_RASTER_TYPE Result = SDL_CreateTextureFromSurface(Renderer, Surface);
	SDL_FreeSurface(Surface);
	return Result;
}

#  define FREE_RASTER(Raster) SDL_DestroyTexture(Raster)
#  define PRESENT() SDL_RenderPresent(Renderer)
#endif

static void RENDER_HOLLOW_RECT(SDL_Rect* DestRect, const SDL_Color* Color)
{
#ifdef SDL_1
	Uint32 MappedColor = SDL_COLOR(*Color);
	{
		SDL_Rect LineRect = { .x = DestRect->x, .y = DestRect->y, .w = DestRect->w, .h = 1 };
		SDL_FillRect(Screen, &LineRect, MappedColor);
		LineRect.y = DestRect->y + DestRect->h - 1;
		SDL_FillRect(Screen, &LineRect, MappedColor);
	}
	{
		SDL_Rect LineRect = { .x = DestRect->x, .y = DestRect->y, .w = 1, .h = DestRect->h };
		SDL_FillRect(Screen, &LineRect, MappedColor);
		LineRect.x = DestRect->x + DestRect->w - 1;
		SDL_FillRect(Screen, &LineRect, MappedColor);
	}
#else
	SDL_SetRenderDrawColor(Renderer, SDL_COLOR(*Color));
	SDL_RenderDrawRect(Renderer, DestRect);
#endif
}

static void RENDER_FILLED_RECT(SDL_Rect* DestRect, const SDL_Color* Color)
{
#ifdef SDL_1
	SDL_FillRect(Screen, DestRect, SDL_COLOR(*Color));
#else
	SDL_SetRenderDrawColor(Renderer, SDL_COLOR(*Color));
	SDL_RenderFillRect(Renderer, DestRect);
#endif
}

static void RENDER_RASTER(SDL_RASTER_TYPE Raster, SDL_Rect* DestRect)
{
#ifdef SDL_1
	SDL_BlitSurface(Raster, NULL, Screen, DestRect);
#else
	SDL_RenderCopy(Renderer, Raster, NULL, DestRect);
#endif
}

// The return value of this function is the raster containing the coordinates
// shown for the joystick, if any.
// The caller is required to free this raster after it calls PRESENT(),
// because the renderer may defer drawing it until presentation occurs
// and another texture may be allocated on top if it is freed straight away.
static SDL_RASTER_TYPE DrawJoystickDot(const Sint16 JoystickX, const Sint16 JoystickY, const Sint16 CX, const Sint16 Y, SDL_RASTER_TYPE Text, const SDL_Color* Color)
{
	if (JoystickX != 0 || JoystickY != 0)
	{
		SDL_Rect TextRect = { .x = CX - WIDTH(Text) / 2, .y = Y, .w = WIDTH(Text), .h = HEIGHT(Text) };
		RENDER_RASTER(Text, &TextRect);

		SDL_Rect DotRect = {
			.x = INNER_SCREEN_X + (Uint32) ((Sint32) JoystickX + 32768) * (INNER_SCREEN_W - 4) / 65536,
			.y = GCW_ZERO_PIC_Y + INNER_SCREEN_Y + (Uint32) ((Sint32) JoystickY + 32768) * (INNER_SCREEN_H - 4) / 65536,
			.w = 4,
			.h = 4
		};
		RENDER_FILLED_RECT(&DotRect, Color);

		// And show the coordinates.
		char Coords[20];
		sprintf(Coords, "(%.2f, %.2f)", JoystickX / 32767.0, JoystickY / 32767.0);
		SDL_Surface* Surface = TTF_RenderUTF8_Blended(Font, Coords, *Color);
		SDL_RASTER_TYPE TextCoords = MAKE_RASTER(Surface);
		SDL_Rect CoordsRect = { .x = DotRect.x, .y = DotRect.y, .w = WIDTH(TextCoords), .h = HEIGHT(TextCoords) };
		if (JoystickX < 0)
			CoordsRect.x += 8;
		else
			CoordsRect.x -= WIDTH(TextCoords) + 4;
		if (JoystickY < 0)
			CoordsRect.y += 4;
		else
			CoordsRect.y -= HEIGHT(TextCoords) + 2;
		RENDER_RASTER(TextCoords, &CoordsRect);
		return TextCoords;
	}
	else return NULL;
}

/* - - - DISPLAY AND INPUT - - - */

static void DrawScreen()
{
	// Background
	SDL_Rect ScreenRect = { .x = 0, .y = 0, .w = SCREEN_WIDTH, .h = SCREEN_HEIGHT };
	RENDER_FILLED_RECT(&ScreenRect, &ColorBackground);

	// Outer border (there to tell you whether your screen is being cut off by
	// the bezel)
	RENDER_HOLLOW_RECT(&ScreenRect, &ColorBorder);

	// Elements
	unsigned int i;
	for (i = 0; i < ELEMENT_COUNT; i++)
	{
		RENDER_FILLED_RECT(&DrawnElements[i].Rect,
			(ElementPressed[i] ? DrawnElements[i].ColorPressed :
				(ElementEverPressed[i] ? DrawnElements[i].ColorEverPressed : &ColorNeverPressed)));
	}

	// Text prompt: Start+Select to exit
	SDL_Rect TextExitRect = { .x = TEXT_EXIT_RX - WIDTH(TextExit), .y = TEXT_EXIT_Y, .w = WIDTH(TextExit), .h = HEIGHT(TextExit) };
	RENDER_RASTER(TextExit, &TextExitRect);

#ifndef SDL_1
	// Text prompt: L+R to rumble
	if (HapticDevice != NULL)
	{
		SDL_Rect TextRumbleRect = { .x = TEXT_RUMBLE_RX - WIDTH(TextRumble), .y = TEXT_RUMBLE_Y, .w = WIDTH(TextRumble), .h = HEIGHT(TextRumble) };
		RENDER_RASTER(TextRumble, &TextRumbleRect);
	}
#endif

	// Text prompt, if a direction is pressed on the cross
	if (ElementPressed[0] || ElementPressed[1] || ElementPressed[2] || ElementPressed[3])
	{
		SDL_Rect TextCrossRect = { .x = TEXT_CROSS_LX, .y = TEXT_CROSS_Y, .w = WIDTH(TextCross), .h = HEIGHT(TextCross) };
		RENDER_RASTER(TextCross, &TextCrossRect);
	}

	// Text prompt, if ever during this run two opposite directions on the
	// cross were pressed at once (this also maintains the status)
	if ((ElementPressed[0] && ElementPressed[1]) || (ElementPressed[2] && ElementPressed[3]) || DPadOppositeEverPressed)
	{
		DPadOppositeEverPressed = true;
		SDL_Rect TextCrossErrorRect = { .x = TEXT_CROSS_ERR_LX, .y = TEXT_CROSS_ERR_Y, .w = WIDTH(TextCrossError), .h = HEIGHT(TextCrossError) };
		RENDER_RASTER(TextCrossError, &TextCrossErrorRect);
	}

	// Text prompt, if a face button is pressed
	if (ElementPressed[4] || ElementPressed[5] || ElementPressed[6] || ElementPressed[7])
	{
		SDL_Rect TextFaceRect = { .x = TEXT_FACE_RX - WIDTH(TextFace), .y = TEXT_FACE_Y, .w = WIDTH(TextFace), .h = HEIGHT(TextFace) };
		RENDER_RASTER(TextFace, &TextFaceRect);
	}

	// Text prompt, if another button is pressed
	if (ElementPressed[8] || ElementPressed[9] || ElementPressed[10] || ElementPressed[11] || ElementPressed[12] || ElementPressed[13])
	{
		SDL_Rect TextOthersRect = { .x = TEXT_OTHERS_RX - WIDTH(TextOthers), .y = TEXT_OTHERS_Y, .w = WIDTH(TextOthers), .h = HEIGHT(TextOthers) };
		RENDER_RASTER(TextOthers, &TextOthersRect);
	}

	// Inner border (there to provide a reference frame for the joystick axes'
	// dots)
	SDL_Rect InnerRect = { .x = INNER_SCREEN_X, .y = GCW_ZERO_PIC_Y + INNER_SCREEN_Y, .w = INNER_SCREEN_W, .h = INNER_SCREEN_H };
	RENDER_HOLLOW_RECT(&InnerRect, &ColorInnerBorder);

	// A dot to indicate where the analog nub is pointed to, relative to the
	// inner screen, as well as its coordinates
	SDL_RASTER_TYPE BuiltInJSCoords = DrawJoystickDot(BuiltInJS_X, BuiltInJS_Y, TEXT_ANALOG_CX, TEXT_ANALOG_Y, TextAnalog, &ColorAnalog);
	// And another for the gravity sensor
	SDL_RASTER_TYPE BuiltInJSCoords2 = DrawJoystickDot(BuiltInJS_X2, BuiltInJS_Y2, TEXT_GRAVITY_CX, TEXT_GRAVITY_Y, TextAnalog2, &ColorAnalog2);

	PRESENT();

	if (BuiltInJSCoords)
		FREE_RASTER(BuiltInJSCoords);
	if (BuiltInJSCoords2)
		FREE_RASTER(BuiltInJSCoords2);

	SDL_Delay(8); // Reduce the delay between this update and the input for the next
}

int main(int argc, char** argv)
{
	unsigned int i;
	bool Error = false;

	printf("SDL " SDL_VER_STR " input tester starting\n");

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
	{
		printf("SDL initialisation failed: %s\n", SDL_GetError());
		Error = true;
		goto end;
	}

	if (TTF_Init() == -1)
	{
		printf("SDL_ttf initialisation failed: %s\n", TTF_GetError());
		Error = true;
		goto cleanup_sdl;
	}

	Font = TTF_OpenFont(FONT_FILE, FONT_SIZE);

	if (Font == NULL)
	{
		printf("Opening " FONT_FILE " failed: %s\n", TTF_GetError());
		Error = true;
		goto cleanup_ttf;
	}

	SDL_ShowCursor(SDL_DISABLE);

#ifdef SDL_1
	Screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE |
#ifdef SDL_TRIPLEBUF
		SDL_TRIPLEBUF
#else
		SDL_DOUBLEBUF
#endif
		);

	if (Screen == NULL)
	{
		printf("SDL_SetVideoMode failed: %s\n", SDL_GetError());
		Error = true;
		goto cleanup_font;
	}
#else
	Screen = SDL_CreateWindow("Input test",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		0, 0,
		SDL_WINDOW_FULLSCREEN_DESKTOP);

	if (Screen == NULL)
	{
		printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
		Error = true;
		goto cleanup_font;
	}

	Renderer = SDL_CreateRenderer(Screen, -1, SDL_RENDERER_PRESENTVSYNC);

	if (Renderer == NULL)
	{
		printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
		Error = true;
		goto cleanup_window;
	}
#endif

	// Pre-render text strings that are always used.
	SDL_Surface* Text;
	Text = TTF_RenderUTF8_Blended(Font, "Directional cross", ColorCross);
	TextCross = MAKE_RASTER(Text);
	Text = TTF_RenderUTF8_Blended(Font, "Analog nub", ColorAnalog);
	TextAnalog = MAKE_RASTER(Text);
	Text = TTF_RenderUTF8_Blended(Font, "Analog nub2", ColorAnalog2);
	TextAnalog2 = MAKE_RASTER(Text);
	Text = TTF_RenderUTF8_Blended(Font, "Face buttons", ColorFace);
	TextFace = MAKE_RASTER(Text);
	Text = TTF_RenderUTF8_Blended(Font, "Other buttons", ColorOthers);
	TextOthers = MAKE_RASTER(Text);

	Text = TTF_RenderUTF8_Blended(Font, "Opposite directions pressed simultaneously on the cross", ColorError);
	TextCrossError = MAKE_RASTER(Text);
	Text = TTF_RenderUTF8_Blended(Font, "Start+Select to exit", ColorPrompt);
	TextExit = MAKE_RASTER(Text);

#ifndef SDL_1
	if (SDL_InitSubSystem(SDL_INIT_HAPTIC) < 0)
	{
		printf("SDL force feedback initialisation failed (non-fatal): %s\n", SDL_GetError());
	}

	for (i = 0; i < SDL_NumHaptics(); i++)
	{
		printf("Force feedback device %u: \"%s\"\n", i, SDL_HapticName(i));
	}

	if (SDL_NumHaptics() > 0)
	{
		HapticDevice = SDL_HapticOpen(0);
		if (HapticDevice == NULL)
		{
			printf("SDL_HapticOpen(0) failed (non-fatal): %s\n", SDL_GetError());
		}
		else if (SDL_HapticRumbleInit(HapticDevice) != 0)
		{
			printf("SDL_HapticRumbleInit failed (non-fatal): %s\n", SDL_GetError());
			SDL_HapticClose(HapticDevice);
			HapticDevice = NULL;
		}
	}

	if (HapticDevice != NULL)
	{
		Text = TTF_RenderUTF8_Blended(Font, "L+R to rumble", ColorPrompt);
		TextRumble = MAKE_RASTER(Text);
	}
#endif

#ifdef SDL_1
	// Make sure we don't get key repeating.
	SDL_EnableKeyRepeat(0, 0);
#endif
	// Initialise joystick input.
	SDL_JoystickEventState(SDL_ENABLE);
	SDL_Joystick* BuiltInJS = NULL;

	for (i = 0; i < SDL_NumJoysticks(); i++)
	{
		printf("Joystick %u: \"%s\"\n", i, JOYSTICK_NAME(i));

		if (strcmp(JOYSTICK_NAME(i), "linkdev device (Analog 2-axis 8-button 2-hat)") == 0)
			BuiltInJS = SDL_JoystickOpen(i);
	}

	bool Exit = false;
	while (!Exit)
	{
		SDL_Event Event;
		while (SDL_PollEvent(&Event) != 0)
		{
			switch (Event.type)
			{
				case SDL_JOYAXISMOTION:
					if (BuiltInJS != NULL
					 && Event.jaxis.which == JOYSTICK_INDEX(BuiltInJS))
					{
						if (Event.jaxis.axis == 0) /* X */
							BuiltInJS_X = Event.jaxis.value;
						else if (Event.jaxis.axis == 1) /* Y */
							BuiltInJS_Y = Event.jaxis.value;
						else if (Event.jaxis.axis == 2) /* X */
							BuiltInJS_X2 = Event.jaxis.value;
						else if (Event.jaxis.axis == 3) /* Y */
							BuiltInJS_Y2 = Event.jaxis.value;
					}

					break;
				case SDL_JOYHATMOTION:
					if (BuiltInJS != NULL
					 && Event.jhat.which == JOYSTICK_INDEX(BuiltInJS)
					 && Event.jhat.hat == 0)
					{
						ElementPressed[ELEMENT_DPAD_UP   ] = !!(Event.jhat.value & SDL_HAT_UP);
						ElementPressed[ELEMENT_DPAD_DOWN ] = !!(Event.jhat.value & SDL_HAT_DOWN);
						ElementPressed[ELEMENT_DPAD_LEFT ] = !!(Event.jhat.value & SDL_HAT_LEFT);
						ElementPressed[ELEMENT_DPAD_RIGHT] = !!(Event.jhat.value & SDL_HAT_RIGHT);
						ElementEverPressed[ELEMENT_DPAD_UP   ] |= ElementPressed[ELEMENT_DPAD_UP];
						ElementEverPressed[ELEMENT_DPAD_DOWN ] |= ElementPressed[ELEMENT_DPAD_DOWN];
						ElementEverPressed[ELEMENT_DPAD_LEFT ] |= ElementPressed[ELEMENT_DPAD_LEFT];
						ElementEverPressed[ELEMENT_DPAD_RIGHT] |= ElementPressed[ELEMENT_DPAD_RIGHT];
					}
					break;
				case SDL_JOYBUTTONDOWN:
				case SDL_JOYBUTTONUP:
					if (BuiltInJS != NULL
					 && Event.jbutton.which == JOYSTICK_INDEX(BuiltInJS))
					{
						i = JoyButtonsToElements[Event.jbutton.button];
						if (ElementPressed[i] && Event.type == SDL_JOYBUTTONDOWN)
							printf("Received SDL_JOYBUTTONDOWN for already-pressed button %s (joystick %d button %d)\n", ElementNames[i], Event.jbutton.which, Event.jbutton.button);
						else if (!ElementPressed[i] && Event.type == SDL_JOYBUTTONUP)
							printf("Received SDL_JOYBUTTONUP for already-released button %s (joystick %d button %d)\n", ElementNames[i], Event.jbutton.which, Event.jbutton.button);
						ElementPressed[i] = (Event.type == SDL_JOYBUTTONDOWN);
						ElementEverPressed[i] |= ElementPressed[i];
					}
					break;
				case SDL_KEYDOWN:
				case SDL_KEYUP:
					for (i = 0; i < sizeof(KeysHavingElements) / sizeof(KeysHavingElements[0]); i++)
					{
#ifdef SDL_1
						if (Event.key.keysym.sym == KeysHavingElements[i])
#else
						if (Event.key.keysym.scancode == KeysHavingElements[i])
#endif
						{
							i = KeysToElements[i];
							if (ElementPressed[i] && Event.type == SDL_KEYDOWN)
								printf("Received SDL_KEYDOWN for already-pressed button %s (keyboard %s)\n", ElementNames[i], SDL_GetKeyName(Event.key.keysym.sym));
							else if (!ElementPressed[i] && Event.type == SDL_KEYUP)
								printf("Received SDL_KEYUP for already-released button %s (keyboard %s)\n", ElementNames[i], SDL_GetKeyName(Event.key.keysym.sym));
							ElementPressed[i] = (Event.type == SDL_KEYDOWN);
							ElementEverPressed[i] = true;
							break;
						}
					}
					break;
				case SDL_QUIT:
					Exit = true;
					break;
			} // switch (Event.type)
		} // while (SDL_PollEvent(&Event))

		DrawScreen();
		Exit |= MustExit();
#ifndef SDL_1
		UpdateHaptic();
#endif
	} // while (!Exit)

#ifndef SDL_1
	if (HapticDevice != NULL)
		SDL_HapticClose(HapticDevice);
#endif

	if (BuiltInJS != NULL)
		SDL_JoystickClose(BuiltInJS);

	FREE_RASTER(TextCross);
	FREE_RASTER(TextAnalog);
	FREE_RASTER(TextAnalog2);
	FREE_RASTER(TextFace);
	FREE_RASTER(TextOthers);

	FREE_RASTER(TextCrossError);
	FREE_RASTER(TextExit);

#ifdef SDL_2
	SDL_DestroyRenderer(Renderer);
cleanup_window:
	SDL_DestroyWindow(Screen);
#endif
cleanup_font:
	TTF_CloseFont(Font);
cleanup_ttf:
	TTF_Quit();
cleanup_sdl:
	SDL_Quit();

end:
	return Error ? 2 : 0;
}
