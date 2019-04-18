#include "global.h"
#include "battle_setup.h"
#include "data.h"
#include "event_data.h"
#include "phone_contact.h"
#include "string_util.h"
#include "battle.h"
#include "gym_leader_rematch.h"
#include "match_call.h"

// Static type declarations

typedef struct MatchCallTextDataStruct {
    const u8 *text;
    u16 flag;
    u16 flag2;
} match_call_text_data_t;

struct MatchCallStructCommon {
    u8 type;
    u8 v1;
    u16 flag;
};

struct MatchCallStruct0 {
    u8 type;
    u8 v1;
    u16 flag;
    const u8 *desc;
    const u8 *name;
    const match_call_text_data_t *textData;
};

struct MatchCallStruct1 {
    u8 type;
    u8 v1;
    u16 flag;
    u16 rematchTableIdx;
    const u8 *desc;
    const u8 *name;
    const match_call_text_data_t *textData;
};

struct MatchCallSubstruct2 {
    u16 flag;
    u8 v2;
};

struct MatchCallStruct2 {
    u8 type;
    u8 v1;
    u16 flag;
    u16 rematchTableIdx;
    const u8 *desc;
    const match_call_text_data_t *textData;
    const struct MatchCallSubstruct2 *v10;
};

struct MatchCallStruct3 {
    u8 type;
    u8 v1;
    u16 flag;
    const u8 *desc;
    const u8 *name;
};

struct MatchCallStruct4 {
    u8 type;
    u8 gender;
    u16 flag;
    const u8 *desc;
    const u8 *name;
    const match_call_text_data_t *textData;
};

// Note: Type1 and Type5 have identical struct layouts.
struct MatchCallStruct5 {
    u8 type;
    u8 v1;
    u16 flag;
    u16 rematchTableIdx;
    const u8 *desc;
    const u8 *name;
    const match_call_text_data_t *textData;
};

typedef union {
    const struct MatchCallStructCommon *common;
    const struct MatchCallStruct0 *type0;
    const struct MatchCallStruct1 *type1;
    const struct MatchCallStruct2 *type2;
    const struct MatchCallStruct3 *type3;
    const struct MatchCallStruct4 *type4;
    const struct MatchCallStruct5 *type5;
} match_call_t;

struct UnkStruct_08625388 {
    u16 idx;
    u16 v2;
    u16 v4;
    const u8 *v8[4];
};

static void sub_81D1920(const match_call_text_data_t *, u8 *);
static void sub_81D199C(const match_call_text_data_t *, u16, u8 *);
static void MatchCall_GetNameAndDescByRematchIdx(u32, const u8 **, const u8 **);

extern const u8 gText_MrStone_Pokenav_2B60C0[];
extern const u8 gText_MrStone_Pokenav_2B61E6[];
extern const u8 gText_MrStone_Pokenav_2B6302[];
extern const u8 gText_MrStone_Pokenav_2B63A0[];
extern const u8 gText_MrStone_Pokenav_2B64A2[];
extern const u8 gText_MrStone_Pokenav_2B6526[];
extern const u8 gText_MrStone_Pokenav_2B65BB[];
extern const u8 gText_MrStone_Pokenav_2B6664[];
extern const u8 gText_MrStone_Pokenav_2B66B1[];
extern const u8 gText_MrStone_Pokenav_2B6703[];
extern const u8 gText_MrStone_Pokenav_2B67ED[];

extern const u8 gMrStoneMatchCallDesc[];
extern const u8 gMrStoneMatchCallName[];

extern const u8 gText_Norman_Pokenav_2B5719[];
extern const u8 gText_Norman_Pokenav_2B5795[];
extern const u8 gText_Norman_Pokenav_2B584D[];
extern const u8 gText_Norman_Pokenav_2B58E3[];
extern const u8 gText_Norman_Pokenav_2B5979[];
extern const u8 gText_Norman_Pokenav_2B5A07[];
extern const u8 gText_Norman_Pokenav_2B5A69[];
extern const u8 gText_Norman_Pokenav_2B5ACF[];
extern const u8 gText_Norman_Pokenav_2B5B5E[];

extern const u8 gNormanMatchCallDesc[];
extern const u8 gNormanMatchCallName[];

extern const u8 gProfBirchMatchCallDesc[];
extern const u8 gProfBirchMatchCallName[];

extern const u8 gText_Mom_Pokenav_2B227B[];
extern const u8 gText_Mom_Pokenav_2B2310[];
extern const u8 gText_Mom_Pokenav_2B23F3[];

extern const u8 gMomMatchCallDesc[];
extern const u8 gMomMatchCallName[];

extern const u8 gText_Steven_Pokenav_2B5B95[];
extern const u8 gText_Steven_Pokenav_2B5C53[];
extern const u8 gText_Steven_Pokenav_2B5CC9[];
extern const u8 gText_Steven_Pokenav_2B5DB4[];
extern const u8 gText_Steven_Pokenav_2B5E26[];
extern const u8 gText_Steven_Pokenav_2B5EA2[];
extern const u8 gText_Steven_Pokenav_2B5ED9[];

extern const u8 gStevenMatchCallDesc[];
extern const u8 gStevenMatchCallName[];

