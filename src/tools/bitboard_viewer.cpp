#include <iostream>
#include <cstdint>
#include <cassert>

using namespace std;

enum Square {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8
};

const uint64_t WHITE_PASSED_MASK[64] = {
    217020518514230016ull,    506381209866536704ull,
    1012762419733073408ull,    2025524839466146816ull,
    4051049678932293632ull,    8102099357864587264ull,
    16204198715729174528ull,    13889313184910721024ull,
    217020518514229248ull,    506381209866534912ull,
    1012762419733069824ull,    2025524839466139648ull,
    4051049678932279296ull,    8102099357864558592ull,
    16204198715729117184ull,    13889313184910671872ull,
    217020518514032640ull,    506381209866076160ull,
    1012762419732152320ull,    2025524839464304640ull,
    4051049678928609280ull,    8102099357857218560ull,
    16204198715714437120ull,    13889313184898088960ull,
    217020518463700992ull,    506381209748635648ull,
    1012762419497271296ull,    2025524838994542592ull,
    4051049677989085184ull,    8102099355978170368ull,
    16204198711956340736ull,    13889313181676863488ull,
    217020505578799104ull,    506381179683864576ull,
    1012762359367729152ull,    2025524718735458304ull,
    4051049437470916608ull,    8102098874941833216ull,
    16204197749883666432ull,    13889312357043142656ull,
    217017207043915776ull,    506373483102470144ull,
    1012746966204940288ull,    2025493932409880576ull,
    4050987864819761152ull,    8101975729639522304ull,
    16203951459279044608ull,    13889101250810609664ull,
    216172782113783808ull,    504403158265495552ull,
    1008806316530991104ull,    2017612633061982208ull,
    4035225266123964416ull,    8070450532247928832ull,
    16140901064495857664ull,    13835058055282163712ull,
    0ull,    0ull,
    0ull,    0ull,
    0ull,    0ull,
    0ull,    0ull
};

const uint64_t BLACK_PASSED_MASK[64] = {
    0ull,    0ull,
    0ull,    0ull,
    0ull,    0ull,
    0ull,    0ull,
    3ull,    7ull,
    14ull,    28ull,
    56ull,    112ull,
    224ull,    192ull,
    771ull,    1799ull,
    3598ull,    7196ull,
    14392ull,    28784ull,
    57568ull,    49344ull,
    197379ull,    460551ull,
    921102ull,    1842204ull,
    3684408ull,    7368816ull,
    14737632ull,    12632256ull,
    50529027ull,    117901063ull,
    235802126ull,    471604252ull,
    943208504ull,    1886417008ull,
    3772834016ull,    3233857728ull,
    12935430915ull,    30182672135ull,
    60365344270ull,    120730688540ull,
    241461377080ull,    482922754160ull,
    965845508320ull,    827867578560ull,
    3311470314243ull,    7726764066567ull,
    15453528133134ull,    30907056266268ull,
    61814112532536ull,    123628225065072ull,
    247256450130144ull,    211934100111552ull,
    847736400446211ull,    1978051601041159ull,
    3956103202082318ull,    7912206404164636ull,
    15824412808329272ull,    31648825616658544ull,
    63297651233317088ull,    54255129628557504ull
};

const uint64_t OUTER_2_SQ_RING_MASK[64] = {
    289363972639948800ull,    578729044791525376ull,
    1229798258109317120ull,    2459596516218634240ull,
    4919193032437268480ull,    9838386064874536960ull,
    1157688987024883712ull,    2315096499073056768ull,
    289360704169836544ull,    578721412634640384ull,
    1229782998090514432ull,    2459565996181028864ull,
    4919131992362057728ull,    9838263984724115456ull,
    1157443727212412928ull,    2314886354913198080ull,
    505533473516158976ull,    1083124541087023104ull,
    2238589255012057088ull,    4477178510024114176ull,
    8954357020048228352ull,    17908714040096456704ull,
    17298343833662128128ull,    16149943589319737344ull,
    1974740130922496ull,    4230955238621184ull,
    8744489277390848ull,    17488978554781696ull,
    34977957109563392ull,    69955914219126784ull,
    67571655600242688ull,    63085717145780224ull,
    7713828636416ull,    16527168900864ull,
    34158161239808ull,    68316322479616ull,
    136632644959232ull,    273265289918464ull,
    263951779688448ull,    246428582600704ull,
    30132143111ull,    64559253519ull,
    133430317343ull,    266860634686ull,
    533721269372ull,    1067442538744ull,
    1031061639408ull,    962611650784ull,
    117703684ull,    252184584ull,
    521212177ull,    1042424354ull,
    2084848708ull,    4169697416ull,
    4027584528ull,    3760201760ull,
    459780ull,    985096ull,
    2035985ull,    4071970ull,
    8143940ull,    16287880ull,
    15732752ull,    14688288ull
};

