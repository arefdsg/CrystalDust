#include "global.h"
#include "bug_catching_contest.h"
#include "main.h"
#include "text.h"
#include "menu.h"
#include "malloc.h"
#include "gpu_regs.h"
#include "palette.h"
#include "party_menu.h"
#include "trig.h"
#include "constants/maps.h"
#include "overworld.h"
#include "constants/flags.h"
#include "event_data.h"
#include "secret_base.h"
#include "string_util.h"
#include "international_string_util.h"
#include "strings.h"
#include "text_window.h"
#include "constants/songs.h"
#include "m4a.h"
#include "field_effect.h"
#include "field_specials.h"
#include "fldeff.h"
#include "region_map.h"
#include "constants/region_map_sections.h"
#include "heal_location.h"
#include "constants/field_specials.h"
#include "constants/heal_locations.h"
#include "constants/map_types.h"
#include "constants/rgb.h"
#include "constants/weather.h"

/*
 *  This file handles region maps generally, and the map used when selecting a fly destination.
 *  Specific features of other region map uses are handled elsewhere
 *
 *  For the region map in the pokenav, see pokenav_region_map.c
 *  For the region map in the pokedex, see pokdex_area_screen.c/pokedex_area_region_map.c
 *  For the region map that can be viewed on the wall of pokemon centers, see field_region_map.c
 *
 */

#define MAP_WIDTH 22
#define MAP_HEIGHT 15
#define MAPCURSOR_X_MIN 4
#define MAPCURSOR_Y_MIN 4

struct WindowCoords
{
    u16 x1;
    u16 y1;
    u16 x2;
    u16 y2;
};

#define FLYDESTICON_RED_OUTLINE 6

// Static type declarations

struct MultiNameFlyDest
{
    const u8 *const *name;
    u16 mapSecId;
    u16 flag;
};

// Static RAM declarations

static EWRAM_DATA struct RegionMap *gRegionMap = NULL;

static EWRAM_DATA struct {
    void (*callback)(void);
    u16 state;
    u16 mapSecId;
    struct RegionMap regionMap;
    u8 tileBuffer[0x1c0];
    u8 nameBuffer[0x26]; // never read
    bool8 choseFlyLocation;
} *sFlyMap = NULL;

static bool32 gUnknown_03001180;

// Static ROM declarations

static u8 ProcessRegionMapInput_Full(void);
static u8 MoveRegionMapCursor_Full(void);
static u8 GetMapSecIdAt(s16 x, s16 y, u8 region, bool8 secondary);
static void RegionMap_SetBG2XAndBG2Y(s16 x, s16 y);
static void InitMapBasedOnPlayerLocation(void);
static void InitMapBasedOnPlayerLocation_(void);
static void RegionMap_InitializeStateBasedOnSSTidalLocation(void);
static u8 GetMapsecType(u16 mapSecId);
static u16 CorrectSpecialMapSecId_Internal(u16 mapSecId);
static u16 GetTerraOrMarineCaveMapSecId(void);
static void GetMarineCaveCoords(u16 *x, u16 *y);
static bool32 IsPlayerInAquaHideout(u8 mapSecId);
static void GetPositionOfCursorWithinMapSec(void);
static bool8 RegionMap_IsMapSecIdInNextRow(u16 y);
static void SpriteCB_CursorMapFull(struct Sprite *sprite);
static void HideRegionMapPlayerIcon(void);
static void UnhideRegionMapPlayerIcon(void);
static void SpriteCB_PlayerIcon(struct Sprite *sprite);
static void VBlankCB_FlyMap(void);
static void CB2_FlyMap(void);
static void SetFlyMapCallback(void callback(void));
static void ShowHelpBar(void);
static void LoadFlyDestIcons(void);
static void CreateFlyDestIcons(void);
static void TryCreateRedOutlineFlyDestIcons(void);
static void SpriteCB_FlyDestIcon(struct Sprite *sprite);
static void CB_FadeInFlyMap(void);
static void CB_HandleFlyMapInput(void);
static void CB_ExitFlyMap(void);
static void LoadPrimaryLayerMapSec(void);
static void LoadSecondaryLayerMapSec(void);
static void SetupShadowBoxes(u8 layerNum, const struct WindowCoords *coords);
static u8 GetMapSecStatusByLayer(u8 layer);
static void SetShadowBoxState(u8 offset, bool8 hide);
static const u32 *GetRegionMapTilemap(u8 region);

// .rodata
static const u16 sRegionMapCursorPal[] = INCBIN_U16("graphics/region_map/cursor.gbapal");
static const u32 sRegionMapCursorGfxLZ[] = INCBIN_U32("graphics/region_map/cursor.4bpp.lz");
static const u16 sRegionMapPal[] = INCBIN_U16("graphics/region_map/region_map.gbapal");
static const u32 sRegionMapTileset[] = INCBIN_U32("graphics/region_map/region_map.4bpp.lz");
static const u32 sRegionMapJohtoTilemap[] = INCBIN_U32("graphics/region_map/johto_map.bin.lz");
static const u32 sRegionMapKantoTilemap[] = INCBIN_U32("graphics/region_map/kanto_map.bin.lz");
static const u16 sRegionMapTownNames_Pal[] = INCBIN_U16("graphics/region_map/town_names.gbapal");
static const u16 sRegionMapPlayerIcon_GoldPal[] = INCBIN_U16("graphics/region_map/gold_icon.gbapal");
static const u32 sRegionMapPlayerIcon_GoldGfx[] = INCBIN_U32("graphics/region_map/gold_icon.4bpp");
static const u16 sRegionMapPlayerIcon_KrisPal[] = INCBIN_U16("graphics/region_map/kris_icon.gbapal");
static const u32 sRegionMapPlayerIcon_KrisGfx[] = INCBIN_U32("graphics/region_map/kris_icon.4bpp");
static const u32 sRegionMapDots_Gfx[] = INCBIN_U32("graphics/region_map/dots.4bpp");
static const u16 sRegionMapDots_Pal[] = INCBIN_U16("graphics/region_map/dots.gbapal");
static const u32 sRegionMapNames_Gfx[] = INCBIN_U32("graphics/region_map/region_names.4bpp");
static const u32 sRegionMapNamesCurve_Gfx[] = INCBIN_U32("graphics/region_map/region_names_curve.4bpp");

static const u8 sMapSectionLayout_JohtoPrimary[] = INCBIN_U8("graphics/region_map/mapsec_layout_johto_primary.bin");
static const u8 sMapSectionLayout_JohtoSecondary[] = INCBIN_U8("graphics/region_map/mapsec_layout_johto_secondary.bin");
static const u8 sMapSectionLayout_KantoPrimary[] = INCBIN_U8("graphics/region_map/mapsec_layout_kanto_primary.bin");
static const u8 sMapSectionLayout_KantoSecondary[] = INCBIN_U8("graphics/region_map/mapsec_layout_kanto_secondary.bin");

#include "data/region_map/region_map_entries.h"
#include "data/region_map/region_map_names_emerald.h"
#include "data/region_map/mapsec_to_region.h"
#include "data/region_map/mapsec_flags.h"

static const u16 sRegionMap_SpecialPlaceLocations[][2] =
{
    {MAPSEC_NONE, MAPSEC_NONE}
};

static const u16 sMarineCaveMapSecIds[] =
{
    MAPSEC_MARINE_CAVE,
    MAPSEC_UNDERWATER_MARINE_CAVE,
    MAPSEC_UNDERWATER_MARINE_CAVE
};

static const u16 sTerraOrMarineCaveMapSecIds[ABNORMAL_WEATHER_LOCATIONS] =
{
    [ABNORMAL_WEATHER_ROUTE_114_NORTH - 1] = MAPSEC_ROUTE_42,
    [ABNORMAL_WEATHER_ROUTE_114_SOUTH - 1] = MAPSEC_ROUTE_42,
    [ABNORMAL_WEATHER_ROUTE_115_WEST  - 1] = MAPSEC_ROUTE_43,
    [ABNORMAL_WEATHER_ROUTE_115_EAST  - 1] = MAPSEC_ROUTE_43,
    [ABNORMAL_WEATHER_ROUTE_116_NORTH - 1] = MAPSEC_ROUTE_44,
    [ABNORMAL_WEATHER_ROUTE_116_SOUTH - 1] = MAPSEC_ROUTE_44,
    [ABNORMAL_WEATHER_ROUTE_118_EAST  - 1] = MAPSEC_ROUTE_46,
    [ABNORMAL_WEATHER_ROUTE_118_WEST  - 1] = MAPSEC_ROUTE_46,
    [ABNORMAL_WEATHER_ROUTE_105_NORTH - 1] = MAPSEC_ROUTE_33,
    [ABNORMAL_WEATHER_ROUTE_105_SOUTH - 1] = MAPSEC_ROUTE_33,
    [ABNORMAL_WEATHER_ROUTE_125_WEST  - 1] = MAPSEC_ROUTE_125,
    [ABNORMAL_WEATHER_ROUTE_125_EAST  - 1] = MAPSEC_ROUTE_125,
    [ABNORMAL_WEATHER_ROUTE_127_NORTH - 1] = MAPSEC_ROUTE_127,
    [ABNORMAL_WEATHER_ROUTE_127_SOUTH - 1] = MAPSEC_ROUTE_127,
    [ABNORMAL_WEATHER_ROUTE_129_WEST  - 1] = MAPSEC_ROUTE_129,
    [ABNORMAL_WEATHER_ROUTE_129_EAST  - 1] = MAPSEC_ROUTE_129
};

#define MARINE_CAVE_COORD(location)(ABNORMAL_WEATHER_##location - MARINE_CAVE_LOCATIONS_START)

static const struct UCoords16 sMarineCaveLocationCoords[MARINE_CAVE_LOCATIONS] =
{
    [MARINE_CAVE_COORD(ROUTE_105_NORTH)] = {0, 10},
    [MARINE_CAVE_COORD(ROUTE_105_SOUTH)] = {0, 12},
    [MARINE_CAVE_COORD(ROUTE_125_WEST)]  = {24, 3},
    [MARINE_CAVE_COORD(ROUTE_125_EAST)]  = {25, 4},
    [MARINE_CAVE_COORD(ROUTE_127_NORTH)] = {25, 6},
    [MARINE_CAVE_COORD(ROUTE_127_SOUTH)] = {25, 7},
    [MARINE_CAVE_COORD(ROUTE_129_WEST)]  = {24, 10},
    [MARINE_CAVE_COORD(ROUTE_129_EAST)]  = {24, 10}
};

static const u8 sMapSecAquaHideoutOld[] =
{
    MAPSEC_AQUA_HIDEOUT_OLD
};

