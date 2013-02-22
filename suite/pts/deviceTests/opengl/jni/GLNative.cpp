/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the License for the specific language governing permissions and limitations under
 * the License.
 */
#include <jni.h>

#include <stdlib.h>
#include <sys/time.h>

#include <android/native_window.h>
#include <android/native_window_jni.h>

#include "Renderer.h"
#include "fullpipeline/FullPipelineRenderer.h"
#include "pixeloutput/PixelOutputRenderer.h"
#include "shaderperf/ShaderPerfRenderer.h"
#include "contextswitch/ContextSwitchRenderer.h"

// Holds the current benchmark's renderer.
Renderer* gRenderer = NULL;

double currentTimeMillis() {
    struct timeval tv;
    gettimeofday(&tv, (struct timezone *) NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_android_pts_opengl_primitive_GLActivity_startBenchmark(
        JNIEnv* env, jclass clazz, jint numFrames, jdoubleArray frameTimes) {
    if (gRenderer == NULL) {
        return false;
    }

    // Sets up the renderer.
    bool success = gRenderer->setUp();

    // Draw to the screen. This allows debugging and also warms up the HW.
    success = success && gRenderer->draw(false);

    // Records the start time.
    double start = currentTimeMillis();

    // Draw off the screen.
    for (int i = 0; i < numFrames && success; i++) {
        // Draw a frame.
        success = gRenderer->draw(true);
    }

    // Records the end time.
    double end = currentTimeMillis();

    // Sets the times in the Java array.
    double times[] = {start, end};
    env->SetDoubleArrayRegion(frameTimes, 0, 2, times);

    // Tears down and deletes the renderer.
    success = gRenderer->tearDown() && success;
    delete gRenderer;
    gRenderer = NULL;
    return success;
}

// The following functions create the renderers for the various benchmarks.
extern "C" JNIEXPORT void JNICALL
Java_com_android_pts_opengl_primitive_GLActivity_setupFullPipelineBenchmark(
        JNIEnv* env, jclass clazz, jobject surface, jint workload) {
    gRenderer = new FullPipelineRenderer(ANativeWindow_fromSurface(env, surface), workload);
}

extern "C" JNIEXPORT void JNICALL
Java_com_android_pts_opengl_primitive_GLActivity_setupPixelOutputBenchmark(
        JNIEnv* env, jclass clazz, jobject surface, jint workload) {
    gRenderer = new PixelOutputRenderer(ANativeWindow_fromSurface(env, surface), workload);
}

extern "C" JNIEXPORT void JNICALL
Java_com_android_pts_opengl_primitive_GLActivity_setupShaderPerfBenchmark(
        JNIEnv* env, jclass clazz, jobject surface, jint workload) {
    gRenderer = new ShaderPerfRenderer(ANativeWindow_fromSurface(env, surface), workload);
}

extern "C" JNIEXPORT void JNICALL
Java_com_android_pts_opengl_primitive_GLActivity_setupContextSwitchBenchmark(
        JNIEnv* env, jclass clazz, jobject surface, jint workload) {
    gRenderer = new ContextSwitchRenderer(ANativeWindow_fromSurface(env, surface), workload);
}
