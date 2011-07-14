/*
 * Copyright © 2010-2011 Linaro Limited
 *
 * This file is part of the glmark2 OpenGL (ES) 2.0 benchmark.
 *
 * glmark2 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * glmark2 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * glmark2.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *  Alexandros Frantzis (glmark2)
 */
#include "scene.h"
#include "mat.h"
#include "stack.h"
#include "vec.h"
#include "log.h"

#include <sstream>

SceneGrid::SceneGrid(Canvas &pCanvas, const std::string &name) :
    Scene(pCanvas, name)
{
    mOptions["grid-size"] = Scene::Option("grid-size", "32",
            "The number of squares per side of the grid (controls the number of vertices)");
    mOptions["grid-length"] = Scene::Option("grid-length", "5.0",
            "The length of each side of the grid (normalized) (controls the area drawn to)");
}

SceneGrid::~SceneGrid()
{
}

int SceneGrid::load()
{
    mRotationSpeed = 36.0f;
    mRunning = false;

    return 1;
}

void SceneGrid::unload()
{
}

void SceneGrid::setup()
{
    Scene::setup();

    int grid_size = 0;
    double grid_length = 0;

    std::stringstream ss;

    ss << mOptions["grid-size"].value;
    ss >> grid_size;
    ss.clear();
    ss << mOptions["grid-length"].value;
    ss >> grid_length;

    /* Create and configure the grid mesh */
    std::vector<int> vertex_format;
    vertex_format.push_back(3);
    mMesh.set_vertex_format(vertex_format);

    /* 
     * The spacing needed in order for the area of the requested grid
     * to be the same as the area of a grid with size 32 and spacing 0.02.
     */
    double spacing = grid_length * (1 - 4.38 / 5.0) / (grid_size - 1.0);

    mMesh.make_grid(grid_size, grid_size, grid_length, grid_length,
                    grid_size > 1 ? spacing : 0);
    mMesh.build_vbo();

    std::vector<GLint> attrib_locations;
    attrib_locations.push_back(mProgram.getAttribIndex("position"));
    mMesh.set_attrib_locations(attrib_locations);

    mCurrentFrame = 0;
    mRotation = 0.0f;
}

void SceneGrid::teardown()
{
    mProgram.stop();
    mProgram.release();
    mMesh.reset();

    Scene::teardown();
}

void SceneGrid::update()
{
    double current_time = Scene::get_timestamp_us() / 1000000.0;
    double dt = current_time - mLastUpdateTime;
    double elapsed_time = current_time - mStartTime;

    mLastUpdateTime = current_time;

    if (elapsed_time >= mDuration) {
        mAverageFPS = mCurrentFrame / elapsed_time;
        mRunning = false;
    }

    mRotation += mRotationSpeed * dt;

    mCurrentFrame++;
}

void SceneGrid::draw()
{
    // Load the ModelViewProjectionMatrix uniform in the shader
    LibMatrix::Stack4 model_view;
    LibMatrix::mat4 model_view_proj(mCanvas.projection());

    model_view.translate(0.0f, 0.0f, -5.0f);
    model_view.rotate(mRotation, 0.0f, 0.0f, 1.0f);
    model_view_proj *= model_view.getCurrent();

    mProgram.loadUniformMatrix(model_view_proj, "ModelViewProjectionMatrix");

    mMesh.render_vbo();
}

Scene::ValidationResult
SceneGrid::validate()
{
    return Scene::ValidationUnknown;
}