static const struct OamData sRegionMapCursorOam =
{
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
    .priority = 2
};

static const union AnimCmd sRegionMapCursorAnim1[] =
{
    ANIMCMD_FRAME(0, 20),
    ANIMCMD_FRAME(4, 20),
    ANIMCMD_JUMP(0)
};

static const union AnimCmd *const sRegionMapCursorAnimTable[] = {
    sRegionMapCursorAnim1
};

static const struct SpritePalette sRegionMapCursorSpritePalette =
{
    .data = sRegionMapCursorPal,
    .tag = 0
};

static const struct SpriteTemplate sRegionMapCursorSpriteTemplate =
{
    .tileTag = 0,
    .paletteTag = 0,
    .oam = &sRegionMapCursorOam,
    .anims = sRegionMapCursorAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_CursorMapFull
};

static const struct OamData sRegionMapPlayerIconOam =
{
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
    .priority = 2
};

static const struct OamData sRegionMapDotsOam = {
    .priority = 2
};

static const union AnimCmd sRegionMapDotsAnim1[] = {
    ANIMCMD_FRAME(0, 5),
    ANIMCMD_END
};

static const union AnimCmd sRegionMapDotsAnim2[] = {
    ANIMCMD_FRAME(1, 5),
    ANIMCMD_END
};

static const union AnimCmd *const sRegionMapDotsAnimTable[] = {
    sRegionMapDotsAnim1,
    sRegionMapDotsAnim2
};

static const struct OamData sRegionMapNameCurveOam = {
    .shape = ST_OAM_SQUARE, .size = 0, .priority = 2
};

static const struct SpriteTemplate sRegionMapNameCurveSpriteTemplate = {
    0,
    0,
    &sRegionMapNameCurveOam,
    gDummySpriteAnimTable,
    NULL,
    gDummySpriteAffineAnimTable,
    SpriteCallbackDummy
};

static const struct OamData sRegionMapNameOam = {
    .shape = ST_OAM_H_RECTANGLE, .size = 1, .priority = 2
};

static const union AnimCmd sRegionMapNameJohto[] = {
    ANIMCMD_FRAME(0, 5),
    ANIMCMD_END
};

static const union AnimCmd sRegionMapNameKanto[] = {
    ANIMCMD_FRAME(4, 5),
    ANIMCMD_END
};

static const union AnimCmd sRegionMapNameSevii[] = {
    ANIMCMD_FRAME(8, 5),
    ANIMCMD_END
};

static const union AnimCmd *const sRegionMapNameAnimTable[] = {
    sRegionMapNameJohto,
    sRegionMapNameKanto,
    sRegionMapNameSevii
};

static const struct SpriteTemplate sRegionMapNameSpriteTemplate = {
    0,
    0,
    &sRegionMapNameOam,
    sRegionMapNameAnimTable,
    NULL,
    gDummySpriteAffineAnimTable,
    SpriteCallbackDummy
};

// Event islands that don't appear on map. (Southern Island does)
static const u8 sMapSecIdsOffMap[] =
{
    MAPSEC_BIRTH_ISLAND,
    MAPSEC_FARAWAY_ISLAND,
    MAPSEC_NAVEL_ROCK
};

const u16 gRegionMapFramePal[] = INCBIN_U16("graphics/region_map/map_frame.gbapal");

const u32 gRegionMapFrameGfxLZ[] = INCBIN_U32("graphics/region_map/map_frame.4bpp.lz");

const u32 gRegionMapFrameTilemapLZ[] = INCBIN_U32("graphics/region_map/map_frame.bin.lz");

static const u32 sFlyTargetIcons_Gfx[] = INCBIN_U32("graphics/region_map/fly_target_icon.4bpp.lz");

static const u8 sMapHealLocations[][3] = {
    {MAP_GROUP(NEW_BARK_TOWN), MAP_NUM(NEW_BARK_TOWN), HEAL_LOCATION_NEW_BARK_TOWN},
    {MAP_GROUP(CHERRYGROVE_CITY), MAP_NUM(CHERRYGROVE_CITY), HEAL_LOCATION_CHERRYGROVE_CITY},
    {MAP_GROUP(AZALEA_TOWN), MAP_NUM(AZALEA_TOWN), HEAL_LOCATION_AZALEA_TOWN},
    {MAP_GROUP(LAVARIDGE_TOWN), MAP_NUM(LAVARIDGE_TOWN), HEAL_LOCATION_LAVARIDGE_TOWN},
    {MAP_GROUP(FALLARBOR_TOWN), MAP_NUM(FALLARBOR_TOWN), HEAL_LOCATION_FALLARBOR_TOWN},
    {MAP_GROUP(VERDANTURF_TOWN), MAP_NUM(VERDANTURF_TOWN), HEAL_LOCATION_VERDANTURF_TOWN},
    {MAP_GROUP(PACIFIDLOG_TOWN), MAP_NUM(PACIFIDLOG_TOWN), HEAL_LOCATION_PACIFIDLOG_TOWN},
    {MAP_GROUP(VIOLET_CITY), MAP_NUM(VIOLET_CITY), HEAL_LOCATION_VIOLET_CITY},
    {MAP_GROUP(SLATEPORT_CITY), MAP_NUM(SLATEPORT_CITY), HEAL_LOCATION_ROUTE32},
    {MAP_GROUP(MAUVILLE_CITY), MAP_NUM(MAUVILLE_CITY), HEAL_LOCATION_MAUVILLE_CITY},
    {MAP_GROUP(RUSTBORO_CITY), MAP_NUM(RUSTBORO_CITY), HEAL_LOCATION_RUSTBORO_CITY},
    {MAP_GROUP(FORTREE_CITY), MAP_NUM(FORTREE_CITY), HEAL_LOCATION_FORTREE_CITY},
    {MAP_GROUP(LILYCOVE_CITY), MAP_NUM(LILYCOVE_CITY), HEAL_LOCATION_LILYCOVE_CITY},
    {MAP_GROUP(MOSSDEEP_CITY), MAP_NUM(MOSSDEEP_CITY), HEAL_LOCATION_MOSSDEEP_CITY},
    {MAP_GROUP(SOOTOPOLIS_CITY), MAP_NUM(SOOTOPOLIS_CITY), HEAL_LOCATION_SOOTOPOLIS_CITY},
    {MAP_GROUP(EVER_GRANDE_CITY), MAP_NUM(EVER_GRANDE_CITY), HEAL_LOCATION_EVER_GRANDE_CITY},
    {MAP_GROUP(ROUTE29), MAP_NUM(ROUTE29), 0},
    {MAP_GROUP(ROUTE30), MAP_NUM(ROUTE30), 0},
    {MAP_GROUP(ROUTE31), MAP_NUM(ROUTE31), 0},
    {MAP_GROUP(ROUTE32), MAP_NUM(ROUTE32), 0},
    {MAP_GROUP(ROUTE33), MAP_NUM(ROUTE33), 0},
    {MAP_GROUP(ROUTE34), MAP_NUM(ROUTE34), 0},
    {MAP_GROUP(ROUTE107), MAP_NUM(ROUTE107), 0},
    {MAP_GROUP(ROUTE36), MAP_NUM(ROUTE36), 0},
    {MAP_GROUP(ROUTE109), MAP_NUM(ROUTE109), 0},
    {MAP_GROUP(ROUTE110), MAP_NUM(ROUTE110), 0},
    {MAP_GROUP(ROUTE111), MAP_NUM(ROUTE111), 0},
    {MAP_GROUP(ROUTE112), MAP_NUM(ROUTE112), 0},
    {MAP_GROUP(ROUTE113), MAP_NUM(ROUTE113), 0},
    {MAP_GROUP(ROUTE114), MAP_NUM(ROUTE114), 0},
    {MAP_GROUP(ROUTE115), MAP_NUM(ROUTE115), 0},
    {MAP_GROUP(ROUTE116), MAP_NUM(ROUTE116), 0},
    {MAP_GROUP(ROUTE117), MAP_NUM(ROUTE117), 0},
    {MAP_GROUP(ROUTE46), MAP_NUM(ROUTE46), 0},
    {MAP_GROUP(ROUTE119), MAP_NUM(ROUTE119), 0},
    {MAP_GROUP(ROUTE120), MAP_NUM(ROUTE120), 0},
    {MAP_GROUP(ROUTE121), MAP_NUM(ROUTE121), 0},
    {MAP_GROUP(ROUTE32), MAP_NUM(ROUTE32), HEAL_LOCATION_ROUTE32},
    {MAP_GROUP(ROUTE123), MAP_NUM(ROUTE123), 0},
    {MAP_GROUP(ROUTE124), MAP_NUM(ROUTE124), 0},
    {MAP_GROUP(ROUTE125), MAP_NUM(ROUTE125), 0},
    {MAP_GROUP(ROUTE126), MAP_NUM(ROUTE126), 0},
    {MAP_GROUP(ROUTE127), MAP_NUM(ROUTE127), 0},
    {MAP_GROUP(ROUTE128), MAP_NUM(ROUTE128), 0},
    {MAP_GROUP(ROUTE129), MAP_NUM(ROUTE129), 0},
    {MAP_GROUP(ROUTE130), MAP_NUM(ROUTE130), 0},
    {MAP_GROUP(ROUTE131), MAP_NUM(ROUTE131), 0},
    {MAP_GROUP(ROUTE132), MAP_NUM(ROUTE132), 0},
    {MAP_GROUP(ROUTE133), MAP_NUM(ROUTE133), 0},
    {MAP_GROUP(ROUTE134), MAP_NUM(ROUTE134), 0}
};

static const u8 *const sEverGrandeCityNames[] =
{
    gText_PokemonLeague,
    gText_PokemonCenter
};

static const struct MultiNameFlyDest sMultiNameFlyDestinations[] =
{
    {
        .name = sEverGrandeCityNames,
        .mapSecId = MAPSEC_EVER_GRANDE_CITY,
        .flag = FLAG_LANDMARK_POKEMON_LEAGUE
    }
};

static const struct BgTemplate sFlyMapBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0
    },
    {
        .bg = 2,
        .charBaseIndex = 2,
        .mapBaseIndex = 29,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2
    },
    {
        .bg = 3,
        .charBaseIndex = 3,
        .mapBaseIndex = 28,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3
    }
};

static const struct WindowTemplate sFlyMapWindowTemplates[] =
{
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 30,
        .height = 2,
        .paletteNum = 14,
        .baseBlock = 0x3D
    },
    DUMMY_WIN_TEMPLATE
};

static const u16 sRedOutlineFlyDestinations[][2] =
{
    {
        FLAG_LANDMARK_BATTLE_FRONTIER,
        MAPSEC_BATTLE_FRONTIER
    },
    {
        -1,
        MAPSEC_NONE
    }
};

