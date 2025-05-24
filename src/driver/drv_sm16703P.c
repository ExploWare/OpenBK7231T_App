#if PLATFORM_BK7231N || WINDOWS


#include "../new_cfg.h"
#include "../new_common.h"
#include "../new_pins.h"
// Commands register, execution API and cmd tokenizer
#include "../cmnds/cmd_public.h"
#include "../hal/hal_pins.h"
#include "../httpserver/new_http.h"
#include "../logging/logging.h"
#include "../mqtt/new_mqtt.h"

#include "drv_spiLED.h"

// Number of pixels that can be addressed
uint32_t pixel_count;

void SM16703P_GetPixel(uint32_t pixel, byte *dst) {
	int i;
	uint8_t *input;

	if (spiLED.msg == 0)
		return;
    switch (color_order) {
        case SM16703P_COLOR_ORDER_RGB:
        case SM16703P_COLOR_ORDER_RBG:
        case SM16703P_COLOR_ORDER_BRG:
        case SM16703P_COLOR_ORDER_BGR:
        case SM16703P_COLOR_ORDER_GRB:
        case SM16703P_COLOR_ORDER_GBR:
            channels = 4; break;
        case SM16703P_COLOR_ORDER_RGBW:
        case SM16703P_COLOR_ORDER_RBGW:
        case SM16703P_COLOR_ORDER_BRGW:
        case SM16703P_COLOR_ORDER_BGRW:
        case SM16703P_COLOR_ORDER_GRBW:
        case SM16703P_COLOR_ORDER_GBRW:
            channels = 5; break;
        case SM16703P_COLOR_ORDER_RGBCW:
        case SM16703P_COLOR_ORDER_RBGCW:
        case SM16703P_COLOR_ORDER_BRGCW:
        case SM16703P_COLOR_ORDER_BGRCW:
        case SM16703P_COLOR_ORDER_GRBCW:
        case SM16703P_COLOR_ORDER_GBRCW:
            channels = 6; break;
        default:
            return;  // Invalid color order
    }
	
	input = spiLED.buf + spiLED.ofs + (pixel * channels * 4);

	for (i = 0; i < channels; i++) {
		*dst++ = reverse_translate_byte(input + i * 4);
	}
}

#define SM16703P_COLOR_ORDER_RGB         0x00
#define SM16703P_COLOR_ORDER_RBG         0x01
#define SM16703P_COLOR_ORDER_BRG         0x02
#define SM16703P_COLOR_ORDER_BGR         0x03
#define SM16703P_COLOR_ORDER_GRB         0x04
#define SM16703P_COLOR_ORDER_GBR         0x05
#define SM16703P_COLOR_ORDER_RGBW         0x06
#define SM16703P_COLOR_ORDER_RBGW         0x07
#define SM16703P_COLOR_ORDER_BRGW         0x08
#define SM16703P_COLOR_ORDER_BGRW         0x09
#define SM16703P_COLOR_ORDER_GRBW         0x0A
#define SM16703P_COLOR_ORDER_GBRW         0x0B
#define SM16703P_COLOR_ORDER_RGBCW         0x0C
#define SM16703P_COLOR_ORDER_RBGCW         0x0D
#define SM16703P_COLOR_ORDER_BRGCW         0x0E
#define SM16703P_COLOR_ORDER_BGRCW         0x0F
#define SM16703P_COLOR_ORDER_GRBCW         0x10
#define SM16703P_COLOR_ORDER_GBRCW         0x11
int color_order = SM16703P_COLOR_ORDER_RGB; // default to RGB


bool SM16703P_VerifyPixel(uint32_t pixel, byte r, byte g, byte b, ...) {
	byte real[5];
	SM16703P_GetPixel(pixel, real);

    va_list args;
    va_start(args, pixel);
	byte c, w;
	if (numArgs > 0){
		c=va_arg(args, int);
		if (numArgs > 1){
			w=va_arg(args, int);
		}
	}
    byte c = (byte)va_arg(args, int); // Get the fourth argument (C)
    byte w = (byte)va_arg(args, int); // Get the fifth argument (W)
    va_end(args);

    // Compare the retrieved pixel color with the provided values
    if (real[0] != r) return false;
    if (real[1] != g) return false;
    if (real[2] != b) return false;
	if (c > -1){
		if (real[3] != c)
			return false;
		if (w > -1){
			if (real[4] != w)
				return false;
		}
	}
	return true;
}


