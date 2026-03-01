#pragma once
#ifndef UNITY_BUILD
#define IMPLEMENT_ALL   1
#include "main.h"
#endif


// > SUBMODULE: LAYOUT
// > INDEX
GUI_LayoutTemp  GUI_MakeLayoutTemp();
void            GUI_SetFontType(EGUI_Font font);
float           GUI_CalcDefaultHeightScaled(EGUI_Font font);
float           GUI_VerticalSizeOrDefault();
float           GUI_HorizontalSizeOrDefault();
void            GUI_LayoutVertical(float size);

Rectangle       GUI_NextInPlace(int horizontal, int vertical);
Rectangle       GUI_NextInPlaceBetween(int horizontal, int vertical, int end_horizontal, int end_vertical);
Rectangle       GUI_NextVertical();

void            GUI_LayoutHorizontal(float size);
Rectangle       GUI_NextHorizontal();
Rectangle       GUI_NextHorizontals(int quantity);
Rectangle       GUI_NextVerticals(int quantity);

Rectangle       GUI_LayoutAvailable(Rectangle workspace);
void            GUI_LayoutReset(Rectangle workspace);
void            GUI_LayoutAutoJump();
void            GUI_LayoutBlock(float width, float height);
void            GUI_LayoutBlockCols(float cols, Rectangle window_workspace, EGUI_Font font);
void            GUI_LayoutDuplicateBlock();

static inline Rectangle GUI_Relative(Rectangle shape);
static inline Rectangle GUI_RelativePositionOnly(Rectangle shape);

// > IMPLEMENTATION
#ifdef IMPLEMENT_ALL

GUI_LayoutTemp GUI_MakeLayoutTemp()
{
    GUI_LayoutTemp layout = {
        .current_workspace          = (Rectangle) { 0.f, 0.f, 0.f, 0.f},
        .vertical_count             = 0,
        .vertical_size              = 0.0f,
        .horizontal_count           = 0,
        .horizontal_size            = 0.0f,
        .used_height                = 0.0f,
        .current_scroll             = 0,
        .current_font               = EGUI_Font_Default,
        .current_window_idx         = GUI_NO_WIN,
        .current_window_workspace   = (Rectangle) { 0.f, 0.f, 0.f, 0.f},
        .force_overflow             = false
    };
    return layout;
}

void GUI_SetFontType(EGUI_Font font)
{
    GUI_CTX.temp->layout.current_font = font;
}

float GUI_CalcDefaultHeightScaled(EGUI_Font font)
{
    GUI_Setup* setup = GUI_CTX.setup;
    GUI_State* state = GUI_CTX.state;
    return setup->fonts[font].default_height * state->scale;
}

void GUI_LayoutVertical(float size)
{
    GUI_CTX.temp->layout.vertical_count = 0;
    GUI_CTX.temp->layout.vertical_size  = size;
}

float GUI_VerticalSizeOrDefault()
{
    return GUI_CTX.temp->layout.vertical_size != DEFAULT_SIZE ? GUI_CTX.temp->layout.vertical_size
                                                              : (float)GetScreenHeight();
}

float GUI_HorizontalSizeOrDefault()
{
    return GUI_CTX.temp->layout.horizontal_size != DEFAULT_SIZE ? GUI_CTX.temp->layout.horizontal_size
                                                                : (float)GetScreenWidth();
}

Rectangle GUI_NextInPlace(int horizontal, int vertical)
{
    float horizontal_size   = GUI_HorizontalSizeOrDefault();
    float vertical_size     = GUI_CTX.temp->layout.vertical_size;
    Rectangle result        = {
        .x      = horizontal_size * (float)(GUI_CTX.temp->layout.horizontal_count + horizontal),
        .y      = vertical_size * (float)(GUI_CTX.temp->layout.vertical_count + vertical),
        .width  = horizontal_size,
        .height = vertical_size
    };
    return result;
}

Rectangle GUI_NextInPlaceBetween(int horizontal, int vertical, int end_horizontal, int end_vertical)
{
    Rectangle begin     = GUI_NextInPlace(horizontal, vertical);
    Rectangle end       = GUI_NextInPlace(end_horizontal, end_vertical);
    Rectangle result    = {
        .x      = begin.x,
        .y      = begin.y,
        .width  = FloatAbs(end.x + end.width) - (begin.x),
        .height = FloatAbs(end.y + end.height) - (begin.y)
    };
    return result;
}

Rectangle GUI_NextVertical()
{
    Rectangle shape         = GUI_NextInPlace(0, 0);
    float vertical_size     = GUI_CTX.temp->layout.vertical_size;

    GUI_CTX.temp->layout.used_height += vertical_size;
    GUI_CTX.temp->layout.vertical_count++;
    return shape;
}

