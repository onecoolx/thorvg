/*
 * Copyright (c) 2023 - 2024 the ThorVG project. All rights reserved.

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Common.h"

/************************************************************************/
/* Drawing Commands                                                     */
/************************************************************************/

static unique_ptr<tvg::Animation> animation;
static void *transit;

void tvgUpdateCmds(void* data, void* obj, double progress)
{
    if (getUpdate() || !getCanvas()) return;

    //Update animation frame only when it's changed
    if (animation->frame(animation->totalFrame() * progress) == tvg::Result::Success) {
        getCanvas()->update();
        setUpdate(true);
    }
}

void tvgDrawCmds(tvg::Canvas* canvas)
{
    //Animation Controller
    animation = tvg::Animation::gen();
    auto picture = animation->picture();

    //Background
    auto shape = tvg::Shape::gen();
    shape->appendRect(0, 0, WIDTH, HEIGHT);
    shape->fill(50, 50, 50);

    if (canvas->push(std::move(shape)) != tvg::Result::Success) return;

    if (picture->load(EXAMPLE_DIR"/lottie/sample.json") != tvg::Result::Success) {
        cout << "Lottie is not supported. Did you enable Lottie Loader?" << endl;
        return;
    }

    //image scaling preserving its aspect ratio
    float scale;
    float shiftX = 0.0f, shiftY = 0.0f;
    float w, h;
    picture->size(&w, &h);

    if (w > h) {
        scale = WIDTH / w;
        shiftY = (HEIGHT - h * scale) * 0.5f;
    } else {
        scale = HEIGHT / h;
        shiftX = (WIDTH - w * scale) * 0.5f;
    }

    picture->scale(scale);
    picture->translate(shiftX, shiftY);

    canvas->push(tvg::cast(picture));

    //Run animation loop
    transit = addAnimatorTransit(animation->duration(), -1, tvgUpdateCmds, NULL);
}

/************************************************************************/
/* Main Code                                                            */
/************************************************************************/

int main(int argc, char **argv)
{
    auto tvgEngine = tvg::CanvasEngine::Sw;

    if (argc > 1) {
        if (!strcmp(argv[1], "gl")) tvgEngine = tvg::CanvasEngine::Gl;
    }

    //Threads Count
    auto threads = std::thread::hardware_concurrency();
    if (threads > 0) --threads;    //Allow the designated main thread capacity

    //Initialize ThorVG Engine
    if (tvg::Initializer::init(threads) == tvg::Result::Success) {

        plat_init(argc, argv);

        if (tvgEngine == tvg::CanvasEngine::Sw) {
            auto view = createSwView();
            setAnimatorSw(view);
        } else {
            auto view = createGlView();
            setAnimatorGl(view);
        }

        plat_run();

        delAnimatorTransit(transit);

        plat_shutdown();

        //Terminate ThorVG Engine
        tvg::Initializer::term();
    } else {
        cout << "engine is not supported" << endl;
    }

    return 0;
}
