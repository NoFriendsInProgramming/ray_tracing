/*
 * Copyright © 2025+ ÁRgB (angel.rodriguez@udit.es)
 *
 * Distributed under the Boost Software License, version 1.0
 * See ./LICENSE or www.boost.org/LICENSE_1_0.txt
 */

#pragma once

#include <engine/Key_Event.hpp>
#include <engine/Stage.hpp>
#include <engine/Starter.hpp>
#include <engine/Timer.hpp>
#include <SDL3/SDL.h>

namespace udit::engine
{

    class Input_Stage : public Stage
    {
        using Key_Event_Pool = Input_Event::Queue_Pool< Key_Event >;

    private:

        Key_Event_Pool key_events;
        int interval_ms;

        void sdl_poll_to_key_push(SDL_Event& event);

    public:

        Input_Stage(Scene & scene) : Stage(scene), interval_ms(1000/60)
        {
        }

        void compute (float) override;

        void cleanup () override;
    };

}
