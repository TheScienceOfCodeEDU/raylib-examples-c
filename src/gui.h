#ifndef UNITY_BUILD
 #include <string.h>
 #include <stdio.h>
 #include "rayext.h"
 #include "str.h"
 #include "env.h"
#endif

#define GUI_MAX_TEXTBOXES 256

#define GUI_Assert(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "GUI_Assert failed: %s, file %s, line %d\n", #cond, __FILE__, __LINE__); \
            exit(1); \
        } \
    } while (0)

// Generates a color with a specified hue and interpolated saturation and value.
// Parameters:
//   hue: Hue value in degrees (e.g., 180.0f for cyan).
//   t: Interpolation factor [0.0, 1.0] controlling saturation (0.13 to 0.24) and value (0.89 to 0.33).
// Returns:
//   A Raylib Color struct with RGB values (0-255) and full alpha (255).
Color GetThemeColorFromHue(float hue, float t) {
    // Clamp t to [0, 1]
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

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

struct {
    Texture2D New;
    Texture2D Open;
    Texture2D Save;
    Texture2D Setup;
    Texture2D Error;
} typedef GUI_Icons;

GUI_Icons GUI_LoadIcons()
{
    GUI_Icons icons = {
        .New    = LoadTexture("ico/new.png"),
        .Open   = LoadTexture("ico/open.png"),
        .Save   = LoadTexture("ico/save.png"),
        .Setup  = LoadTexture("ico/setup.png"),
        .Error  = LoadTexture("ico/error.png")
    };
    return icons;
}

struct {
    float       icon_size;
    Vector2     icon_delta; 
    GUI_Icons   icons;
} typedef GUI_IconSetup;

GUI_IconSetup GUI_MakeIconSetupDefault()
{
    GUI_IconSetup setup = {
        .icon_size  = 29,
        .icon_delta = (Vector2){0, 0},
        .icons      = GUI_LoadIcons()
    };
    return setup;
}

enum {
    EGUI_Status_Default,
    EGUI_Status_Collide,
    EGUI_Status_Focused
} typedef GUI_ElementStatus;

struct {
    Color tx_color;
    Color bg_color_0;
    Color bg_color_1;
    Color bg_color_2;
    Color bg_color_3;
} typedef GUI_ThemeColors;

GUI_ThemeColors GUI_MakeThemeColors(float hue)
{
    GUI_ThemeColors colors = {
        .tx_color   = GetThemeColorFromHue(hue, 0.0f),
        .bg_color_0 = GetThemeColorFromHue(hue, 0.25f),
        .bg_color_1 = GetThemeColorFromHue(hue, 0.5f),
        .bg_color_2 = GetThemeColorFromHue(hue, 0.75f),
        .bg_color_3 = GetThemeColorFromHue(hue, 1.0f)
    };
    return colors;
}

struct {
    Vector2 padding;        // Internal padding
    float border;           // Border thickness
    float default_height;

    GUI_ThemeColors gray;
    GUI_ThemeColors red;
    GUI_ThemeColors green;
} typedef GUI_Theme;


GUI_Theme GUI_MakeThemeDefault(unsigned char opacity)
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
        .padding        = (Vector2) { 8, 6 },
        .border         = 2.0f,
        .default_height = 32,

        // Theme colors
        .gray   = GUI_MakeThemeColors(180.0f),
        .red    = GUI_MakeThemeColors(3.0f),
        .green  = GUI_MakeThemeColors(97.0f)
    };

    return theme;
}

#define COLOR_CHANGE        0.05
#define FOCUS_AVAILABLE     -1
#define FOCUS_LOCKED        0
#define GUI_DEF_CTRFOCUS    0
#define GUI_MAX_OPEN_WINS   16

enum {
    GUI_Focus_Available,
    GUI_Focus_CanOverride,
    GUI_Focus_Granted
} typedef GUI_Focus;

// NOTE: Always that this kind of conditions are gonna be used,
//       define a function near the type instead of using it everywhere.
//       Now, we know that the order matters for this Enum.
bool FocusOverridable(GUI_Focus focus)
{
    return focus <= GUI_Focus_CanOverride;
}

struct {
    float           font_height;
    float           font_scale;
    Vector2         font_delta;             // Delta adjustement
    Font            font_custom;
    bool            font_use_custom;        // Indicates if a custom font is used
    float           font_spacing;           
    Vector2         blink_size;             // Size of the blinking cursor
    Vector2         blink_delta;            // Blink adjustment
} typedef GUI_FontSetup;

GUI_FontSetup GUI_MakeFontSetupDefault() {
    GUI_FontSetup result = {
        .font_height        = 20.f,
        .font_scale         = 2.0f,
        .font_delta         = (Vector2){ 0.f, -1.0f },
        .font_custom        = LoadFont("fnt/unifont-17.0.01.otf"),
        .font_use_custom    = 0,
        .font_spacing       = 1.0f,
        .blink_size         = (Vector2){ 2.0f, 10.0f },
        .blink_delta        = (Vector2){ 1.9f, 0.0f },
    };
    return result;
}