void SM16703P_setPixel(int pixel,, int r, int g, int b, ...) {
	if (!spiLED.ready)
		return;
	va_list args;
	va_start(args, numArgs)
	int c = -1, w = -1;
	if (numArgs > 0){
		c=va_arg(args, int);
		if (numArgs > 1){
			w=va_arg(args, int);
		}
	}
	va_end(args);

	// Load data in correct format
	int b0, b1, b2, b3 = -1, b4 = -1;
    // Set color channels based on color order
    switch (color_order) {
        case SM16703P_COLOR_ORDER_RGB:
        case SM16703P_COLOR_ORDER_RGBW:
        case SM16703P_COLOR_ORDER_RGBCW:
            b0 = r; b1 = g; b2 = b; b3 = c; b4 = w; break;
        case SM16703P_COLOR_ORDER_RBG:
        case SM16703P_COLOR_ORDER_RBGW:
        case SM16703P_COLOR_ORDER_RBGCW:
            b0 = r; b1 = b; b2 = g; b3 = c; b4 = w; break;
        case SM16703P_COLOR_ORDER_BRG:
        case SM16703P_COLOR_ORDER_BRGW:
        case SM16703P_COLOR_ORDER_BRGCW:
            b0 = b; b1 = r; b2 = g; b3 = c; b4 = w; break;
        case SM16703P_COLOR_ORDER_BGR:
        case SM16703P_COLOR_ORDER_BGRW:
        case SM16703P_COLOR_ORDER_BGRCW:
            b0 = b; b1 = g; b2 = r; b3 = c; b4 = w; break;
        case SM16703P_COLOR_ORDER_GRB:
        case SM16703P_COLOR_ORDER_GRBW:
        case SM16703P_COLOR_ORDER_GRBCW:
            b0 = g; b1 = r; b2 = b; b3 = c; b4 = w; break;
        case SM16703P_COLOR_ORDER_GBR:
        case SM16703P_COLOR_ORDER_GBRW:
        case SM16703P_COLOR_ORDER_GBRCW:
            b0 = g; b1 = b; b2 = r; b3 = c; b4 = w; break;
        default:
            return;  // Invalid color order
    }
    switch (color_order) {
        case SM16703P_COLOR_ORDER_RGB:
        case SM16703P_COLOR_ORDER_RBG:
        case SM16703P_COLOR_ORDER_BRG:
        case SM16703P_COLOR_ORDER_BGR:
        case SM16703P_COLOR_ORDER_GRB:
        case SM16703P_COLOR_ORDER_GBR:
            channels = 3; break;
        case SM16703P_COLOR_ORDER_RGBW:
        case SM16703P_COLOR_ORDER_RBGW:
        case SM16703P_COLOR_ORDER_BRGW:
        case SM16703P_COLOR_ORDER_BGRW:
        case SM16703P_COLOR_ORDER_GRBW:
        case SM16703P_COLOR_ORDER_GBRW:
            channels = 4; break;
        case SM16703P_COLOR_ORDER_RGBCW:
        case SM16703P_COLOR_ORDER_RBGCW:
        case SM16703P_COLOR_ORDER_BRGCW:
        case SM16703P_COLOR_ORDER_BGRCW:
        case SM16703P_COLOR_ORDER_GRBCW:
        case SM16703P_COLOR_ORDER_GBRCW:
            channels = 5; break;
        default:
            return;  // Invalid color order
    }
	translate_byte(b0, spiLED.buf + (spiLED.ofs + 0 + (pixel * channels * 4)));
	translate_byte(b1, spiLED.buf + (spiLED.ofs + 4 + (pixel * channels * 4)));
	translate_byte(b2, spiLED.buf + (spiLED.ofs + 8 + (pixel * channels * 4)));
	if (b3 > -1){
		translate_byte(b3, spiLED.buf + (spiLED.ofs + 12 + (pixel * channels * 4)));
		if (b4 > -1){
			translate_byte(b4, spiLED.buf + (spiLED.ofs + 16 + (pixel * channels * 4)));
		}
	}
}
void SM16703P_setMultiplePixel(uint32_t pixel, uint8_t *data, bool push) {

	// Return if driver is not loaded
	if (!spiLED.ready)
		return;

	// Check max pixel
	if (pixel > pixel_count)
		pixel = pixel_count;
		
	int c = -1, w = -1;
    switch (color_order) {
        case SM16703P_COLOR_ORDER_RGBW:
        case SM16703P_COLOR_ORDER_RBGW:
        case SM16703P_COLOR_ORDER_BRGW:
        case SM16703P_COLOR_ORDER_BGRW:
        case SM16703P_COLOR_ORDER_GRBW:
        case SM16703P_COLOR_ORDER_GBRW:
            c=0; break;
        case SM16703P_COLOR_ORDER_RGBCW:
        case SM16703P_COLOR_ORDER_RBGCW:
        case SM16703P_COLOR_ORDER_BRGCW:
        case SM16703P_COLOR_ORDER_BGRCW:
        case SM16703P_COLOR_ORDER_GRBCW:
        case SM16703P_COLOR_ORDER_GBRCW:
            w=0; break;
    }
	
	// Iterate over pixel
	uint8_t *dst = spiLED.buf + spiLED.ofs;
	for (uint32_t i = 0; i < pixel; i++) {
		uint8_t r, g, b;
		r = *data++;
		g = *data++;
		b = *data++;
		if (c > -1){
			c = *data++;
			if (w > -1){
				w = *data++;
			}
		}
		SM16703P_setPixel((int)i, (int)r, (int)g, (int)b, (int)c, (int)w);
	}
	if (push) {
		SPIDMA_StartTX(spiLED.msg);
	}
}
extern float g_brightness0to100;//TODO

