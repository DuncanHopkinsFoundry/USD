//
// Copyright 2019 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#include "hdPrman/camera.h"
#include "hdPrman/cameraContext.h"

#include "pxr/imaging/hd/sceneDelegate.h"

PXR_NAMESPACE_OPEN_SCOPE

HdPrmanCamera::HdPrmanCamera(SdfPath const& id)
    : HdCamera(id)
{
}

HdPrmanCamera::~HdPrmanCamera() = default;

/* virtual */
void
HdPrmanCamera::Sync(HdSceneDelegate *sceneDelegate,
                    HdRenderParam   *renderParam,
                    HdDirtyBits     *dirtyBits)
{  
    HD_TRACE_FUNCTION();
    HF_MALLOC_TAG_FUNCTION();

    if (!TF_VERIFY(sceneDelegate)) {
        return;
    }

    HdPrman_RenderParam * const param =
        static_cast<HdPrman_RenderParam*>(renderParam);

    SdfPath const &id = GetId();
    // Save state of dirtyBits before HdCamera::Sync clears them.
    const HdDirtyBits bits = *dirtyBits;

    if (bits & DirtyTransform) {
        sceneDelegate->SampleTransform(id, &_sampleXforms);
    }

    if (bits & AllDirty) {
        param->GetCameraContext().MarkCameraInvalid(id);
    }

    HdCamera::Sync(sceneDelegate, renderParam, dirtyBits);

    if (bits & DirtyParams) {
        if (id == param->GetCameraContext().GetCameraPath()) {
            // Motion blur in Riley only works correctly if the
            // shutter interval is set before any rprims are synced
            // (and the transform of the riley camera is updated).
            //
            // Thus, we immediately call UpdateRileyShutterInterval
            // here.
            param->UpdateRileyShutterInterval(
                &sceneDelegate->GetRenderIndex());
        }
    }

    // XXX: Should we flip the proj matrix (RHS vs LHS) as well here?

    // We don't need to clear the dirty bits since HdCamera::Sync always clears
    // all the dirty bits.
}

PXR_NAMESPACE_CLOSE_SCOPE