struct {
    Texture2D   pointer_texture;
    Vector2     pointer_delta_normalized;
    float       pointer_scale;
    float       pointer_alpha;
} typedef GUI_PointerSetup;

enum {
    EGUI_Pointer_Default,
    EGUI_Pointer_AGS,
    EGUI_Pointer_Text,
    EGUI_Pointer_Count
} typedef EGUI_Pointer;

struct {
    GUI_FontSetup       font_setup;
    GUI_PointerSetup    pointer_setups[EGUI_Pointer_Count];
    GUI_Theme           theme;
    GUI_IconSetup       icon_setup;
} typedef GUI_Setup;

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

GUI_Setup GUI_MakeSetupDefault(float opacity)
{
    GUI_Setup setup = {
        .font_setup = GUI_MakeFontSetupDefault(),
        .pointer_setups = {
            GUI_MakePointerSetupForType(EGUI_Pointer_Default),
            GUI_MakePointerSetupForType(EGUI_Pointer_AGS),
            GUI_MakePointerSetupForType(EGUI_Pointer_Text)
        },
        .theme      = GUI_MakeThemeDefault(opacity),
        .icon_setup = GUI_MakeIconSetupDefault()
    };
    return setup;
}

//
// WINDOW
//
#define MAX_WINDOW_TITLE 16
struct GUI_Window;
typedef struct GUI_Window {
    int             id;
    Rectangle       shape;
    GUI_ThemeColors colors;
    char            *title;
    Texture2D       *icon;
    void (*contents) (struct GUI_Window*, void*);
} GUI_Window;

GUI_Window GUI_MakeEmptyWindow(void)
{
    GUI_Window window = {
        .id       = 0,
        .shape    = {0},
        .colors   = {0},
        .title    = NULL,
        .icon     = NULL,
        .contents = NULL
    };
    return window;
}
typedef struct {
    float           scale;
    bool            window_focus_moving;
    int             control_focus_id;
    GUI_Focus       focus_state_current;

    EGUI_Pointer    current_pointer;
    Vector2         mouse_last;
    Vector2         mouse_current;

    float           default_height;

    GUI_Window      window_s[GUI_MAX_OPEN_WINS];
    int             force_z_index;
    int             z_index[GUI_MAX_OPEN_WINS];

    int             textbox_cursors[GUI_MAX_TEXTBOXES];
} GUI_State;


GUI_State GUI_MakeStateDefault()
{
    GUI_State state = {
        .scale                  = 1.0f,
        .window_focus_moving    = false,
        .control_focus_id       = GUI_DEF_CTRFOCUS,
        .focus_state_current    = GUI_Focus_Available,

        .current_pointer        = EGUI_Pointer_Default,
        .mouse_last             = (Vector2){ 0.0f, 0.0f },
        .mouse_current          = (Vector2){ 0.0f, 0.0f },

        .default_height         = 0.0f,
    };

    for (int i = 0; i < GUI_MAX_OPEN_WINS; i++) {
        state.window_s[i] = GUI_MakeEmptyWindow();
    }

    state.force_z_index = 0;
    memset(state.z_index, 0, sizeof(state.z_index));
    memset(state.textbox_cursors, 0, sizeof(state.textbox_cursors));
    // SetTextureFilter(state.font.texture, TEXTURE_FILTER_POINT);
    return state;
}

static struct {
    GUI_State* state;
    GUI_Setup* setup;

    // LAYOUT DATA
    // Vertical
    int    vertical_count;
    float  vertical_size;

    // Horizontal
    int    horizontal_count;
    float  horizontal_size;
} GUI_CTX = { 0 };
void GUI_SetContext(GUI_State* state, GUI_Setup* setup)
{
    GUI_CTX.setup = setup;
    GUI_CTX.state = state;
}

GUI_State* GUI_GetState()
{
    return GUI_CTX.state;
}
GUI_Setup* GUI_GetSetup()
{
    return GUI_CTX.setup;
}

GUI_PointerSetup* GUI_GetPointerSetup()
{
    GUI_Setup* setup = GUI_GetSetup();
    GUI_State* state = GUI_GetState();

    EGUI_Pointer pointer = state->current_pointer;
    return &setup->pointer_setups[pointer];
}

void GUI_DrawPointer()
{
    GUI_State* state                    = GUI_GetState();
    GUI_PointerSetup* pointer_setup     = GUI_GetPointerSetup();
    Texture pointer_texture             = pointer_setup->pointer_texture;     

    Vector2 mouse_shape = (Vector2){
        state->mouse_current.x -
            (pointer_texture.width * state->scale * pointer_setup->pointer_scale * pointer_setup->pointer_delta_normalized.x),
        state->mouse_current.y - 
            (pointer_texture.height * state->scale * pointer_setup->pointer_scale * pointer_setup->pointer_delta_normalized.y)
    };
    DrawTextureEx(pointer_texture, mouse_shape, 0, state->scale * pointer_setup->pointer_scale , ColorAlpha(WHITE, pointer_setup->pointer_alpha));
}

