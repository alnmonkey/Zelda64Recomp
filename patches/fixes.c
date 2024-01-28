#include "patches.h"
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"

#define PAGE_BG_WIDTH (PAGE_BG_COLS * PAGE_BG_QUAD_WIDTH)
#define PAGE_BG_HEIGHT (PAGE_BG_ROWS * PAGE_BG_QUAD_HEIGHT)

#define RECOMP_PAGE_ROW_HEIGHT 14
#define RECOMP_PAGE_ROW_COUNT ((PAGE_BG_HEIGHT + RECOMP_PAGE_ROW_HEIGHT - 1) / RECOMP_PAGE_ROW_HEIGHT)

extern s16* sVtxPageQuadsX[VTX_PAGE_MAX];
extern s16* sVtxPageQuadsWidth[VTX_PAGE_MAX];
extern s16* sVtxPageQuadsY[VTX_PAGE_MAX];
extern s16* sVtxPageQuadsHeight[VTX_PAGE_MAX];

s16 sVtxPageGameOverSaveQuadsY[VTX_PAGE_SAVE_QUADS] = {
    14,  // promptPageVtx[60] QUAD_PROMPT_MESSAGE
    -2,  // promptPageVtx[64] QUAD_PROMPT_CURSOR_LEFT
    -2,  // promptPageVtx[68] QUAD_PROMPT_CURSOR_RIGHT
    -18, // promptPageVtx[72] QUAD_PROMPT_CHOICE_YES
    -18, // promptPageVtx[76] QUAD_PROMPT_CHOICE_NO
};