static const struct OamData sFlyDestIcon_OamData =
{
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
    .priority = 2
};

static const union AnimCmd sFlyDestIcon_Anim[] =
{
    ANIMCMD_FRAME(0, 30),
    ANIMCMD_FRAME(4, 60),
    ANIMCMD_JUMP(0)
};

static const union AnimCmd *const sFlyDestIcon_Anims[] =
{
    sFlyDestIcon_Anim,
};

static const struct SpriteTemplate sFlyDestIconSpriteTemplate =
{
    .tileTag = 5,
    .paletteTag = 5,
    .oam = &sFlyDestIcon_OamData,
    .anims = sFlyDestIcon_Anims,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy
};

static const u8 whiteTextColor[] = {
	0x00, 0x01, 0x02
};

static const struct WindowCoords blankWindowCoords = {
    .x1 = 0,
    .y1 = 0,
    .x2 = 0,
    .y2 = 0,
};

static const struct WindowCoords windowCoords[] = {
    {
        .x1 = 24,
        .y1 = 16,
        .x2 = 144,
        .y2 = 32,
    },
    {
        .x1 = 24,
        .y1 = 32,
        .x2 = 144,
        .y2 = 48,
    },
};

static const bool8 sRegionMapPermissions[3][4] = {
    {FALSE, TRUE , TRUE , FALSE},
    {TRUE,  FALSE, FALSE, FALSE},
    {TRUE,  FALSE, FALSE, TRUE }
};

// .text

void InitRegionMap(struct RegionMap *regionMap, u8 mode, s8 xOffset)
{
    InitRegionMapData(regionMap, NULL, mode, xOffset);
    while (LoadRegionMapGfx(gRegionMap->bgManaged));
    while (LoadRegionMapGfx_Pt2());
}

void InitRegionMapData(struct RegionMap *regionMap, const struct BgTemplate *template, u8 mapMode, s8 xOffset)
{
    u8 i;
    
    gRegionMap = regionMap;
    gRegionMap->initStep = 0;
    gRegionMap->xOffset = xOffset;
    gRegionMap->currentRegion = GetCurrentRegion();
    gRegionMap->mapMode = mapMode;
    gRegionMap->onButton = FALSE;
    gRegionMap->inputCallback = ProcessRegionMapInput_Full;

    for (i = 0; i < 4; i++)
    {
        gRegionMap->permissions[i] = sRegionMapPermissions[gRegionMap->mapMode][i];
    }

    //TODO: Make conditional on visiting Kanto once
    gRegionMap->permissions[MAPPERM_SWITCH] = FALSE;
    
    for (i = 0; i < sizeof(gRegionMap->spriteIds); i++)
    {
        gRegionMap->spriteIds[i] = 0xFF;
    }

    if (template != NULL)
    {
        gRegionMap->bgNum = template->bg;
        gRegionMap->charBaseIdx = template->charBaseIndex;
        gRegionMap->mapBaseIdx = template->mapBaseIndex;
        gRegionMap->bgManaged = TRUE;
    }
    else
    {
        gRegionMap->bgNum = 2;
        gRegionMap->charBaseIdx = 2;
        gRegionMap->mapBaseIdx = 29;
        gRegionMap->bgManaged = FALSE;
    }
}

void ShowRegionMapForPokedexAreaScreen(struct RegionMap *regionMap)
{
    gRegionMap = regionMap;
    InitMapBasedOnPlayerLocation();
    gRegionMap->playerIconSpritePosX = gRegionMap->cursorPosX;
    gRegionMap->playerIconSpritePosY = gRegionMap->cursorPosY;
}

bool8 LoadRegionMapGfx(bool8 shouldBuffer)
{
    const u8 *regionTilemap;
    u8 i;
    u16 *ptr;

    switch (gRegionMap->initStep)
    {
        case 0:
            if (shouldBuffer)
            {
                decompress_and_copy_tile_data_to_vram(gRegionMap->bgNum, sRegionMapTileset, 0, 0, 0);
            }
            else
            {
                LZ77UnCompVram(sRegionMapTileset, (u16 *)BG_CHAR_ADDR(gRegionMap->charBaseIdx));
            }
            break;
        case 1:
            /*regionTilemap = GetRegionMapTilemap(gRegionMap->currentRegion);
            if (gRegionMap->bgManaged)
            {
                if (!free_temp_tile_data_buffers_if_possible())
                {
                    decompress_and_copy_tile_data_to_vram(gRegionMap->bgNum, regionTilemap, 0, 0, 1);
                }
            }
            else
            {
                LZ77UnCompVram(regionTilemap, (u16 *)BG_SCREEN_ADDR(28));
            }*/
            {
                u32 size;
                u8 x, y;
                u16 *ptr = malloc_and_decompress(GetRegionMapTilemap(gRegionMap->currentRegion), &size);
                
                if (gRegionMap->permissions[MAPPERM_SWITCH])
                {
                    ptr[25 + 17 * 32] = 0x90F4;
                }
                else if (!gRegionMap->permissions[MAPPERM_CLOSE])
                {
                    for (y = 16; y < 19; y++)
                    {
                        for (x = 24; x < 27; x++)
                        {
                            ptr[x + y * 32] = 0x9096;
                        }
                    }
                }

                if (shouldBuffer)
                {
                    if (!free_temp_tile_data_buffers_if_possible())
                    {
                        copy_decompressed_tile_data_to_vram(gRegionMap->bgNum, ptr, size, gRegionMap->xOffset, 1);
                    }
                }
                else
                {
                    CpuFastCopy(ptr, (u16 *)BG_SCREEN_ADDR(gRegionMap->mapBaseIdx) + gRegionMap->xOffset, size);
                }

                FREE_AND_SET_NULL(ptr);
            }
            break;
        case 2:
            //if (!free_temp_tile_data_buffers_if_possible())   // why are we checking for DMA3 being busy?
            {
                LoadPalette(sRegionMapPal, 0x70, sizeof(sRegionMapPal));
                LoadPalette(sRegionMapTownNames_Pal, 0xE0, sizeof(sRegionMapTownNames_Pal));
            }
            gRegionMap->initStep++;
        default:
            return FALSE;
    }
    gRegionMap->initStep++;
    return TRUE;
}

bool8 LoadRegionMapGfx_Pt2(void)
{
    const struct WindowTemplate layerTemplates[] = {
        {0, 3, 2, 15, 2, 14, 1},
        {0, 3, 4, 15, 2, 14, 31}
    };

    struct WindowTemplate window;

    switch (gRegionMap->initStep)
    {
        case 3:
            SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_EFFECT_NONE);
            SetGpuReg(REG_OFFSET_BLDY, 6);
            SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG0 |
                                        WININ_WIN0_BG1 |
                                        WININ_WIN0_BG2 |
                                        WININ_WIN0_BG3 |
                                        WININ_WIN0_OBJ |
                                        WININ_WIN0_CLR |
                                        WININ_WIN1_BG0 |
                                        WININ_WIN1_BG1 |
                                        WININ_WIN1_BG2 |
                                        WININ_WIN1_BG3 |
                                        WININ_WIN1_OBJ |
                                        WININ_WIN1_CLR);
            SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG0 |
                                         WINOUT_WIN01_BG1 |
                                         WINOUT_WIN01_BG2 |
                                         WINOUT_WIN01_BG3 |
                                         WINOUT_WIN01_OBJ);
            SetupShadowBoxes(0, &windowCoords[0]);
            SetupShadowBoxes(1, &windowCoords[1]);

            window = layerTemplates[0];
            window.tilemapLeft += gRegionMap->xOffset;
            gRegionMap->primaryWindowId = AddWindow(&window);

            window = layerTemplates[1];
            window.tilemapLeft += gRegionMap->xOffset;
            gRegionMap->secondaryWindowId = AddWindow(&window);
        case 4:
            LZ77UnCompWram(sRegionMapCursorGfxLZ, gRegionMap->cursorImage);
            break;
        case 5:
            InitMapBasedOnPlayerLocation();
            SetShadowBoxState(0, FALSE);

            if (gRegionMap->secondaryMapSecId != MAPSEC_NONE)
                SetShadowBoxState(1, FALSE);
            
            gRegionMap->playerIconSpritePosX = gRegionMap->cursorPosX;
            gRegionMap->playerIconSpritePosY = gRegionMap->cursorPosY;
            gRegionMap->primaryMapSecId = CorrectSpecialMapSecId_Internal(gRegionMap->primaryMapSecId);
            gRegionMap->primaryMapSecStatus = GetMapsecType(gRegionMap->primaryMapSecId);
            gRegionMap->secondaryMapSecId = CorrectSpecialMapSecId_Internal(gRegionMap->secondaryMapSecId);
            gRegionMap->secondaryMapSecStatus = GetMapsecType(gRegionMap->secondaryMapSecId);

            schedule_bg_copy_tilemap_to_vram(0);
            break;
        case 6:
            GetPositionOfCursorWithinMapSec();
            SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG2 | BLDCNT_TGT1_OBJ | BLDCNT_EFFECT_DARKEN);

            LoadPrimaryLayerMapSec();
            LoadSecondaryLayerMapSec();

            gRegionMap->cursorMovementFrameCounter = 0;
            gRegionMap->blinkPlayerIcon = FALSE;
            gRegionMap->initStep++;
        default:
            return FALSE;
    }
    gRegionMap->initStep++;
    return TRUE;
}

static const u32 *GetRegionMapTilemap(u8 region)
{
    const u32 *const tilemaps[] = {
        sRegionMapJohtoTilemap,
        sRegionMapKantoTilemap,
        sRegionMapJohtoTilemap,
        sRegionMapJohtoTilemap,
        sRegionMapJohtoTilemap,
    };

    return tilemaps[region];
}

void sub_8123030(u16 color, u32 coeff)
{
    BlendPalettes(0x380, coeff, color);
    CpuCopy16(gPlttBufferFaded + 0x70, gPlttBufferUnfaded + 0x70, 0x60);
}