Font GUI_GetFont(GUI_FontSetup* setup)
{
    Font font = setup->font_use_custom ? setup->font_custom : GetFontDefault();
    return font;
}

float GUI_CalcShapeAvailableHeight(Rectangle shape)
{
    GUI_Setup* setup = GUI_GetSetup();
    GUI_State* state = GUI_GetState();
    return (shape.height - setup->theme.padding.y * 2) * state->scale;
}

float GUI_CalcDefaultScaledHeight()
{
    GUI_Setup* setup = GUI_GetSetup();
    GUI_State* state = GUI_GetState();
    return setup->theme.default_height * state->scale;
}

void GUI_DrawBorders(Rectangle shape, Color dark, Color light, float border, bool remove_corner)
{
    if (!remove_corner) {
        // Draw top border (horizontal line)    
        DrawRectangle(shape.x, shape.y, shape.width, border, dark);

        // Draw left border (vertical line)
        DrawRectangle(shape.x, shape.y, border, shape.height, dark);

        // Draw bottom border (horizontal line)
        DrawRectangle(shape.x, shape.y + shape.height - border, shape.width, border, light);

        // Draw right border (vertical line)
        DrawRectangle(shape.x + shape.width - border, shape.y, border, shape.height, light);
    } else {
        // Top border (horizontal line, leaving gaps at corners)
        DrawRectangle(shape.x + border, shape.y, shape.width - 2 * border, border, dark);

        // Left border (vertical line, leaving gaps at corners)
        DrawRectangle(shape.x, shape.y + border, border, shape.height - 2 * border, dark);

        // Bottom border
        DrawRectangle(shape.x + border, shape.y + shape.height - border, shape.width - 2 * border, border, light);

        // Right border
        DrawRectangle(shape.x + shape.width - border, shape.y + border, border, shape.height - 2 * border, light);
    }
}

void GUI_DrawAdjustedTextEx(const char* text, Vector2 position, Color tint, float scale)
{
    GUI_FontSetup* setup    = &GUI_GetSetup()->font_setup;
    Font font               = GUI_GetFont(setup);
    DrawTextEx(font, text, Vector2Add(position, Vector2Scale(setup->font_delta, setup->font_scale)), font.baseSize * setup->font_scale * scale, setup->font_spacing, tint);
}

Vector2 GUI_MeasureAdjustedText(const char* text)
{
    GUI_FontSetup* setup    = &GUI_GetSetup()->font_setup;
    GUI_State* state        = GUI_GetState();
    Font font               = GUI_GetFont(setup);
    
    Vector2 result = {
        MeasureTextEx(font, text, font.baseSize * setup->font_scale * state->scale, setup->font_spacing).x + setup->blink_delta.x * state->scale,
        MeasureTextEx(font, text, font.baseSize * setup->font_scale * state->scale, setup->font_spacing).y + setup->blink_delta.y * state->scale
    };

    GUI_Theme* theme = &GUI_GetSetup()->theme;
    return result;
}

void GUI_DrawButton(const char* text, Rectangle shape,  GUI_ElementStatus status, GUI_Theme theme, GUI_ThemeColors colors, float scale, float icon_w) 
{
    Color bg_color =    status == EGUI_Status_Focused  ? colors.bg_color_3 :
                        status == EGUI_Status_Focused  ? colors.bg_color_3 :
                        status == EGUI_Status_Collide  ? ColorBrightness(colors.bg_color_2, COLOR_CHANGE) :
                                                       colors.bg_color_2;

    Color b_color_a =   status == EGUI_Status_Focused  ? colors.bg_color_3 :
                                                       colors.bg_color_1;

    Color b_color_b =   status == EGUI_Status_Focused  ? colors.bg_color_2 :
                                                       colors.bg_color_3;

    DrawRectangleRec(shape, bg_color);
    GUI_DrawBorders(shape, b_color_a, b_color_b, theme.border * scale, false);

    
    GUI_DrawAdjustedTextEx(text, 
        (Vector2){ shape.x + icon_w + theme.padding.x + theme.border * scale, shape.y + theme.padding.y + theme.border * scale}, 
        colors.tx_color, scale);
}

void GUI_Icon(Texture2D* texture2d, Vector2 position, float height, float scale, Color tint)
{
    GUI_Setup *setup = GUI_GetSetup();

    float texture_scale = scale * (height / texture2d->height);
    float truncated = floorf(texture_scale * 100.0f) / 100.0f;
    if (DEV_DEBUG_GUI) {
        DrawRectangleRec((Rectangle) { position.x, position.y, height, height }, ORANGE);
    }
    DrawTextureEx(*texture2d, Vector2Add(position, setup->icon_setup.icon_delta), 0, texture_scale, tint);
}

