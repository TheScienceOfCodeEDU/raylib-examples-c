#ifndef UNITY_BUILD
 #include <string.h>
 #include <stdio.h>
 #include "rayext.h"
 #include "str.h"
 #include "env.h"
#endif

// > ICON SETUP
//   STABILITY : ███░░░░░░░  30%
//   NOTES     : Save and restore

typedef struct {
    Texture2D New;
    Texture2D Open;
    Texture2D Save;
    Texture2D Setup;
    Texture2D Error;
    Texture2D Face;
} GUI_Icons;

GUI_Icons GUI_LoadIcons()
{
    GUI_Icons icons = {
        .New    = LoadTexture("ico/new.png"),
        .Open   = LoadTexture("ico/open.png"),
        .Save   = LoadTexture("ico/save.png"),
        .Setup  = LoadTexture("ico/setup.png"),
        .Error  = LoadTexture("ico/error.png"),
        .Face   = LoadTexture("ico/face.png")
    };
    return icons;
}

typedef struct {
    float       icon_size;
    Vector2     icon_delta; 
    GUI_Icons   icons;
} GUI_IconSetup;

GUI_IconSetup GUI_MakeIconSetupDefault()
{
    GUI_IconSetup setup = {
        .icon_size  = 32,
        .icon_delta = (Vector2){0, 0},
        .icons      = GUI_LoadIcons()
    };
    return setup;
}

// > THEME SETUP
//   STABILITY : ███░░░░░░░  30%
//   NOTES     : Edit and more themes.

#define COLOR_CHANGE        0.05

typedef enum {
    EGUI_Content_Default,
    EGUI_Content_GUI,
    EGUI_Content_Count
} EGUI_Content;


typedef struct {
    Color tx_color_0;
    Color tx_color_1;
    Color bg_color_0;
    Color bg_color_1;
    Color bg_color_2;
    Color bg_color_3;
} GUI_ThemeColors;

// Generates a color with a specified hue and interpolated saturation and value.
// Parameters:
//   hue: Hue value in degrees (e.g., 180.0f for cyan).
//   t: Interpolation factor [0.0, 1.0] controlling saturation (0.13 to 0.24) and value (0.89 to 0.33).
// Returns:
//   A Raylib Color struct with RGB values (0-255) and full alpha (255).
Color GetThemeColorFromHue(float hue, float t) {
    // Clamp t to [0, 1.25] allowing "extra darkness"
    if (t < 0.0f) t = 0.0f;
    if (t > 1.25f) t = 1.25f;

    // Interpolate saturation (S) and value (V)
    // S ranges from 0.13 (light) to 0.24 (dark)
    // V ranges from 0.89 (light) to 0.33 (dark)
    float s = 0.13f + t * (0.24f - 0.13f);
    float v = 0.89f - t * (0.89f - 0.33f);
    s = s > 1.0f ? 1.0f : s;
    v = v > 1.0f ? 1.0f : v;

    // Convert HSV to RGB
    float c = v * s;            // Chroma
    float h_prime = hue / 60.0f; // Hue sector
    float x = c * (1.0f - fabsf(fmodf(h_prime, 2.0f) - 1.0f));
    float m = v - c;

    float r_prime, g_prime, b_prime;
    int sector = (int)h_prime;

    // Assign RGB based on hue sector
    switch (sector) {
        case 0: r_prime = c; g_prime = x; b_prime = 0; break;
        case 1: r_prime = x; g_prime = c; b_prime = 0; break;
        case 2: r_prime = 0; g_prime = c; b_prime = x; break;
        case 3: r_prime = 0; g_prime = x; b_prime = c; break;
        case 4: r_prime = x; g_prime = 0; b_prime = c; break;
        case 5: r_prime = c; g_prime = 0; b_prime = x; break;
        default: r_prime = 0; g_prime = 0; b_prime = 0; break;
    }

    // Scale to RGB (0-255) and create Raylib Color
    unsigned char r = (unsigned char)((r_prime + m) * 255.0f);
    unsigned char g = (unsigned char)((g_prime + m) * 255.0f);
    unsigned char b = (unsigned char)((b_prime + m) * 255.0f);

    return (Color){r, g, b, 255}; // Full alpha
}

GUI_ThemeColors GUI_MakeThemeColors(float hue)
{
    GUI_ThemeColors colors = {
        .tx_color_0 = GetThemeColorFromHue(hue, 0.0f),
        .tx_color_1 = GetThemeColorFromHue(hue, 1.15f), // darker than bg_color_3
        .bg_color_0 = GetThemeColorFromHue(hue, 0.25f),
        .bg_color_1 = GetThemeColorFromHue(hue, 0.5f),
        .bg_color_2 = GetThemeColorFromHue(hue, 0.75f),
        .bg_color_3 = GetThemeColorFromHue(hue, 1.0f)
    };
    return colors;
}

typedef struct {
    GUI_ThemeColors gray;
    GUI_ThemeColors red;
    GUI_ThemeColors green;
} GUI_Theme;