void FreeRegionMapResources(void)
{
    u8 i;

    if (gRegionMap->spriteIds[0] != 0xFF)
    {
        DestroySprite(&gSprites[gRegionMap->spriteIds[0]]);
        FreeSpriteTilesByTag(gRegionMap->cursorTileTag);
        FreeSpritePaletteByTag(gRegionMap->cursorPaletteTag);
    }
    if (gRegionMap->spriteIds[1] != 0xFF)
    {
        DestroySprite(&gSprites[gRegionMap->spriteIds[1]]);
        FreeSpriteTilesByTag(gRegionMap->playerIconTileTag);
        FreeSpritePaletteByTag(gRegionMap->playerIconPaletteTag);
    }
    if (gRegionMap->spriteIds[2] != 0xFF)
    {
        DestroySprite(&gSprites[gRegionMap->spriteIds[2]]);
        FreeSpriteTilesByTag(gRegionMap->regionNameCurveTileTag);
    }
    if (gRegionMap->spriteIds[3] != 0xFF)
    {
        DestroySprite(&gSprites[gRegionMap->spriteIds[3]]);
        FreeSpriteTilesByTag(gRegionMap->regionNameMainTileTag);
    }

    for (i = 4; i < sizeof(gRegionMap->spriteIds); i++)
    {
        if (gRegionMap->spriteIds[i] != 0xFF)
        {
            DestroySprite(&gSprites[gRegionMap->spriteIds[i]]);
        }
    }
    
    FreeSpriteTilesByTag(gRegionMap->dotsTileTag);
    FreeSpritePaletteByTag(gRegionMap->miscSpritesPaletteTag);
    
    FillWindowPixelBuffer(gRegionMap->primaryWindowId, 0);
    ClearWindowTilemap(gRegionMap->primaryWindowId);
    CopyWindowToVram(gRegionMap->primaryWindowId, 2);
    RemoveWindow(gRegionMap->primaryWindowId);

    FillWindowPixelBuffer(gRegionMap->secondaryWindowId, 0);
    ClearWindowTilemap(gRegionMap->secondaryWindowId);
    CopyWindowToVram(gRegionMap->secondaryWindowId, 2);
    RemoveWindow(gRegionMap->secondaryWindowId);

    schedule_bg_copy_tilemap_to_vram(0);

    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_BLDY, 0);
    SetGpuReg(REG_OFFSET_WININ, 0);
    SetGpuReg(REG_OFFSET_WINOUT, 0);
    ClearGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_WIN1_ON);

    FREE_AND_SET_NULL(gRegionMap);
}

u8 DoRegionMapInputCallback(void)
{
    return gRegionMap->inputCallback();
}

static u8 ProcessRegionMapInput_Full(void)
{
    u8 input;

    input = MAP_INPUT_NONE;
    gRegionMap->cursorDeltaX = 0;
    gRegionMap->cursorDeltaY = 0;
    if (gMain.heldKeys & DPAD_UP && gRegionMap->cursorPosY > 0)
    {
        gRegionMap->cursorDeltaY = -1;
        input = MAP_INPUT_MOVE_START;
    }
    if (gMain.heldKeys & DPAD_DOWN && gRegionMap->cursorPosY < MAP_HEIGHT - 1)
    {
        gRegionMap->cursorDeltaY = +1;
        input = MAP_INPUT_MOVE_START;
    }
    if (gMain.heldKeys & DPAD_LEFT && gRegionMap->cursorPosX > 0)
    {
        gRegionMap->cursorDeltaX = -1;
        input = MAP_INPUT_MOVE_START;
    }
    if (gMain.heldKeys & DPAD_RIGHT && gRegionMap->cursorPosX < MAP_WIDTH - 1)
    {
        gRegionMap->cursorDeltaX = +1;
        input = MAP_INPUT_MOVE_START;
    }
    if (gMain.newKeys & A_BUTTON)
    {
        input = MAP_INPUT_A_BUTTON;
    }
    else if (gMain.newKeys & B_BUTTON)
    {
        input = MAP_INPUT_B_BUTTON;
    }
    if (input == MAP_INPUT_MOVE_START)
    {
        gRegionMap->cursorMovementFrameCounter = 4;
        gRegionMap->inputCallback = MoveRegionMapCursor_Full;
    }
    return input;
}

static bool8 LoadMapLayersFromPosition(u16 x, u16 y)
{
    u8 mapSecId;
    bool8 sameSecondary = TRUE;

    mapSecId = GetMapSecIdAt(x, y, gRegionMap->currentRegion, FALSE);
    gRegionMap->primaryMapSecStatus = GetMapsecType(mapSecId);
    if (mapSecId != gRegionMap->primaryMapSecId)
    {
        gRegionMap->primaryMapSecId = mapSecId;
        LoadPrimaryLayerMapSec();
    }

    mapSecId = GetMapSecIdAt(x, y, gRegionMap->currentRegion, TRUE);
    gRegionMap->secondaryMapSecStatus = GetMapsecType(mapSecId);
    if (mapSecId != gRegionMap->secondaryMapSecId)
    {
        gRegionMap->secondaryMapSecId = mapSecId;
        LoadSecondaryLayerMapSec();
        sameSecondary = FALSE;
    }

    schedule_bg_copy_tilemap_to_vram(0);
    SetupShadowBoxes(1, &windowCoords[1]);

    return sameSecondary;
}

static u8 MoveRegionMapCursor_Full(void)
{
    bool8 sameSecondary;
    u8 inputEvent;

    if (gRegionMap->cursorMovementFrameCounter != 0)
        return MAP_INPUT_MOVE_CONT;

    if (gRegionMap->cursorDeltaX > 0)
    {
        gRegionMap->cursorPosX++;
    }
    if (gRegionMap->cursorDeltaX < 0)
    {
        gRegionMap->cursorPosX--;
    }
    if (gRegionMap->cursorDeltaY > 0)
    {
        gRegionMap->cursorPosY++;
    }
    if (gRegionMap->cursorDeltaY < 0)
    {
        gRegionMap->cursorPosY--;
    }

    sameSecondary = LoadMapLayersFromPosition(gRegionMap->cursorPosX, gRegionMap->cursorPosY);
    inputEvent = MAP_INPUT_MOVE_END;

    if ((!sameSecondary && gRegionMap->secondaryMapSecStatus >= MAPSECTYPE_CITY_CANFLY) || gRegionMap->primaryMapSecStatus >= MAPSECTYPE_CITY_CANFLY)
    {
        m4aSongNumStart(SE_Z_SCROLL);
    }
    else if ((gRegionMap->permissions[MAPPERM_CLOSE] || gRegionMap->permissions[MAPPERM_SWITCH]) && gRegionMap->cursorPosX == CORNER_BUTTON_X && gRegionMap->cursorPosY == CORNER_BUTTON_Y)
    {
        m4aSongNumStart(SE_W255);
        inputEvent = MAP_INPUT_ON_BUTTON;
    }

    if (gRegionMap->secondaryMapSecStatus == MAPSECTYPE_CITY_CANFLY)
    {
        inputEvent = MAP_INPUT_LANDMARK;
    }
    
    GetPositionOfCursorWithinMapSec();
    gRegionMap->inputCallback = ProcessRegionMapInput_Full;
    return inputEvent;
}

static void LoadPrimaryLayerMapSec(void)
{
    ClearWindowTilemap(gRegionMap->primaryWindowId);
	FillWindowPixelBuffer(gRegionMap->primaryWindowId, 0);

	if (gRegionMap->primaryMapSecId != MAPSEC_NONE)
    {
		GetMapName(gRegionMap->primaryMapSecName, gRegionMap->primaryMapSecId, 0);
		AddTextPrinterParameterized3(gRegionMap->primaryWindowId, 1, 2, 2, whiteTextColor, 0, gRegionMap->primaryMapSecName);
		PutWindowTilemap(gRegionMap->primaryWindowId);
		CopyWindowToVram(gRegionMap->primaryWindowId, 3);
		SetupShadowBoxes(0, &windowCoords[0]);
    }
	else
    {
		SetupShadowBoxes(0, &blankWindowCoords);
	}
}

static void LoadSecondaryLayerMapSec(void)
{
    static const u8 mapNamePalDataPointerTable[][3] = {
        {0x00, 0x07, 0x02}, // green (visited)
        {0x00, 0x0A, 0x02}  // red (not yet visited)
    };

    SetShadowBoxState(1, TRUE);
    ClearWindowTilemap(gRegionMap->secondaryWindowId);

	if (gRegionMap->secondaryMapSecId != MAPSEC_NONE)
    {
        SetShadowBoxState(1, FALSE);
	    FillWindowPixelBuffer(gRegionMap->secondaryWindowId, 0);
		GetMapName(gRegionMap->secondaryMapSecName, gRegionMap->secondaryMapSecId, 0);
		AddTextPrinterParameterized3(gRegionMap->secondaryWindowId, 1, 12, 2, mapNamePalDataPointerTable[GetMapSecStatusByLayer(1) - 2], 0, gRegionMap->secondaryMapSecName);
		PutWindowTilemap(gRegionMap->secondaryWindowId);
		CopyWindowToVram(gRegionMap->secondaryWindowId, 3);
	}
}

static void SetupShadowBoxes(u8 layerNum, const struct WindowCoords *coords)
{
    static const u8 windowIORegs[2][2] = {
        { REG_OFFSET_WIN0V, REG_OFFSET_WIN0H },
        { REG_OFFSET_WIN1V, REG_OFFSET_WIN1H }
    };

	SetGpuReg(windowIORegs[layerNum][0], WIN_RANGE(coords->y1, coords->y2));
	SetGpuReg(windowIORegs[layerNum][1], WIN_RANGE(coords->x1 + gRegionMap->xOffset * 8, coords->x2 + gRegionMap->xOffset * 8));
}

static void SetShadowBoxState(u8 offset, bool8 hide)
{
    static const u16 windowBits[2] = {
        0x2000, 0x4000
    };

	if (!hide)
    {
		SetGpuRegBits(REG_OFFSET_DISPCNT, windowBits[offset]);
	}
	else
    {
		ClearGpuRegBits(REG_OFFSET_DISPCNT, windowBits[offset]);
	}
}

static u8 GetMapSecStatusByLayer(u8 layer)
{
	if (layer == 1)
        return gRegionMap->secondaryMapSecStatus;
	else
        return gRegionMap->primaryMapSecStatus;
}

void SetRegionMapDataForZoom(void)
{

}

bool8 UpdateRegionMapZoom(void)
{
    return FALSE;
}

void UpdateRegionMapVideoRegs(void)
{

}

void PokedexAreaScreen_UpdateRegionMapVariablesAndVideoRegs(s16 x, s16 y)
{

}

static u8 GetMapSecIdAt(s16 x, s16 y, u8 region, bool8 secondary)
{
    static const u8 *const layouts[][2] = {
        {sMapSectionLayout_JohtoPrimary, sMapSectionLayout_JohtoSecondary},
        {sMapSectionLayout_KantoPrimary, sMapSectionLayout_KantoSecondary},
        {sMapSectionLayout_JohtoPrimary, sMapSectionLayout_JohtoSecondary},
        {sMapSectionLayout_JohtoPrimary, sMapSectionLayout_JohtoSecondary},
        {sMapSectionLayout_JohtoPrimary, sMapSectionLayout_JohtoSecondary}
    };

    if (y < 0 || y >= MAP_HEIGHT || x < 0 || x >= MAP_WIDTH)
    {
        return MAPSEC_NONE;
    }
    return layouts[gRegionMap->currentRegion][secondary][x + y * MAP_WIDTH];
}

