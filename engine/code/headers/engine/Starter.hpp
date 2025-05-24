/*
 * Copyright © 2025+ ÁRgB (angel.rodriguez@udit.es)
 *
 * Distributed under the Boost Software License, version 1.0
 * See ./LICENSE or www.boost.org/LICENSE_1_0.txt
 */
#pragma once

#include <functional>
#include <engine/Control.hpp>
#include <engine/Key_Event.hpp>
#include <engine/Path_Tracing.hpp>
#include <engine/Starter.hpp>
#include <engine/Scene.hpp>
#include <engine/Window.hpp>
#include <engine/Entity.hpp>

#include "Camera_Controller.hpp"



#if __has_include("concurrency_tools/ThreadPool.hpp")
    #include<concurrency_tools/ThreadPool.hpp>
    //#define USE_CONCURRENCY
    using namespace udit::concurrencytools;
#endif



using namespace std;
using namespace udit;
    
namespace udit::engine
{
    class Starter
    {
    public:

        static Starter & instance ()
        {
            static Starter engine;
            return engine;
        }
#ifdef USE_CONCURRENCY
        static ThreadPool& thread_pool()
        {
            return main_pool;
        }
    private:
        static ThreadPool main_pool;
#endif
    public:

        void run(const std::function<void()> & runnable);

        void load_camera(Scene& scene);
        void load_ground(Scene& scene);
        void load_shape (Scene& scene);
        void load       (Scene& scene);
        void engine_application();
    private:
        bool initialize ();


        struct Finalizer
        {
            ~Finalizer();
        };

    };

    extern Starter & starter;

}
