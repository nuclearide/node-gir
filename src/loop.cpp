/*
 * Copyright 2017 Jasper St. Pierre, James Batt
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to
 * do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "loop.h"
#include <glib.h>
#include <nan.h>
#include <uv.h>
#include <v8.h>

namespace gir {

struct uv_loop_source {
    GSource source;
    uv_loop_t *loop;
};

static gboolean uv_loop_source_prepare(GSource *base, int *timeout) {
    struct uv_loop_source *source = (struct uv_loop_source *)base;
    uv_update_time(source->loop);

    bool loop_alive = uv_loop_alive(source->loop);

    /* If the loop is dead, we can simply sleep forever until a GTK+ source
     * (presumably) wakes us back up again. */
    if (!loop_alive)
        return FALSE;

    /* Otherwise, check the timeout. If the timeout is 0, that means we're
     * ready to go. Otherwise, keep sleeping until the timeout happens again. */
    int t = uv_backend_timeout(source->loop);
    *timeout = t;

    if (t == 0)
        return TRUE;
    else
        return FALSE;
}

/**
 * This function is used to call "process._tickCallback()" inside NodeJS.
 * We want to do this after we run the LibUV eventloop because there might
 * be pending Micro-tasks from Promises or calls to 'process.nextTick()'.
 */
static void call_next_tick_callback() {
    Nan::HandleScope scope;
    // get "process" from node's global scope
    v8::Local<v8::Value> process_value = Nan::GetCurrentContext()->Global()->Get(
        Nan::New<v8::String>("process").ToLocalChecked());
    if (process_value->IsObject()) {
        // if it's a JS object, the type cast it to a Local<Object>.
        v8::Local<v8::Object> process_object = process_value->ToObject();
        // get the "_tickCallback" property from the "process" object
        v8::Local<v8::Value> tick_callback_value = process_object->Get(Nan::New("_tickCallback").ToLocalChecked());
        if (tick_callback_value->IsFunction()) {
            // if it's a Function then we can call it! passing the "process" object as it's context (this)
            // and 0 arguments (nullptr because argc is 0).
            Nan::CallAsFunction(tick_callback_value->ToObject(), process_object, 0, nullptr);
        }
    }
}

static gboolean uv_loop_source_dispatch(GSource *base, GSourceFunc callback, gpointer user_data) {
    struct uv_loop_source *source = (struct uv_loop_source *)base;
    Nan::HandleScope scope;
    uv_run(source->loop, UV_RUN_NOWAIT);
    call_next_tick_callback();
    return G_SOURCE_CONTINUE;
}

static GSourceFuncs uv_loop_source_funcs = {
    uv_loop_source_prepare,
    nullptr,
    uv_loop_source_dispatch,
    nullptr,

    nullptr,
    nullptr,
};

static GSource *uv_loop_source_new(uv_loop_t *loop) {
    struct uv_loop_source *source = (struct uv_loop_source *)g_source_new(&uv_loop_source_funcs, sizeof(*source));
    source->loop = loop;
    g_source_add_unix_fd(&source->source, uv_backend_fd(loop), (GIOCondition)(G_IO_IN | G_IO_OUT | G_IO_ERR));
    return &source->source;
}

NAN_METHOD(start_loop) {
    GSource *source = uv_loop_source_new(uv_default_loop());
    g_source_attach(source, nullptr);
    info.GetReturnValue().Set(Nan::Undefined());
}

}; // namespace gir