static void InitMapBasedOnPlayerLocation(void)
{
    if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(SS_TIDAL_CORRIDOR)
        && (gSaveBlock1Ptr->location.mapNum == MAP_NUM(SS_TIDAL_CORRIDOR)
            || gSaveBlock1Ptr->location.mapNum == MAP_NUM(SS_TIDAL_LOWER_DECK)
            || gSaveBlock1Ptr->location.mapNum == MAP_NUM(SS_TIDAL_ROOMS)))
    {
        RegionMap_InitializeStateBasedOnSSTidalLocation();
    }
    else
    {
        // This is actually how FireRed fixes gatehouse map positions
        switch (GetCurrentRegionMapSectionId())
        {
            case MAPSEC_ROUTE_29:
                if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(ROUTE29_GATEHOUSE) &&
                    gSaveBlock1Ptr->location.mapNum == MAP_NUM(ROUTE29_GATEHOUSE))
                {
                    gRegionMap->cursorPosX = 18;
                    gRegionMap->cursorPosY = 10;
                }
                else
                {
                    InitMapBasedOnPlayerLocation_();
                }
                break;
            case MAPSEC_ROUTE_31:
                if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(ROUTE31_GATEHOUSE) &&
                    gSaveBlock1Ptr->location.mapNum == MAP_NUM(ROUTE31_GATEHOUSE))
                {
                    gRegionMap->cursorPosX = 12;
                    gRegionMap->cursorPosY = 5;
                }
                else
                {
                    InitMapBasedOnPlayerLocation_();
                }
                break;
            case MAPSEC_ROUTE_32:
                if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(ROUTE32_GATEHOUSE) &&
                    gSaveBlock1Ptr->location.mapNum == MAP_NUM(ROUTE32_GATEHOUSE))
                {
                    gRegionMap->cursorPosX = 11;
                    gRegionMap->cursorPosY = 6;
                }
                else
                {
                    InitMapBasedOnPlayerLocation_();
                }
                break;
            case MAPSEC_ROUTE_34:
                if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(ROUTE34_ILEX_EAST_GATEHOUSE) &&
                    gSaveBlock1Ptr->location.mapNum == MAP_NUM(ROUTE34_ILEX_EAST_GATEHOUSE))
                {
                    gRegionMap->cursorPosX = 8;
                    gRegionMap->cursorPosY = 13;
                }
                else if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(ROUTE34_ILEX_NORTH_GATEHOUSE) &&
                         gSaveBlock1Ptr->location.mapNum == MAP_NUM(ROUTE34_ILEX_NORTH_GATEHOUSE))
                {
                    gRegionMap->cursorPosX = 7;
                    gRegionMap->cursorPosY = 12;
                }
                else
                {
                    InitMapBasedOnPlayerLocation_();
                }
                break;
            case MAPSEC_ROUTE_36:
                if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(ROUTE36_RUINS_OF_ALPH_GATEHOUSE) &&
                    gSaveBlock1Ptr->location.mapNum == MAP_NUM(ROUTE36_RUINS_OF_ALPH_GATEHOUSE))
                {
                    gRegionMap->cursorPosX = 10;
                    gRegionMap->cursorPosY = 5;
                }
                else
                {
                    InitMapBasedOnPlayerLocation_();
                }
                break;
            default:
                InitMapBasedOnPlayerLocation_();
                break;
        }
    }

    gRegionMap->primaryMapSecId = GetMapSecIdAt(gRegionMap->cursorPosX, gRegionMap->cursorPosY, gRegionMap->currentRegion, FALSE);
    gRegionMap->secondaryMapSecId = GetMapSecIdAt(gRegionMap->cursorPosX, gRegionMap->cursorPosY, gRegionMap->currentRegion, TRUE);
}

static void InitMapBasedOnPlayerLocation_(void)
{
    const struct MapHeader *mapHeader;
    u16 mapWidth;
    u16 mapHeight;
    u16 x;
    u16 y;
    u16 dimensionScale;
    u16 xOnMap;
    struct WarpData *warp;

    switch (GetMapTypeByGroupAndId(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum))
    {
        default:
        case MAP_TYPE_TOWN:
        case MAP_TYPE_CITY:
        case MAP_TYPE_ROUTE:
        case MAP_TYPE_UNDERWATER:
        case MAP_TYPE_OCEAN_ROUTE:
            gRegionMap->primaryMapSecId = gMapHeader.regionMapSectionId;
            gRegionMap->playerIsInCave = FALSE;
            mapWidth = gMapHeader.mapLayout->width;
            mapHeight = gMapHeader.mapLayout->height;
            x = gSaveBlock1Ptr->pos.x;
            y = gSaveBlock1Ptr->pos.y;
            if (gRegionMap->primaryMapSecId == MAPSEC_UNDERWATER_128 || gRegionMap->primaryMapSecId == MAPSEC_UNDERWATER_MARINE_CAVE)
            {
                gRegionMap->playerIsInCave = TRUE;
            }
            break;
        case MAP_TYPE_UNDERGROUND:
        case MAP_TYPE_UNKNOWN:
            if (gMapHeader.flags & MAP_ALLOW_ESCAPING)
            {
                mapHeader = Overworld_GetMapHeaderByGroupAndId(gSaveBlock1Ptr->escapeWarp.mapGroup, gSaveBlock1Ptr->escapeWarp.mapNum);
                gRegionMap->primaryMapSecId = mapHeader->regionMapSectionId;
                gRegionMap->playerIsInCave = TRUE;
                mapWidth = mapHeader->mapLayout->width;
                mapHeight = mapHeader->mapLayout->height;
                x = gSaveBlock1Ptr->escapeWarp.x;
                y = gSaveBlock1Ptr->escapeWarp.y;
            }
            else
            {
                gRegionMap->primaryMapSecId = gMapHeader.regionMapSectionId;
                gRegionMap->playerIsInCave = TRUE;
                mapWidth = 1;
                mapHeight = 1;
                x = 1;
                y = 1;
            }
            break;
        case MAP_TYPE_SECRET_BASE:
            mapHeader = Overworld_GetMapHeaderByGroupAndId((u16)gSaveBlock1Ptr->dynamicWarp.mapGroup, (u16)gSaveBlock1Ptr->dynamicWarp.mapNum);
            gRegionMap->primaryMapSecId = mapHeader->regionMapSectionId;
            gRegionMap->playerIsInCave = TRUE;
            mapWidth = mapHeader->mapLayout->width;
            mapHeight = mapHeader->mapLayout->height;
            x = gSaveBlock1Ptr->dynamicWarp.x;
            y = gSaveBlock1Ptr->dynamicWarp.y;
            break;
        case MAP_TYPE_INDOOR:
            gRegionMap->primaryMapSecId = gMapHeader.regionMapSectionId;
            if (gRegionMap->primaryMapSecId != MAPSEC_DYNAMIC)
            {
                warp = &gSaveBlock1Ptr->escapeWarp;
                mapHeader = Overworld_GetMapHeaderByGroupAndId(warp->mapGroup, warp->mapNum);
            }
            else
            {
                warp = &gSaveBlock1Ptr->dynamicWarp;
                mapHeader = Overworld_GetMapHeaderByGroupAndId(warp->mapGroup, warp->mapNum);
                gRegionMap->primaryMapSecId = mapHeader->regionMapSectionId;
            }

            if (IsPlayerInAquaHideout(gRegionMap->primaryMapSecId))
                gRegionMap->playerIsInCave = TRUE;
            else
                gRegionMap->playerIsInCave = FALSE;

            mapWidth = mapHeader->mapLayout->width;
            mapHeight = mapHeader->mapLayout->height;
            x = warp->x;
            y = warp->y;
            break;
    }

    xOnMap = x;

    dimensionScale = mapWidth / gRegionMapEntries[gRegionMap->primaryMapSecId].width;
    if (dimensionScale == 0)
    {
        dimensionScale = 1;
    }
    x /= dimensionScale;
    if (x >= gRegionMapEntries[gRegionMap->primaryMapSecId].width)
    {
        x = gRegionMapEntries[gRegionMap->primaryMapSecId].width - 1;
    }

    dimensionScale = mapHeight / gRegionMapEntries[gRegionMap->primaryMapSecId].height;
    if (dimensionScale == 0)
    {
        dimensionScale = 1;
    }
    y /= dimensionScale;
    if (y >= gRegionMapEntries[gRegionMap->primaryMapSecId].height)
    {
        y = gRegionMapEntries[gRegionMap->primaryMapSecId].height - 1;
    }

    switch (gRegionMap->primaryMapSecId)
    {
        case MAPSEC_ROUTE_33:
            x = 0;
            if (gSaveBlock1Ptr->pos.x > 8)
                x = 1;
            break;
        case MAPSEC_ROUTE_36:
            x = 0;
            if (gSaveBlock1Ptr->pos.x > 36)
                x = 3;
            else if (gSaveBlock1Ptr->pos.x > 16)
                x = 2;
            else if (gSaveBlock1Ptr->pos.x > 9)
                x = 1;
            break;
    }
    gRegionMap->cursorPosX = gRegionMapEntries[gRegionMap->primaryMapSecId].x + x;
    gRegionMap->cursorPosY = gRegionMapEntries[gRegionMap->primaryMapSecId].y + y;
}