void GUI_LayoutHorizontal(float size)
{
    GUI_CTX.temp->layout.horizontal_count = 0;
    GUI_CTX.temp->layout.horizontal_size = size;
}

Rectangle GUI_NextHorizontal()
{
    Rectangle shape = GUI_NextInPlace(0, 0);
    GUI_CTX.temp->layout.horizontal_count++;
    return shape;
}

Rectangle GUI_NextHorizontals(int quantity)
{
    Assert(quantity > 1);

    // Push value for next element
    Rectangle first = GUI_NextHorizontal();
    Rectangle last = {0};
    for (int i = 1; i < quantity; ++i) {
        last = GUI_NextHorizontal();
    }

    Rectangle result = {
        .x      = first.x,
        .y      = first.y,
        .width  = first.width + last.width,
        .height = first.height
    };
    return result;
}

Rectangle GUI_NextVerticals(int quantity)
{
    Assert(quantity > 1);

    // Push value for next element
    Rectangle first = GUI_NextVertical();
    Rectangle last = {0};
    for (int i = 1; i < quantity; ++i) {
        last = GUI_NextVertical();
    }

    Rectangle result = {
        .x      = first.x,
        .y      = first.y,
        .width  = first.width,
        .height = first.height + last.height
    };
    return result;
}
Rectangle GUI_LayoutAvailable(Rectangle workspace)
{
    float used_w = GUI_CTX.temp->layout.horizontal_size * (float)GUI_CTX.temp->layout.horizontal_count;
    float used_h = GUI_CTX.temp->layout.vertical_size   * (float)GUI_CTX.temp->layout.vertical_count;
    Rectangle result = {
        .x      = workspace.x + used_w,
        .y      = workspace.y + used_h,
        .width  = workspace.width - used_w,
        .height = workspace.height - used_h
    };

    // Vertical scroll
    if (result.height < GUI_CTX.temp->layout.vertical_size)
        result.height = GUI_CTX.temp->layout.vertical_size;
    return result;
}
void GUI_LayoutReset(Rectangle workspace)
{
    GUI_CTX.temp->layout = GUI_MakeLayoutTemp();
    GUI_CTX.temp->layout.current_workspace      = workspace;
}
void GUI_LayoutAutoJump()
{
    bool used_space = GUI_CTX.temp->layout.horizontal_count > 0 && GUI_CTX.temp->layout.vertical_count == 0;
    if (used_space) {
        GUI_NextVertical();
    }
}
void GUI_LayoutBlock(float width, float height)
{
    GUI_LayoutAutoJump();

    // Horizontal
    if (width > 0.0) {
        GUI_LayoutHorizontal(width);
    } else if (width < 0.0) {
        // width is already negative
        // so this takes available space minus width
        GUI_LayoutHorizontal(GUI_CTX.temp->layout.current_workspace.width + width);
    } else {
        GUI_LayoutHorizontal(GUI_CTX.temp->layout.current_workspace.width);
    }

    // Adjust to get y-available space
    if (GUI_CTX.temp->layout.vertical_count != 0) {
        GUI_CTX.temp->layout.current_workspace = GUI_LayoutAvailable(GUI_CTX.temp->layout.current_workspace);
    }

    // Vertical
    if (height > 0.0) {
        GUI_LayoutVertical(height);
    } else if (height < 0.0) {
        // height is already negative
        // so this takes available space minus height
        float available  = GUI_CTX.temp->layout.current_workspace.height + height;
        if (available > 0) {
            GUI_LayoutVertical(available);
        } else {
            GUI_LayoutVertical(height * -1);
        }
    } else {
        GUI_LayoutVertical(GUI_CTX.temp->layout.current_workspace.height);
    }
}

void GUI_LayoutBlockCols(float cols, Rectangle window_workspace, EGUI_Font font)
{
    float default_height = GUI_CalcDefaultHeightScaled(font);
    GUI_LayoutBlock(window_workspace.width / cols, default_height);
    GUI_SetFontType(font);
}

void GUI_LayoutDuplicateBlock()
{
    GUI_LayoutBlock(GUI_CTX.temp->layout.horizontal_size, GUI_CTX.temp->layout.vertical_size);
}

static inline Rectangle GUI_Relative(Rectangle shape)
{
    bool is_active_layout = GUI_CTX.temp->layout.current_workspace.width  > 0 &&
                            GUI_CTX.temp->layout.current_workspace.height > 0;
    if (is_active_layout) {
        shape = RelativeToRect(shape, GUI_CTX.temp->layout.current_workspace);
    }
    return shape;
}

static inline Rectangle GUI_RelativePositionOnly(Rectangle shape)
{
    Rectangle shape_relative    = GUI_Relative(shape);
    // Keep dimensions
    shape_relative.width        = shape.width;
    shape_relative.height       = shape.height;
    return shape_relative;
}
#endif