void SM16703P_setPixelWithBrig(int pixel, int r, int g, int b, ...) {
	va_list args;
	va_start(args, numArgs)
	int c = -1, w = -1;
	if (numArgs > 0){
		c=va_arg(args, int);
		if (numArgs > 1){
			w=va_arg(args, int);
		}
	}
	va_end(args);
#if ENABLE_LED_BASIC
	r = (int)(r * g_brightness0to100*0.01f);
	g = (int)(g * g_brightness0to100*0.01f);
	b = (int)(b * g_brightness0to100*0.01f);
	if (c > -1){
		c = (int)(c * g_brightness0to100*0.01f);
		if ( w > -1){
			w = (int)(w * g_brightness0to100*0.01f);
		}
	}
#endif
	SM16703P_setPixel(pixel, r, g, b, c, w);
}

#define SCALE8_PIXEL(x, scale) (uint8_t)(((uint32_t)x * (uint32_t)scale) / 256)

void SM16703P_scaleAllPixels(int scale, channels) {
	if (channels == -1){
		switch (color_order) {
			case SM16703P_COLOR_ORDER_RGB:
			case SM16703P_COLOR_ORDER_RBG:
			case SM16703P_COLOR_ORDER_BRG:
			case SM16703P_COLOR_ORDER_BGR:
			case SM16703P_COLOR_ORDER_GRB:
			case SM16703P_COLOR_ORDER_GBR:
				channels = 4; break;
			case SM16703P_COLOR_ORDER_RGBW:
			case SM16703P_COLOR_ORDER_RBGW:
			case SM16703P_COLOR_ORDER_BRGW:
			case SM16703P_COLOR_ORDER_BGRW:
			case SM16703P_COLOR_ORDER_GRBW:
			case SM16703P_COLOR_ORDER_GBRW:
				channels = 5; break;
			case SM16703P_COLOR_ORDER_RGBCW:
			case SM16703P_COLOR_ORDER_RBGCW:
			case SM16703P_COLOR_ORDER_BRGCW:
			case SM16703P_COLOR_ORDER_BGRCW:
			case SM16703P_COLOR_ORDER_GRBCW:
			case SM16703P_COLOR_ORDER_GBRCW:
				channels = 6; break;
			default:
				return;  // Invalid color order
		}
	}

	int pixel;
	byte b;
	int ofs;
	byte *data, *input;

	for (pixel = 0; pixel < pixel_count; pixel++) {
		for (ofs = 0; ofs < 3; ofs++) {
			data = spiLED.buf + (spiLED.ofs + ofs * 4 + (pixel * channels * 4));
			b = reverse_translate_byte(data);
			b = SCALE8_PIXEL(b, scale);
			translate_byte(b, data);
		}
	}
}