static void RegionMap_InitializeStateBasedOnSSTidalLocation(void)
{
    u16 y;
    u16 x;
    u8 mapGroup;
    u8 mapNum;
    u16 dimensionScale;
    s16 xOnMap;
    s16 yOnMap;
    const struct MapHeader *mapHeader;

    y = 0;
    x = 0;
    switch (GetSSTidalLocation(&mapGroup, &mapNum, &xOnMap, &yOnMap))
    {
        case SS_TIDAL_LOCATION_SLATEPORT:
            gRegionMap->primaryMapSecId = MAPSEC_SLATEPORT_CITY;
            break;
        case SS_TIDAL_LOCATION_LILYCOVE:
            gRegionMap->primaryMapSecId = MAPSEC_LILYCOVE_CITY;
            break;
        case SS_TIDAL_LOCATION_ROUTE124:
            gRegionMap->primaryMapSecId = MAPSEC_ROUTE_124;
            break;
        case SS_TIDAL_LOCATION_ROUTE131:
            gRegionMap->primaryMapSecId = MAPSEC_ROUTE_131;
            break;
        default:
        case SS_TIDAL_LOCATION_CURRENTS:
            mapHeader = Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum);

            gRegionMap->primaryMapSecId = mapHeader->regionMapSectionId;
            dimensionScale = mapHeader->mapLayout->width / gRegionMapEntries[gRegionMap->primaryMapSecId].width;
            if (dimensionScale == 0)
                dimensionScale = 1;
            x = xOnMap / dimensionScale;
            if (x >= gRegionMapEntries[gRegionMap->primaryMapSecId].width)
                x = gRegionMapEntries[gRegionMap->primaryMapSecId].width - 1;

            dimensionScale = mapHeader->mapLayout->height / gRegionMapEntries[gRegionMap->primaryMapSecId].height;
            if (dimensionScale == 0)
                dimensionScale = 1;
            y = yOnMap / dimensionScale;
            if (y >= gRegionMapEntries[gRegionMap->primaryMapSecId].height)
                y = gRegionMapEntries[gRegionMap->primaryMapSecId].height - 1;
            break;
    }
    gRegionMap->playerIsInCave = FALSE;
    gRegionMap->cursorPosX = gRegionMapEntries[gRegionMap->primaryMapSecId].x + x;
    gRegionMap->cursorPosY = gRegionMapEntries[gRegionMap->primaryMapSecId].y + y;
}

static u8 GetMapsecType(u16 mapSecId)
{
    u8 mapSecStatus = MAPSECTYPE_NONE;

    // ensure no landmark sound on any map besides fly map
    if (mapSecId == MAPSEC_ROUTE_32_FLYDUP && !gRegionMap->permissions[MAPPERM_FLY])
    {
        mapSecStatus = MAPSECTYPE_ROUTE;
    }
    else if (mapSecId != MAPSEC_NONE)
    {
        u16 flag = sMapSecFlags[mapSecId];
        mapSecStatus = MAPSECTYPE_ROUTE;

        if (flag != 0)
        {
            mapSecStatus = MAPSECTYPE_CITY_CANTFLY;
            if (FlagGet(flag))
            {
                mapSecStatus = MAPSECTYPE_CITY_CANFLY;
            }
        }
    }

    return mapSecStatus;
}

u16 GetRegionMapSectionIdAt(u16 x, u16 y)
{
    // TODO: Region
    return GetMapSecIdAt(x, y, REGION_JOHTO, FALSE);
}

static u16 CorrectSpecialMapSecId_Internal(u16 mapSecId)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sMarineCaveMapSecIds); i++)
    {
        if (sMarineCaveMapSecIds[i] == mapSecId)
        {
            return GetTerraOrMarineCaveMapSecId();
        }
    }
    for (i = 0; sRegionMap_SpecialPlaceLocations[i][0] != MAPSEC_NONE; i++)
    {
        if (sRegionMap_SpecialPlaceLocations[i][0] == mapSecId)
        {
            return sRegionMap_SpecialPlaceLocations[i][1];
        }
    }
    return mapSecId;
}

static u16 GetTerraOrMarineCaveMapSecId(void)
{
    s16 idx;

    idx = VarGet(VAR_ABNORMAL_WEATHER_LOCATION) - 1;

    if (idx < 0 || idx > ABNORMAL_WEATHER_LOCATIONS - 1)
        idx = 0;

    return sTerraOrMarineCaveMapSecIds[idx];
}

static void GetMarineCaveCoords(u16 *x, u16 *y)
{
    u16 idx;

    idx = VarGet(VAR_ABNORMAL_WEATHER_LOCATION);
    if (idx < MARINE_CAVE_LOCATIONS_START || idx > ABNORMAL_WEATHER_LOCATIONS)
    {
        idx = MARINE_CAVE_LOCATIONS_START;
    }
    idx -= MARINE_CAVE_LOCATIONS_START;

    *x = sMarineCaveLocationCoords[idx].x;
    *y = sMarineCaveLocationCoords[idx].y;
}

// Probably meant to be an "IsPlayerInIndoorDungeon" function, but in practice it only has the one mapsec
// Additionally, because the mapsec doesnt exist in Emerald, this function always returns FALSE
static bool32 IsPlayerInAquaHideout(u8 mapSecId)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sMapSecAquaHideoutOld); i++)
    {
        if (sMapSecAquaHideoutOld[i] == mapSecId)
            return TRUE;
    }
    return FALSE;
}

u16 CorrectSpecialMapSecId(u16 mapSecId)
{
    return CorrectSpecialMapSecId_Internal(mapSecId);
}

static void GetPositionOfCursorWithinMapSec(void)
{
    u16 x;
    u16 y;
    u16 posWithinMapSec;

    if (gRegionMap->primaryMapSecId == MAPSEC_NONE)
    {
        gRegionMap->posWithinMapSec = 0;
        return;
    }
    x = gRegionMap->cursorPosX;
    y = gRegionMap->cursorPosY;
    posWithinMapSec = 0;
    while (1)
    {
        if (x <= 0)
        {
            if (RegionMap_IsMapSecIdInNextRow(y))
            {
                y--;
                x = MAP_WIDTH;
            }
            else
            {
                break;
            }
        }
        else
        {
            x--;
            if (GetMapSecIdAt(x, y, gRegionMap->currentRegion, FALSE) == gRegionMap->primaryMapSecId)
            {
                posWithinMapSec++;
            }
        }
    }
    gRegionMap->posWithinMapSec = posWithinMapSec;
}

static bool8 RegionMap_IsMapSecIdInNextRow(u16 y)
{
    u16 x;

    if (y-- == 0)
    {
        return FALSE;
    }
    for (x = 0; x < MAP_WIDTH; x++)
    {
        if (GetMapSecIdAt(x, y, gRegionMap->currentRegion, FALSE) == gRegionMap->primaryMapSecId)
        {
            return TRUE;
        }
    }
    return FALSE;
}

static void SpriteCB_CursorMapFull(struct Sprite *sprite)
{
    if (gRegionMap->cursorMovementFrameCounter != 0)
    {
        sprite->pos1.x += 2 * gRegionMap->cursorDeltaX;
        sprite->pos1.y += 2 * gRegionMap->cursorDeltaY;
        gRegionMap->cursorMovementFrameCounter--;
    }
}

void CreateRegionMapCursor(u16 tileTag, u16 paletteTag, bool8 visible)
{
    u8 spriteId;
    struct Sprite *sprite;
    struct SpriteTemplate template;
    struct SpritePalette palette;
    struct SpriteSheet sheet;

    palette = sRegionMapCursorSpritePalette;
    template = sRegionMapCursorSpriteTemplate;
    sheet.tag = tileTag;
    template.tileTag = tileTag;
    gRegionMap->cursorTileTag = tileTag;
    palette.tag = paletteTag;
    template.paletteTag = paletteTag;
    gRegionMap->cursorPaletteTag = paletteTag;
    sheet.data = gRegionMap->cursorImage;
    sheet.size = sizeof(gRegionMap->cursorImage);
    LoadSpriteSheet(&sheet);
    LoadSpritePalette(&palette);
    spriteId = CreateSprite(&template, 0x38, 0x48, 0);
    if (spriteId != MAX_SPRITES)
    {
        gRegionMap->spriteIds[0] = spriteId;
        sprite = &gSprites[spriteId];
        sprite->oam.size = 1;

        if (visible)
        {
            sprite->pos1.x = (gRegionMap->cursorPosX + gRegionMap->xOffset + MAPCURSOR_X_MIN) * 8 + 4;
            sprite->pos1.y = (gRegionMap->cursorPosY + MAPCURSOR_Y_MIN) * 8 + 4;
        }
        else
        {
            sprite->invisible = TRUE;
            sprite->callback = SpriteCallbackDummy;
        }

        sprite->data[1] = 2;
        sprite->data[2] = (IndexOfSpritePaletteTag(paletteTag) << 4) + 0x101;
        sprite->data[3] = TRUE;
    }
}

void ShowRegionMapCursorSprite(void)
{
    if (gRegionMap->spriteIds[0] != 0xFF)
    {
        struct Sprite *sprite = &gSprites[gRegionMap->spriteIds[0]];

        sprite->pos1.x = (gRegionMap->cursorPosX + gRegionMap->xOffset + MAPCURSOR_X_MIN) * 8 + 4;
        sprite->pos1.y = (gRegionMap->cursorPosY + MAPCURSOR_Y_MIN) * 8 + 4;
        sprite->callback = SpriteCB_CursorMapFull;
        StartSpriteAnim(sprite, 0);
        sprite->invisible = FALSE;
    }
}

void HideRegionMapCursorSprite(void)
{
    if (gRegionMap->spriteIds[0] != 0xFF)
    {
        struct Sprite *sprite = &gSprites[gRegionMap->spriteIds[0]];

        gRegionMap->cursorPosX = gRegionMap->playerIconSpritePosX;
        gRegionMap->cursorPosY = gRegionMap->playerIconSpritePosY;
        LoadMapLayersFromPosition(gRegionMap->cursorPosX, gRegionMap->cursorPosY);

        sprite->invisible = TRUE;
        sprite->callback = SpriteCallbackDummy;
    }
}

// Unused
static void SetUnkCursorSpriteData(void)
{
    gSprites[gRegionMap->spriteIds[0]].data[3] = TRUE;
}

// Unused
static void ClearUnkCursorSpriteData(void)
{
    gSprites[gRegionMap->spriteIds[0]].data[3] = FALSE;
}

void CreateRegionMapPlayerIcon(u16 tileTag, u16 paletteTag)
{
    struct Sprite *sprite;
    struct SpriteSheet sheet = {sRegionMapPlayerIcon_GoldGfx, 0x80, tileTag};
    struct SpritePalette palette = {sRegionMapPlayerIcon_GoldPal, paletteTag};
    struct SpriteTemplate template = {tileTag, paletteTag, &sRegionMapPlayerIconOam, gDummySpriteAnimTable, NULL, gDummySpriteAffineAnimTable, SpriteCallbackDummy};

    if (IsEventIslandMapSecId(gMapHeader.regionMapSectionId))
    {
        gRegionMap->spriteIds[1] = 0xFF;
        return;
    }
    if (gSaveBlock2Ptr->playerGender == FEMALE)
    {
        sheet.data = sRegionMapPlayerIcon_KrisGfx;
        palette.data = sRegionMapPlayerIcon_KrisPal;
    }
    LoadSpriteSheet(&sheet);
    LoadSpritePalette(&palette);
    gRegionMap->spriteIds[1] = CreateSprite(&template, 0, 0, 2);
    sprite = &gSprites[gRegionMap->spriteIds[1]];
    sprite->pos1.x = (gRegionMap->playerIconSpritePosX + gRegionMap->xOffset + MAPCURSOR_X_MIN) * 8 + 4;
    sprite->pos1.y = (gRegionMap->playerIconSpritePosY + MAPCURSOR_Y_MIN) * 8 + 4;
    //sprite->callback = SpriteCB_PlayerIcon;
}