GUI_Theme GUI_MakeThemeDefault()
{
    /*// Background colors for another theme...
        (Color) { 80, 67, 48, opacity },    // bg_color_0: Dark brown with variable opacity
        (Color) { 116, 100, 67, opacity },  // bg_color_1: Medium brown with variable opacity
        (Color) { 58, 49, 35, opacity },    // bg_color_2: Very dark brown with variable opacity

        // Primary colors
        (Color) { 171, 158, 127, 255 },    // color_0: Light beige, fully opaque
        (Color) { 238, 208, 147, 255 },    // color_1: Warm beige, fully opaque
        (Color) { 253, 250, 85, 255 },     // color_2: Light yellow, fully opaque

        // Border colors
        (Color) { 33, 33, 33, 200 },       // b_color_0: Very dark gray, semi-opaque
        (Color) { 118, 118, 118, 200 },    // b_color_1: Medium gray, semi-opaque*/

    GUI_Theme theme = {
        // Theme colors
        .gray   = GUI_MakeThemeColors(180.0f),
        .red    = GUI_MakeThemeColors(3.0f),
        .green  = GUI_MakeThemeColors(97.0f)
    };

    return theme;
}


// > STATE
//   STABILITY : █████░░░░░  50%
//   NOTES     : Review naming

typedef struct {
    float           default_height;
    float           border;

    float           font_scale;
    Vector2         font_delta;             // Delta adjustement
    Font            font_custom;
    bool            font_use_custom;        // Indicates if a custom font is used
    float           font_spacing;           
    Vector2         blink_size;             // Size of the blinking cursor
    Vector2         blink_delta;            // Blink adjustment
} GUI_FontSetup;

GUI_FontSetup GUI_MakeFontSetupDefault(EGUI_Content content) {
    switch (content) {
    case EGUI_Content_GUI: {
        GUI_FontSetup result = {
            .default_height     = 36,
            .border             = 2.0f,

            .font_scale         = 2.0f,
            .font_delta         = (Vector2){ 6.0f, 6.0f },
            .font_custom        = LoadFontEx("fnt/unifont-17.0.01.otf", 16, 0, 0),
            .font_use_custom    = 0,
            .font_spacing       = 1.0f,
            .blink_size         = (Vector2){ 1.0f, 30.0f },
            .blink_delta        = (Vector2){ 0.0f, 0.0f },
        };
        return result;
    }
    case EGUI_Content_Default:
    default: {
        GUI_FontSetup result = {
            .default_height     = 30,
            .border             = 2.0f,

            .font_scale         = 1.0f,
            .font_delta         = (Vector2){ 4.f, 4.f },
            .font_custom        = LoadFontEx("fnt/unifont-17.0.01.otf", 16, 0, 0),
            .font_use_custom    = 1,
            .font_spacing       = 1.0f,
            .blink_size         = (Vector2){ 1.0f, 24.0f },
            .blink_delta        = (Vector2){ -0.0f, 0.0f },
        };
        SetTextureFilter(result.font_custom.texture, TEXTURE_FILTER_POINT);
        return result;
    }
    }
}



// > POINTER SETUP (MOUSE CURSORS)
//   STABILITY : █████░░░░░  50%
//   NOTES     : Add more

typedef struct {
    Texture2D   pointer_texture;
    Vector2     pointer_delta_normalized;
    float       pointer_scale;
    float       pointer_alpha;
} GUI_PointerSetup;

typedef enum {
    EGUI_Pointer_Default,
    EGUI_Pointer_AGS,
    EGUI_Pointer_Text,
    EGUI_Pointer_Count
} EGUI_Pointer;

typedef struct {
    GUI_FontSetup       font_setups[EGUI_Content_Count];
    GUI_PointerSetup    pointer_setups[EGUI_Pointer_Count];
    GUI_Theme           theme;
    GUI_IconSetup       icon_setup;
} GUI_Setup;

GUI_PointerSetup GUI_MakePointerSetup(Texture2D texture, Vector2 delta_normalized)
{
    return (GUI_PointerSetup) {
        .pointer_texture = texture,
        .pointer_delta_normalized = delta_normalized
    };
}

GUI_PointerSetup GUI_MakePointerSetupForType(EGUI_Pointer pointer_type)
{
    GUI_PointerSetup setup = { 0 };

    switch (pointer_type)
    {
        case EGUI_Pointer_AGS:
            setup.pointer_texture           = LoadTexture("ico/cursor.png");
            setup.pointer_delta_normalized  = (Vector2){ 0.5f, 0.5f };
            setup.pointer_scale             = 2.0f;
            setup.pointer_alpha             = 1.0;
            break;

        case EGUI_Pointer_Text:
            setup.pointer_texture           = LoadTexture("ico/pointer_txt.png");
            setup.pointer_delta_normalized  = (Vector2){ 0.0f, 0.5f };
            setup.pointer_scale             = 2.0f;
            setup.pointer_alpha             = 0.75;
            break;

        case EGUI_Pointer_Default:
        default:
            setup.pointer_texture           = LoadTexture("ico/pointer.png");
            setup.pointer_delta_normalized  = (Vector2){ 0.0f, 0.0f };
            setup.pointer_scale             = 2.0f;
            setup.pointer_alpha             = 1.0;
            break;

    }

    return setup;
}


// > STATE
//   STABILITY : █████░░░░░  50%
//   NOTES     : Save, restore and edit

GUI_Setup GUI_MakeSetupDefault()
{
    GUI_Setup setup = {
        .font_setups = {
            GUI_MakeFontSetupDefault(EGUI_Content_Default),
            GUI_MakeFontSetupDefault(EGUI_Content_GUI)
        },
        .pointer_setups = {
            GUI_MakePointerSetupForType(EGUI_Pointer_Default),
            GUI_MakePointerSetupForType(EGUI_Pointer_AGS),
            GUI_MakePointerSetupForType(EGUI_Pointer_Text)
        },
        .theme      = GUI_MakeThemeDefault(),
        .icon_setup = GUI_MakeIconSetupDefault()
    };
    return setup;
}
