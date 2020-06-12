/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "NewsItem.h"

#include "../Context.h"
#include "../Input.h"
#include "../OpenRCT2.h"
#include "../audio/audio.h"
#include "../interface/Window.h"
#include "../interface/Window_internal.h"
#include "../localisation/Date.h"
#include "../localisation/Localisation.h"
#include "../management/Research.h"
#include "../ride/Ride.h"
#include "../util/Util.h"
#include "../windows/Intent.h"
#include "../world/Sprite.h"

NewsItemQueue gNewsItems;

/** rct2: 0x0097BE7C */
const uint8_t news_type_properties[] = {
    0,                                              // NEWS_ITEM_NULL
    NEWS_TYPE_HAS_LOCATION | NEWS_TYPE_HAS_SUBJECT, // NEWS_ITEM_RIDE
    NEWS_TYPE_HAS_LOCATION | NEWS_TYPE_HAS_SUBJECT, // NEWS_ITEM_PEEP_ON_RIDE
    NEWS_TYPE_HAS_LOCATION | NEWS_TYPE_HAS_SUBJECT, // NEWS_ITEM_PEEP
    NEWS_TYPE_HAS_SUBJECT,                          // NEWS_ITEM_MONEY
    NEWS_TYPE_HAS_LOCATION,                         // NEWS_ITEM_BLANK
    NEWS_TYPE_HAS_SUBJECT,                          // NEWS_ITEM_RESEARCH
    NEWS_TYPE_HAS_SUBJECT,                          // NEWS_ITEM_PEEPS
    NEWS_TYPE_HAS_SUBJECT,                          // NEWS_ITEM_AWARD
    NEWS_TYPE_HAS_SUBJECT,                          // NEWS_ITEM_GRAPH
};

NewsItem& NewsItemQueue::Current()
{
    return Recent[0];
}

const NewsItem& NewsItemQueue::Current() const
{
    return Recent[0];
}

NewsItem& NewsItemQueue::Oldest()
{
    return Archived[0];
}

const NewsItem& NewsItemQueue::Oldest() const
{
    return Archived[0];
}

bool news_item_is_valid_idx(int32_t index)
{
    if (index >= MAX_NEWS_ITEMS)
    {
        log_error("Tried to get news item past MAX_NEWS.");
        return false;
    }
    return true;
}

NewsItem* news_item_get(int32_t index)
{
    return gNewsItems.At(index);
}

NewsItem& NewsItemQueue::operator[](size_t index)
{
    return const_cast<NewsItem&>(const_cast<const NewsItemQueue&>(*this)[index]);
}

const NewsItem& NewsItemQueue::operator[](size_t index) const
{
    if (index < NEWS_ITEM_HISTORY_START)
        return Recent[index];
    else
        return Archived[index - NEWS_ITEM_HISTORY_START];
}

NewsItem* NewsItemQueue::At(int32_t index)
{
    return const_cast<NewsItem*>(const_cast<const NewsItemQueue&>(*this).At(index));
}

const NewsItem* NewsItemQueue::At(int32_t index) const
{
    if (news_item_is_valid_idx(index))
    {
        return &(*this)[index];
    }
    else
    {
        return nullptr;
    }
}

bool news_item_is_empty(int32_t index)
{
    NewsItem* news = gNewsItems.At(index);
    return news != nullptr && news->IsEmpty();
}

bool news_item_is_queue_empty()
{
    return gNewsItems.IsEmpty();
}

bool NewsItemQueue::IsEmpty() const
{
    return Current().IsEmpty();
}

/**
 *
 *  rct2: 0x0066DF32
 */
void NewsItemQueue::Init()
{
    Current().Type = NEWS_ITEM_NULL;
    Oldest().Type = NEWS_ITEM_NULL;
}

void news_item_init_queue()
{
    gNewsItems.Init();

    // Throttles for warning types (PEEP_*_WARNING)
    for (auto& warningThrottle : gPeepWarningThrottle)
    {
        warningThrottle = 0;
    }

    auto intent = Intent(INTENT_ACTION_INVALIDATE_TICKER_NEWS);
    context_broadcast_intent(&intent);
}