static void HideRegionMapPlayerIcon(void)
{
    if (gRegionMap->spriteIds[1] != 0xFF)
    {
        struct Sprite *sprite = &gSprites[gRegionMap->spriteIds[1]];

        sprite->invisible = TRUE;
        sprite->callback = SpriteCallbackDummy;
    }
}

static void UnhideRegionMapPlayerIcon(void)
{
    if (gRegionMap->spriteIds[1] != 0xFF)
    {
        struct Sprite *sprite = &gSprites[gRegionMap->spriteIds[1]];

        sprite->pos1.x = (gRegionMap->playerIconSpritePosX + gRegionMap->xOffset + MAPCURSOR_X_MIN) * 8 + 4;
        sprite->pos1.y = (gRegionMap->playerIconSpritePosY + MAPCURSOR_Y_MIN) * 8 + 4;
        sprite->pos2.x = 0;
        sprite->pos2.y = 0;
        //sprite->callback = SpriteCB_PlayerIcon;
        sprite->invisible = FALSE;
    }
}

static void SpriteCB_PlayerIcon(struct Sprite *sprite)
{
    if (gRegionMap->blinkPlayerIcon)
    {
        if (++sprite->data[7] > 16)
        {
            sprite->data[7] = 0;
            sprite->invisible = sprite->invisible ? FALSE : TRUE;
        }
    }
    else
    {
        sprite->invisible = FALSE;
    }
}

void TrySetPlayerIconBlink(void)
{
    if (gRegionMap->playerIsInCave)
        gRegionMap->blinkPlayerIcon = TRUE;
}

void CreateRegionMapName(u16 tileTagCurve, u16 tileTagMain)
{
    u8 nameToDisplay;

    struct SpriteTemplate template;
    struct SpriteSheet curveSheet = {sRegionMapNamesCurve_Gfx, sizeof(sRegionMapNamesCurve_Gfx), tileTagCurve};
    struct SpriteSheet mainSheet = {sRegionMapNames_Gfx, sizeof(sRegionMapNames_Gfx), tileTagMain};

    template = sRegionMapNameCurveSpriteTemplate;
    template.tileTag = tileTagCurve;
    template.paletteTag = gRegionMap->miscSpritesPaletteTag;
    LoadSpriteSheet(&curveSheet);
    gRegionMap->spriteIds[2] = CreateSprite(&template, 180 + gRegionMap->xOffset * 8, 20, 0);
    gRegionMap->regionNameCurveTileTag = tileTagCurve;
    
    template = sRegionMapNameSpriteTemplate;
    template.tileTag = tileTagMain;
    template.paletteTag = gRegionMap->miscSpritesPaletteTag;
    LoadSpriteSheet(&mainSheet);
    gRegionMap->spriteIds[3] = CreateSprite(&template, 200 + gRegionMap->xOffset * 8, 20, 0);
    gRegionMap->regionNameMainTileTag = tileTagMain;

    if (gRegionMap->currentRegion >= REGION_SEVII1)
    {
        nameToDisplay = REGION_SEVII1;
    }
    else
    {
        nameToDisplay = gRegionMap->currentRegion;
    }

    StartSpriteAnim(&gSprites[gRegionMap->spriteIds[3]], gRegionMap->currentRegion);
}

void CreateSecondaryLayerDots(u16 tileTag, u16 paletteTag)
{
    u8 i = 0;
    u16 x, y, newX, newY;

    struct SpriteSheet sheet = {sRegionMapDots_Gfx, sizeof(sRegionMapDots_Gfx), tileTag};
    struct SpritePalette palette = {sRegionMapDots_Pal, paletteTag};
    struct SpriteTemplate template = {tileTag, paletteTag, &sRegionMapDotsOam, sRegionMapDotsAnimTable, NULL, gDummySpriteAffineAnimTable, SpriteCallbackDummy};

    LoadSpriteSheet(&sheet);
    LoadSpritePalette(&palette);
    gRegionMap->dotsTileTag = tileTag;
    gRegionMap->miscSpritesPaletteTag = paletteTag;

    for (y = 0; y < MAP_HEIGHT; y++)
    {
        for (x = 0; x < MAP_WIDTH; x++)
        {
            u8 secondaryMapSec = GetMapSecIdAt(x, y, gRegionMap->currentRegion, TRUE);
            
            if (secondaryMapSec != MAPSEC_NONE)
            {
                u8 spriteId;

                if ((gRegionMapEntries[secondaryMapSec].width > 1 || gRegionMapEntries[secondaryMapSec].height > 1))
                {
                    if (x == gRegionMapEntries[secondaryMapSec].x && y == gRegionMapEntries[secondaryMapSec].y)
                    {
                        newX = (gRegionMapEntries[secondaryMapSec].width * 8) / 2 + (gRegionMapEntries[secondaryMapSec].x + MAPCURSOR_X_MIN + gRegionMap->xOffset) * 8;
                        newY = (gRegionMapEntries[secondaryMapSec].height * 8) / 2 + (gRegionMapEntries[secondaryMapSec].y + MAPCURSOR_Y_MIN) * 8;
                        spriteId = CreateSprite(&template, newX, newY, 3);
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    u8 offset = 0;

                    if (GetMapsecType(GetMapSecIdAt(x, y, gRegionMap->currentRegion, FALSE)) >= MAPSECTYPE_CITY_CANFLY)
                    {
                        offset = 2;
                    }

                    spriteId = CreateSprite(&template, (x + MAPCURSOR_X_MIN + gRegionMap->xOffset) * 8 + offset + 4, (y + MAPCURSOR_Y_MIN) * 8 + offset + 4, 3);
                }

                if (GetMapsecType(secondaryMapSec) == MAPSECTYPE_CITY_CANFLY)
                {
                    StartSpriteAnim(&gSprites[spriteId], 1);
                }

                gRegionMap->spriteIds[i++ + 4] = spriteId;
                
                if (i + 4 > sizeof(gRegionMap->spriteIds))
                {
                    return;
                }
            }
        }
    }
}

u8 *GetMapName(u8 *dest, u16 regionMapId, u16 padLength)
{
    u8 *str;
    u16 i;

    if (regionMapId == MAPSEC_SECRET_BASE || regionMapId == MAPSEC_E_SECRET_BASE)
    {
        str = GetSecretBaseMapName(dest);
    }
    else if (regionMapId < MAPSEC_NONE)
    {
        str = StringCopy(dest, gRegionMapEntries[regionMapId].name);
    }
    else if (regionMapId >= EMERALD_MAPSEC_START && regionMapId < EMERALD_MAPSEC_END)
    {
        str = StringCopy(dest, gRegionMapNames_Emerald[regionMapId - EMERALD_MAPSEC_START]);
    }
    else
    {
        if (padLength == 0)
        {
            padLength = 18;
        }
        return StringFill(dest, CHAR_SPACE, padLength);
    }
    if (padLength != 0)
    {
        for (i = str - dest; i < padLength; i++)
        {
            *str++ = CHAR_SPACE;
        }
        *str = EOS;
    }
    return str;
}

// TODO: probably needs a better name
u8 *GetMapNameGeneric(u8 *dest, u16 mapSecId)
{
    switch (mapSecId)
    {
    case MAPSEC_DYNAMIC:
    case MAPSEC_E_DYNAMIC:
        return StringCopy(dest, gText_Ferry);
    case MAPSEC_SECRET_BASE:
    case MAPSEC_E_SECRET_BASE:
        return StringCopy(dest, gText_SecretBase);
    default:
        return GetMapName(dest, mapSecId, 0);
    }
}

u8 *GetMapNameHandleAquaHideout(u8 *dest, u16 mapSecId)
{
    if (mapSecId == MAPSEC_E_AQUA_HIDEOUT_OLD)
        return StringCopy(dest, gText_Hideout);
    else
        return GetMapNameGeneric(dest, mapSecId);
}

static void GetMapSecDimensions(u16 mapSecId, u16 *x, u16 *y, u16 *width, u16 *height)
{
    *x = gRegionMapEntries[mapSecId].x;
    *y = gRegionMapEntries[mapSecId].y;
    *width = gRegionMapEntries[mapSecId].width;
    *height = gRegionMapEntries[mapSecId].height;
}

bool8 IsRegionMapZoomed(void)
{
    return FALSE;
}

bool32 IsEventIslandMapSecId(u8 mapSecId)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sMapSecIdsOffMap); i++)
    {
        if (mapSecId == sMapSecIdsOffMap[i])
            return TRUE;
    }
    return FALSE;
}

void CB2_OpenFlyMap(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        SetGpuReg(REG_OFFSET_BG0HOFS, 0);
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        SetGpuReg(REG_OFFSET_BG1HOFS, 0);
        SetGpuReg(REG_OFFSET_BG1VOFS, 0);
        SetGpuReg(REG_OFFSET_BG2VOFS, 0);
        SetGpuReg(REG_OFFSET_BG2HOFS, 0);
        SetGpuReg(REG_OFFSET_BG3HOFS, 0);
        SetGpuReg(REG_OFFSET_BG3VOFS, 0);
        sFlyMap = malloc(sizeof(*sFlyMap));
        if (sFlyMap == NULL)
        {
            SetMainCallback2(CB2_ReturnToFieldWithOpenMenu);
        }
        else
        {
            ResetPaletteFade();
            ResetSpriteData();
            FreeSpriteTileRanges();
            FreeAllSpritePalettes();
            gMain.state++;
        }
        break;
    case 1:
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sFlyMapBgTemplates, ARRAY_COUNT(sFlyMapBgTemplates));
        gMain.state++;
        break;
    case 2:
        InitWindows(sFlyMapWindowTemplates);
        DeactivateAllTextPrinters();
        gMain.state++;
        break;
    case 3:
        clear_scheduled_bg_copies_to_vram();
        gMain.state++;
        break;
    case 4:
        InitRegionMap(&sFlyMap->regionMap, MAPMODE_FLY, 0);
        CreateRegionMapCursor(0, 0, TRUE);
        CreateRegionMapPlayerIcon(1, 1);
        CreateSecondaryLayerDots(2, 2);
        CreateRegionMapName(3, 4);
        ShowHelpBar();
        sFlyMap->mapSecId = sFlyMap->regionMap.primaryMapSecId;
        gUnknown_03001180 = TRUE;
        gMain.state++;
        break;
    case 5:
        LZ77UnCompVram(gRegionMapFrameGfxLZ, (u16 *)BG_CHAR_ADDR(3));
        gMain.state++;
        break;
    case 6:
        LZ77UnCompVram(gRegionMapFrameTilemapLZ, (u16 *)BG_SCREEN_ADDR(28));
        gMain.state++;
        break;
    case 7:
        LoadPalette(gRegionMapFramePal, 0x10, 0x20);
        gMain.state++;
        break;
    case 8:
        LoadFlyDestIcons();
        gMain.state++;
        break;
    case 9:
        BlendPalettes(-1, 16, 0);
        SetVBlankCallback(VBlankCB_FlyMap);
        gMain.state++;
        break;
    case 10:
        SetGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        ShowBg(0);
        ShowBg(2);
        ShowBg(3);
        SetFlyMapCallback(CB_FadeInFlyMap);
        SetMainCallback2(CB2_FlyMap);
        gMain.state++;
        break;
    }
}