extern const u8 gText_May_Pokenav_2B3AB3[];
extern const u8 gText_May_Pokenav_2B3B3F[];
extern const u8 gText_May_Pokenav_2B3C13[];
extern const u8 gText_May_Pokenav_2B3CF3[];
extern const u8 gText_May_Pokenav_2B3D4B[];
extern const u8 gText_May_Pokenav_2B3DD1[];
extern const u8 gText_May_Pokenav_2B3E69[];
extern const u8 gText_May_Pokenav_2B3ECD[];
extern const u8 gText_May_Pokenav_2B3F2B[];
extern const u8 gText_May_Pokenav_2B3FFB[];
extern const u8 gText_May_Pokenav_2B402B[];
extern const u8 gText_May_Pokenav_2B414B[];
extern const u8 gText_May_Pokenav_2B4228[];
extern const u8 gText_May_Pokenav_2B42E0[];
extern const u8 gText_May_Pokenav_2B4350[];
extern const u8 gMayBrendanMatchCallDesc[];
extern const u8 gExpandedPlaceholder_May[];
extern const u8 gText_Brendan_Pokenav_2B43EF[];
extern const u8 gText_Brendan_Pokenav_2B4486[];
extern const u8 gText_Brendan_Pokenav_2B4560[];
extern const u8 gText_Brendan_Pokenav_2B463F[];
extern const u8 gText_Brendan_Pokenav_2B46B7[];
extern const u8 gText_Brendan_Pokenav_2B4761[];
extern const u8 gText_Brendan_Pokenav_2B47F4[];
extern const u8 gText_Brendan_Pokenav_2B4882[];
extern const u8 gText_Brendan_Pokenav_2B4909[];
extern const u8 gText_Brendan_Pokenav_2B49C4[];
extern const u8 gText_Brendan_Pokenav_2B4A44[];
extern const u8 gText_Brendan_Pokenav_2B4B28[];
extern const u8 gText_Brendan_Pokenav_2B4C15[];
extern const u8 gText_Brendan_Pokenav_2B4CD8[];
extern const u8 gText_Brendan_Pokenav_2B4D46[];
extern const u8 gExpandedPlaceholder_Brendan[];
extern const u8 gText_Wally_Pokenav_2B4DE2[];
extern const u8 gText_Wally_Pokenav_2B4E57[];
extern const u8 gText_Wally_Pokenav_2B4EA5[];
extern const u8 gText_Wally_Pokenav_2B4F41[];
extern const u8 gText_Wally_Pokenav_2B4FF3[];
extern const u8 gText_Wally_Pokenav_2B50B1[];
extern const u8 gText_Wally_Pokenav_2B5100[];
extern const u8 gWallyMatchCallDesc[];
extern const u8 gText_Scott_Pokenav_2B5184[];
extern const u8 gText_Scott_Pokenav_2B5275[];
extern const u8 gText_Scott_Pokenav_2B5323[];
extern const u8 gText_Scott_Pokenav_2B53DB[];
extern const u8 gText_Scott_Pokenav_2B54A5[];
extern const u8 gText_Scott_Pokenav_2B5541[];
extern const u8 gText_Scott_Pokenav_2B56CA[];
extern const u8 gScottMatchCallDesc[];
extern const u8 gScottMatchCallName[];
extern const u8 gText_Roxanne_Pokenav_2B2456[];
extern const u8 gText_Roxanne_Pokenav_2B250E[];
extern const u8 gText_Roxanne_Pokenav_2B25C1[];
extern const u8 gText_Roxanne_Pokenav_2B2607[];
extern const u8 gRoxanneMatchCallDesc[];
extern const u8 gText_Brawly_Pokenav_2B2659[];
extern const u8 gText_Brawly_Pokenav_2B275D[];
extern const u8 gText_Brawly_Pokenav_2B286F[];
extern const u8 gText_Brawly_Pokenav_2B28D1[];
extern const u8 gBrawlyMatchCallDesc[];
extern const u8 gText_Wattson_Pokenav_2B2912[];
extern const u8 gText_Wattson_Pokenav_2B29CA[];
extern const u8 gText_Wattson_Pokenav_2B2AB6[];
extern const u8 gText_Wattson_Pokenav_2B2B01[];
extern const u8 gWattsonMatchCallDesc[];
extern const u8 gText_Flannery_Pokenav_2B2B4D[];
extern const u8 gText_Flannery_Pokenav_2B2C0E[];
extern const u8 gText_Flannery_Pokenav_2B2CF1[];
extern const u8 gText_Flannery_Pokenav_2B2D54[];
extern const u8 gFlanneryMatchCallDesc[];
extern const u8 gText_Winona_Pokenav_2B2DA4[];
extern const u8 gText_Winona_Pokenav_2B2E2B[];
extern const u8 gText_Winona_Pokenav_2B2EC2[];
extern const u8 gText_Winona_Pokenav_2B2F16[];
extern const u8 gWinonaMatchCallDesc[];
extern const u8 gText_TateLiza_Pokenav_2B2F97[];
extern const u8 gText_TateLiza_Pokenav_2B306E[];
extern const u8 gText_TateLiza_Pokenav_2B3158[];
extern const u8 gText_TateLiza_Pokenav_2B31CD[];
extern const u8 gTateLizaMatchCallDesc[];
extern const u8 gText_Juan_Pokenav_2B3249[];
extern const u8 gText_Juan_Pokenav_2B32EC[];
extern const u8 gText_Juan_Pokenav_2B33AA[];
extern const u8 gText_Juan_Pokenav_2B341E[];
extern const u8 gJuanMatchCallDesc[];
extern const u8 gText_Sidney_Pokenav_2B34CC[];
extern const u8 gEliteFourMatchCallDesc[];
extern const u8 gText_Phoebe_Pokenav_2B3561[];
extern const u8 gText_Glacia_Pokenav_2B35E4[];
extern const u8 gText_Drake_Pokenav_2B368B[];
extern const u8 gText_Wallace_Pokenav_2B3790[];
extern const u8 gChampionMatchCallDesc[];
extern const u8 gMatchCallStevenStrategyText[];
extern const u8 gMatchCall_StevenTrainersPokemonText[];
extern const u8 gMatchCall_StevenSelfIntroductionText_Line1_BeforeMeteorFallsBattle[];
extern const u8 gMatchCall_StevenSelfIntroductionText_Line2_BeforeMeteorFallsBattle[];
extern const u8 gMatchCall_StevenSelfIntroductionText_Line1_AfterMeteorFallsBattle[];
extern const u8 gMatchCall_StevenSelfIntroductionText_Line2_AfterMeteorFallsBattle[];
extern const u8 gMatchCall_BrendanStrategyText[];
extern const u8 gMatchCall_BrendanTrainersPokemonText[];
extern const u8 gMatchCall_BrendanSelfIntroductionText_Line1[];
extern const u8 gMatchCall_BrendanSelfIntroductionText_Line2[];
extern const u8 gMatchCall_MayStrategyText[];
extern const u8 gMatchCall_MayTrainersPokemonText[];
extern const u8 gMatchCall_MaySelfIntroductionText_Line1[];
extern const u8 gMatchCall_MaySelfIntroductionText_Line2[];
// .rodata

