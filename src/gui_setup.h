#ifndef UNITY_BUILD
 #define UNITY_BUILD 0
 #include "common.h"
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
    Texture2D Dog;
    Texture2D Close;
    Texture2D CloseSmall;
    Texture2D MinimizeSmall;
    Texture2D Layouts;
} GUI_Icons;

GUI_Icons GUI_LoadIcons()
{
    GUI_Icons icons = {
        .New            = LoadTexture("ico/new.png"),
        .Open           = LoadTexture("ico/open.png"),
        .Save           = LoadTexture("ico/save.png"),
        .Setup          = LoadTexture("ico/setup.png"),
        .Error          = LoadTexture("ico/error.png"),
        .Face           = LoadTexture("ico/face.png"),
        .Dog            = LoadTexture("ico/dog.png"),
        .Close          = LoadTexture("ico/close.png"),
        .CloseSmall     = LoadTexture("ico/close-sm.png"),
        .MinimizeSmall  = LoadTexture("ico/minimize-sm.png"),
        .Layouts        = LoadTexture("ico/layouts.png"),
    };
    return icons;
}

typedef struct {
    float       icon_size;
    float       icon_size_sm;
    Vector2     icon_delta; 
    GUI_Icons   icons;
} GUI_IconSetup;

GUI_IconSetup GUI_MakeIconSetupDefault()
{
    GUI_IconSetup setup = {
        .icon_size      = 32,
        .icon_size_sm   = 16,
        .icon_delta     = (Vector2){0, 0},
        .icons          = GUI_LoadIcons()
    };
    return setup;
}

// > THEME SETUP
//   STABILITY : ███░░░░░░░  30%
//   NOTES     : Edit and more themes.

typedef enum {
    EGUI_FontType_Default,
    EGUI_FontType_GUI,
    EGUI_FontType_Count
} EGUI_FontType;


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
Color GUI_GetThemeColorFromHue(float hue, float t) {
    if (t < 0.0f)  t = 0.0f;
    if (t > 1.25f) t = 1.25f;

    float s = 0.13f + t * (0.24f - 0.13f); // 0.13 → 0.24
    float v = 0.89f - t * (0.89f - 0.33f); // 0.89 → 0.33

    // Normaliza hue al rango [0, 360)
    while (hue < 0.0f)   hue += 360.0f;
    while (hue >= 360.0f) hue -= 360.0f;

    return ColorFromHSV(hue, s, v);
}

GUI_ThemeColors GUI_MakeThemeColors(float hue)
{
    GUI_ThemeColors colors = {
        .tx_color_0 = GUI_GetThemeColorFromHue(hue, 0.0f),
        .tx_color_1 = GUI_GetThemeColorFromHue(hue, 1.15f), // darker than bg_color_3
        .bg_color_0 = GUI_GetThemeColorFromHue(hue, 0.25f),
        .bg_color_1 = GUI_GetThemeColorFromHue(hue, 0.5f),
        .bg_color_2 = GUI_GetThemeColorFromHue(hue, 0.75f),
        .bg_color_3 = GUI_GetThemeColorFromHue(hue, 1.0f)
    };
    return colors;
}

typedef struct {
    GUI_ThemeColors     gray;
    GUI_ThemeColors     red;
    GUI_ThemeColors     green;
    float               bg_alpha;
    float               color_change;
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
        .gray           = GUI_MakeThemeColors(180.0f),
        .red            = GUI_MakeThemeColors(3.0f),
        .green          = GUI_MakeThemeColors(97.0f),
        .bg_alpha       = 0.95,
        .color_change   = 0.05
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
    float           blink_alpha;
} GUI_FontSetup;

GUI_FontSetup GUI_MakeFontSetupDefault(EGUI_FontType content) 
{
    _Static_assert(EGUI_FontType_Count == 2,  "Update fonts here");

    switch (content) {
    case EGUI_FontType_GUI: {
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
            .blink_alpha        = 0.95f
        };
        SetTextureFilter(result.font_custom.texture, TEXTURE_FILTER_POINT);
        return result;
    }
    case EGUI_FontType_Default:
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
            .blink_delta        = (Vector2){ 0.0f, 0.0f },
            .blink_alpha        = 0.95f
        };
        SetTextureFilter(result.font_custom.texture, TEXTURE_FILTER_POINT);
        return result;
    }
    }
}