static void VBlankCB_FlyMap(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_FlyMap(void)
{
    sFlyMap->callback();
    AnimateSprites();
    BuildOamBuffer();
    do_scheduled_bg_tilemap_copies_to_vram();
}

static void SetFlyMapCallback(void callback(void))
{
    sFlyMap->callback = callback;
    sFlyMap->state = 0;
}

static void ShowHelpBar(void)
{
    const u8 color[3] = { 15, 1, 2 };

    FillWindowPixelBuffer(0, PIXEL_FILL(15));
    AddTextPrinterParameterized3(0, 0, 144, 0, color, 0, gText_DpadMove);

    if (sFlyMap->regionMap.primaryMapSecStatus == MAPSECTYPE_CITY_CANFLY || sFlyMap->regionMap.primaryMapSecStatus == MAPSECTYPE_BATTLE_FRONTIER)
    {
        AddTextPrinterParameterized3(0, 0, 192, 0, color, 0, gText_AOK);
    }
    else if (sFlyMap->regionMap.onButton)
    {
        AddTextPrinterParameterized3(0, 0, 192, 0, color, 0, gText_ACancel);
    }

    PutWindowTilemap(0);
    CopyWindowToVram(0, 3);
}

static void LoadFlyDestIcons(void)
{
    struct SpriteSheet sheet;

    LZ77UnCompWram(sFlyTargetIcons_Gfx, sFlyMap->tileBuffer);
    sheet.data = sFlyMap->tileBuffer;
    sheet.size = sizeof(sFlyMap->tileBuffer);
    sheet.tag = 5;
    LoadSpriteSheet(&sheet);
    CreateFlyDestIcons();
}

// Sprite data for SpriteCB_FlyDestIcon
#define sIconMapSec   data[0]
#define sFlickerTimer data[1]

static void CreateFlyDestIcons(void)
{
    u16 mapSecId;
    u16 x;
    u16 y;
    u8 spriteId;

    struct SpriteTemplate template = sFlyDestIconSpriteTemplate;
    template.paletteTag = sFlyMap->regionMap.miscSpritesPaletteTag;

    for (y = 0; y < MAP_HEIGHT; y++)
    {
        for (x = 0; x < MAP_WIDTH; x++)
        {
            if (GetMapsecType(GetMapSecIdAt(x, y, gRegionMap->currentRegion, FALSE)) == MAPSECTYPE_CITY_CANFLY)
            {
                spriteId = CreateSprite(&template, (x + MAPCURSOR_X_MIN + gRegionMap->xOffset) * 8 + 4, (y + MAPCURSOR_Y_MIN) * 8 + 4, 1);
            }
        }
    }
    /*for (mapSecId = MAPSEC_LITTLEROOT_TOWN; mapSecId <= MAPSEC_EVER_GRANDE_CITY; mapSecId++)
    {
        GetMapSecDimensions(mapSecId, &x, &y, &width, &height);
        x = (x + MAPCURSOR_X_MIN) * 8 + 4;
        y = (y + MAPCURSOR_Y_MIN) * 8 + 4;

        if (width == 2)
            shape = SPRITE_SHAPE(16x8);
        else if (height == 2)
            shape = SPRITE_SHAPE(8x16);
        else
            shape = SPRITE_SHAPE(8x8);

        spriteId = CreateSprite(&sFlyDestIconSpriteTemplate, x, y, 10);
        if (spriteId != MAX_SPRITES)
        {
            gSprites[spriteId].oam.shape = shape;

            if (FlagGet(canFlyFlag))
                gSprites[spriteId].callback = SpriteCB_FlyDestIcon;
            else
                shape += 3;

            StartSpriteAnim(&gSprites[spriteId], shape);
            gSprites[spriteId].sIconMapSec = mapSecId;
        }
        canFlyFlag++;
    }*/
}

// Draw a red outline box on the mapsec if its corresponding flag has been set
// Only used for Battle Frontier, but set up to handle more
static void TryCreateRedOutlineFlyDestIcons(void)
{
    u16 i;
    u16 x;
    u16 y;
    u16 width;
    u16 height;
    u16 mapSecId;
    u8 spriteId;

    for (i = 0; sRedOutlineFlyDestinations[i][1] != MAPSEC_NONE; i++)
    {
        if (FlagGet(sRedOutlineFlyDestinations[i][0]))
        {
            mapSecId = sRedOutlineFlyDestinations[i][1];
            GetMapSecDimensions(mapSecId, &x, &y, &width, &height);
            x = (x + MAPCURSOR_X_MIN) * 8;
            y = (y + MAPCURSOR_Y_MIN) * 8;
            spriteId = CreateSprite(&sFlyDestIconSpriteTemplate, x, y, 10);
            if (spriteId != MAX_SPRITES)
            {
                gSprites[spriteId].oam.size = SPRITE_SIZE(16x16);
                gSprites[spriteId].callback = SpriteCB_FlyDestIcon;
                StartSpriteAnim(&gSprites[spriteId], FLYDESTICON_RED_OUTLINE);
                gSprites[spriteId].sIconMapSec = mapSecId;
            }
        }
    }
}

// Flickers fly destination icon color (by hiding the fly icon sprite) if the cursor is currently on it
static void SpriteCB_FlyDestIcon(struct Sprite *sprite)
{
    if (sFlyMap->regionMap.primaryMapSecId == sprite->sIconMapSec)
    {
        if (++sprite->sFlickerTimer > 16)
        {
            sprite->sFlickerTimer = 0;
            sprite->invisible = sprite->invisible ? FALSE : TRUE;
        }
    }
    else
    {
        sprite->sFlickerTimer = 16;
        sprite->invisible = FALSE;
    }
}

#undef sIconMapSec
#undef sFlickerTimer

static void CB_FadeInFlyMap(void)
{
    switch (sFlyMap->state)
    {
    case 0:
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
        sFlyMap->state++;
        break;
    case 1:
        if (!UpdatePaletteFade())
        {
            SetFlyMapCallback(CB_HandleFlyMapInput);
        }
        break;
    }
}

static void CB_HandleFlyMapInput(void)
{
    if (sFlyMap->state == 0)
    {
        switch (DoRegionMapInputCallback())
        {
            case MAP_INPUT_NONE:
            case MAP_INPUT_MOVE_START:
            case MAP_INPUT_MOVE_CONT:
                break;
            case MAP_INPUT_ON_BUTTON:
                sFlyMap->regionMap.onButton = TRUE;
                ShowHelpBar();
                break;
            case MAP_INPUT_MOVE_END:
                sFlyMap->regionMap.onButton = FALSE;
                if (sFlyMap->regionMap.primaryMapSecStatus == MAPSECTYPE_CITY_CANFLY || sFlyMap->regionMap.primaryMapSecStatus == MAPSECTYPE_BATTLE_FRONTIER)
                {
                    m4aSongNumStart(SE_Z_PAGE);
                }
                ShowHelpBar();
                break;
            case MAP_INPUT_A_BUTTON:
                if (sFlyMap->regionMap.primaryMapSecStatus == MAPSECTYPE_CITY_CANFLY || sFlyMap->regionMap.primaryMapSecStatus == MAPSECTYPE_BATTLE_FRONTIER)
                {
                    m4aSongNumStart(SE_KAIFUKU);
                    sFlyMap->choseFlyLocation = TRUE;
                    SetFlyMapCallback(CB_ExitFlyMap);
                    break;
                }
                else if (!sFlyMap->regionMap.onButton)
                {
                    break;
                }
                m4aSongNumStart(SE_W063B);
            case MAP_INPUT_B_BUTTON:
                sFlyMap->choseFlyLocation = FALSE;
                SetFlyMapCallback(CB_ExitFlyMap);
                break;
        }
    }
}

static void CB_ExitFlyMap(void)
{
    switch (sFlyMap->state)
    {
    case 0:
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
        sFlyMap->state++;
        break;
    case 1:
        if (!UpdatePaletteFade())
        {
            FreeRegionMapResources();
            if (sFlyMap->choseFlyLocation)
            {
                switch (sFlyMap->regionMap.primaryMapSecId)
                {
                    case MAPSEC_SOUTHERN_ISLAND:
                        SetWarpDestinationToHealLocation(HEAL_LOCATION_SOUTHERN_ISLAND_EXTERIOR);
                        break;
                    case MAPSEC_BATTLE_FRONTIER:
                        SetWarpDestinationToHealLocation(HEAL_LOCATION_BATTLE_FRONTIER_OUTSIDE_EAST);
                        break;
                    case MAPSEC_EVER_GRANDE_CITY:
                        SetWarpDestinationToHealLocation(FlagGet(FLAG_LANDMARK_POKEMON_LEAGUE) && sFlyMap->regionMap.posWithinMapSec == 0 ? HEAL_LOCATION_EVER_GRANDE_CITY_POKEMON_LEAGUE : HEAL_LOCATION_EVER_GRANDE_CITY);
                        break;
                    default:
                        if (sMapHealLocations[sFlyMap->regionMap.primaryMapSecId][2] != 0)
                        {
                            SetWarpDestinationToHealLocation(sMapHealLocations[sFlyMap->regionMap.primaryMapSecId][2]);
                        }
                        else
                        {
                            SetWarpDestinationToMapWarp(sMapHealLocations[sFlyMap->regionMap.primaryMapSecId][0], sMapHealLocations[sFlyMap->regionMap.primaryMapSecId][1], -1);
                        }
                        break;
                }
                ReturnToFieldFromFlyMapSelect();
                TryEndBugCatchingContest();
            }
            else
            {
                SetMainCallback2(CB2_ReturnToPartyMenuFromFlyMap);
            }
            if (sFlyMap != NULL)
            {
                free(sFlyMap);
                sFlyMap = NULL;
            }
            FreeAllWindowBuffers();
        }
        break;
    }
}

u8 GetCurrentRegion(void)
{
    return sMapSecToRegion[gMapHeader.regionMapSectionId];
}