static const match_call_text_data_t sMrStoneTextScripts[] = {
    { gText_MrStone_Pokenav_2B60C0, 0xFFFF,              FLAG_ENABLE_MR_STONE_POKENAV },
    { gText_MrStone_Pokenav_2B61E6, FLAG_ENABLE_MR_STONE_POKENAV,          0xFFFF },
    { gText_MrStone_Pokenav_2B6302, FLAG_DELIVERED_STEVEN_LETTER,          0xFFFF },
    { gText_MrStone_Pokenav_2B63A0, FLAG_RECEIVED_EXP_SHARE,          0xFFFF },
    { gText_MrStone_Pokenav_2B64A2, FLAG_RECEIVED_HM04,          0xFFFF },
    { gText_MrStone_Pokenav_2B6526, FLAG_DEFEATED_PETALBURG_GYM,          0xFFFF },
    { gText_MrStone_Pokenav_2B65BB, FLAG_RECEIVED_CASTFORM,          0xFFFF },
    { gText_MrStone_Pokenav_2B6664, FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT,          0xFFFF },
    { gText_MrStone_Pokenav_2B66B1, FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE,          0xFFFF },
    { gText_MrStone_Pokenav_2B6703, FLAG_DEFEATED_SOOTOPOLIS_GYM,          0xFFFF },
    { gText_MrStone_Pokenav_2B67ED, FLAG_SYS_GAME_CLEAR, 0xFFFF },
    { NULL,                         0xFFFF,              0xFFFF }
};

static const struct MatchCallStruct0 sMrStoneMatchCallHeader =
{
    .type = 0,
    .v1 = 10,
    .flag = 0xFFFF,
    .desc = gMrStoneMatchCallDesc,
    .name = gMrStoneMatchCallName,
    .textData = sMrStoneTextScripts
};

static const match_call_text_data_t sNormanTextScripts[] = {
    { gText_Norman_Pokenav_2B5719, FLAG_ENABLE_NORMAN_MATCH_CALL,           0xFFFF },
    { gText_Norman_Pokenav_2B5795, FLAG_DEFEATED_DEWFORD_GYM,           0xFFFF },
    { gText_Norman_Pokenav_2B584D, FLAG_DEFEATED_LAVARIDGE_GYM,           0xFFFF },
    { gText_Norman_Pokenav_2B58E3, FLAG_DEFEATED_PETALBURG_GYM,           0xFFFF },
    { gText_Norman_Pokenav_2B5979, FLAG_RECEIVED_RED_OR_BLUE_ORB,           0xFFFF },
    { gText_Norman_Pokenav_2B5A07, 0xFFFE,               0xFFFF },
    { gText_Norman_Pokenav_2B5A69, FLAG_SYS_GAME_CLEAR,  0xFFFF },
    { gText_Norman_Pokenav_2B5ACF, FLAG_SYS_GAME_CLEAR,  0xFFFF },
    { gText_Norman_Pokenav_2B5B5E, FLAG_SYS_GAME_CLEAR,  0xFFFF },
    { NULL,                        0xFFFF,               0xFFFF }
};

static const struct MatchCallStruct5 sNormanMatchCallHeader =
{
    .type = 5,
    .v1 = 7,
    .flag = FLAG_ENABLE_NORMAN_MATCH_CALL,
    .rematchTableIdx = REMATCH_NORMAN,
    .desc = gNormanMatchCallDesc,
    .name = gNormanMatchCallName,
    .textData = sNormanTextScripts
};

