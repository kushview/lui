// Copyright 2022 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

#include <lui/embed.hpp>
#include <lui/rectangle.hpp>
#include <lui/view.hpp>

#include <Cocoa/Cocoa.h>
#include <iostream>

#include "detail/embed.hpp"

namespace lui
{
namespace detail
{

class NSViewEmbed : public detail::Embed
{
   public:
    NSViewEmbed(lui::Embed& o) : detail::Embed(o) {}
    ~NSViewEmbed() { current_view = nullptr; }

   private:
    class Attachment
    {
       public:
        Attachment(NSView* v) : view(v)
        {
            [view retain];
            [view setPostsFrameChangedNotifications:YES];
        }

        ~Attachment()
        {
            if (view != nullptr)
            {
                [view release];
            }
        }

        NSView* view{nullptr};
    };

    lui::View* current_view{nullptr};
    std::shared_ptr<Attachment> attachment;
};

std::unique_ptr<detail::Embed> Embed::create(lui::Embed& owner)
{
    return std::make_unique<detail::NSViewEmbed>(owner);
}

} /* namespace detail */
} // namespace lui
