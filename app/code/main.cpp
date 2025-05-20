/*
 * Copyright © 2025+ ÁRgB (angel.rodriguez@udit.es)
 *
 * Distributed under the Boost Software License, version 1.0
 * See ./LICENSE or www.boost.org/LICENSE_1_0.txt
 */


#if __has_include("concurrency_tools/ThreadPool.hpp")
    #include<concurrency_tools/ThreadPool.hpp>
    #define USE_CONCURRENCY
#endif

/*
* Structure to test things without concurrency tools if necessaryy
* #ifdef USE_CONCURRENCY

#else

#endif
*/


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
using namespace udit::concurrencytools;

namespace
{

    ThreadPool main_pool;

    void load_camera (Scene & scene)
    {
        auto & entity = scene.create_entity ();

#ifdef USE_CONCURRENCY

#else

#endif

        //main_pool.add_task(&Scene::create_component<Transform>, scene, entity);

        scene.create_component< Transform > (entity);
        scene.create_component< Path_Tracing::Camera > (entity, Path_Tracing::Camera::Sensor_Type::APS_C, 16.f / 1000.f);

        std::shared_ptr< Controller > camera_controller = std::make_shared< Camera_Controller > (scene, entity.id);

        scene.create_component< Control::Component > (entity, camera_controller);
    }

    void load_ground (Scene & scene)
    {
        auto & entity = scene.create_entity ();

#ifdef USE_CONCURRENCY

#else

#endif

        scene.create_component< Transform > (entity);

        auto model_component = scene.create_component< Path_Tracing::Model > (entity);

        model_component->add_plane (Vector3{0, -1, 0}, model_component->add_diffuse_material (Path_Tracing::Color(.4f, .4f, .5f)));
    }

    void load_shape (Scene & scene)
    {
        auto & entity = scene.create_entity ();

#ifdef USE_CONCURRENCY

#else

#endif

        scene.create_component< Transform > (entity);

        auto model_component = scene.create_component< Path_Tracing::Model > (entity);

        model_component->add_sphere (.25f, model_component->add_diffuse_material (Path_Tracing::Color(.8f, .8f, .8f)));
    }

    void load (Scene & scene)
    {
#ifdef USE_CONCURRENCY

        auto entity_a = main_pool.add_task(load_camera, std::ref(scene));
        auto entity_b = main_pool.add_task(load_ground, std::ref(scene));
        auto entity_c = main_pool.add_task(load_shape , std::ref(scene));

        entity_a->wait();
        entity_b->wait();
        entity_c->wait();

#else

        load_camera (scene);
        load_ground (scene);
        load_shape  (scene);
#endif
    }

    void engine_application ()
    {
        Window window("Ray Tracing Engine", 1024, 600);
        Scene scene(window);

#ifdef USE_CONCURRENCY
        main_pool.start();
        load(scene);
        //main_pool.stop();
#else
        load(scene);
#endif

        scene.run ();
    }

}

class Foo
{
public:
    int hi;
    int& PrintNumber(const int& number)
    {
        cout << number << ", ";
        hi = 50;
        return hi;
    }
};

int& PrintNumber(int& number)
{
    cout << (number*= 5) << ", ";
    //int hi = 20;
    return number;
}


int main (int , char * [])
{
#ifdef USE_CONCURRENCY
    /*
    ThreadPool pool;
    Foo foo;
    int c = 2001;
    pool.start();
    auto a = pool.add_task(PrintNumber, std::ref(c));
    
    
    for (int i = 1; i < 1000; ++i)
    {
        a = pool.add_task(&Foo::PrintNumber, foo, i);
        
    }
    
    
    
    //a->wait();
    cout << " value: " << a->get() << " old value: " << c ;
    */
#endif // USE_CONCURRENCY

   
    engine::starter.run (engine_application);

    return 0;
}