void SM16703P_setAllPixels(int r, int g, int b, ...) {
	if (!spiLED.ready)
		return;
	va_list args;
	va_start(args, numArgs)
	int c = -1, w = -1;
	if (numArgs > 0){
		c=va_arg(args, int);
		if (numArgs > 1){
			w=va_arg(args, int);
		}
	}
	va_end(args);

    // Color order mappings: Determine the correct byte order and channel count
    int b0, b1, b2, b3 = -1, b4 = -1, channels;

    // Set color channels based on color order
    switch (color_order) {
        case SM16703P_COLOR_ORDER_RGB:
        case SM16703P_COLOR_ORDER_RGBW:
        case SM16703P_COLOR_ORDER_RGBCW:
            b0 = r; b1 = g; b2 = b; b3 = c; b4 = w; break;
        case SM16703P_COLOR_ORDER_RBG:
        case SM16703P_COLOR_ORDER_RBGW:
        case SM16703P_COLOR_ORDER_RBGCW:
            b0 = r; b1 = b; b2 = g; b3 = c; b4 = w; break;
        case SM16703P_COLOR_ORDER_BRG:
        case SM16703P_COLOR_ORDER_BRGW:
        case SM16703P_COLOR_ORDER_BRGCW:
            b0 = b; b1 = r; b2 = g; b3 = c; b4 = w; break;
        case SM16703P_COLOR_ORDER_BGR:
        case SM16703P_COLOR_ORDER_BGRW:
        case SM16703P_COLOR_ORDER_BGRCW:
            b0 = b; b1 = g; b2 = r; b3 = c; b4 = w; break;
        case SM16703P_COLOR_ORDER_GRB:
        case SM16703P_COLOR_ORDER_GRBW:
        case SM16703P_COLOR_ORDER_GRBCW:
            b0 = g; b1 = r; b2 = b; b3 = c; b4 = w; break;
        case SM16703P_COLOR_ORDER_GBR:
        case SM16703P_COLOR_ORDER_GBRW:
        case SM16703P_COLOR_ORDER_GBRCW:
            b0 = g; b1 = b; b2 = r; b3 = c; b4 = w; break;
        default:
            return;  // Invalid color order
    }

    // Set channel count based on color order
    switch (color_order) {
        case SM16703P_COLOR_ORDER_RGB:
        case SM16703P_COLOR_ORDER_RBG:
        case SM16703P_COLOR_ORDER_BRG:
        case SM16703P_COLOR_ORDER_BGR:
        case SM16703P_COLOR_ORDER_GRB:
        case SM16703P_COLOR_ORDER_GBR:
            channels = 3; break;
        case SM16703P_COLOR_ORDER_RGBW:
        case SM16703P_COLOR_ORDER_RBGW:
        case SM16703P_COLOR_ORDER_BRGW:
        case SM16703P_COLOR_ORDER_BGRW:
        case SM16703P_COLOR_ORDER_GRBW:
        case SM16703P_COLOR_ORDER_GBRW:
            channels = 4; break;
        case SM16703P_COLOR_ORDER_RGBCW:
        case SM16703P_COLOR_ORDER_RBGCW:
        case SM16703P_COLOR_ORDER_BRGCW:
        case SM16703P_COLOR_ORDER_BGRCW:
        case SM16703P_COLOR_ORDER_GRBCW:
        case SM16703P_COLOR_ORDER_GBRCW:
            channels = 5; break;
        default:
            return;  // Invalid color order
    }

    for (int pixel = 0; pixel < pixel_count; pixel++) {
        int pixel_offset = spiLED.ofs + pixel * channels * 4;  // 4 bytes per channel (R, G, B, optional white, optional ColdWhite and WarmWhite)

        translate_byte(b0, spiLED.buf + pixel_offset);
        translate_byte(b1, spiLED.buf + pixel_offset + 4);
        translate_byte(b2, spiLED.buf + pixel_offset + 8);
        
        if (b3 > -1) {
            translate_byte(b3, spiLED.buf + pixel_offset + 12);
            if (b4 > -1) {
                translate_byte(b4, spiLED.buf + pixel_offset + 16);
            }
        }
    }
}



