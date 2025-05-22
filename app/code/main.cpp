/*
 * Copyright © 2025+ ÁRgB (angel.rodriguez@udit.es)
 *
 * Distributed under the Boost Software License, version 1.0
 * See ./LICENSE or www.boost.org/LICENSE_1_0.txt
 */

/*
#if __has_include("concurrency_tools/ThreadPool.hpp")
    #include<concurrency_tools/ThreadPool.hpp>
    #define USE_CONCURRENCY
#endif
*/

/*
* Structure to test things without concurrency tools if necessaryy
* #ifdef USE_CONCURRENCY

#else

#endif
*/
#if __has_include("concurrency_tools/ThreadPool.hpp")
#include<concurrency_tools/ThreadPool.hpp>
//#define USE_CONCURRENCY
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
using namespace udit::concurrencytools;

    
namespace
{
#ifdef USE_CONCURRENCY
    //ThreadPool main_pool;
#endif

    void load_camera (Scene & scene)
    {
        auto & entity = scene.create_entity ();

#ifdef USE_CONCURRENCY

        //auto component_a = main_pool.add_task(&Scene::create_component<Transform>, std::ref(scene), std::ref(entity));
        auto component_a = Starter::thread_pool().add_task(&Scene::create_component<Path_Tracing::Camera, Path_Tracing::Camera::Sensor_Type, float>, std::ref(scene), std::ref(entity), Path_Tracing::Camera::Sensor_Type::APS_C, 16.f / 1000.f);

        scene.create_component< Transform >(entity);
        std::shared_ptr< Controller > camera_controller = std::make_shared< Camera_Controller >(scene, entity.id);
        scene.create_component< Control::Component >(entity, camera_controller);

        component_a->wait();

#else

        scene.create_component< Transform > (entity);
        scene.create_component< Path_Tracing::Camera > (entity, Path_Tracing::Camera::Sensor_Type::APS_C, 16.f / 1000.f);

        std::shared_ptr< Controller > camera_controller = std::make_shared< Camera_Controller > (scene, entity.id);

        scene.create_component< Control::Component > (entity, camera_controller);
#endif

        //

    }

    void load_ground (Scene & scene)
    {
        auto & entity = scene.create_entity ();

#ifdef USE_CONCURRENCY
        auto component_a = Starter::thread_pool().add_task(&Scene::create_component<Transform>, std::ref(scene), std::ref(entity));
        auto model_component = scene.create_component< Path_Tracing::Model >(entity);
        model_component->add_plane(Vector3{ 0, -1, 0 }, model_component->add_diffuse_material(Path_Tracing::Color(.4f, .4f, .5f)));
        component_a->wait();
#else

        scene.create_component< Transform > (entity);

        auto model_component = scene.create_component< Path_Tracing::Model > (entity);

        model_component->add_plane (Vector3{0, -1, 0}, model_component->add_diffuse_material (Path_Tracing::Color(.4f, .4f, .5f)));
#endif

    }

    void load_shape (Scene & scene)
    {
        auto & entity = scene.create_entity ();

#ifdef USE_CONCURRENCY
        auto component_a = Starter::thread_pool().add_task(&Scene::create_component<Transform>, std::ref(scene), std::ref(entity));
        auto model_component = scene.create_component< Path_Tracing::Model >(entity);
        model_component->add_sphere(.25f, model_component->add_diffuse_material(Path_Tracing::Color(.8f, .8f, .8f)));
        component_a->wait();
#else

        scene.create_component< Transform > (entity);

        auto model_component = scene.create_component< Path_Tracing::Model > (entity);

        model_component->add_sphere (.25f, model_component->add_diffuse_material (Path_Tracing::Color(.8f, .8f, .8f)));
#endif

    }

    void load (Scene & scene)
    {
#ifdef USE_CONCURRENCY

        auto entity_a = Starter::thread_pool().add_task(load_camera, std::ref(scene));
        auto entity_b = Starter::thread_pool().add_task(load_ground, std::ref(scene));
        auto entity_c = Starter::thread_pool().add_task(load_shape , std::ref(scene));

        entity_a->wait();
        entity_b->wait();
        entity_c->wait();

#else
        
        engine::starter.load_camera (scene);
        engine::starter.load_ground (scene);
        engine::starter.load_shape  (scene);
#endif
    }

    void engine_application ()
    {
        Window window("Ray Tracing Engine", 1024, 600);
        Scene scene(window);

#ifdef USE_CONCURRENCY
        Starter::thread_pool().start();
#endif

        load(scene);

        scene.run ();
    }

}



int main (int , char * [])
{
    
    engine::starter.run(engine_application);//std::bind(&Starter::engine_application, &engine::starter));

    return 0;
}
