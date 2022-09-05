/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include "Firework.h"

#include "../Context.h"
#include "../Game.h"
#include "../drawing/Drawing.h"
#include "../entity/EntityRegistry.h"
#include "../object/FireworkObject.h"
#include "../object/Object.h"
#include "../object/ObjectManager.h"
#include "../object/ObjectRepository.h"
#include "../paint/Paint.h"
#include "../profiling/Profiling.h"
#include "FireworksSequence.h"

#include <algorithm>

static constexpr const int16_t TILE_OFFSET = 2;

Firework* Firework::Create(
    const TileCoordsXY& tile, const int32_t height, const std::string& objectId, const colour_t color1, const colour_t color2,
    const colour_t color3, const uint8_t forme)
{
    auto* firework = CreateEntity<Firework>();
    if (firework == nullptr)
        return nullptr;

    firework->_currentFrame = 0;
    firework->_color1 = color1;
    firework->_color2 = color2;
    firework->_color3 = color3;
    firework->forme = forme;

    // get the base image
    auto context = OpenRCT2::GetContext();
    auto& objectManager = context->GetObjectManager();

    auto object = objectManager.LoadObject(objectId);

    if (object)
    {
        auto fireworkObject = reinterpret_cast<FireworkObject*>(object);
        if (fireworkObject)
        {
            firework->_numFrames = fireworkObject->GetNumImages() / 4;
            firework->_baseImage = fireworkObject->GetFirework().Image;
            firework->sprite_height_negative = 128;
            firework->sprite_height_positive = 128;
            firework->sprite_width = 96;
        }
    }

    // set the position
    auto pos = CoordsXYZ{ tile.ToCoordsXY(), height * LAND_HEIGHT_STEP };
    switch (forme)
    {
        case 0:
            Firework::Create(tile, height, objectId, color1, color2, color3, 1);
            Firework::Create(tile, height, objectId, color1, color2, color3, 2);
            break;
        case 1:
            pos = CoordsXYZ{ (tile + TileCoordsXY(TILE_OFFSET, 0)).ToCoordsXY(), height * LAND_HEIGHT_STEP };
            break;
        case 2:
            pos = CoordsXYZ{ (tile + TileCoordsXY(-TILE_OFFSET, 0)).ToCoordsXY(), height * LAND_HEIGHT_STEP };
            firework->sprite_direction = OpenRCT2::Entity::Yaw::BaseRotation / 2;
            break;
    }
    firework->MoveTo(pos);
    return firework;
}

void Firework::Update()
{
    Invalidate();

    auto targetFrame = _currentFrame + 1;
    if (targetFrame >= _numFrames)
    {
        EntityRemove(this);
        // targetFrame = _currentFrame;
        targetFrame = 0;
    }
    _currentFrame = targetFrame;
}

void Firework::Paint(paint_session& session, int32_t imageDirection)
{
    PROFILED_FUNCTION();

    // rct_drawpixelinfo& dpi = session.DPI;
    /*if (dpi.zoom_level > ZoomLevel{ 1 })
        return;*/

    session.InteractionType = ViewportInteractionItem::None;
    auto imageId = ImageId(_baseImage + _currentFrame * 4 + imageDirection);

    switch (forme)
    {
        case 0:
            PaintAddImageAsParent(session, imageId, { 0, 0, z }, { 32, 32, 64 }, { 0, 0, z });
            break;
        case 1:
        case 2:
            auto offset = CoordsXYZ(
                CoordsXY(-TILE_OFFSET * COORDS_XY_STEP, 0).Rotate(OpenRCT2::Entity::Yaw::YawTo4(imageDirection)), z);
            PaintAddImageAsParent(session, imageId, offset, { 32, 32, 64 }, { 0, 0, z });
            break;
    }
}

template<> bool EntityBase::Is<Firework>() const
{
    return Type == EntityType::Firework;
}