static const struct MatchCallStruct3 sProfBirchMatchCallHeader =
{
    .type = 3,
    .v1 = 0,
    .flag = FLAG_ENABLE_PROF_ELM_MATCH_CALL,
    .desc = gProfBirchMatchCallDesc,
    .name = gProfBirchMatchCallName
};

static const match_call_text_data_t sMomTextScripts[] = {
    { gText_Mom_Pokenav_2B227B, 0xffff,              0xffff },
    { gText_Mom_Pokenav_2B2310, FLAG_DEFEATED_PETALBURG_GYM,          0xffff },
    { gText_Mom_Pokenav_2B23F3, FLAG_SYS_GAME_CLEAR, 0xffff },
    { NULL,                     0xffff,              0xffff }
};

static const struct MatchCallStruct0 sMomMatchCallHeader =
{
    .type = 0,
    .v1 = 0,
    .flag = FLAG_ENABLE_MOM_MATCH_CALL,
    .desc = gMomMatchCallDesc,
    .name = gMomMatchCallName,
    .textData = sMomTextScripts
};

static const match_call_text_data_t sStevenTextScripts[] = {
    { gText_Steven_Pokenav_2B5B95, 0xffff,              0xffff },
    { gText_Steven_Pokenav_2B5C53, FLAG_RUSTURF_TUNNEL_OPENED,          0xffff },
    { gText_Steven_Pokenav_2B5CC9, FLAG_RECEIVED_RED_OR_BLUE_ORB,          0xffff },
    { gText_Steven_Pokenav_2B5DB4, FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE,          0xffff },
    { gText_Steven_Pokenav_2B5E26, FLAG_DEFEATED_MOSSDEEP_GYM,          0xffff },
    { gText_Steven_Pokenav_2B5EA2, FLAG_KYOGRE_ESCAPED_SEAFLOOR_CAVERN,          0xffff },
    { gText_Steven_Pokenav_2B5ED9, FLAG_SYS_GAME_CLEAR, 0xffff },
    { NULL,                        0xffff,              0xffff },
};

static const struct MatchCallStruct0 sStevenMatchCallHeader =
{
    .type = 0,
    .v1 = 0xD5,
    .flag = FLAG_REGISTERED_STEVEN_POKENAV,
    .desc = gStevenMatchCallDesc,
    .name = gStevenMatchCallName,
    .textData = sStevenTextScripts
};

static const match_call_text_data_t sMayTextScripts[] = {
    { gText_May_Pokenav_2B3AB3, 0xFFFF,              0xFFFF },
    { gText_May_Pokenav_2B3B3F, FLAG_DEFEATED_DEWFORD_GYM,          0xFFFF },
    { gText_May_Pokenav_2B3C13, FLAG_DELIVERED_DEVON_GOODS,          0xFFFF },
    { gText_May_Pokenav_2B3CF3, FLAG_HIDE_MAUVILLE_CITY_WALLY,          0xFFFF },
    { gText_May_Pokenav_2B3D4B, FLAG_RECEIVED_HM04,          0xFFFF },
    { gText_May_Pokenav_2B3DD1, FLAG_DEFEATED_LAVARIDGE_GYM,          0xFFFF },
    { gText_May_Pokenav_2B3E69, FLAG_DEFEATED_PETALBURG_GYM,          0xFFFF },
    { gText_May_Pokenav_2B3ECD, FLAG_RECEIVED_CASTFORM,          0xFFFF },
    { gText_May_Pokenav_2B3F2B, FLAG_RECEIVED_RED_OR_BLUE_ORB,          0xFFFF },
    { gText_May_Pokenav_2B3FFB, FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT,          0xFFFF },
    { gText_May_Pokenav_2B402B, FLAG_MET_TEAM_AQUA_HARBOR,          0xFFFF },
    { gText_May_Pokenav_2B414B, FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE,          0xFFFF },
    { gText_May_Pokenav_2B4228, FLAG_KYOGRE_ESCAPED_SEAFLOOR_CAVERN,          0xFFFF },
    { gText_May_Pokenav_2B42E0, FLAG_DEFEATED_SOOTOPOLIS_GYM,          0xFFFF },
    { gText_May_Pokenav_2B4350, FLAG_SYS_GAME_CLEAR, 0xFFFF },
    { NULL,                     0xFFFF,              0xFFFF }
};

static const struct MatchCallStruct4 sBrendanMatchCallHeader =
{
    .type = 4,
    .gender = MALE,
    .flag = FLAG_ENABLE_RIVAL_MATCH_CALL,
    .desc = gMayBrendanMatchCallDesc,
    .name = gExpandedPlaceholder_May,
    .textData = sMayTextScripts
};

