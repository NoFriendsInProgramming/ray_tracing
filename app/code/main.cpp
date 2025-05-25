/*
 * Copyright © 2025+ ÁRgB (angel.rodriguez@udit.es)
 *
 * Distributed under the Boost Software License, version 1.0
 * See ./LICENSE or www.boost.org/LICENSE_1_0.txt
 */


#if __has_include("concurrency_tools/ThreadPool.hpp")
    #include<concurrency_tools/ThreadPool.hpp>
    using namespace udit::concurrencytools;
#endif

#include <engine/Control.hpp>
#include <engine/Key_Event.hpp>
#include <engine/Path_Tracing.hpp>
#include <engine/Starter.hpp>
#include <engine/Scene.hpp>
#include <engine/Window.hpp>
#include <engine/Entity.hpp>

#include "Camera_Controller.hpp"

using namespace std;
using namespace udit;
using namespace udit::engine;


int main (int , char * [])
{
    
    engine::starter.run(std::bind(&Starter::engine_application, &engine::starter));

    return 0;
}