// SM16703P_SetRaw bUpdate byteOfs HexData
// SM16703P_SetRaw 1 0 FF000000FF000000FF
commandResult_t SM16703P_CMD_setRaw(const void *context, const char *cmd, const char *args, int flags) {
	int ofs, bPush;
	Tokenizer_TokenizeString(args, 0);
	bPush = Tokenizer_GetArgInteger(0);
	ofs = Tokenizer_GetArgInteger(1);
	SPILED_SetRawHexString(ofs, Tokenizer_GetArg(2), bPush);
	return CMD_RES_OK;
}
commandResult_t SM16703P_CMD_setPixel(const void *context, const char *cmd, const char *args, int flags) {
	int i, r, g, b, c=-1, w=-1;
	int pixel = 0;
	const char *all = 0;
	Tokenizer_TokenizeString(args, 0);

	int args = Tokenizer_GetArgsCount()

	if (args < 4) {
		ADDLOG_INFO(LOG_FEATURE_CMD, "Not Enough Arguments for init SM16703P: Amount of LEDs missing");
		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
	}

	all = Tokenizer_GetArg(0);
	if (*all == 'a') {

	}
	else {
		pixel = Tokenizer_GetArgInteger(0);
		all = 0;
	}
	r = Tokenizer_GetArgIntegerRange(1, 0, 255);
	g = Tokenizer_GetArgIntegerRange(2, 0, 255);
	b = Tokenizer_GetArgIntegerRange(3, 0, 255);
	int channels = 3;
	if (args > 4) {
		c = Tokenizer_GetArgIntegerRange(4, 0, 255);
		channels = 4;
		if (args > 5) {
			w = Tokenizer_GetArgIntegerRange(5, 0, 255);
			channels = 5;
		}
    }

	ADDLOG_INFO(LOG_FEATURE_CMD, "Set Pixel %i to R %i G %i B %i and  %i , %i for the whites?", pixel, r, g, b, c, w);

	if (all) {
		for (i = 0; i < pixel_count; i++) {
			SM16703P_setPixel(i, r, g, b, c, w);
		}
	}
	else {
		SM16703P_setPixel(pixel, r, g, b, c, w);

		ADDLOG_INFO(LOG_FEATURE_CMD, "Raw Data 0x%02x 0x%02x 0x%02x 0x%02x - 0x%02x 0x%02x 0x%02x 0x%02x - 0x%02x 0x%02x 0x%02x 0x%02x",
			spiLED.buf[spiLED.ofs + 0 + (pixel * channels * 4)],
			spiLED.buf[spiLED.ofs + 1 + (pixel * channels * 4)],
			spiLED.buf[spiLED.ofs + 2 + (pixel * channels * 4)],
			spiLED.buf[spiLED.ofs + 3 + (pixel * channels * 4)],
			spiLED.buf[spiLED.ofs + 4 + (pixel * channels * 4)],
			spiLED.buf[spiLED.ofs + 5 + (pixel * channels * 4)],
			spiLED.buf[spiLED.ofs + 6 + (pixel * channels * 4)],
			spiLED.buf[spiLED.ofs + 7 + (pixel * channels * 4)],
			spiLED.buf[spiLED.ofs + 8 + (pixel * channels * 4)],
			spiLED.buf[spiLED.ofs + 9 + (pixel * channels * 4)],
			spiLED.buf[spiLED.ofs + 10 + (pixel * channels * 4)],
			spiLED.buf[spiLED.ofs + 11 + (pixel * channels * 4)]);
	}


	return CMD_RES_OK;
}