float GUI_GetIconWidth(Texture2D* icon)
{
    if (icon == NULL) return 0;
    
    GUI_State *state = GUI_GetState();
    GUI_Setup *setup = GUI_GetSetup();
    return state->default_height - setup->theme.border * 2 * state->scale;
}

bool GUI_Button(const char* text, Rectangle shape, Texture2D* icon, GUI_ThemeColors colors)
{
    GUI_Setup* setup = GUI_GetSetup();
    GUI_State* state = GUI_GetState();
    GUI_Theme theme = setup->theme;
    GUI_ElementStatus status = EGUI_Status_Default;

    bool collide            = CheckCollisionPointRec(state->mouse_current, shape);
    bool moving_window      = state->window_focus_moving == 0;
    bool focusable          = collide && moving_window;
    if (focusable) {
        if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            status = EGUI_Status_Collide;
        } else {
            status = EGUI_Status_Focused;
        }
    }
    
    float icon_w = GUI_GetIconWidth(icon);
    GUI_DrawButton(text, shape, status, theme, colors, state->scale, icon_w);
    if (icon_w > 0) {
        GUI_Icon(icon, (Vector2) { shape.x + theme.border * state->scale, shape.y + theme.border * state->scale }, icon_w, 1.0f, WHITE);
    }
    return collide && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

void GUI_DrawLabel(const char* text, Rectangle shape, GUI_Theme theme, GUI_ThemeColors colors, float scale)
{
    GUI_DrawAdjustedTextEx(text, 
        (Vector2){ shape.x + theme.padding.x + theme.border * scale, shape.y + theme.padding.y + theme.border * scale}, 
        colors.tx_color, scale);
}

void GUI_Label(const char* text, Rectangle shape, GUI_ThemeColors colors)
{
    GUI_State* state = GUI_GetState();
    GUI_Setup* setup = GUI_GetSetup();
    GUI_Theme theme = setup->theme;
    GUI_DrawLabel(text, shape, theme, colors, state->scale);
}

void GUI_DrawTextBox(char* value, int *cursor, Rectangle shape,  GUI_ElementStatus status, GUI_Theme theme, GUI_ThemeColors colors, float scale, bool blink)
{
    if (status == EGUI_Status_Default) 
        DrawRectangleRec(shape, colors.bg_color_3);
    else if (status == EGUI_Status_Collide) 
        DrawRectangleRec(shape, ColorBrightness(colors.bg_color_3, COLOR_CHANGE));
    else if (status == EGUI_Status_Focused) 
        DrawRectangleRec(shape, ColorBrightness(colors.bg_color_2, -COLOR_CHANGE));
    

    if (status == EGUI_Status_Focused) 
        GUI_DrawBorders(shape, ColorBrightness(colors.bg_color_2, -COLOR_CHANGE), ColorBrightness(colors.bg_color_0, COLOR_CHANGE), theme.border * scale, false);
    else
        GUI_DrawBorders(shape, colors.bg_color_2, colors.bg_color_0, theme.border * scale, false);

    GUI_DrawAdjustedTextEx(value, 
        (Vector2){ shape.x + theme.padding.x + theme.border * scale, shape.y + theme.padding.y + theme.border * scale}, 
        colors.tx_color, scale);

    if (status == EGUI_Status_Focused && blink) {
        GUI_State* state = GUI_GetState();
        GUI_FontSetup* font_setup = &GUI_GetSetup()->font_setup;
        Vector2 text_size = GUI_MeasureAdjustedText(value);
        
        Font font = GUI_GetFont(font_setup);
        char tmp[256] = {0};
        strncpy(tmp, value, *cursor);

        text_size = GUI_MeasureAdjustedText(tmp);
        DrawRectangle(
            shape.x + theme.padding.x + text_size.x,
            shape.y + theme.padding.y, 
            font_setup->blink_size.x * font_setup->font_scale * scale,
            text_size.y, 
            ColorAlpha(colors.tx_color, 0.95));
    }    
}