const uint64_t WHITE_AHEAD_MASK[64] = {
    72340172838076672ull,    144680345676153344ull,
    289360691352306688ull,    578721382704613376ull,
    1157442765409226752ull,    2314885530818453504ull,
    4629771061636907008ull,    9259542123273814016ull,
    72340172838076416ull,    144680345676152832ull,
    289360691352305664ull,    578721382704611328ull,
    1157442765409222656ull,    2314885530818445312ull,
    4629771061636890624ull,    9259542123273781248ull,
    72340172838010880ull,    144680345676021760ull,
    289360691352043520ull,    578721382704087040ull,
    1157442765408174080ull,    2314885530816348160ull,
    4629771061632696320ull,    9259542123265392640ull,
    72340172821233664ull,    144680345642467328ull,
    289360691284934656ull,    578721382569869312ull,
    1157442765139738624ull,    2314885530279477248ull,
    4629771060558954496ull,    9259542121117908992ull,
    72340168526266368ull,    144680337052532736ull,
    289360674105065472ull,    578721348210130944ull,
    1157442696420261888ull,    2314885392840523776ull,
    4629770785681047552ull,    9259541571362095104ull,
    72339069014638592ull,    144678138029277184ull,
    289356276058554368ull,    578712552117108736ull,
    1157425104234217472ull,    2314850208468434944ull,
    4629700416936869888ull,    9259400833873739776ull,
    72057594037927936ull,    144115188075855872ull,
    288230376151711744ull,    576460752303423488ull,
    1152921504606846976ull,    2305843009213693952ull,
    4611686018427387904ull,    9223372036854775808ull,
    0ull,    0ull,
    0ull,    0ull,
    0ull,    0ull,
    0ull,    0ull
};

const uint64_t BLACK_AHEAD_MASK[64] = {
    0ull,    0ull,
    0ull,    0ull,
    0ull,    0ull,
    0ull,    0ull,
    1ull,    2ull,
    4ull,    8ull,
    16ull,    32ull,
    64ull,    128ull,
    257ull,    514ull,
    1028ull,    2056ull,
    4112ull,    8224ull,
    16448ull,    32896ull,
    65793ull,    131586ull,
    263172ull,    526344ull,
    1052688ull,    2105376ull,
    4210752ull,    8421504ull,
    16843009ull,    33686018ull,
    67372036ull,    134744072ull,
    269488144ull,    538976288ull,
    1077952576ull,    2155905152ull,
    4311810305ull,    8623620610ull,
    17247241220ull,    34494482440ull,
    68988964880ull,    137977929760ull,
    275955859520ull,    551911719040ull,
    1103823438081ull,    2207646876162ull,
    4415293752324ull,    8830587504648ull,
    17661175009296ull,    35322350018592ull,
    70644700037184ull,    141289400074368ull,
    282578800148737ull,    565157600297474ull,
    1130315200594948ull,    2260630401189896ull,
    4521260802379792ull,    9042521604759584ull,
    18085043209519168ull,    36170086419038336ull
};

