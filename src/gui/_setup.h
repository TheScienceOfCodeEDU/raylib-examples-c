#pragma once
#ifndef UNITY_BUILD
#define IMPLEMENT_ALL   1
#include "main.h"
#endif


// > SUBMODULE: SETUP
// > INDEX
GUI_Icons           GUI_LoadIcons();
GUI_IconSetup       GUI_MakeIconSetup(GUI_Icons icons);

Color               GUI_GenerateThemeColor(float hue, float intensity);
GUI_ThemeColors     GUI_GenerateThemeColors(float hue);
GUI_Theme           GUI_GenerateTheme();

GUI_FontSetup       GUI_LoadFontSetupDefault(EGUI_FontType content);
GUI_PointerSetup    GUI_LoadPointerSetupForType(EGUI_Pointer pointer_type);
GUI_Setup           GUI_LoadSetupDefault();

GUI_Icons*          GUI_GetIcons();
float               GUI_GetIconWidth();
float               GUI_GetIconWidthForShape(Rectangle shape, float border);
float               GUI_GetIconSmallWidth();

// > IMPLEMENTATION
#ifdef IMPLEMENT_ALL

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

GUI_IconSetup GUI_MakeIconSetup(GUI_Icons icons)
{
    GUI_IconSetup setup = {
        .icon_size      = 32,
        .icon_size_sm   = 16,
        .icon_delta     = (Vector2){ 0, 0 },
        .icons          = icons
    };
    return setup;
}

// Generates a color with a specified hue and interpolated saturation and value.
// Parameters:
//   hue: Hue value in degrees (e.g., 180.0f for cyan).
//   intensity: Interpolation factor [0.0, 1.25] controlling saturation and value .
// Returns:
//   A Raylib Color struct with RGB values (0-255) and full alpha (255).
Color GUI_GenerateThemeColor(float hue, float intensity) {
    // Normalize hue
    hue = fmodf(hue, 360.0f);
    if (hue < 0.0f) hue += 360.0f;

    // Clamp intensity
    if (intensity < 0.0f)  intensity = 0.0f;
    if (intensity > 1.25f) intensity = 1.25f;

    // Interpolation
    float saturation    = 0.13f + intensity * (0.24f - 0.13f); // 0.13 → 0.24
    float value         = 0.89f - intensity * (0.89f - 0.33f); // 0.89 → 0.33

    return ColorFromHSV(hue, saturation, value);
}


GUI_ThemeColors GUI_GenerateThemeColors(float hue)
{
    GUI_ThemeColors colors = {
        .tx_color_0 = GUI_GenerateThemeColor(hue, 0.0f),
        .tx_color_1 = GUI_GenerateThemeColor(hue, 1.15f), // darker than bg_color_3
        .bg_color_0 = GUI_GenerateThemeColor(hue, 0.25f),
        .bg_color_1 = GUI_GenerateThemeColor(hue, 0.5f),
        .bg_color_2 = GUI_GenerateThemeColor(hue, 0.75f),
        .bg_color_3 = GUI_GenerateThemeColor(hue, 1.0f)
    };
    return colors;
}

GUI_Theme GUI_GenerateTheme()
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
        .gray           = GUI_GenerateThemeColors(180.0f),
        .red            = GUI_GenerateThemeColors(3.0f),
        .green          = GUI_GenerateThemeColors(97.0f),
        .bg_alpha       = 1.0f,
        .color_change   = 0.05f
    };

    return theme;
}


GUI_FontSetup GUI_LoadFontSetupDefault(EGUI_FontType content)
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

GUI_PointerSetup GUI_LoadPointerSetupForType(EGUI_Pointer pointer_type)
{
    GUI_PointerSetup setup = { 0 };
    _Static_assert(EGUI_Pointer_Count  == 5,  "Update pointers here!");

    switch (pointer_type)
    {
        case EGUI_Pointer_AGS:
            setup.pointer_texture           = LoadTexture("ico/cursor.png");
            setup.pointer_delta_normalized  = (Vector2){ 0.5f, 0.5f };
            setup.pointer_scale             = 2.0f;
            setup.pointer_alpha             = 1.0f;
            setup.trail_delta_normalized    = (Vector2){ 1.0f, 1.0f };
            setup.additional                = EGUI_Pointer_None;
            break;

        case EGUI_Pointer_Text:
            setup.pointer_texture           = LoadTexture("ico/pointer_txt.png");
            setup.pointer_delta_normalized  = (Vector2){ -1.0f, 0.0f };
            setup.pointer_scale             = 2.0f;
            setup.pointer_alpha             = 0.85f;
            setup.trail_delta_normalized    = (Vector2){ 1.0f, 1.0f };
            setup.additional                = EGUI_Pointer_Default;
            break;

        case EGUI_Pointer_Resize:
            setup.pointer_texture           = LoadTexture("ico/pointer_resize.png");
            setup.pointer_delta_normalized  = (Vector2){ 0.0f, 0.0f };
            setup.pointer_scale             = 2.0f;
            setup.pointer_alpha             = 1.0f;
            setup.trail_delta_normalized    = (Vector2){ 1.0f, 1.0f };
            setup.additional                = EGUI_Pointer_None;
            break;

        case EGUI_Pointer_Default:
        default:
            setup.pointer_texture           = LoadTexture("ico/pointer.png");
            setup.pointer_delta_normalized  = (Vector2){ 0.0f, 0.0f };
            setup.pointer_scale             = 2.0f;
            setup.pointer_alpha             = 1.0f;
            setup.trail_delta_normalized    = (Vector2){ 1.0f, 1.0f };
            setup.additional                = EGUI_Pointer_None;
            break;

    }

    return setup;
}

GUI_Setup GUI_LoadSetupDefault()
{
    GUI_Setup setup = {
        .theme      = GUI_GenerateTheme(),
        .icon_setup = GUI_MakeIconSetup(GUI_LoadIcons())
    };

    // Fonts
    for (int i = 0; i < EGUI_FontType_Count; i++) {
        setup.font_setups[i] = GUI_LoadFontSetupDefault((EGUI_FontType)i);
    }

    // Pointers
    // 0 is None
    for (int i = 1; i < EGUI_Pointer_Count; i++) {
        setup.pointer_setups[i] = GUI_LoadPointerSetupForType((EGUI_Pointer)i);
    }
    return setup;
}

GUI_Icons* GUI_GetIcons()
{
    return &GUI_CTX.setup->icon_setup.icons;
}

float GUI_GetIconWidth()
{
    return GUI_CTX.setup->icon_setup.icon_size * GUI_CTX.state->scale;
}

float GUI_GetIconWidthForShape(Rectangle shape, float border)
{
    return shape.height - border * 2 * GUI_CTX.state->scale;
}

float GUI_GetIconSmallWidth()
{
    return GUI_CTX.setup->icon_setup.icon_size_sm * GUI_CTX.state->scale;
}
#endif