void GUI_TextBox(int id, char* value, Rectangle shape, GUI_ThemeColors colors)
{
    // Data
    GUI_Setup* setup = GUI_GetSetup();
    GUI_State* state = GUI_GetState();
    GUI_Theme theme = setup->theme;

    // Blink
    const float blink_speed     = 0.5f;
    static float blink_timer    = 0.0f;
    static bool blink_state     = 0;

    // Cursor per Id
    int *cursor = &state->textbox_cursors[id % GUI_MAX_TEXTBOXES];

    // Conditions
    bool collide        = CheckCollisionPointRec(state->mouse_current, shape);
    bool interacting    = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    if (collide) {
        state->current_pointer      = EGUI_Pointer_Text;
    }

    // Focus
    bool receives_focus = collide && interacting;
    if (receives_focus && FocusOverridable(state->focus_state_current)) {        
        state->control_focus_id     = id;
        state->focus_state_current  = GUI_Focus_Granted;
        blink_state                 = 1;
        blink_timer                 = 0;

        // Locate cursor
        int textLength = StringSize(value);
        int mouse_x = state->mouse_current.x - shape.x;
        int cursor_position = 0;
        for (int i = 0; i <= textLength; i++) {
            cursor_position = i; 
            Font font = GUI_GetFont(&setup->font_setup);
            int w = GUI_MeasureAdjustedText(TextSubtext(value, 0, i)).x + theme.padding.x;
            if (mouse_x < w) break;
        }
        *cursor = cursor_position;
    }

    // Update focused control
    bool focused = state->control_focus_id == id;
    if (focused) {
        int textLength = StringSize(value);

        // Handle text input
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126 && textLength < 255) { // Printable ASCII characters
                // Move chars to the right
                for (int i = textLength; i >= *cursor; i--) {
                    value[i+1] = value[i];
                }
                value[*cursor] = (char)key;
                (*cursor)++;
                textLength++;
            }
            key = GetCharPressed();
        }

        // Erase
        if (IsKeyPressed(KEY_BACKSPACE) && *cursor > 0) {
            for (int i = *cursor - 1; i < textLength; i++) {
                value[i] = value[i+1];
            }
            (*cursor)--;
            textLength--;
        }
        if (IsKeyPressed(KEY_DELETE) && *cursor < textLength) {
            for (int i = *cursor; i < textLength; i++) {
                value[i] = value[i+1];
            }
            textLength--;
        }

        // Cursor movement
        if (IsKeyPressed(KEY_LEFT) && *cursor > 0) (*cursor)--;
        if (IsKeyPressed(KEY_RIGHT) && *cursor < textLength) (*cursor)++;
        if (IsKeyPressed(KEY_HOME)) *cursor = 0;
        if (IsKeyPressed(KEY_END))  *cursor = textLength;

        // Blink
        if (blink_state)    blink_timer += GetFrameTime();
        else                blink_timer -= GetFrameTime();

        if (blink_timer > blink_speed)  blink_state = 0;
        if (blink_timer < 0)            blink_state = 1;
    }

    GUI_ElementStatus status = focused ? EGUI_Status_Focused : 
                              collide ? EGUI_Status_Collide : 
                                        EGUI_Status_Default;

    GUI_DrawTextBox(value, cursor, shape, status, theme, colors, state->scale, blink_state);
}


//
// Check box
//
void GUI_DrawCheckBox(bool value, char *on_txt, char *off_txt, Rectangle shape,  GUI_ElementStatus status, GUI_Theme theme, GUI_FontSetup* font_setup, GUI_ThemeColors colors, float scale)
{
    Color tx = value ? colors.tx_color : colors.bg_color_0;
    Color bg = value ? colors.bg_color_3 : colors.bg_color_2;
    Color b1 = value ? colors.bg_color_2 : colors.bg_color_0;
    Color b2 = value ? colors.bg_color_0 : colors.bg_color_2;
    if (status == EGUI_Status_Default) 
        DrawRectangleRec(shape, bg);
    else if (status == EGUI_Status_Collide) 
        DrawRectangleRec(shape, ColorBrightness(bg, COLOR_CHANGE));
    else if (status == EGUI_Status_Focused) 
        DrawRectangleRec(shape, ColorBrightness(bg, -COLOR_CHANGE));
    

    if (status == EGUI_Status_Focused) 
        GUI_DrawBorders(shape, ColorBrightness(b1, -COLOR_CHANGE), ColorBrightness(b2, COLOR_CHANGE), theme.border * scale, false);
    else
        GUI_DrawBorders(shape, b1, b2, theme.border * scale, false);

    GUI_DrawAdjustedTextEx(value ? on_txt : off_txt,
        (Vector2){ shape.x + theme.padding.x + theme.border * scale, shape.y + theme.padding.y + theme.border * scale},
        tx, scale);
}

void GUI_CheckBox(int id, bool *value, char *on_txt, char *off_txt, Rectangle shape, GUI_ThemeColors colors)
{
    // Data
    GUI_Setup* setup = GUI_GetSetup();
    GUI_State* state = GUI_GetState();
    GUI_Theme theme = setup->theme;

    // Conditions
    bool collide        = CheckCollisionPointRec(state->mouse_current, shape);
    bool interacting    = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyEnterPressed();

    // Focus
    bool receives_focus = collide && interacting;
    if (receives_focus && FocusOverridable(GUI_Focus_CanOverride)) {
        state->control_focus_id       = id;
        state->focus_state_current    = GUI_Focus_Granted;
    }
    
    // Update focused control
    bool focused = state->control_focus_id == id;
    if (focused) 
    {        
        if (interacting)
        {
             *value = !(*value); // Toggle the checkbox value
        }
    }

    GUI_ElementStatus status    = focused ? EGUI_Status_Focused : 
                                  collide ? EGUI_Status_Collide : 
                                            EGUI_Status_Default;
    GUI_DrawCheckBox(*value, on_txt, off_txt, shape, status, theme, &setup->font_setup, colors, state->scale);
}