// @recomp patched to draw as strips with bilerp compensation instead of tiles.
s16 KaleidoScope_SetPageVertices(PlayState* play, Vtx* vtx, s16 vtxPage, s16 numQuads) {
    PauseContext* pauseCtx = &play->pauseCtx;
    GameOverContext* gameOverCtx = &play->gameOverCtx;
    s16* quadsX;
    s16* quadsWidth;
    s16* quadsY;
    s16* quadsHeight;
    s32 cur_y;
    u32 row;

    gSegments[0x0D] = OS_K0_TO_PHYSICAL(play->pauseCtx.iconItemLangSegment);

    cur_y = PAGE_BG_HEIGHT / 2;

    // 2 verts per row plus 2 extra verts at the start and the end.
    for (row = 0; row < RECOMP_PAGE_ROW_COUNT + 2; row++) {
        s32 next_y = MAX(cur_y - RECOMP_PAGE_ROW_HEIGHT, -PAGE_BG_HEIGHT / 2);

        vtx[4 * row + 0].v.ob[0] = -PAGE_BG_WIDTH / 2;
        vtx[4 * row + 1].v.ob[0] =  PAGE_BG_WIDTH / 2;
        vtx[4 * row + 2].v.ob[0] = -PAGE_BG_WIDTH / 2;
        vtx[4 * row + 3].v.ob[0] =  PAGE_BG_WIDTH / 2;
        
        vtx[4 * row + 0].v.ob[1] = cur_y + pauseCtx->offsetY;
        vtx[4 * row + 1].v.ob[1] = cur_y + pauseCtx->offsetY;
        vtx[4 * row + 2].v.ob[1] = next_y + pauseCtx->offsetY;
        vtx[4 * row + 3].v.ob[1] = next_y + pauseCtx->offsetY;

        vtx[4 * row + 0].v.ob[2] = vtx[4 * row + 1].v.ob[2] = vtx[4 * row + 2].v.ob[2] = vtx[4 * row + 3].v.ob[2] = 0;

        vtx[4 * row + 0].v.flag = vtx[4 * row + 1].v.flag = vtx[4 * row + 2].v.flag = vtx[4 * row + 3].v.flag = 0;

        #define PIXEL_OFFSET ((1 << 4))

        vtx[4 * row + 0].v.tc[0] = PIXEL_OFFSET;
        vtx[4 * row + 0].v.tc[1] = (1 << 5) + PIXEL_OFFSET;
        vtx[4 * row + 1].v.tc[0] = PAGE_BG_WIDTH * (1 << 5) + PIXEL_OFFSET;
        vtx[4 * row + 1].v.tc[1] = (1 << 5) + PIXEL_OFFSET;
        vtx[4 * row + 2].v.tc[0] = PIXEL_OFFSET;
        vtx[4 * row + 2].v.tc[1] = (cur_y - next_y + 1) * (1 << 5) + PIXEL_OFFSET;
        vtx[4 * row + 3].v.tc[0] = PAGE_BG_WIDTH * (1 << 5) + PIXEL_OFFSET;
        vtx[4 * row + 3].v.tc[1] = (cur_y - next_y + 1) * (1 << 5) + PIXEL_OFFSET;

        vtx[4 * row + 0].v.cn[0] = vtx[4 * row + 1].v.cn[0] = vtx[4 * row + 2].v.cn[0] = vtx[4 * row + 3].v.cn[0] = 0;
        vtx[4 * row + 0].v.cn[1] = vtx[4 * row + 1].v.cn[1] = vtx[4 * row + 2].v.cn[1] = vtx[4 * row + 3].v.cn[1] = 0;
        vtx[4 * row + 0].v.cn[2] = vtx[4 * row + 1].v.cn[2] = vtx[4 * row + 2].v.cn[2] = vtx[4 * row + 3].v.cn[2] = 0;
        vtx[4 * row + 0].v.cn[3] = vtx[4 * row + 1].v.cn[3] = vtx[4 * row + 2].v.cn[3] = vtx[4 * row + 3].v.cn[3] = pauseCtx->alpha;

        cur_y = next_y;
    }
    
    // These are overlay symbols, so their addresses need to be offset to get their actual loaded vram address.
    // TODO remove this once the recompiler is able to handle overlay symbols automatically for patch functions.
    s16** sVtxPageQuadsXRelocated =      (s16**)((u8*)&sVtxPageQuadsX[0]      + gKaleidoMgrOverlayTable[0].offset);
    s16** sVtxPageQuadsWidthRelocated =  (s16**)((u8*)&sVtxPageQuadsWidth[0]  + gKaleidoMgrOverlayTable[0].offset);
    s16** sVtxPageQuadsYRelocated =      (s16**)((u8*)&sVtxPageQuadsY[0]      + gKaleidoMgrOverlayTable[0].offset);
    s16** sVtxPageQuadsHeightRelocated = (s16**)((u8*)&sVtxPageQuadsHeight[0] + gKaleidoMgrOverlayTable[0].offset);
    
    s16 k = 60;

    if (numQuads != 0) {
        quadsX = sVtxPageQuadsXRelocated[vtxPage];
        quadsWidth = sVtxPageQuadsWidthRelocated[vtxPage];
        quadsY = sVtxPageQuadsYRelocated[vtxPage];
        quadsHeight = sVtxPageQuadsHeightRelocated[vtxPage];
        s16 i;

        for (i = 0; i < numQuads; i++, k += 4) {
            vtx[k + 2].v.ob[0] = vtx[k + 0].v.ob[0] = quadsX[i];

            vtx[k + 1].v.ob[0] = vtx[k + 3].v.ob[0] = vtx[k + 0].v.ob[0] + quadsWidth[i];

            if (!IS_PAUSE_STATE_GAMEOVER) {
                vtx[k + 0].v.ob[1] = vtx[k + 1].v.ob[1] = quadsY[i] + pauseCtx->offsetY;
            } else if (gameOverCtx->state == GAMEOVER_INACTIVE) {
                vtx[k + 0].v.ob[1] = vtx[k + 1].v.ob[1] = quadsY[i] + pauseCtx->offsetY;
            } else {
                vtx[k + 0].v.ob[1] = vtx[k + 1].v.ob[1] = sVtxPageGameOverSaveQuadsY[i] + pauseCtx->offsetY;
            }

            vtx[k + 2].v.ob[1] = vtx[k + 3].v.ob[1] = vtx[k + 0].v.ob[1] - quadsHeight[i];

            vtx[k + 0].v.ob[2] = vtx[k + 1].v.ob[2] = vtx[k + 2].v.ob[2] = vtx[k + 3].v.ob[2] = 0;

            vtx[k + 0].v.flag = vtx[k + 1].v.flag = vtx[k + 2].v.flag = vtx[k + 3].v.flag = 0;

            vtx[k + 0].v.tc[0] = vtx[k + 0].v.tc[1] = vtx[k + 1].v.tc[1] = vtx[k + 2].v.tc[0] = 0;
            vtx[k + 1].v.tc[0] = vtx[k + 3].v.tc[0] = quadsWidth[i] << 5;
            vtx[k + 2].v.tc[1] = vtx[k + 3].v.tc[1] = quadsHeight[i] << 5;

            vtx[k + 0].v.cn[0] = vtx[k + 2].v.cn[0] = vtx[k + 0].v.cn[1] = vtx[k + 2].v.cn[1] = vtx[k + 0].v.cn[2] =
                vtx[k + 2].v.cn[2] = 255;

            vtx[k + 1].v.cn[0] = vtx[k + 3].v.cn[0] = vtx[k + 1].v.cn[1] = vtx[k + 3].v.cn[1] = vtx[k + 1].v.cn[2] =
                vtx[k + 3].v.cn[2] = 255;

            vtx[k + 0].v.cn[3] = vtx[k + 2].v.cn[3] = vtx[k + 1].v.cn[3] = vtx[k + 3].v.cn[3] = pauseCtx->alpha;
        }
    }
    return k;
}

// There's one extra row and column of padding on each side, so the size is +2 in each dimension.
typedef u8 bg_image_t[(2 + PAGE_BG_WIDTH) * (2 + PAGE_BG_HEIGHT)];

