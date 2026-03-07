#ifndef NON_EDITOR_BUILD
 #define NON_EDITOR_BUILD 0
 #include "common.h"
#endif

#ifndef WALLPAPER_LOGICAL_W
#define WALLPAPER_LOGICAL_W 320
#endif
#ifndef WALLPAPER_LOGICAL_H
#define WALLPAPER_LOGICAL_H 240
#endif

typedef struct {
    int w, h;
    int variant;      // 0: island, 1: portal, 2: nebula
    uint32_t seed;    // for deterministic stars
    bool only_stars;
    RenderTexture2D rt;
} Wallpaper;

// ---------- Palette (subset from your list; añade si quieres) ----------
static inline Color Hex(uint32_t x){
    return (Color){ (unsigned char)((x>>16)&0xFF),
                    (unsigned char)((x>>8 )&0xFF),
                    (unsigned char)((x    )&0xFF), 255 };
}
enum { P0, P1, P2, P3, P4, P5, P6, PWHITE, PA0, PA1, PA2, PA3, PV1, PV2, PV3, PV4, MINT, TEAL, TEAL_D, TEAL_D2, TEAL_D3, TEAL_D4, DEEP, LASTC };
static Color PALETTE[] = {
    {0x00, 0x00, 0x00, 255}, // #000000
    {0x12, 0x17, 0x3d, 255}, // #12173d
    {0x29, 0x32, 0x68, 255}, // #293268
    {0x46, 0x4b, 0x8c, 255}, // #464b8c
    {0x6b, 0x74, 0xb2, 255}, // #6b74b2
    {0x90, 0x9e, 0xdd, 255}, // #909edd
    {0xc1, 0xd9, 0xf2, 255}, // #c1d9f2
    {0xff, 0xff, 0xff, 255}, // #ffffff
    {0xa2, 0x93, 0xc4, 255}, // #a293c4
    {0x7b, 0x6a, 0xa5, 255}, // #7b6aa5
    {0x53, 0x42, 0x7f, 255}, // #53427f
    {0x3c, 0x2c, 0x68, 255}, // #3c2c68
    {0x43, 0x1e, 0x66, 255}, // #431e66
    {0x5d, 0x2f, 0x8c, 255}, // #5d2f8c
    {0x85, 0x4c, 0xbf, 255}, // #854cbf
    {0xb4, 0x83, 0xef, 255}, // #b483ef
    {0x8c, 0xff, 0x9b, 255}, // #8cff9b
    {0x42, 0xbc, 0x7f, 255}, // #42bc7f
    {0x22, 0x89, 0x6e, 255}, // #22896e
    {0x14, 0x66, 0x5b, 255}, // #14665b
    {0x0f, 0x4a, 0x4c, 255}, // #0f4a4c
    {0x0a, 0x2a, 0x33, 255}, // #0a2a33
    {0x1d, 0x1a, 0x59, 255}, // #1d1a59
};
#define C(k) PALETTE[(k)]

// ---------- tiny RNG ----------
static inline uint32_t xorshift(uint32_t *s){ uint32_t x=*s; x^=x<<13; x^=x>>17; x^=x<<5; *s=x; return x; }
static inline float rf(uint32_t *s){ return (xorshift(s) & 0xFFFFFF)/16777215.0f; }

// ---------- helpers ----------
static inline void px(int x,int y, Color c){ DrawRectangle(x,y,1,1,c); }
static inline void pxf(int x,int y, float a, Color c){ Color k=c; k.a=(unsigned char)(k.a*a); DrawRectangle(x,y,1,1,k); }

static void DitherCircle(int cx,int cy,int r, Color inner, Color outer){
    for(int y=-r;y<=r;y++){
        for(int x=-r;x<=r;x++){
            int d2 = x*x+y*y;
            if(d2<=r*r){
                float t = (float)d2/(float)(r*r);
                // simple 2-tone dither: more inner near center
                Color col = (t<0.6f)? inner : ((t<0.9f)? (Color){(unsigned char)((inner.r+outer.r)/2),(unsigned char)((inner.g+outer.g)/2),(unsigned char)((inner.b+outer.b)/2),255} : outer);
                px(cx+x, cy+y, col);
            }
        }
    }
}

static void Starfield(uint32_t seed, float t, int layers){
    for(int L=0; L<layers; ++L){
        uint32_t s = seed + 0x9E3779B9u*L;
        int count = (L==0)? 140 : (L==1? 90: 50);
        float par = (L==0)? 0.2f : (L==1? 0.6f : 1.0f);
        for(int i=0;i<count;i++){
            int x = (int)(rf(&s)*WALLPAPER_LOGICAL_W);
            int y = (int)(rf(&s)*WALLPAPER_LOGICAL_H);
            int twinkle = (int)fmodf((float)i + t* (2.0f + L), 4.0f);
            Color c = (twinkle==0)? C(P6) : (twinkle==1? C(P5) : (twinkle==2? C(P4):C(P3)));
            // slight parallax drift on X
            int xd = (int)(sinf((x*0.05f + t*0.6f*par))*1.5f);
            px(x+xd, y, c);
            if(rf(&s)>0.93f) px(x+xd+1,y, C(PWHITE)); // sparkle
            if(rf(&s)>0.97f) px(x+xd, y+1, C(16));    // mint pop
        }
    }
}