// > POINTER SETUP (MOUSE CURSORS)
//   STABILITY : █████░░░░░  50%
//   NOTES     : Add more

typedef enum {
    EGUI_Pointer_None,
    EGUI_Pointer_Default,
    EGUI_Pointer_AGS,
    EGUI_Pointer_Text,
    EGUI_Pointer_Resize,    
    EGUI_Pointer_Count,    
} EGUI_Pointer;

typedef struct {
    Texture2D       pointer_texture;
    Vector2         pointer_delta_normalized;
    float           pointer_scale;
    float           pointer_alpha;
    Vector2         trail_delta_normalized;
    EGUI_Pointer    additional;
} GUI_PointerSetup;

typedef struct {
    GUI_Theme           theme;
    GUI_IconSetup       icon_setup;
    GUI_FontSetup       font_setups[EGUI_FontType_Count];
    GUI_PointerSetup    pointer_setups[EGUI_Pointer_Count];
} GUI_Setup;


GUI_PointerSetup GUI_MakePointerSetupForType(EGUI_Pointer pointer_type)
{
    GUI_PointerSetup setup = { 0 };
    _Static_assert(EGUI_Pointer_Count  == 5,  "Update pointers here!");

    switch (pointer_type)
    {
        case EGUI_Pointer_AGS:
            setup.pointer_texture           = LoadTexture("ico/cursor.png");
            setup.pointer_delta_normalized  = (Vector2){ 0.5f, 0.5f };
            setup.pointer_scale             = 2.0f;
            setup.pointer_alpha             = 1.0;
            setup.trail_delta_normalized    = (Vector2){ 1.0f, 1.0f };
            setup.additional                = EGUI_Pointer_None;
            break;

        case EGUI_Pointer_Text:
            setup.pointer_texture           = LoadTexture("ico/pointer_txt.png");
            setup.pointer_delta_normalized  = (Vector2){ -1.0f, 0.0f };
            setup.pointer_scale             = 2.0f;
            setup.pointer_alpha             = 0.85;
            setup.trail_delta_normalized    = (Vector2){ 1.0f, 1.0f };
            setup.additional                = EGUI_Pointer_Default;
            break;

        case EGUI_Pointer_Resize:
            setup.pointer_texture           = LoadTexture("ico/pointer_resize.png");
            setup.pointer_delta_normalized  = (Vector2){ 0.0f, 0.0f };
            setup.pointer_scale             = 2.0f;
            setup.pointer_alpha             = 1.0;
            setup.trail_delta_normalized    = (Vector2){ 1.0f, 1.0f };
            setup.additional                = EGUI_Pointer_None;
            break;

        case EGUI_Pointer_Default:
        default:
            setup.pointer_texture           = LoadTexture("ico/pointer.png");
            setup.pointer_delta_normalized  = (Vector2){ 0.0f, 0.0f };
            setup.pointer_scale             = 2.0f;
            setup.pointer_alpha             = 1.0;
            setup.trail_delta_normalized    = (Vector2){ 1.0f, 1.0f };
            setup.additional                = EGUI_Pointer_None;
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
        .theme      = GUI_MakeThemeDefault(),
        .icon_setup = GUI_MakeIconSetupDefault()
    };

    // Fonts
    for (int i = 0; i < EGUI_FontType_Count; i++) {
        setup.font_setups[i] = GUI_MakeFontSetupDefault((EGUI_FontType)i);
    }

    // Pointers
    // 0 is None
    for (int i = 1; i < EGUI_Pointer_Count; i++) {
        setup.pointer_setups[i] = GUI_MakePointerSetupForType((EGUI_Pointer)i);
    }
    return setup;
}