#define BG_IMAGE_COUNT 4
TexturePtr* bg_pointers[BG_IMAGE_COUNT];
bg_image_t bg_images[BG_IMAGE_COUNT];
u32 bg_image_count = 0;

void assemble_image(TexturePtr* textures, bg_image_t* image_out) {
    u8* pixels_out_start = *image_out;
    // Skip a row, it'll be filled in later.
    u8* pixels_out = pixels_out_start + PAGE_BG_WIDTH + 2;
    for (u32 row = 0; row < PAGE_BG_ROWS; row++) {
        u8* texture0 = Lib_SegmentedToVirtual(textures[row + 0]);
        u8* texture1 = Lib_SegmentedToVirtual(textures[row + 5]);
        u8* texture2 = Lib_SegmentedToVirtual(textures[row + 10]);
        for (u32 tile_row = 0; tile_row < PAGE_BG_QUAD_HEIGHT; tile_row++) {
            // Write the first column, setting alpha to 0.
            *pixels_out = (*texture0) & 0xF0;
            pixels_out++;
            // Copy a row from each of the tiles into the output texture.
            Lib_MemCpy(pixels_out, texture0, PAGE_BG_QUAD_WIDTH * sizeof(u8));
            pixels_out += PAGE_BG_QUAD_WIDTH;
            texture0 += PAGE_BG_QUAD_WIDTH;
            Lib_MemCpy(pixels_out, texture1, PAGE_BG_QUAD_WIDTH * sizeof(u8));
            pixels_out += PAGE_BG_QUAD_WIDTH;
            texture1 += PAGE_BG_QUAD_WIDTH;
            Lib_MemCpy(pixels_out, texture2, PAGE_BG_QUAD_WIDTH * sizeof(u8));
            pixels_out += PAGE_BG_QUAD_WIDTH;
            texture2 += PAGE_BG_QUAD_WIDTH;
            // Write the last column, setting alpha to 0.
            *pixels_out = (*(texture2 - 1)) & 0xF0;
            pixels_out++;
        }
    }
    // Fill in the padding rows with duplicates of the first and last row but with zero alpha.
    for (u32 col = 0; col < PAGE_BG_WIDTH + 2; col++) {
        pixels_out_start[col] = pixels_out_start[col + PAGE_BG_WIDTH + 2] & 0xF0;
        pixels_out[col] = pixels_out[col - PAGE_BG_WIDTH - 2] & 0xF0;
    }
}

// @recomp patched to fix bilerp seams.
Gfx* KaleidoScope_DrawPageSections(Gfx* gfx, Vtx* vertices, TexturePtr* textures) {
    s32 i;
    s32 j;

    bg_image_t* cur_image = NULL;

    // Check if this texture set has already been assembled into an image.
    u32 image_index;
    for (image_index = 0; image_index < bg_image_count; image_index++) {
        if (bg_pointers[image_index] == textures) {
            cur_image = &bg_images[image_index];
        }
    }

    // If no image was found and there's a free image slot, assemble the image.
    if (cur_image == NULL && image_index < BG_IMAGE_COUNT) {
        assemble_image(textures, &bg_images[image_index]);
        bg_pointers[image_index] = textures;
        cur_image = &bg_images[image_index];
        bg_image_count++;
    }

    if (cur_image == NULL) {
        // No image was found and there are no free slots.
        return gfx;
    }

    // Draw the rows.
    for (u32 bg_row = 0; bg_row < RECOMP_PAGE_ROW_COUNT; bg_row++) {
        gDPLoadTextureTile(gfx++, *cur_image,
            G_IM_FMT_IA, G_IM_SIZ_8b, // fmt, siz
            PAGE_BG_WIDTH + 2, PAGE_BG_HEIGHT + 2, // width, height
            0, (bg_row + 0) * RECOMP_PAGE_ROW_HEIGHT, // uls, ult
            PAGE_BG_WIDTH + 2, (bg_row + 1) * RECOMP_PAGE_ROW_HEIGHT + 2, // lrs, lrt
            0, // pal
            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP,
            G_TX_NOMASK, G_TX_NOMASK,
            G_TX_NOLOD, G_TX_NOLOD);
        gDPSetTileSize(gfx++, G_TX_RENDERTILE,
                0 << G_TEXTURE_IMAGE_FRAC,
                0 << G_TEXTURE_IMAGE_FRAC,
                (PAGE_BG_WIDTH + 2) <<G_TEXTURE_IMAGE_FRAC,
                (RECOMP_PAGE_ROW_HEIGHT + 2) << G_TEXTURE_IMAGE_FRAC);
        gSPVertex(gfx++, vertices + 4 * bg_row, 4, 0);
        gSP2Triangles(gfx++, 0, 3, 1, 0x0, 3, 0, 2, 0x0);
    }

    return gfx;
}