static const match_call_text_data_t sBrendanTextScripts[] = {
    { gText_Brendan_Pokenav_2B43EF, 0xFFFF,              0xFFFF },
    { gText_Brendan_Pokenav_2B4486, FLAG_DEFEATED_DEWFORD_GYM,          0xFFFF },
    { gText_Brendan_Pokenav_2B4560, FLAG_DELIVERED_DEVON_GOODS,          0xFFFF },
    { gText_Brendan_Pokenav_2B463F, FLAG_HIDE_MAUVILLE_CITY_WALLY,          0xFFFF },
    { gText_Brendan_Pokenav_2B46B7, FLAG_RECEIVED_HM04,          0xFFFF },
    { gText_Brendan_Pokenav_2B4761, FLAG_DEFEATED_LAVARIDGE_GYM,          0xFFFF },
    { gText_Brendan_Pokenav_2B47F4, FLAG_DEFEATED_PETALBURG_GYM,          0xFFFF },
    { gText_Brendan_Pokenav_2B4882, FLAG_RECEIVED_CASTFORM,          0xFFFF },
    { gText_Brendan_Pokenav_2B4909, FLAG_RECEIVED_RED_OR_BLUE_ORB,          0xFFFF },
    { gText_Brendan_Pokenav_2B49C4, FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT,          0xFFFF },
    { gText_Brendan_Pokenav_2B4A44, FLAG_MET_TEAM_AQUA_HARBOR,          0xFFFF },
    { gText_Brendan_Pokenav_2B4B28, FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE,          0xFFFF },
    { gText_Brendan_Pokenav_2B4C15, FLAG_KYOGRE_ESCAPED_SEAFLOOR_CAVERN,          0xFFFF },
    { gText_Brendan_Pokenav_2B4CD8, FLAG_DEFEATED_SOOTOPOLIS_GYM,          0xFFFF },
    { gText_Brendan_Pokenav_2B4D46, FLAG_SYS_GAME_CLEAR, 0xFFFF },
    { NULL,                         0xFFFF,              0xFFFF }
};

static const struct MatchCallStruct4 sMayMatchCallHeader =
{
    .type = 4,
    .gender = FEMALE,
    .flag = FLAG_ENABLE_RIVAL_MATCH_CALL,
    .desc = gMayBrendanMatchCallDesc,
    .name = gExpandedPlaceholder_Brendan,
    .textData = sBrendanTextScripts
};

static const match_call_text_data_t sWallyTextScripts[] = {
    { gText_Wally_Pokenav_2B4DE2, 0xFFFF,     0xFFFF },
    { gText_Wally_Pokenav_2B4E57, FLAG_RUSTURF_TUNNEL_OPENED, 0xFFFF },
    { gText_Wally_Pokenav_2B4EA5, FLAG_DEFEATED_LAVARIDGE_GYM, 0xFFFF },
    { gText_Wally_Pokenav_2B4F41, FLAG_RECEIVED_CASTFORM, 0xFFFF },
    { gText_Wally_Pokenav_2B4FF3, FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT, 0xFFFF },
    { gText_Wally_Pokenav_2B50B1, FLAG_KYOGRE_ESCAPED_SEAFLOOR_CAVERN, 0xFFFF },
    { gText_Wally_Pokenav_2B5100, FLAG_DEFEATED_WALLY_VICTORY_ROAD, 0xFFFF },
    { NULL,                       0xFFFF,     0xFFFF }
};

const struct MatchCallSubstruct2 sWallyAdditionalData[] = {
    { FLAG_HIDE_MAUVILLE_CITY_WALLY, 0x05 },
    { FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT, 0xD5 },
    { FLAG_HIDE_VICTORY_ROAD_ENTRANCE_WALLY, 0x46 },
    { 0xFFFF,     0xD5 }
};

static const struct MatchCallStruct2 sWallyMatchCallHeader =
{
    .type = 2,
    .v1 = 0,
    .flag = FLAG_ENABLE_WALLY_MATCH_CALL,
    .rematchTableIdx = REMATCH_WALLY_3,
    .desc = gWallyMatchCallDesc,
    .textData = sWallyTextScripts,
    .v10 = sWallyAdditionalData
};

static const match_call_text_data_t sScottTextScripts[] = {
    { gText_Scott_Pokenav_2B5184, 0xFFFF,              0xFFFF },
    { gText_Scott_Pokenav_2B5275, FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY,          0xFFFF },
    { gText_Scott_Pokenav_2B5323, FLAG_RECEIVED_CASTFORM,          0xFFFF },
    { gText_Scott_Pokenav_2B53DB, FLAG_RECEIVED_RED_OR_BLUE_ORB,          0xFFFF },
    { gText_Scott_Pokenav_2B54A5, FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE,          0xFFFF },
    { gText_Scott_Pokenav_2B5541, FLAG_DEFEATED_SOOTOPOLIS_GYM,          0xFFFF },
    { gText_Scott_Pokenav_2B56CA, FLAG_SYS_GAME_CLEAR, 0xFFFF },
    { NULL,                       0xFFFF,              0xFFFF }
};


static const struct MatchCallStruct0 sScottMatchCallHeader =
{
    .type = 0,
    .v1 = 0xD5,
    .flag = FLAG_ENABLE_SCOTT_MATCH_CALL,
    .desc = gScottMatchCallDesc,
    .name = gScottMatchCallName,
    .textData = sScottTextScripts
};

static const match_call_text_data_t sRoxanneTextScripts[] = {
    { gText_Roxanne_Pokenav_2B2456, 0xFFFE,              0xFFFF },
    { gText_Roxanne_Pokenav_2B250E, 0xFFFF,              0xFFFF },
    { gText_Roxanne_Pokenav_2B25C1, 0xFFFF,              0xFFFF },
    { gText_Roxanne_Pokenav_2B2607, FLAG_SYS_GAME_CLEAR, 0xFFFF },
    { NULL,                         0xFFFF,              0xFFFF }
};