const uint64_t WHITE_FRONT_MASK[64] = {
    256ull,    512ull,
    1024ull,    2048ull,
    4096ull,    8192ull,
    16384ull,    32768ull,
    65536ull,    131072ull,
    262144ull,    524288ull,
    1048576ull,    2097152ull,
    4194304ull,    8388608ull,
    16777216ull,    33554432ull,
    67108864ull,    134217728ull,
    268435456ull,    536870912ull,
    1073741824ull,    2147483648ull,
    4294967296ull,    8589934592ull,
    17179869184ull,    34359738368ull,
    68719476736ull,    137438953472ull,
    274877906944ull,    549755813888ull,
    1099511627776ull,    2199023255552ull,
    4398046511104ull,    8796093022208ull,
    17592186044416ull,    35184372088832ull,
    70368744177664ull,    140737488355328ull,
    281474976710656ull,    562949953421312ull,
    1125899906842624ull,    2251799813685248ull,
    4503599627370496ull,    9007199254740992ull,
    18014398509481984ull,    36028797018963968ull,
    72057594037927936ull,    144115188075855872ull,
    288230376151711744ull,    576460752303423488ull,
    1152921504606846976ull,    2305843009213693952ull,
    4611686018427387904ull,    9223372036854775808ull,
    0ull,    0ull,
    0ull,    0ull,
    0ull,    0ull,
    0ull,    0ull
};

const uint64_t BLACK_FRONT_MASK[64] = {
    0ull,    0ull,
    0ull,    0ull,
    0ull,    0ull,
    0ull,    0ull,
    1ull,    2ull,
    4ull,    8ull,
    16ull,    32ull,
    64ull,    128ull,
    256ull,    512ull,
    1024ull,    2048ull,
    4096ull,    8192ull,
    16384ull,    32768ull,
    65536ull,    131072ull,
    262144ull,    524288ull,
    1048576ull,    2097152ull,
    4194304ull,    8388608ull,
    16777216ull,    33554432ull,
    67108864ull,    134217728ull,
    268435456ull,    536870912ull,
    1073741824ull,    2147483648ull,
    4294967296ull,    8589934592ull,
    17179869184ull,    34359738368ull,
    68719476736ull,    137438953472ull,
    274877906944ull,    549755813888ull,
    1099511627776ull,    2199023255552ull,
    4398046511104ull,    8796093022208ull,
    17592186044416ull,    35184372088832ull,
    70368744177664ull,    140737488355328ull,
    281474976710656ull,    562949953421312ull,
    1125899906842624ull,    2251799813685248ull,
    4503599627370496ull,    9007199254740992ull,
    18014398509481984ull,    36028797018963968ull
};

const uint64_t QUEENSIDE_HALF_MASK[64] = {
    17361641481138401520ull,    17361641481138401520ull,
    17361641481138401520ull,    17361641481138401520ull,
    1085102592571150095ull,    1085102592571150095ull,
    1085102592571150095ull,    1085102592571150095ull,
    17361641481138401520ull,    17361641481138401520ull,
    17361641481138401520ull,    17361641481138401520ull,
    1085102592571150095ull,    1085102592571150095ull,
    1085102592571150095ull,    1085102592571150095ull,
    17361641481138401520ull,    17361641481138401520ull,
    17361641481138401520ull,    17361641481138401520ull,
    1085102592571150095ull,    1085102592571150095ull,
    1085102592571150095ull,    1085102592571150095ull,
    17361641481138401520ull,    17361641481138401520ull,
    17361641481138401520ull,    17361641481138401520ull,
    1085102592571150095ull,    1085102592571150095ull,
    1085102592571150095ull,    1085102592571150095ull,
    17361641481138401520ull,    17361641481138401520ull,
    17361641481138401520ull,    17361641481138401520ull,
    1085102592571150095ull,    1085102592571150095ull,
    1085102592571150095ull,    1085102592571150095ull,
    17361641481138401520ull,    17361641481138401520ull,
    17361641481138401520ull,    17361641481138401520ull,
    1085102592571150095ull,    1085102592571150095ull,
    1085102592571150095ull,    1085102592571150095ull,
    17361641481138401520ull,    17361641481138401520ull,
    17361641481138401520ull,    17361641481138401520ull,
    1085102592571150095ull,    1085102592571150095ull,
    1085102592571150095ull,    1085102592571150095ull,
    17361641481138401520ull,    17361641481138401520ull,
    17361641481138401520ull,    17361641481138401520ull,
    1085102592571150095ull,    1085102592571150095ull,
    1085102592571150095ull,    1085102592571150095ull
};