//
// LAYOUT HELPERS
//

// DEBUG:
// Add these watches:
// GUI_TrackHorizontalCount::count
// GUI_TrackVerticalCount::count

#define RESET_COUNT     0
#define ADD_COUNT       1
#define ONLY_GET_COUNT  2
#define DEFAULT_SIZE    0.0

// NOTE: Only allow stateful operations that require Begin (like a reset)
//       If an end is required, that could create hard to debug problems.
void GUI_BeginVertical(float size)
{
    GUI_CTX.vertical_count = 0;
    GUI_CTX.vertical_size  = size;
}
Rectangle GUI_NextVertical(void)
{
    float horizontal_size = GUI_CTX.horizontal_size != DEFAULT_SIZE ? GUI_CTX.horizontal_size 
                                                                    : (float)GetScreenWidth();
    float vertical_size = GUI_CTX.vertical_size;

    Rectangle shape = {
        /* X */ horizontal_size * GUI_CTX.horizontal_count,
        /* Y */ vertical_size * GUI_CTX.vertical_count++,
        /* W */ horizontal_size,
        /* H */ vertical_size
    };
    return shape;
}
float GUI_GetAvailableHorizontal(Rectangle window_workspace)
{
    return window_workspace.width - (GUI_CTX.horizontal_size * GUI_CTX.horizontal_count);
}
void GUI_BeginHorizontal(float size)
{
    GUI_CTX.horizontal_count = 0;
    GUI_CTX.horizontal_size = size;
}
Rectangle GUI_NextHorizontal(void)
{
    float vertical_size = GUI_CTX.vertical_size != DEFAULT_SIZE ? GUI_CTX.vertical_size 
                                                                : (float)GetScreenHeight();
    float horizontal_size = GUI_CTX.horizontal_size;

    Rectangle shape = { 
        /* X */ horizontal_size * GUI_CTX.horizontal_count++,
        /* Y */ vertical_size * GUI_CTX.vertical_count,
        /* W */ horizontal_size,
        /* H */ vertical_size
    };
    return shape;
}
Rectangle GUI_NextHorizontals(int quantity)
{
    GUI_Assert(quantity > 1);
    
    // Push value for next element
    Rectangle first = GUI_NextHorizontal();
    Rectangle last = {0};
    for (int i = 1; i < quantity; ++i) {
        last = GUI_NextHorizontal();
    }
    
    Rectangle result = {
        first.x,
        first.y,
        first.width + last.width,
        first.height
    };
    return result;
}
Rectangle GUI_NextVerticals(int quantity)
{
    GUI_Assert(quantity > 1);

    // Push value for next element
    Rectangle first = GUI_NextVertical();
    Rectangle last = {0};
    for (int i = 1; i < quantity; ++i) {
        last = GUI_NextVertical();
    }

    Rectangle result = {
        first.x,
        first.y,
        first.width,
        first.height + last.height
    };
    return result;
}

Rectangle GUI_WorkspaceAvailable(Rectangle workspace)
{
    float used_w = GUI_CTX.horizontal_size * GUI_CTX.horizontal_count;
    float used_h = GUI_CTX.vertical_size   * GUI_CTX.vertical_count;
    Rectangle result = {
        workspace.x + used_w,
        workspace.y + used_h,
        workspace.width - used_w,
        workspace.height - used_h
    };
    return result;
}
void GUI_ResetLayout()
{
    GUI_CTX.horizontal_count = 0;
    GUI_CTX.vertical_count   = 0;
}
void GUI_BeginBlock(float width, float height, Rectangle* workspace)
{
    // Add jump if necessary after ONLY horizontal blocks
    if (GUI_CTX.horizontal_count > 0 && GUI_CTX.vertical_count == 0) {
        GUI_NextVertical();
    }

    // Horizontal
    if (width > 0.0) {
        GUI_BeginHorizontal(width);
    } else if (width < 0.0) {
        GUI_BeginHorizontal(workspace->width + width); // width is already negative
    } else {
        GUI_BeginHorizontal(workspace->width);
    }

    // Adjust to get y-available space
    if (GUI_CTX.vertical_count != 0) {
        *workspace = GUI_WorkspaceAvailable(*workspace);
    }

    // Vertical
    if (height > 0.0) {
        GUI_BeginVertical(height);
    } else if (height < 0.0) {
        GUI_BeginVertical(workspace->height + height); // height is already negative
    } else {
        GUI_BeginVertical(workspace->height);
    }
}
void GUI_BeginDuplicateBlock(Rectangle* workspace)
{
    GUI_BeginBlock(GUI_CTX.horizontal_size, GUI_CTX.vertical_size, workspace);
}

//
// WINDOW FUNCTIONS
//