uint16_t NewsItemQueue::IncrementTicks()
{
    return ++Current().Ticks;
}

static void news_item_tick_current()
{
    int32_t ticks = gNewsItems.IncrementTicks();
    // Only play news item sound when in normal playing mode
    if (ticks == 1 && (gScreenFlags == SCREEN_FLAGS_PLAYING))
    {
        // Play sound
        audio_play_sound(SoundId::NewsItem, 0, context_get_width() / 2);
    }
}

int32_t NewsItemQueue::RemoveTime() const
{
    if (!Recent[5].IsEmpty() && !Recent[4].IsEmpty() && !Recent[3].IsEmpty() && !Recent[2].IsEmpty())
    {
        return 256;
    }
    return 320;
}

bool NewsItemQueue::CurrentShouldBeArchived() const
{
    return Current().Ticks >= RemoveTime();
}

/**
 *
 *  rct2: 0x0066E252
 */
void news_item_update_current()
{
    // Check if there is a current news item
    if (gNewsItems.IsEmpty())
        return;

    auto intent = Intent(INTENT_ACTION_INVALIDATE_TICKER_NEWS);
    context_broadcast_intent(&intent);

    // Update the current news item
    news_item_tick_current();

    // Removal of current news item
    if (gNewsItems.CurrentShouldBeArchived())
        gNewsItems.ArchiveCurrent();
}

/**
 *
 *  rct2: 0x0066E377
 */
void news_item_close_current()
{
    gNewsItems.ArchiveCurrent();
}

void NewsItemQueue::ArchiveCurrent()
{
    // Check if there is a current message
    if (IsEmpty())
        return;

    AppendToArchive(Current());

    // Invalidate the news window
    window_invalidate_by_class(WC_RECENT_NEWS);

    // Dequeue the current news item, shift news up
    memmove(Recent, Recent + 1, sizeof(NewsItem) * (std::size(Recent) - 1));
    Recent[NEWS_ITEM_HISTORY_START - 1].Type = NEWS_ITEM_NULL;

    // Invalidate current news item bar
    auto intent = Intent(INTENT_ACTION_INVALIDATE_TICKER_NEWS);
    context_broadcast_intent(&intent);
}

/**
 * Finds a spare history slot or replaces an existing one if there are no spare
 * slots available.
 */
void NewsItemQueue::AppendToArchive(NewsItem& item)
{
    auto it = std::find_if(std::begin(Archived), std::end(Archived), [](const auto& newsItem) { return newsItem.IsEmpty(); });
    if (it != std::end(Archived))
    {
        *it = item;
        ++it;
        if (it != std::end(Archived))
            it->Type = NEWS_ITEM_NULL;
        return;
    }

    // Dequeue the first history news item, shift history up
    memmove(Archived, Archived + 1, sizeof(NewsItem) * (std::size(Archived) - 1));
    Archived[MAX_NEWS_ITEMS_ARCHIVE - 1] = item;
}

/**
 * Get the (x,y,z) coordinates of the subject of a news item.
 * If the new item is no longer valid, return LOCATION_NULL in the x-coordinate
 *
 *  rct2: 0x0066BA74
 */