static const struct MatchCallStruct5 sRoxanneMatchCallHeader =
{
    .type = 5,
    .v1 = 10,
    .flag = FLAG_ENABLE_ROXANNE_MATCH_CALL,
    .rematchTableIdx = REMATCH_ROXANNE,
    .desc = gRoxanneMatchCallDesc,
    .name = NULL,
    .textData = sRoxanneTextScripts
};

static const match_call_text_data_t sBrawlyTextScripts[] = {
    { gText_Brawly_Pokenav_2B2659, 0xFFFE,              0xFFFF },
    { gText_Brawly_Pokenav_2B275D, 0xFFFF,              0xFFFF },
    { gText_Brawly_Pokenav_2B286F, 0xFFFF,              0xFFFF },
    { gText_Brawly_Pokenav_2B28D1, FLAG_SYS_GAME_CLEAR, 0xFFFF },
    { NULL,                        0xFFFF,              0xFFFF }
};

static const struct MatchCallStruct5 sBrawlyMatchCallHeader =
{
    .type = 5,
    .v1 = 2,
    .flag = FLAG_ENABLE_BRAWLY_MATCH_CALL,
    .rematchTableIdx = REMATCH_BRAWLY,
    .desc = gBrawlyMatchCallDesc,
    .name = NULL,
    .textData = sBrawlyTextScripts
};

static const match_call_text_data_t sWattsonTextScripts[] = {
    { gText_Wattson_Pokenav_2B2912, 0xFFFE,              0xFFFF },
    { gText_Wattson_Pokenav_2B29CA, 0xFFFF,              0xFFFF },
    { gText_Wattson_Pokenav_2B2AB6, 0xFFFF,              0xFFFF },
    { gText_Wattson_Pokenav_2B2B01, FLAG_SYS_GAME_CLEAR, 0xFFFF },
    { NULL,                         0xFFFF,              0xFFFF }
};

static const struct MatchCallStruct5 sWattsonMatchCallHeader =
{
    .type = 5,
    .v1 = 9,
    .flag = FLAG_ENABLE_WATTSON_MATCH_CALL,
    .rematchTableIdx = REMATCH_WATTSON,
    .desc = gWattsonMatchCallDesc,
    .name = NULL,
    .textData = sWattsonTextScripts
};

static const match_call_text_data_t sFlanneryTextScripts[] = {
    { gText_Flannery_Pokenav_2B2B4D, 0xFFFE,              0xFFFF },
    { gText_Flannery_Pokenav_2B2C0E, 0xFFFF,              0xFFFF },
    { gText_Flannery_Pokenav_2B2CF1, 0xFFFF,              0xFFFF },
    { gText_Flannery_Pokenav_2B2D54, FLAG_SYS_GAME_CLEAR, 0xFFFF },
    { NULL,                          0xFFFF,              0xFFFF }
};

static const struct MatchCallStruct5 sFlanneryMatchCallHeader =
{
    .type = 5,
    .v1 = 3,
    .flag = FLAG_ENABLE_FLANNERY_MATCH_CALL,
    .rematchTableIdx = REMATCH_FLANNERY,
    .desc = gFlanneryMatchCallDesc,
    .name = NULL,
    .textData = sFlanneryTextScripts
};

static const match_call_text_data_t sWinonaTextScripts[] = {
    { gText_Winona_Pokenav_2B2DA4, 0xFFFE,              0xFFFF },
    { gText_Winona_Pokenav_2B2E2B, 0xFFFF,              0xFFFF },
    { gText_Winona_Pokenav_2B2EC2, 0xFFFF,              0xFFFF },
    { gText_Winona_Pokenav_2B2F16, FLAG_SYS_GAME_CLEAR, 0xFFFF },
    { NULL,                        0xFFFF,              0xFFFF }
};

static const struct MatchCallStruct5 sWinonaMatchCallHeader =
{
    .type = 5,
    .v1 = 11,
    .flag = FLAG_ENABLE_WINONA_MATCH_CALL,
    .rematchTableIdx = REMATCH_WINONA,
    .desc = gWinonaMatchCallDesc,
    .name = NULL,
    .textData = sWinonaTextScripts
};

static const match_call_text_data_t sTateLizaTextScripts[] = {
    { gText_TateLiza_Pokenav_2B2F97, 0xFFFE,              0xFFFF },
    { gText_TateLiza_Pokenav_2B306E, 0xFFFF,              0xFFFF },
    { gText_TateLiza_Pokenav_2B3158, 0xFFFF,              0xFFFF },
    { gText_TateLiza_Pokenav_2B31CD, FLAG_SYS_GAME_CLEAR, 0xFFFF },
    { NULL,                          0xFFFF,              0xFFFF }
};

static const struct MatchCallStruct5 sTateLizaMatchCallHeader =
{
    .type = 5,
    .v1 = 13,
    .flag = FLAG_ENABLE_TATE_AND_LIZA_MATCH_CALL,
    .rematchTableIdx = REMATCH_TATE_AND_LIZA,
    .desc = gTateLizaMatchCallDesc,
    .name = NULL,
    .textData = sTateLizaTextScripts
};