static void NebulaBands(float t){
    // layered soft bands with dither circles
    int cx1 = 60, cy1 = 70;  int r1 = 80;
    int cx2 = 230, cy2 = 150; int r2 = 90;
    DitherCircle(cx1, cy1, r1, C(9),  C(2));
    DitherCircle(cx2, cy2, r2, C(14), C(3));
    // moving wisps
    for(int i=0;i<4;i++){
        int cx = 40 + i*70 + (int)(sinf(t*0.3f + i)*8.0f);
        int cy = 40 + (i%2)*60 + (int)(cosf(t*0.25f + i)*6.0f);
        int rr = 24 + (i%3)*6;
        DitherCircle(cx, cy, rr, C(15), C(4));
    }
}

static void FloatingIsland(void){
    // island body (simple polygon)
    Vector2 pts[] = {
        {140,130},{180,120},{205,130},{195,150},{175,165},{160,170},{150,165},{135,150}
    };
    DrawTriangle(pts[0],pts[1],pts[2], C(11));
    DrawTriangle(pts[0],pts[2],pts[3], C(12));
    DrawTriangle(pts[0],pts[3],pts[4], C(10));
    DrawTriangle(pts[0],pts[4],pts[5], C(10));
    DrawTriangle(pts[0],pts[5],pts[6], C(11));
    DrawTriangle(pts[0],pts[6],pts[7], C(12));

    // grass top
    DrawRectangle(140,118,70,6, C(17));
    for(int x=140;x<210;x+=2) px(x,117, C(16));

    // tree trunk
    for(int y=90;y<118;y++) DrawRectangle(174,y,4,1, C(8));
    // canopy (blobby circle + highlights)
    DitherCircle(176,88,18, C(17), C(18));
    DitherCircle(170,80,10, C(16), C(17));
}

static void Portal(float t){
    int cx=240, cy=120;
    int R=36;
    // outer glow
    DitherCircle(cx, cy, R+8, C(5), C(2));
    // ring
    for(int r=R-1;r<=R+1;r++){
        for(int a=0;a<360;a++){
            float rad = a*DEG2RAD;
            int x = cx + (int)(cosf(rad)*r);
            int y = cy + (int)(sinf(rad)*r);
            px(x,y, C(7));
        }
    }
    // scanlines shimmer
    for(int y=-R+1;y<R;y+=2){
        float phase = sinf(t*2.0f + y*0.2f);
        int span = (int)(phase* (R-2));
        for(int x=-span;x<=span;x++){
            int d2=x*x+y*y; if(d2<(R-2)*(R-2)) pxf(cx+x, cy+y, 0.7f, C(6));
        }
    }
}

// ---------- API ----------
Wallpaper Make_Wallpaper(int variant, uint32_t seed, bool only_stars)
{
    Wallpaper wp = {
        .w          = WALLPAPER_LOGICAL_W,
        .h          = WALLPAPER_LOGICAL_W,
        .variant    = variant,
        .seed       = seed ? seed : 0xC0FFEEu,
        .only_stars = only_stars
    };
    wp.rt = LoadRenderTexture(wp.w, wp.h);
    // VERY IMPORTANT for crisp upscaling
    SetTextureFilter(wp.rt.texture, TEXTURE_FILTER_POINT);
    return wp;
}

void WallpaperUnload(Wallpaper *wp)
{
    if(wp->rt.id) UnloadRenderTexture(wp->rt);
    *wp = (Wallpaper){0};
}

void WallpaperUpdate(Wallpaper *wp, float t)
{
    BeginTextureMode(wp->rt);
        if (wp->only_stars) {
            Starfield(wp->seed, t, 3);
        } else {
            // Background gradient (vertical)
            for(int y=0;y<wp->h;y++){
                float u = (float)y/(float)(wp->h-1);
                // night blue to violet
                Color a = C(P1), b = C(11);
                Color c = (Color){
                    (unsigned char)(a.r + (b.r-a.r)*u),
                    (unsigned char)(a.g + (b.g-a.g)*u),
                    (unsigned char)(a.b + (b.b-a.b)*u),
                    255
                };
                DrawRectangle(0,y,wp->w,1,c);
            }
            Starfield(wp->seed, t, 3);
            NebulaBands(t);

            if(wp->variant==0){        // island + subtle portal behind
                Portal(t*0.6f);
                FloatingIsland();
            } else if(wp->variant==1){ // portal focus
                Portal(t);
            } else {                   // dense nebula only
                // add extra wisps
                for(int i=0;i<6;i++){
                    int cx = 40 + i*45;
                    int cy = 30 + (i%3)*45;
                    DitherCircle(cx,cy,16+(i%2)*8, C(15), C(4));
                }
            }
        }
    EndTextureMode();
}

void WallpaperDraw(Wallpaper *wp, int screenW, int screenH)
{
    // compute integer scale for pixel-perfect
    int scaleX = screenW / wp->w;
    int scaleY = screenH / wp->h;
    int scale  = (scaleX<scaleY? scaleX: scaleY);
    if(scale<1) scale = 1;
    int outW = wp->w * scale;
    int outH = wp->h * scale;
    int ox = (screenW - outW)/2;
    int oy = (screenH - outH)/2;

    Rectangle src = {0,0,(float)wp->w, -(float)wp->h}; // flip Y
    Rectangle dst = {(float)ox,(float)oy,(float)outW,(float)outH};
    DrawTexturePro(wp->rt.texture, src, dst, (Vector2){0,0}, 0.0f, WHITE);
}