commandResult_t SM16703P_InitForLEDCount(const void *context, const char *cmd, const char *args, int flags) {

	Tokenizer_TokenizeString(args, 0);

	if (Tokenizer_GetArgsCount() == 0) {
		ADDLOG_INFO(LOG_FEATURE_CMD, "Not Enough Arguments for init SM16703P: Amount of LEDs missing");
		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
	}

	// First arg: number of pixel to address
	pixel_count = Tokenizer_GetArgIntegerRange(0, 0, 255);
	// Second arg (optional, default "RGB"): pixel format of "RGB" or "GRB"
	if (Tokenizer_GetArgsCount() > 1) {
		const char *format = Tokenizer_GetArg(1);
		if (!stricmp(format, "RGBCW")) {
			color_order = SM16703P_COLOR_ORDER_RGBCW;
		}
		else if (!stricmp(format, "RBGCW")) {
			color_order = SM16703P_COLOR_ORDER_RBGCW;
		}
		else if (!stricmp(format, "BRGCW")) {
			color_order = SM16703P_COLOR_ORDER_BRGCW;
		}
		else if (!stricmp(format, "BGRCW")) {
			color_order = SM16703P_COLOR_ORDER_BGRCW;
		}
		else if (!stricmp(format, "GRBCW")) {
			color_order = SM16703P_COLOR_ORDER_GRBCW;
		}
		else if (!stricmp(format, "GBRCW")) {
			color_order = SM16703P_COLOR_ORDER_GBRCW;
		}
		else if (!stricmp(format, "RGBW")) {
			color_order = SM16703P_COLOR_ORDER_RGBW;
		}
		else if (!stricmp(format, "RBGW")) {
			color_order = SM16703P_COLOR_ORDER_RBGW;
		}
		else if (!stricmp(format, "BRGW")) {
			color_order = SM16703P_COLOR_ORDER_BRGW;
		}
		else if (!stricmp(format, "BGRW")) {
			color_order = SM16703P_COLOR_ORDER_BGRW;
		}
		else if (!stricmp(format, "GRBW")) {
			color_order = SM16703P_COLOR_ORDER_GRBW;
		}
		else if (!stricmp(format, "GBRW")) {
			color_order = SM16703P_COLOR_ORDER_GBRW;
		}
		else if (!stricmp(format, "RGB")) {
			color_order = SM16703P_COLOR_ORDER_RGB;
		}
		else if (!stricmp(format, "RBG")) {
			color_order = SM16703P_COLOR_ORDER_RBG;
		}
		else if (!stricmp(format, "BRG")) {
			color_order = SM16703P_COLOR_ORDER_BRG;
		}
		else if (!stricmp(format, "BGR")) {
			color_order = SM16703P_COLOR_ORDER_BGR;
		}
		else if (!stricmp(format, "GRB")) {
			color_order = SM16703P_COLOR_ORDER_GRB;
		}
		else if (!stricmp(format, "GBR")) {
			color_order = SM16703P_COLOR_ORDER_GBR;
		}
		else {
			ADDLOG_INFO(LOG_FEATURE_CMD, "Invalid color format, should be combination of R,G,B followed by optional White or ColdWhite and WarmWhite", format);
			return CMD_RES_ERROR;
		}
	}
	// Third arg (optional, default "0"): spiLED.ofs to prepend to each transmission
	if (Tokenizer_GetArgsCount() > 2) {
		spiLED.ofs = Tokenizer_GetArgIntegerRange(2, 0, 255);
	}
	// Fourth arg (optional, default "64"): spiLED.padding to append to each transmission
	//spiLED.padding = 64;
	//if (Tokenizer_GetArgsCount() > 3) {
	//	spiLED.padding = Tokenizer_GetArgIntegerRange(3, 0, 255);
	//}

	ADDLOG_INFO(LOG_FEATURE_CMD, "Register driver with %i LEDs", pixel_count);
	
	// each pixel is RGB, so 3 bytes per pixel
	SPILED_InitDMA(pixel_count * 3);

	return CMD_RES_OK;
}