static const match_call_text_data_t sJuanTextScripts[] = {
    { gText_Juan_Pokenav_2B3249, 0xFFFE,              0xFFFF },
    { gText_Juan_Pokenav_2B32EC, 0xFFFF,              0xFFFF },
    { gText_Juan_Pokenav_2B33AA, 0xFFFF,              0xFFFF },
    { gText_Juan_Pokenav_2B341E, FLAG_SYS_GAME_CLEAR, 0xFFFF },
    { NULL,                      0xFFFF,              0xFFFF }
};

static const struct MatchCallStruct5 sJuanMatchCallHeader =
{
    .type = 5,
    .v1 = 14,
    .flag = FLAG_ENABLE_JUAN_MATCH_CALL,
    .rematchTableIdx = REMATCH_JUAN,
    .desc = gJuanMatchCallDesc,
    .name = NULL,
    .textData = sJuanTextScripts
};

static const match_call_text_data_t sSidneyTextScripts[] = {
    { gText_Sidney_Pokenav_2B34CC, 0xFFFF, 0xFFFF },
    { NULL,                        0xFFFF, 0xFFFF }
};

static const struct MatchCallStruct5 sSidneyMatchCallHeader =
{
    .type = 5,
    .v1 = 15,
    .flag = FLAG_PHONE_CARD_SIDNEY,
    .rematchTableIdx = REMATCH_SIDNEY,
    .desc = gEliteFourMatchCallDesc,
    .name = NULL,
    .textData = sSidneyTextScripts
};

static const match_call_text_data_t sPhoebeTextScripts[] = {
    { gText_Phoebe_Pokenav_2B3561, 0xFFFF, 0xFFFF },
    { NULL,                        0xFFFF, 0xFFFF }
};

static const struct MatchCallStruct5 sPhoebeMatchCallHeader =
{
    .type = 5,
    .v1 = 15,
    .flag = FLAG_PHONE_CARD_PHOEBE,
    .rematchTableIdx = REMATCH_PHOEBE,
    .desc = gEliteFourMatchCallDesc,
    .name = NULL,
    .textData = sPhoebeTextScripts
};

static const match_call_text_data_t sGlaciaTextScripts[] = {
    { gText_Glacia_Pokenav_2B35E4, 0xFFFF, 0xFFFF },
    { NULL,                        0xFFFF, 0xFFFF }
};

static const struct MatchCallStruct5 sGlaciaMatchCallHeader =
{
    .type = 5,
    .v1 = 15,
    .flag = FLAG_PHONE_CARD_GLACIA,
    .rematchTableIdx = REMATCH_GLACIA,
    .desc = gEliteFourMatchCallDesc,
    .name = NULL,
    .textData = sGlaciaTextScripts
};

static const match_call_text_data_t sDrakeTextScripts[] = {
    { gText_Drake_Pokenav_2B368B, 0xFFFF, 0xFFFF },
    { NULL,                       0xFFFF, 0xFFFF }
};

static const struct MatchCallStruct5 sDrakeMatchCallHeader =
{
    .type = 5,
    .v1 = 15,
    .flag = FLAG_PHONE_CARD_DRAKE,
    .rematchTableIdx = REMATCH_DRAKE,
    .desc = gEliteFourMatchCallDesc,
    .name = NULL,
    .textData = sDrakeTextScripts
};

static const match_call_text_data_t sWallaceTextScripts[] = {
    { gText_Wallace_Pokenav_2B3790, 0xFFFF, 0xFFFF },
    { NULL,                         0xFFFF, 0xFFFF }
};

static const struct MatchCallStruct5 sWallaceMatchCallHeader =
{
    .type = 5,
    .v1 = 15,
    .flag = FLAG_PHONE_CARD_WALLACE,
    .rematchTableIdx = REMATCH_WALLACE,
    .desc = gChampionMatchCallDesc,
    .name = NULL,
    .textData = sWallaceTextScripts
};

static const match_call_t sMatchCallHeaders[] = {
    {.type0 = &sMrStoneMatchCallHeader},
    {.type3 = &sProfBirchMatchCallHeader},
    {.type4 = &sMayMatchCallHeader},
    {.type4 = &sBrendanMatchCallHeader},
    {.type2 = &sWallyMatchCallHeader},
    {.type5 = &sNormanMatchCallHeader},
    {.type0 = &sMomMatchCallHeader},
    {.type0 = &sStevenMatchCallHeader},
    {.type0 = &sScottMatchCallHeader},
    {.type5 = &sRoxanneMatchCallHeader},
    {.type5 = &sBrawlyMatchCallHeader},
    {.type5 = &sWattsonMatchCallHeader},
    {.type5 = &sFlanneryMatchCallHeader},
    {.type5 = &sWinonaMatchCallHeader},
    {.type5 = &sTateLizaMatchCallHeader},
    {.type5 = &sJuanMatchCallHeader},
    {.type5 = &sSidneyMatchCallHeader},
    {.type5 = &sPhoebeMatchCallHeader},
    {.type5 = &sGlaciaMatchCallHeader},
    {.type5 = &sDrakeMatchCallHeader},
    {.type5 = &sWallaceMatchCallHeader}
};

