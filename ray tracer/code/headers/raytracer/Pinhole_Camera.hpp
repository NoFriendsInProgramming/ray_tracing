/*
 * Copyright © 2025+ ÁRgB (angel.rodriguez@udit.es)
 *
 * Distributed under the Boost Software License, version 1.0
 * See ./LICENSE or www.boost.org/LICENSE_1_0.txt
 */

#pragma once

#include <raytracer/Camera.hpp>
#include <future>
#include <vector>
#include <memory>
#define USE_CONCURRENCY
#include <concurrency_tools/ThreadPool.hpp>
using namespace udit::concurrencytools;
namespace udit::raytracer
{

    class Pinhole_Camera : public Camera
    {
    private:
        const static unsigned int chunk_count;
        ThreadPool buffer_pool;
        std::vector<std::shared_ptr<std::future<void>>> pixel_chunk_futures;
    public:

        Pinhole_Camera(const Sensor_Type& given_sensor_type, float given_focal_length)
            :
            Camera(given_sensor_type, given_focal_length), pixel_chunk_futures{chunk_count }
        {
            buffer_pool.start();
        }

        void calculate (Buffer< Ray > & primary_rays) override;

        void calculate_row_chunks(
            const unsigned int& starting_height,
            const unsigned int& ending_height,
            const unsigned int& buffer_width,
            const Vector3& vertical_step,
            const Vector3& horizontal_step,
            const Vector3& focal_point,
            Buffer< Ray >& primary_rays,
            unsigned int buffer_offset,
            Vector3 scanline_start
        );

    };

}
