const struct MonCoords gTrainerBackPicCoords[] =
{
    [TRAINER_BACK_PIC_GOLD] = {.size = 8, .y_offset = 5},
    [TRAINER_BACK_PIC_KRIS] = {.size = 8, .y_offset = 5},
    [TRAINER_BACK_PIC_RED] = {.size = 8, .y_offset = 5},
    [TRAINER_BACK_PIC_LEAF] = {.size = 8, .y_offset = 5},
    [TRAINER_BACK_PIC_RUBY_SAPPHIRE_BRENDAN] = {.size = 8, .y_offset = 4},
    [TRAINER_BACK_PIC_RUBY_SAPPHIRE_MAY] = {.size = 8, .y_offset = 4},
    [TRAINER_BACK_PIC_WALLY] = {.size = 8, .y_offset = 4},
    [TRAINER_BACK_PIC_STEVEN] = {.size = 8, .y_offset = 4},
    [TRAINER_BACK_PIC_EMERALD_BRENDAN] = {.size = 8, .y_offset = 4},
    [TRAINER_BACK_PIC_EMERALD_MAY] = {.size = 8, .y_offset = 4},
};

// this table goes functionally unused, since none of these pics are compressed
// and the place they would get extracted to gets overwritten later anyway
// the casts are so they'll play nice with the strict struct definition
const struct CompressedSpriteSheet gTrainerBackPicTable[] =
{
    (const u32 *)gTrainerBackPic_Gold, 0x2000, TRAINER_BACK_PIC_GOLD,
    (const u32 *)gTrainerBackPic_Kris, 0x2000, TRAINER_BACK_PIC_KRIS,
    (const u32 *)gTrainerBackPic_EmeraldBrendan, 0x2000, TRAINER_BACK_PIC_EMERALD_BRENDAN,
    (const u32 *)gTrainerBackPic_EmeraldMay, 0x2000, TRAINER_BACK_PIC_EMERALD_MAY,
    (const u32 *)gTrainerBackPic_Red, 0x2800, TRAINER_BACK_PIC_RED,
    (const u32 *)gTrainerBackPic_Leaf, 0x2800, TRAINER_BACK_PIC_LEAF,
    (const u32 *)gTrainerBackPic_RubySapphireBrendan, 0x2000, TRAINER_BACK_PIC_RUBY_SAPPHIRE_BRENDAN,
    (const u32 *)gTrainerBackPic_RubySapphireMay, 0x2000, TRAINER_BACK_PIC_RUBY_SAPPHIRE_MAY,
    (const u32 *)gTrainerBackPic_Wally, 0x2000, TRAINER_BACK_PIC_WALLY,
    (const u32 *)gTrainerBackPic_Steven, 0x2000, TRAINER_BACK_PIC_STEVEN,
};

const struct CompressedSpritePalette gTrainerBackPicPaletteTable[] =
{
    TRAINER_BACK_PAL(GOLD, gTrainerBackPicPalette_Gold),
    TRAINER_BACK_PAL(KRIS, gTrainerBackPicPalette_Kris),
    TRAINER_BACK_PAL(EMERALD_BRENDAN, gTrainerPalette_Brendan),
    TRAINER_BACK_PAL(EMERALD_MAY, gTrainerPalette_May),
    TRAINER_BACK_PAL(RED, gTrainerBackPicPalette_Red),
    TRAINER_BACK_PAL(LEAF, gTrainerBackPicPalette_Leaf),
    TRAINER_BACK_PAL(RUBY_SAPPHIRE_BRENDAN, gTrainerPalette_RubySapphireBrendan),
    TRAINER_BACK_PAL(RUBY_SAPPHIRE_MAY, gTrainerPalette_RubySapphireMay),
    TRAINER_BACK_PAL(WALLY, gTrainerPalette_Rival),
    TRAINER_BACK_PAL(STEVEN, gTrainerPalette_Steven),
};