static const struct UnkStruct_08625388 sMatchCallCheckPageOverrides[] = {
    { 7, 0x4B, 0xffff, { gMatchCallStevenStrategyText, gMatchCall_StevenTrainersPokemonText, gMatchCall_StevenSelfIntroductionText_Line1_BeforeMeteorFallsBattle, gMatchCall_StevenSelfIntroductionText_Line2_BeforeMeteorFallsBattle } }, // STEVEN
    { 7, 0x4B, FLAG_DEFEATED_MOSSDEEP_GYM, { gMatchCallStevenStrategyText, gMatchCall_StevenTrainersPokemonText, gMatchCall_StevenSelfIntroductionText_Line1_AfterMeteorFallsBattle, gMatchCall_StevenSelfIntroductionText_Line2_AfterMeteorFallsBattle } }, // STEVEN
    { 2, 0x3c, 0xffff, { gMatchCall_BrendanStrategyText, gMatchCall_BrendanTrainersPokemonText, gMatchCall_BrendanSelfIntroductionText_Line1, gMatchCall_BrendanSelfIntroductionText_Line2 } }, // Brendan
    { 3, 0x3f, 0xffff, { gMatchCall_MayStrategyText, gMatchCall_MayTrainersPokemonText, gMatchCall_MaySelfIntroductionText_Line1, gMatchCall_MaySelfIntroductionText_Line2 } } // May
};

// .text

static u32 MatchCallGetFunctionIndex(match_call_t matchCall)
{
    switch (matchCall.common->type)
    {
        case 0:
        default:
            return 0;
        case 1:
        case 5:
            return 1;
        case 2:
            return 2;
        case 4:
            return 3;
        case 3:
            return 4;
    }
}

u32 GetTrainerIdxByRematchIdx(u32 rematchIdx)
{
    return gRematchTable[rematchIdx].trainerIds[0];
}

s32 GetRematchIdxByTrainerIdx(s32 trainerIdx)
{
    s32 rematchIdx;

    for (rematchIdx = 0; rematchIdx < REMATCH_TABLE_ENTRIES; rematchIdx++)
    {
        if (gRematchTable[rematchIdx].trainerIds[0] == trainerIdx)
            return rematchIdx;
    }
    return -1;
}

bool32 MatchCallFlagGetByIndex(u32 idx)
{
    return FALSE;
}

u8 sub_81D16DC(u32 idx)
{
    return FALSE;
}

bool32 MatchCall_IsRematchable(u32 idx)
{
    return FALSE;
}

bool32 sub_81D17E8(u32 idx)
{
    return FALSE;
}

u32 MatchCall_GetRematchTableIdx(u32 idx)
{
    return REMATCH_TABLE_ENTRIES;
}

void MatchCall_GetMessage(u32 idx, u8 *dest)
{
    return;
}

static void sub_81D1920(const match_call_text_data_t *sub0, u8 *dest)
{
    u32 i;
    for (i = 0; sub0[i].text != NULL; i++)
        ;
    if (i)
        i--;
    while (i)
    {
        if (sub0[i].flag != 0xffff && FlagGet(sub0[i].flag) == TRUE)
            break;
        i--;
    }
    if (sub0[i].flag2 != 0xffff)
        FlagSet(sub0[i].flag2);
    StringExpandPlaceholders(dest, sub0[i].text);
}

static void sub_81D199C(const match_call_text_data_t *sub0, u16 idx, u8 *dest)
{
    u32 i;
    for (i = 0; sub0[i].text != NULL; i++)
    {
        if (sub0[i].flag == 0xfffe)
            break;
        if (sub0[i].flag != 0xffff && !FlagGet(sub0[i].flag))
            break;
    }
    if (sub0[i].flag != 0xfffe)
    {
        if (i)
            i--;
        if (sub0[i].flag2 != 0xffff)
            FlagSet(sub0[i].flag2);
        StringExpandPlaceholders(dest, sub0[i].text);
    }
    else
    {
        if (FlagGet(FLAG_SYS_GAME_CLEAR))
        {
            do
            {
                if (gSaveBlock1Ptr->trainerRematches[idx])
                    i += 2;
                else if (CountBattledRematchTeams(idx) >= 2)
                    i += 3;
                else
                    i++;
            } while (0);
        }

        StringExpandPlaceholders(dest, sub0[i].text);
    }
}

void sub_81D1A78(u32 idx, const u8 **desc, const u8 **name)
{
    return;
}

static void MatchCall_GetNameAndDescByRematchIdx(u32 idx, const u8 **desc, const u8 **name)
{
    const struct Trainer *trainer = gTrainers + GetTrainerIdxByRematchIdx(idx);
    *desc = gTrainerClassNames[trainer->trainerClass];
    *name = trainer->trainerName;
}

const u8 *sub_81D1B40(u32 idx, u32 offset)
{
    return NULL;
}

int sub_81D1BD0(u32 idx)
{
    return -1;
}

bool32 sub_81D1BF8(u32 idx)
{
    int i;

    for (i = 0; i < (int)ARRAY_COUNT(sMatchCallHeaders); i++)
    {
        u32 r0 = MatchCall_GetRematchTableIdx(i);
        if (r0 != REMATCH_TABLE_ENTRIES && r0 == idx)
            return TRUE;
    }
    return FALSE;
}

void SetMatchCallRegisteredFlag(void)
{
    int r0 = GetRematchIdxByTrainerIdx(gSpecialVar_0x8004);
    if (r0 >= 0)
        FlagSet(gPhoneContacts[gRematchTable[r0].phoneContactId].registeredFlag);
}