std::optional<CoordsXYZ> news_item_get_subject_location(int32_t type, int32_t subject)
{
    std::optional<CoordsXYZ> subjectLoc{ std::nullopt };

    switch (type)
    {
        case NEWS_ITEM_RIDE:
        {
            Ride* ride = get_ride(subject);
            if (ride == nullptr || ride->overall_view.isNull())
            {
                break;
            }
            auto rideViewCentre = ride->overall_view.ToTileCentre();
            subjectLoc = CoordsXYZ{ rideViewCentre, tile_element_height(rideViewCentre) };
            break;
        }
        case NEWS_ITEM_PEEP_ON_RIDE:
        {
            auto sprite = try_get_sprite(subject);
            if (sprite == nullptr)
                break;

            auto peep = sprite->AsPeep();
            if (peep == nullptr)
                break;

            subjectLoc = CoordsXYZ{ peep->x, peep->y, peep->z };
            if (subjectLoc->x != LOCATION_NULL)
                break;

            if (peep->state != PEEP_STATE_ON_RIDE && peep->state != PEEP_STATE_ENTERING_RIDE)
            {
                subjectLoc = std::nullopt;
                break;
            }

            // Find which ride peep is on
            Ride* ride = get_ride(peep->current_ride);
            if (ride == nullptr || !(ride->lifecycle_flags & RIDE_LIFECYCLE_ON_TRACK))
            {
                subjectLoc = std::nullopt;
                break;
            }

            // Find the first car of the train peep is on
            sprite = try_get_sprite(ride->vehicles[peep->current_train]);
            // Find the actual car peep is on
            for (int32_t i = 0; i < peep->current_car && sprite != nullptr; i++)
            {
                sprite = try_get_sprite(sprite->vehicle.next_vehicle_on_train);
            }
            if (sprite != nullptr)
            {
                subjectLoc = CoordsXYZ{ sprite->vehicle.x, sprite->vehicle.y, sprite->vehicle.z };
            }
            break;
        }
        case NEWS_ITEM_PEEP:
        {
            auto sprite = try_get_sprite(subject);
            if (sprite != nullptr)
            {
                auto peep = sprite->AsPeep();
                if (peep != nullptr)
                {
                    subjectLoc = CoordsXYZ{ peep->x, peep->y, peep->z };
                }
            }
            break;
        }
        case NEWS_ITEM_BLANK:
        {
            auto subjectUnsigned = static_cast<uint32_t>(subject);
            auto subjectXY = CoordsXY{ static_cast<int16_t>(subjectUnsigned & 0xFFFF),
                                       static_cast<int16_t>(subjectUnsigned >> 16) };
            if (!subjectXY.isNull())
            {
                subjectLoc = CoordsXYZ{ subjectXY, tile_element_height(subjectXY) };
            }
            break;
        }
        default:
            break;
    }
    return subjectLoc;
}

NewsItem* NewsItemQueue::FirstOpenOrNewSlot()
{
    auto it = std::begin(Recent);
    for (; !it->IsEmpty();)
    {
        if (it + 2 >= std::end(Recent))
            ArchiveCurrent();
        else
            it++;
    }
    return &*it;
}

/**
 *
 *  rct2: 0x0066DF55
 */
NewsItem* news_item_add_to_queue(uint8_t type, rct_string_id string_id, uint32_t assoc)
{
    utf8 buffer[256];
    void* args = gCommonFormatArgs;

    // overflows possible?
    format_string(buffer, 256, string_id, args);
    return news_item_add_to_queue_raw(type, buffer, assoc);
}

NewsItem* news_item_add_to_queue_raw(uint8_t type, const utf8* text, uint32_t assoc)
{
    NewsItem* newsItem = gNewsItems.FirstOpenOrNewSlot();
    newsItem->Type = type;
    newsItem->Flags = 0;
    newsItem->Assoc = assoc;
    newsItem->Ticks = 0;
    newsItem->MonthYear = gDateMonthsElapsed;
    newsItem->Day = ((days_in_month[date_get_month(newsItem->MonthYear)] * gDateMonthTicks) >> 16) + 1;
    safe_strcpy(newsItem->Text, text, sizeof(newsItem->Text));

    NewsItem* res = newsItem;

    // Blatant disregard for what happens on the last element.
    // TODO: Change this when we implement the queue ourselves.
    newsItem++;
    newsItem->Type = NEWS_ITEM_NULL;

    return res;
}

/**
 * Opens the window/tab for the subject of the news item
 *
 *  rct2: 0x0066EBE6
 *
 */
