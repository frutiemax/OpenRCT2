/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include "../../../sprites.h"
#include "../../RideData.h"
#include "../../ShopItem.h"
#include "../../Track.h"
#include "../../TrackData.h"

// clang-format off
#define TRACK_BLOCK_END { 255, 255, 255, 255, 255, {255, 255}, 255 }
static constexpr const rct_preview_track TrackBlocksFlatTrack8x3[] = {
    { 0, -112, -32, 0, 0, { 0b1111, 0 }, 0 },
    { 1, -112, 0, 0, 0, { 0b1111, 0 }, 0 },
    { 2, -112, 32, 0, 0, { 0b1111, 0 }, 0 },

    { 3, -80, -32, 0, 0, { 0b1111, 0 }, 0 },
    { 4, -80, 0, 0, 0, { 0b1111, 0 }, 0 },
    { 5, -80, 32, 0, 0, { 0b1111, 0 }, 0 },

    { 6, -48, -32, 0, 0, { 0b1111, 0 }, 0 },
    { 7, -48, 0, 0, 0, { 0b1111, 0 }, 0 },
    { 8, -48, 32, 0, 0, { 0b1111, 0 }, 0 },

    { 9, -16, -32, 0, 0, { 0b1111, 0 }, 0 },
    { 10, -16, 0, 0, 0, { 0b1111, 0 }, 0 },
    { 11, -16, 32, 0, 0, { 0b1111, 0 }, 0 },

    { 12, 16, -32, 0, 0, { 0b1111, 0 }, 0 },
    { 13, 16, 0, 0, 0, { 0b1111, 0 }, 0 },
    { 14, 16, 32, 0, 0, { 0b1111, 0 }, 0 },

    { 15, 48, -32, 0, 0, { 0b1111, 0 }, 0 },
    { 16, 48, 0, 0, 0, { 0b1111, 0 }, 0 },
    { 17, 48, 32, 0, 0, { 0b1111, 0 }, 0 },

    { 18, 80, -32, 0, 0, { 0b1111, 0 }, 0 },
    { 19, 80, 0, 0, 0, { 0b1111, 0 }, 0 },
    { 20, 80, 32, 0, 0, { 0b1111, 0 }, 0 },

    { 21, 112, -32, 0, 0, { 0b1111, 0 }, 0 },
    { 22, 112, 0, 0, 0, { 0b1111, 0 }, 0 },
    { 23, 112, 32, 0, 0, { 0b1111, 0 }, 0 },
    TRACK_BLOCK_END
};

constexpr const TrackElementDescriptor TestTED = {
    SET_FIELD(Description, STR_EMPTY),
    SET_FIELD(Coordinates, {0,0,0,0,0,0}),
    SET_FIELD(Block, const_cast<rct_preview_track*>(TrackBlocksFlatTrack8x3)),
    SET_FIELD(PieceLength, 0),
    SET_FIELD(CurveChain, { TRACK_CURVE_NONE, TRACK_CURVE_NONE }),
    SET_FIELD(AlternativeType, TrackElemType::None),
    SET_FIELD(Price, 0),
    SET_FIELD(MirorElement, TrackElemType::GiantFerrisWheelBase),
    SET_FIELD(HeightMarkerPositions, (1 << 0)),
    SET_FIELD(Flags, 0),
    SET_FIELD(SequenceElementAllowedWallEdges, {
        0, 0, 0,
        0, 0, 0,
        0, 0, 0,
        0, 0, 0,
        0, 0, 0,
        0, 0, 0,
        0, 0, 0,
        0, 0, 0}),
    SET_FIELD(SequenceProperties, {
        0, 0, 0,
        0, 0, 0,
        0, 0, 0,
        0, 0, 0,
        0, 0, 0,
        0, 0, 0,
        0, 0, 0,
        0, 0, 0}),
    SET_FIELD(Definition, { TRACK_FLAT, TRACK_SLOPE_NONE, TRACK_SLOPE_NONE, TRACK_BANK_NONE, TRACK_BANK_NONE, 0}),
    SET_FIELD(SpinFunction, NO_SPIN)
};

constexpr const RideTypeDescriptor GiantFerrisWheelRTD =
{
    SET_FIELD(AlternateType, RIDE_TYPE_NULL),
    SET_FIELD(Category, RIDE_CATEGORY_GENTLE),
    SET_FIELD(EnabledTrackPieces, 0),
    SET_FIELD(ExtraTrackPieces, 0),
    SET_FIELD(CoveredTrackPieces, 0),
    SET_FIELD(StartTrackPiece, TrackElemType::GiantFerrisWheelBase),
    SET_FIELD(TrackPaintFunction, GiantFerrisWheel::GetTrackPaintFunction),
    SET_FIELD(Flags, RIDE_TYPE_FLAG_HAS_SINGLE_PIECE_STATION | RIDE_TYPE_FLAG_CANNOT_HAVE_GAPS | RIDE_TYPE_FLAG_HAS_LOAD_OPTIONS |
                     RIDE_TYPE_FLAG_HAS_NO_TRACK | RIDE_TYPE_FLAG_VEHICLE_IS_INTEGRAL | RIDE_TYPE_FLAG_TRACK_NO_WALLS | RIDE_TYPE_FLAG_FLAT_RIDE |
                     RIDE_TYPE_FLAG_IN_RIDE | RIDE_TYPE_FLAG_HAS_VEHICLE_COLOURS | RIDE_TYPE_FLAG_MUSIC_ON_DEFAULT |
                     RIDE_TYPE_FLAG_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG_SINGLE_SESSION | RIDE_TYPE_FLAG_LIST_VEHICLES_SEPARATELY),
    SET_FIELD(RideModes, EnumsToFlags(RideMode::Circus)),
    SET_FIELD(DefaultMode, RideMode::Circus),
    SET_FIELD(OperatingSettings, { 0, 0, 0, 0, 0, 0 }),
    SET_FIELD(Naming, { STR_RIDE_NAME_GIANT_FERRIS_WHEEL, STR_RIDE_DESCRIPTION_GIANT_FERRIS_WHEEL }),
    SET_FIELD(NameConvention, { RideComponentType::Building, RideComponentType::Structure, RideComponentType::Station }),
    SET_FIELD(EnumName, nameof(RIDE_TYPE_GIANT_FERRIS_WHEEL)),
    SET_FIELD(AvailableBreakdowns, (1 << BREAKDOWN_SAFETY_CUT_OUT)),
    SET_FIELD(Heights, { 12, 128, 3, 2, }),
    SET_FIELD(MaxMass, 255),
    SET_FIELD(LiftData, { OpenRCT2::Audio::SoundId::Null, 5, 5 }),
    SET_FIELD(RatingsCalculationFunction, ride_ratings_calculate_circus),
    SET_FIELD(RatingsMultipliers, { 20, 10, 0 }),
    SET_FIELD(UpkeepCosts, { 50, 1, 0, 0, 0, 0 }),
    SET_FIELD(BuildCosts, { 125, 2, 1, }),
    SET_FIELD(DefaultPrices, { 15, 0 }),
    SET_FIELD(DefaultMusic, {}),
    SET_FIELD(PhotoItem, ShopItem::Photo),
    SET_FIELD(BonusValue, 39),
    SET_FIELD(ColourPresets, DEFAULT_FLAT_RIDE_COLOUR_PRESET),
    SET_FIELD(ColourPreview, { 0, 0 }),
    SET_FIELD(ColourKey, RideColourKey::Ride),
};
// clang-format on