const uint64_t LEFT_RIGHT_COLUMN_MASK[64] = {
    144680345676153346ull,    361700864190383365ull,
    723401728380766730ull,    1446803456761533460ull,
    2893606913523066920ull,    5787213827046133840ull,
    11574427654092267680ull,    4629771061636907072ull,
    144680345676153346ull,    361700864190383365ull,
    723401728380766730ull,    1446803456761533460ull,
    2893606913523066920ull,    5787213827046133840ull,
    11574427654092267680ull,    4629771061636907072ull,
    144680345676153346ull,    361700864190383365ull,
    723401728380766730ull,    1446803456761533460ull,
    2893606913523066920ull,    5787213827046133840ull,
    11574427654092267680ull,    4629771061636907072ull,
    144680345676153346ull,    361700864190383365ull,
    723401728380766730ull,    1446803456761533460ull,
    2893606913523066920ull,    5787213827046133840ull,
    11574427654092267680ull,    4629771061636907072ull,
    144680345676153346ull,    361700864190383365ull,
    723401728380766730ull,    1446803456761533460ull,
    2893606913523066920ull,    5787213827046133840ull,
    11574427654092267680ull,    4629771061636907072ull,
    144680345676153346ull,    361700864190383365ull,
    723401728380766730ull,    1446803456761533460ull,
    2893606913523066920ull,    5787213827046133840ull,
    11574427654092267680ull,    4629771061636907072ull,
    144680345676153346ull,    361700864190383365ull,
    723401728380766730ull,    1446803456761533460ull,
    2893606913523066920ull,    5787213827046133840ull,
    11574427654092267680ull,    4629771061636907072ull,
    144680345676153346ull,    361700864190383365ull,
    723401728380766730ull,    1446803456761533460ull,
    2893606913523066920ull,    5787213827046133840ull,
    11574427654092267680ull,    4629771061636907072ull
};

const uint64_t WHITE_LEFT_MASK[64] = {
    0ull,    1ull,
    2ull,    4ull,
    8ull,    16ull,
    32ull,    64ull,
    0ull,    256ull,
    512ull,    1024ull,
    2048ull,    4096ull,
    8192ull,    16384ull,
    0ull,    65536ull,
    131072ull,    262144ull,
    524288ull,    1048576ull,
    2097152ull,    4194304ull,
    0ull,    16777216ull,
    33554432ull,    67108864ull,
    134217728ull,    268435456ull,
    536870912ull,    1073741824ull,
    0ull,    4294967296ull,
    8589934592ull,    17179869184ull,
    34359738368ull,    68719476736ull,
    137438953472ull,    274877906944ull,
    0ull,    1099511627776ull,
    2199023255552ull,    4398046511104ull,
    8796093022208ull,    17592186044416ull,
    35184372088832ull,    70368744177664ull,
    0ull,    281474976710656ull,
    562949953421312ull,    1125899906842624ull,
    2251799813685248ull,    4503599627370496ull,
    9007199254740992ull,    18014398509481984ull,
    0ull,    72057594037927936ull,
    144115188075855872ull,    288230376151711744ull,
    576460752303423488ull,    1152921504606846976ull,
    2305843009213693952ull,    4611686018427387904ull
};

const uint64_t BLACK_LEFT_MASK[64] = {
    2ull,    4ull,
    8ull,    16ull,
    32ull,    64ull,
    128ull,    0ull,
    512ull,    1024ull,
    2048ull,    4096ull,
    8192ull,    16384ull,
    32768ull,    0ull,
    131072ull,    262144ull,
    524288ull,    1048576ull,
    2097152ull,    4194304ull,
    8388608ull,    0ull,
    33554432ull,    67108864ull,
    134217728ull,    268435456ull,
    536870912ull,    1073741824ull,
    2147483648ull,    0ull,
    8589934592ull,    17179869184ull,
    34359738368ull,    68719476736ull,
    137438953472ull,    274877906944ull,
    549755813888ull,    0ull,
    2199023255552ull,    4398046511104ull,
    8796093022208ull,    17592186044416ull,
    35184372088832ull,    70368744177664ull,
    140737488355328ull,    0ull,
    562949953421312ull,    1125899906842624ull,
    2251799813685248ull,    4503599627370496ull,
    9007199254740992ull,    18014398509481984ull,
    36028797018963968ull,    0ull,
    144115188075855872ull,    288230376151711744ull,
    576460752303423488ull,    1152921504606846976ull,
    2305843009213693952ull,    4611686018427387904ull,
    9223372036854775808ull,    0ull
};