void news_item_open_subject(int32_t type, int32_t subject)
{
    switch (type)
    {
        case NEWS_ITEM_RIDE:
        {
            auto intent = Intent(WC_RIDE);
            intent.putExtra(INTENT_EXTRA_RIDE_ID, subject);
            context_open_intent(&intent);
            break;
        }
        case NEWS_ITEM_PEEP_ON_RIDE:
        case NEWS_ITEM_PEEP:
        {
            auto sprite = try_get_sprite(subject);
            if (sprite != nullptr)
            {
                auto peep = sprite->AsPeep();
                if (peep != nullptr)
                {
                    auto intent = Intent(WC_PEEP);
                    intent.putExtra(INTENT_EXTRA_PEEP, peep);
                    context_open_intent(&intent);
                }
            }
            break;
        }
        case NEWS_ITEM_MONEY:
            context_open_window(WC_FINANCES);
            break;
        case NEWS_ITEM_RESEARCH:
        {
            auto item = ResearchItem(subject, 0, 0);
            if (item.type == RESEARCH_ENTRY_TYPE_RIDE)
            {
                auto intent = Intent(INTENT_ACTION_NEW_RIDE_OF_TYPE);
                intent.putExtra(INTENT_EXTRA_RIDE_TYPE, item.baseRideType);
                intent.putExtra(INTENT_EXTRA_RIDE_ENTRY_INDEX, item.entryIndex);
                context_open_intent(&intent);
                break;
            }

            // Check if window is already open
            auto window = window_bring_to_front_by_class(WC_SCENERY);
            if (window == nullptr)
            {
                window = window_find_by_class(WC_TOP_TOOLBAR);
                if (window != nullptr)
                {
                    window->Invalidate();
                    if (!tool_set(window, WC_TOP_TOOLBAR__WIDX_SCENERY, TOOL_ARROW))
                    {
                        input_set_flag(INPUT_FLAG_6, true);
                        context_open_window(WC_SCENERY);
                    }
                }
            }

            // Switch to new scenery tab
            window = window_find_by_class(WC_SCENERY);
            if (window != nullptr)
                window_event_mouse_down_call(window, WC_SCENERY__WIDX_SCENERY_TAB_1 + subject);
            break;
        }
        case NEWS_ITEM_PEEPS:
        {
            auto intent = Intent(WC_GUEST_LIST);
            intent.putExtra(INTENT_EXTRA_GUEST_LIST_FILTER, GLFT_GUESTS_THINKING_X);
            intent.putExtra(INTENT_EXTRA_RIDE_ID, subject);
            context_open_intent(&intent);
            break;
        }
        case NEWS_ITEM_AWARD:
            context_open_window_view(WV_PARK_AWARDS);
            break;
        case NEWS_ITEM_GRAPH:
            context_open_window_view(WV_PARK_RATING);
            break;
    }
}

/**
 *
 *  rct2: 0x0066E407
 */
void news_item_disable_news(uint8_t type, uint32_t assoc)
{
    // TODO: write test invalidating windows
    gNewsItems.ForeachRecentNews([type, assoc](auto& newsItem) {
        if (type == newsItem.Type && assoc == newsItem.Assoc)
        {
            newsItem.Flags |= NEWS_FLAG_HAS_BUTTON;
            if (&newsItem == &gNewsItems.Current())
            {
                auto intent = Intent(INTENT_ACTION_INVALIDATE_TICKER_NEWS);
                context_broadcast_intent(&intent);
            }
        }
    });

    gNewsItems.ForeachArchivedNews([type, assoc](auto& newsItem) {
        if (type == newsItem.Type && assoc == newsItem.Assoc)
        {
            newsItem.Flags |= NEWS_FLAG_HAS_BUTTON;
            window_invalidate_by_class(WC_RECENT_NEWS);
        }
    });
}

void news_item_add_to_queue_custom(NewsItem* newNewsItem)
{
    NewsItem* newsItem = gNewsItems.FirstOpenOrNewSlot();
    *newsItem = *newNewsItem;
    newsItem++;
    newsItem->Type = NEWS_ITEM_NULL;
}

void news_item_remove(int32_t index)
{
    if (index < 0 || index >= MAX_NEWS_ITEMS)
        return;

    // News item is already null, no need to remove it
    if (gNewsItems[index].Type == NEWS_ITEM_NULL)
        return;

    size_t newsBoundary = index < NEWS_ITEM_HISTORY_START ? NEWS_ITEM_HISTORY_START : MAX_NEWS_ITEMS;
    for (size_t i = index; i < newsBoundary - 1; i++)
    {
        gNewsItems[i] = gNewsItems[i + 1];
    }
    gNewsItems[newsBoundary - 1].Type = NEWS_ITEM_NULL;
}