void SM16703P_Show() {
	SPIDMA_StartTX(spiLED.msg);
}
static commandResult_t SM16703P_StartTX(const void *context, const char *cmd, const char *args, int flags) {
	if (!spiLED.ready)
		return CMD_RES_ERROR;
	SM16703P_Show();
	return CMD_RES_OK;
}
//static commandResult_t SM16703P_CMD_sendBytes(const void *context, const char *cmd, const char *args, int flags) {
//	if (!spiLED.ready)
//		return CMD_RES_ERROR;
//	const char *s = args;
//	int i = spiLED.ofs;
//	while (*s && s[1]) {
//		*(spiLED.buf + (i)) = hexbyte(s);
//		s += 2;
//		i++;
//	}
//	while (i < spiLED.msg->send_len) {
//		*(spiLED.buf + (i)) = 0;
//	}
//	SPIDMA_StartTX(spiLED.msg);
//	return CMD_RES_OK;
//}

// startDriver SM16703P
// backlog startDriver SM16703P; SM16703P_Test
void SM16703P_Init() {
	SPILED_Init();

	//cmddetail:{"name":"SM16703P_Init","args":"[NumberOfLEDs][ColorOrder]",
	//cmddetail:"descr":"This will setup LED driver for a strip with given number of LEDs. Please note that it also works for WS2812B and similiar LEDs. You can optionally set the color order with either RGB, RBG, BRG, BGB, GRB or GBR (default RGB). See [tutorial](https://www.elektroda.com/rtvforum/topic4036716.html).",
	//cmddetail:"fn":"NULL);","file":"driver/drv_sm16703P.c","requires":"",
	//cmddetail:"examples":""}
	CMD_RegisterCommand("SM16703P_Init", SM16703P_InitForLEDCount, NULL);
	//cmddetail:{"name":"SM16703P_Start","args":"",
	//cmddetail:"descr":"This will send the currently set data to the strip. Please note that it also works for WS2812B and similiar LEDs. See [tutorial](https://www.elektroda.com/rtvforum/topic4036716.html).",
	//cmddetail:"fn":"NULL);","file":"driver/drv_sm16703P.c","requires":"",
	//cmddetail:"examples":""}
	CMD_RegisterCommand("SM16703P_Start", SM16703P_StartTX, NULL);
	//cmddetail:{"name":"SM16703P_SetPixel","args":"[index/all] [R] [G] [B] [W|[CW, WW]]",
	//cmddetail:"descr":"Sets a pixel for LED strip. Index can be a number or 'all' keyword to set all. Then, 3 integer values for R, G and B, followed by option White or ColdWhite & WarmWhite. Please note that it also works for WS2812B and similiar LEDs. See [tutorial](https://www.elektroda.com/rtvforum/topic4036716.html).",
	//cmddetail:"fn":"NULL);","file":"driver/drv_sm16703P.c","requires":"",
	//cmddetail:"examples":""}
	CMD_RegisterCommand("SM16703P_SetPixel", SM16703P_CMD_setPixel, NULL);
	//cmddetail:{"name":"SM16703P_SetRaw","args":"[bUpdate] [byteOfs] [HexData]",
	//cmddetail:"descr":"Sets the raw data bytes for SPI DMA LED driver at the given offset. Hex data should be as a hex string, for example, FF00AA, etc. The bUpdate, if set to 1, will run SM16703P_Start automatically after setting data. Please note that it also works for WS2812B and similiar LEDs. See [tutorial](https://www.elektroda.com/rtvforum/topic4036716.html).",
	//cmddetail:"fn":"NULL);","file":"driver/drv_sm16703P.c","requires":"",
	//cmddetail:"examples":""}
	CMD_RegisterCommand("SM16703P_SetRaw", SM16703P_CMD_setRaw, NULL);

	//CMD_RegisterCommand("SM16703P_SendBytes", SM16703P_CMD_sendBytes, NULL);
}
#endif