// Helper: Set a bit
inline void set_bit(uint64_t& bb, int square) {
    bb |= (1ULL << square);
}

// Helper: Count no. set bits
inline int32_t count(uint64_t bb){
    return __builtin_popcountll(bb);
}

// Helper: LSB index
inline int lsb(uint64_t bb) {
    assert(bb != 0);
    return __builtin_ctzll(bb);
}

// Print bitboard in top-down format
void print_bitboard(uint64_t bb) {
    int board[64]{};
    while (bb) {
        int sq = lsb(bb);
        board[sq ^ 56] = 1; // Flip for top-down view
        bb &= bb - 1;
    }

    for (int i = 0; i < 64; ++i) {
        if (i % 8 == 0 && i != 0) cout << "\n";
        cout << board[i] << " ";
    }
    cout << "\n\n";
}

// Generate white passed pawn mask for a square
uint64_t white_passed_mask(int square) {
    int file = square % 8;
    int rank = square / 8;
    uint64_t mask = 0;

    for (int df = -1; df <= 1; ++df) {
        int f = file + df;
        if (f < 0 || f > 7) continue;
        for (int r = rank + 1; r < 8; ++r) {
            int sq = r * 8 + f;
            mask |= (1ULL << sq);
        }
    }
    return mask;
}

// Generate black passed pawn mask for a square
uint64_t black_passed_mask(int square) {
    int file = square % 8;
    int rank = square / 8;
    uint64_t mask = 0;

    for (int df = -1; df <= 1; ++df) {
        int f = file + df;
        if (f < 0 || f > 7) continue;
        for (int r = rank - 1; r >= 0; --r) {
            int sq = r * 8 + f;
            mask |= (1ULL << sq);
        }
    }
    return mask;
}

// Check if square is passed pawn for white
bool is_white_passed_pawn(int square, uint64_t black_pawns) {
    return (WHITE_PASSED_MASK[square] & black_pawns) == 0;
}

// Check if square is passed pawn for black
bool is_black_passed_pawn(int square, uint64_t white_pawns) {
    return (BLACK_PASSED_MASK[square] & white_pawns) == 0;
}

// Get passed pawns for white
uint64_t get_white_passed_pawns(uint64_t white_pawns, uint64_t black_pawns) {
    uint64_t result = 0;
    uint64_t bb = white_pawns;
    while (bb) {
        int sq = lsb(bb);
        if (is_white_passed_pawn(sq, black_pawns))
            result |= (1ULL << sq);
        bb &= bb - 1;
    }
    return result;
}

// Get passed pawns for black
uint64_t get_black_passed_pawns(uint64_t black_pawns, uint64_t white_pawns) {
    uint64_t result = 0;
    uint64_t bb = black_pawns;
    while (bb) {
        int sq = lsb(bb);
        if (is_black_passed_pawn(sq, white_pawns))
            result |= (1ULL << sq);
        bb &= bb - 1;
    }
    return result;
}

// Count passed pawns for white
int count_white_passed_pawns(uint64_t white_pawns, uint64_t black_pawns) {
    int count = 0;
    uint64_t bb = white_pawns;
    while (bb) {
        int sq = __builtin_ctzll(bb); // or lsb(bb)
        if (is_white_passed_pawn(sq, black_pawns))
            ++count;
        bb &= bb - 1; // Clear least significant bit
    }
    return count;
}

// Count passed pawns for black
int count_black_passed_pawns(uint64_t black_pawns, uint64_t white_pawns) {
    int count = 0;
    uint64_t bb = black_pawns;
    while (bb) {
        int sq = __builtin_ctzll(bb); // or lsb(bb)
        if (is_black_passed_pawn(sq, white_pawns))
            ++count;
        bb &= bb - 1;
    }
    return count;
}

// King outer ring mask
uint64_t outer_2_sq_ring_mask(int square) {
    int file = square % 8;
    int rank = square / 8;
    uint64_t mask = 0;

    for (int dr = -2; dr <= 2; ++dr) {
        for (int df = -2; df <= 2; ++df) {
            // Skip anything within the inner 3x3 square (including center)
            if (abs(dr) <= 1 && abs(df) <= 1) continue;

            int r = rank + dr;
            int f = file + df;
            if (r >= 0 && r < 8 && f >= 0 && f < 8) {
                int sq = r * 8 + f;
                mask |= (1ULL << sq);
            }
        }
    }

    return mask;
}