void GUI_DrawWindow(char* title, Rectangle shape, Rectangle shapeTitle,  GUI_ElementStatus status, GUI_Theme theme, GUI_FontSetup* font_setup, GUI_ThemeColors colors, float scale, bool icon, float icon_w)
{
    // Background
    DrawRectangleRec((Rectangle){shape.x + theme.border * scale, shape.y + theme.border * scale, shape.width - theme.border * scale, shape.height - 2 * theme.border * scale}, colors.bg_color_1);
    GUI_DrawBorders(shape, colors.bg_color_0, colors.bg_color_2, theme.border * scale, true);

    if (status == EGUI_Status_Default) {
        DrawRectangleRec(shapeTitle, colors.bg_color_3);
        GUI_DrawBorders(shapeTitle, colors.bg_color_2, colors.bg_color_0, theme.border * scale, false);
    } if (status == EGUI_Status_Focused) {
        DrawRectangleRec(shapeTitle, ColorBrightness(colors.bg_color_3, -COLOR_CHANGE));
        GUI_DrawBorders(shapeTitle, colors.bg_color_2, colors.bg_color_0, theme.border * scale, false);
        GUI_DrawBorders(shape, colors.bg_color_0, ColorBrightness(colors.bg_color_3,-COLOR_CHANGE), theme.border * scale, true);
    } if (status == EGUI_Status_Collide) {
        DrawRectangleRec(shapeTitle, ColorBrightness(colors.bg_color_3, COLOR_CHANGE));
        GUI_DrawBorders(shapeTitle, colors.bg_color_2, colors.bg_color_0, theme.border * scale, false);
        GUI_DrawBorders(shape, colors.bg_color_0, ColorBrightness(colors.bg_color_3, COLOR_CHANGE), theme.border * scale, true);
    }

    BeginScissorModeRect(AddRect(shapeTitle, 0, 0, -theme.border * scale, -theme.border * scale));
        GUI_DrawAdjustedTextEx(title,
            (Vector2) { shapeTitle.x + icon_w + theme.padding.x + theme.border * scale, shapeTitle.y + theme.padding.y + theme.border * scale }, 
            colors.tx_color, scale);
    EndScissorMode();
}

Rectangle GUI_WindowTitle(Rectangle shape)
{
    GUI_Setup* setup = GUI_GetSetup();
    GUI_State* state = GUI_GetState();
    GUI_Theme theme = setup->theme;

    Rectangle shapeTitle = {
        shape.x + theme.border * state->scale,
        shape.y + theme.border * state->scale,
        shape.width - (theme.border * state->scale * 2),
        state->default_height
    };
    return shapeTitle;
}

Rectangle GUI_WindowWorkspace(Rectangle shape)
{
    GUI_Setup* setup = GUI_GetSetup();
    GUI_State* state = GUI_GetState();
    GUI_Theme theme = setup->theme;

    Rectangle shape_title = GUI_WindowTitle(shape);
    Rectangle shape_workspace = {
        shape_title.x,
        shape_title.y + shape_title.height + (shape_title.y - shape.y),
        shape.width - (shape_title.x - shape.x ) * 2,
        shape.height - shape_title.height - (shape_title.y - shape.y) - theme.border * state->scale * 2
    };

    if (DEV_DEBUG_GUI) {
        DrawRectangleRec(shape_title, ColorAlpha(ORANGE, 0.5));
        DrawRectangleRec(shape_workspace, ColorAlpha(GREEN, 0.5));
    }
    return shape_workspace;
}

void GUI_UpdateAndDrawWindow(GUI_Window *window, Rectangle limits)
{
    GUI_Setup* setup = GUI_GetSetup();
    GUI_State* state = GUI_GetState();
    GUI_Theme theme = setup->theme;
    Rectangle shape_title   = GUI_WindowTitle(window->shape);

    // Conditions
    bool collide            = CheckCollisionPointRec(state->mouse_current, window->shape);
    bool collide_title      = CheckCollisionPointRec(state->mouse_current, shape_title);
    bool interaction_starts = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool window_focusable   = state->focus_state_current == GUI_Focus_Available && state->window_focus_moving == 0;
    bool window_focused     = state->z_index[0] == window->id;

    // Focus ?
    if (collide && interaction_starts && window_focusable) {
        if (window_focused) {
            state->focus_state_current  = GUI_Focus_CanOverride;
            state->window_focus_moving  = collide_title;
        }
    }
    

    // Active
    if (window_focused){
        // Movement
        bool interacting        = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        bool moving             = interacting && state->window_focus_moving;
        if (moving) {
            Vector2 mouse_current_valid     = LimitVector2Rect(state->mouse_current, limits);
            Vector2 mouse_last_valid        = LimitVector2Rect(state->mouse_last, limits);
            Vector2 displacement            = Vector2Subtract(mouse_current_valid, mouse_last_valid);
            
            window->shape.x += displacement.x;
            window->shape.y += displacement.y;
        } else {
            state->window_focus_moving = false;
        }
    }

    // Limit
    window->shape   = LimitRect(window->shape, limits);
    shape_title     = GUI_WindowTitle(window->shape);

    // Draw
    GUI_ElementStatus status = window_focused   ? EGUI_Status_Focused :
                               collide_title    ? EGUI_Status_Collide :
                                                  EGUI_Status_Default;
    float icon_w = GUI_GetIconWidth(window->icon);
    GUI_DrawWindow(window->title, window->shape, shape_title, status, theme, &setup->font_setup, window->colors, state->scale, false, icon_w);
    if (icon_w > 0) {
        GUI_Icon(window->icon, (Vector2) { shape_title.x + theme.border * state->scale, shape_title.y + theme.border * state->scale }, icon_w, 1.0f, WHITE);
    }    
}

