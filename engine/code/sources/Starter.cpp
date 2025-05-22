/*
 * Copyright © 2025+ ÁRgB (angel.rodriguez@udit.es)
 *
 * Distributed under the Boost Software License, version 1.0
 * See ./LICENSE or www.boost.org/LICENSE_1_0.txt
 */

#define SDL_MAIN_HANDLED

#include <mutex>
#include <engine/Starter.hpp>
#include <SDL3/SDL_main.h>



namespace udit::engine
{
#ifdef USE_CONCURRENCY

    ThreadPool Starter::main_pool;
#endif // USE_CONCURRENCY

    Starter & starter = Starter::instance ();


    void Starter::run(const std::function<void()>& runnable)
    {
        static std::mutex mutex;

        std::lock_guard lock(mutex);


        if (initialize())
        {
            Finalizer finalizer;

            runnable();
        }
    }

    bool Starter::initialize ()
    {
        static int run_before = 0;

        if (not run_before++)
        {
            SDL_SetMainReady ();
        }

        return SDL_Init (0);
    }

    Starter::Finalizer::~Finalizer()
    {
        SDL_Quit ();
    }

    void Starter::load_camera(Scene& scene)
    {
        
        auto& entity = scene.create_entity();

#ifdef USE_CONCURRENCY

        //auto component_a = main_pool.add_task(&Scene::create_component<Transform>, std::ref(scene), std::ref(entity));
        auto component_a = Starter::thread_pool().add_task(&Scene::create_component<Path_Tracing::Camera, Path_Tracing::Camera::Sensor_Type, float>, std::ref(scene), std::ref(entity), Path_Tracing::Camera::Sensor_Type::APS_C, 16.f / 1000.f);

        scene.create_component< Transform >(entity);
        std::shared_ptr< Controller > camera_controller = std::make_shared< Camera_Controller >(scene, entity.id);
        scene.create_component< Control::Component >(entity, camera_controller);

        component_a->wait();

#else

        scene.create_component< Transform >(entity);

        std::shared_ptr< Controller > camera_controller = std::make_shared< Camera_Controller >(scene, entity.id);

        scene.create_component< Control::Component >(entity, camera_controller);
#endif



    }

    void Starter::load_ground(Scene& scene)
    {
        auto& entity = scene.create_entity();

#ifdef USE_CONCURRENCY
        auto component_a = Starter::thread_pool().add_task(&Scene::create_component<Transform>, std::ref(scene), std::ref(entity));
        auto model_component = scene.create_component< Path_Tracing::Model >(entity);
        model_component->add_plane(Vector3{ 0, -1, 0 }, model_component->add_diffuse_material(Path_Tracing::Color(.4f, .4f, .5f)));
        component_a->wait();
#else

        scene.create_component< Transform >(entity);

        auto model_component = scene.create_component< Path_Tracing::Model >(entity);

        model_component->add_plane(Vector3{ 0, -1, 0 }, model_component->add_diffuse_material(Path_Tracing::Color(.4f, .4f, .5f)));
#endif

    }

    void Starter::load_shape(Scene& scene)
    {
        auto& entity = scene.create_entity();

#ifdef USE_CONCURRENCY
        auto component_a = Starter::thread_pool().add_task(&Scene::create_component<Transform>, std::ref(scene), std::ref(entity));
        auto model_component = scene.create_component< Path_Tracing::Model >(entity);
        model_component->add_sphere(.25f, model_component->add_diffuse_material(Path_Tracing::Color(.8f, .8f, .8f)));
        component_a->wait();
#else

        scene.create_component< Transform >(entity);

        auto model_component = scene.create_component< Path_Tracing::Model >(entity);

        model_component->add_sphere(.25f, model_component->add_diffuse_material(Path_Tracing::Color(.8f, .8f, .8f)));
#endif

    }

    void Starter::load(Scene& scene)
    {
#ifdef USE_CONCURRENCY

        auto entity_a = Starter::thread_pool().add_task(&Starter::load_camera, &starter, std::ref(scene));
        auto entity_b = Starter::thread_pool().add_task(&Starter::load_camera, &starter, std::ref(scene));
        auto entity_c = Starter::thread_pool().add_task(&Starter::load_camera, &starter, std::ref(scene));

        entity_a->wait();
        entity_b->wait();
        entity_c->wait();

#else

        load_camera(scene);
        load_ground(scene);
        load_shape(scene);

#endif
    }

    void Starter::engine_application()
    {
        Window window("Ray Tracing Engine", 1024, 600);
        Scene scene(window);

#ifdef USE_CONCURRENCY


        Starter::thread_pool().start();
#endif
        load(scene);

        scene.run();
    }

}