/*
uint64_t OUTER_2_SQ_RING_MASK[64];

void init_outer_2_sq_ring_masks() {
    for (int sq = 0; sq < 64; ++sq)
        OUTER_2_SQ_RING_MASK[sq^56] = outer_2_sq_ring_mask(sq);
}
        */



// Ahead mask for white pawn: all squares in front (same file)
uint64_t white_ahead_mask(int square) {
    int file = square % 8;
    int rank = square / 8;
    uint64_t mask = 0;
    for (int r = rank + 1; r < 8; ++r) {
        int sq = r * 8 + file;
        mask |= (1ULL << sq);
    }
    return mask;
}

// Ahead mask for black pawn: all squares in front (same file)
uint64_t black_ahead_mask(int square) {
    int file = square % 8;
    int rank = square / 8;
    uint64_t mask = 0;
    for (int r = rank - 1; r >= 0; --r) {
        int sq = r * 8 + file;
        mask |= (1ULL << sq);
    }
    return mask;
}

void generate_ahead_masks() {
    cout << "const uint64_t WHITE_AHEAD_MASK[64] = {\n";
    for (int i = 0; i < 64; ++i) {
        cout << "    " << white_ahead_mask(i) << "ull";
        if (i != 63) cout << ",";
        if (i % 2 == 1) cout << "\n";
    }
    cout << "};\n\n";

    cout << "const uint64_t BLACK_AHEAD_MASK[64] = {\n";
    for (int i = 0; i < 64; ++i) {
        cout << "    " << black_ahead_mask(i) << "ull";
        if (i != 63) cout << ",";
        if (i % 2 == 1) cout << "\n";
    }
    cout << "};\n";
}

// Square directly in front for white pawn
uint64_t white_front_mask(int square) {
    int file = square % 8;
    int rank = square / 8;
    if (rank == 7) return 0; // No square ahead on rank 8
    int sq = (rank + 1) * 8 + file;
    return (1ULL << sq);
}

// Square directly in front for black pawn
uint64_t black_front_mask(int square) {
    int file = square % 8;
    int rank = square / 8;
    if (rank == 0) return 0; // No square ahead on rank 1
    int sq = (rank - 1) * 8 + file;
    return (1ULL << sq);
}

void generate_front_masks() {
    cout << "const uint64_t WHITE_FRONT_MASK[64] = {\n";
    for (int i = 0; i < 64; ++i) {
        cout << "    " << white_front_mask(i) << "ull";
        if (i != 63) cout << ",";
        if (i % 2 == 1) cout << "\n";
    }
    cout << "};\n\n";

    cout << "const uint64_t BLACK_FRONT_MASK[64] = {\n";
    for (int i = 0; i < 64; ++i) {
        cout << "    " << black_front_mask(i) << "ull";
        if (i != 63) cout << ",";
        if (i % 2 == 1) cout << "\n";
    }
    cout << "};\n";
}

uint64_t generate_queenside_half_mask(int square) {
    int file = square % 8;
    uint64_t mask = 0;

    if (file <= 3) {
        // King on queenside: return full kingside
        for (int r = 0; r < 8; ++r) {
            for (int f = 4; f < 8; ++f) {
                mask |= (1ULL << (r * 8 + f));
            }
        }
    } else {
        // King on kingside: return full queenside
        for (int r = 0; r < 8; ++r) {
            for (int f = 0; f < 4; ++f) {
                mask |= (1ULL << (r * 8 + f));
            }
        }
    }

    return mask;
}

void generate_queenside_half_masks() {
    cout << "const uint64_t QUEENSIDE_HALF_MASK[64] = {\n";
    for (int i = 0; i < 64; ++i) {
        cout << "    " << generate_queenside_half_mask(i) << "ull";
        if (i != 63) cout << ",";
        if (i % 2 == 1) cout << "\n";
    }
    cout << "};\n";
}


uint64_t left_and_right_column_mask(int square) {
    int file = square % 8;
    uint64_t mask = 0;

    // Left file
    if (file > 0) {
        int left_file = file - 1;
        for (int r = 0; r < 8; ++r)
            mask |= (1ULL << (r * 8 + left_file));
    }

    // Right file
    if (file < 7) {
        int right_file = file + 1;
        for (int r = 0; r < 8; ++r)
            mask |= (1ULL << (r * 8 + right_file));
    }

    return mask;
}