GUI_Window* GUI_MakeWindow(int id, char *title, Rectangle shape, GUI_ThemeColors colors, Texture2D *icon, void (*contents)(GUI_Window*, void*)) {
    GUI_State* state = GUI_GetState();

    for (int i = 0; i < GUI_MAX_OPEN_WINS; ++i) {
        GUI_Window* window = &state->window_s[i];
        if (window->id == 0) {
            window->id          = id;
            window->shape       = shape;
            window->colors      = colors;
            window->title       = title;
            window->icon        = icon;
            window->contents    = contents;
            return window;
        }
    }
    return 0;
}

GUI_Window* GUI_GetWindow(int id, GUI_State* state)
{
    for (int i = 0; i < GUI_MAX_OPEN_WINS; ++i) {
        GUI_Window* window = &state->window_s[i];
        if (window->id == id) {
            return window;
        }
    }
    return NULL;
}

void GUI_UpdateAndDrawWindows(Rectangle limits, void* win_state)
{
    GUI_State* state = GUI_GetState();

    // Remove unused window ids
    for (int j = 0; j < GUI_MAX_OPEN_WINS; ++j) {
        int id = state->z_index[j];
        if (id == 0)
            continue;

        bool found_id = false;
        for (int i = 0; i < GUI_MAX_OPEN_WINS; ++i) {
            GUI_Window* window = &state->window_s[i];
            if (window->id == id) {
                found_id = true;
                break;
            }
        }

        if (found_id == false) {
            state->z_index[j] = 0; // Clean
        }
    }
    
    // Grant that all window_s are in the z-index
    for (int i = 0; i < GUI_MAX_OPEN_WINS; i++) {
        GUI_Window* window = &state->window_s[i];
        if (window->id == 0)
            continue;

        int first_zero = -1;
        bool found_id = false;
        for (int j = 0; j < GUI_MAX_OPEN_WINS; ++j) {
            if (state->z_index[j] == window->id){
                found_id = true;
                break;
            }
            if (state->z_index[j] == 0 && first_zero < 0) {
                first_zero = j;
            }
        }
        if (found_id == false) {
            state->z_index[first_zero] = window->id;
        }
    }
    
    bool force_z_index  = state->force_z_index > 0;
    bool interacting    = !force_z_index && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    if (interacting || force_z_index) {
        // Find ID
        int interacted_id = state->force_z_index;
        int current_zindex = -1;

        // Restore state values z-index is being updated
        state->force_z_index = 0;

        for (int j = 0; j < GUI_MAX_OPEN_WINS; ++j) { 
            int id = state->z_index[j];
            if (id == 0) continue;

            GUI_Window* window = GUI_GetWindow(id, state);
            if (window == NULL) continue;

            bool find_window    = interacted_id > 0 && interacted_id == window->id;
            bool check_window   = interacted_id == 0 && CheckCollisionPointRec(state->mouse_current, window->shape);
            if (find_window || check_window) {
                interacted_id = window->id;
                current_zindex = j;
                break;
            }
        }

        if (interacted_id > 0 && current_zindex > 0) {
            // Move all elements to the right starting at current_zindex
            bool found_id = false;
            for (int j = current_zindex; j > 0; --j) {
                state->z_index[j] = state->z_index[j - 1];
            }
            
            // Add this one
            state->z_index[0] = interacted_id;
        }
    }

    // Process
    for (int j = GUI_MAX_OPEN_WINS - 1; j >= 0 ; --j) { 
        int id = state->z_index[j];
        if (id == 0) continue;

        for (int i = 0; i < GUI_MAX_OPEN_WINS; i++) {
            GUI_Window* window = &state->window_s[i];
            if (window->id != id) continue;

            GUI_UpdateAndDrawWindow(window, limits);
            window->contents(window, win_state);
        }
    }
}

Rectangle GUI_BeginWindowContents(GUI_Window* window)
{
    Rectangle window_workspace = GUI_WindowWorkspace(window->shape);
    GUI_ResetLayout();
    BeginScissorModeRect(window_workspace);
    return window_workspace;
}

void GUI_EndWindowContents()
{
    EndScissorMode();
}