void generate_left_and_right_column_masks() {
    cout << "const uint64_t LEFT_RIGHT_COLUMN_MASK[64] = {\n";
    for (int i = 0; i < 64; ++i) {
        cout << "    " << left_and_right_column_mask(i) << "ull";
        if (i != 63) cout << ",";
        if (i % 2 == 1) cout << "\n";
    }
    cout << "};\n";
}

uint64_t white_direct_left_mask(int square) {
    int file = square % 8;
    if (file == 0) return 0;
    return 1ULL << (square - 1);
}

uint64_t black_direct_left_mask(int square) {
    int file = square % 8;
    if (file == 7) return 0;
    return 1ULL << (square + 1);
}

void generate_white_black_left_masks() {
    cout << "const uint64_t WHITE_LEFT_MASK[64] = {\n";
    for (int i = 0; i < 64; ++i) {
        cout << "    " << white_direct_left_mask(i) << "ull";
        if (i != 63) cout << ",";
        if (i % 2 == 1) cout << "\n";
    }
    cout << "};\n\n";

    cout << "const uint64_t BLACK_LEFT_MASK[64] = {\n";
    for (int i = 0; i < 64; ++i) {
        cout << "    " << black_direct_left_mask(i) << "ull";
        if (i != 63) cout << ",";
        if (i % 2 == 1) cout << "\n";
    }
    cout << "};\n";
}


// Test
int main() {
    /*
    uint64_t white_pawns = 0;
    uint64_t black_pawns = 0;

    set_bit(white_pawns, A1); 
    set_bit(white_pawns, A2); 
    set_bit(white_pawns, B1); 
    set_bit(white_pawns, C1);
    set_bit(white_pawns, D1);

    set_bit(black_pawns, A8);
    set_bit(black_pawns, C8);
    set_bit(black_pawns, F8); 

    cout << "White pawns:\n";
    print_bitboard(white_pawns);

    cout << "Black pawns:\n";
    print_bitboard(black_pawns);

    uint64_t white_passed = get_white_passed_pawns(white_pawns, black_pawns);
    uint64_t black_passed = get_black_passed_pawns(black_pawns, white_pawns);

    cout << "White passed pawns:\n";
    print_bitboard(white_passed);
    cout << "No. white passed pawns\n";
    cout << count_white_passed_pawns(white_pawns, black_pawns) << "\n";

    cout << "Black passed pawns:\n";
    print_bitboard(black_passed);
    cout << count_black_passed_pawns(black_pawns, white_pawns) << "\n";
    */

    /*

    init_outer_2_sq_ring_masks();

    for (int sq = A1; sq <= H8; ++sq) {
        cout << "Outer 2x2 ring of square " << sq << ":\n";
        print_bitboard(OUTER_2_SQ_RING_MASK[sq]);
    }

    */
/*
    init_outer_2_sq_ring_masks();

    cout << "const uint64_t OUTER_2_SQ_RING_MASK[64] = {\n";
    for (int i = 0; i < 64; ++i) {
        cout << "    " << OUTER_2_SQ_RING_MASK[i] << "ull";
        if (i != 63) cout << ",";
        if (i % 2 == 1) cout << "\n";
    }
    cout << "};\n";
    */

    // generate_queenside_half_masks();
    // print_bitboard(QUEENSIDE_HALF_MASK[D8]);

    // print_bitboard(1ull << E4);

    // generate_left_and_right_column_masks();

    /*
    print_bitboard(LEFT_RIGHT_COLUMN_MASK[A1]);
    print_bitboard(LEFT_RIGHT_COLUMN_MASK[A8]);
    print_bitboard(LEFT_RIGHT_COLUMN_MASK[E3]);
    */

    // generate_white_black_left_masks();
    print_bitboard(WHITE_LEFT_MASK[A1]);
    print_bitboard(WHITE_LEFT_MASK[E4]);
    print_bitboard(BLACK_LEFT_MASK[A8]);
    print_bitboard(BLACK_LEFT_MASK[E5]);

    return 0;

}
