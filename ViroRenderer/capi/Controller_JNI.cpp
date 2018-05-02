//
//  Controller_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <memory>
#include "ViroContext_JNI.h"
#include "EventDelegate_JNI.h"
#include "ARUtils_JNI.h"
#include "VROInputControllerBase.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Controller_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type Controller_##method_name
#endif

extern "C" {

VRO_METHOD(void, nativeSetEventDelegate)(VRO_ARGS
                                         VRO_REF(ViroContext) render_context_ref,
                                         VRO_REF(EventDelegate_JNI) native_delegate_ref) {
    std::weak_ptr<ViroContext> nativeContext_w = VRO_REF_GET(ViroContext,  render_context_ref);
    std::weak_ptr<EventDelegate_JNI> delegate_w = VRO_REF_GET(EventDelegate_JNI, native_delegate_ref);

    VROPlatformDispatchAsyncRenderer([nativeContext_w, delegate_w] {
        std::shared_ptr<ViroContext> nativeContext = nativeContext_w.lock();
        std::shared_ptr<EventDelegate_JNI> delegate = delegate_w.lock();

        if (nativeContext && delegate) {
            std::shared_ptr<VROInputPresenter> controllerPresenter
                    = nativeContext->getInputController()->getPresenter();
            controllerPresenter->setEventDelegate(delegate);
        }
    });
}

VRO_METHOD(void, nativeEnableReticle)(VRO_ARGS
                                      VRO_REF(ViroContext) render_context_ref,
                                      VRO_BOOL enable) {
    std::weak_ptr<ViroContext> nativeContext_w = VRO_REF_GET(ViroContext, render_context_ref);

    VROPlatformDispatchAsyncRenderer([nativeContext_w, enable] {
        std::shared_ptr<ViroContext> nativeContext = nativeContext_w.lock();
        if (!nativeContext) {
            return;
        }

        std::shared_ptr<VROInputPresenter> controllerPresenter = nativeContext->getInputController()->getPresenter();
        std::shared_ptr<VROReticle> reticle = controllerPresenter->getReticle();
        if (reticle != nullptr) {
            reticle->setEnabled(enable);
        }
    });
}

VRO_METHOD(void, nativeEnableController)(VRO_ARGS
                                         VRO_REF(ViroContext) render_context_ref,
                                         VRO_BOOL enable) {
    std::weak_ptr<ViroContext> nativeContext_w = VRO_REF_GET(ViroContext, render_context_ref);

    VROPlatformDispatchAsyncRenderer([nativeContext_w, enable] {
        std::shared_ptr<ViroContext> nativeContext = nativeContext_w.lock();
        if (!nativeContext) {
            return;
        }

        std::shared_ptr<VROInputPresenter> controllerPresenter = nativeContext->getInputController()->getPresenter();
        controllerPresenter->getRootNode()->setHidden(!enable);
    });
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeGetControllerForwardVector)(VRO_ARGS
                                                              VRO_REF(ViroContext) context_j) {
    std::shared_ptr<ViroContext> context = VRO_REF_GET(ViroContext, context_j);
    VROVector3f position = context->getInputController()->getPresenter()->getLastKnownForward();
    return ARUtilsCreateFloatArrayFromVector3f(position);
}

VRO_METHOD(void, nativeGetControllerForwardVectorAsync)(VRO_ARGS
                                                        VRO_REF(ViroContext) native_render_context_ref,
                                                        VRO_OBJECT callback) {
    VRO_WEAK weakCallback = VRO_NEW_WEAK_GLOBAL_REF(callback);
    std::weak_ptr<ViroContext> helperContext_w = VRO_REF_GET(ViroContext, native_render_context_ref);

    VROPlatformDispatchAsyncApplication([helperContext_w, weakCallback] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT jCallback = VRO_NEW_LOCAL_REF(weakCallback);
        if (VRO_IS_OBJECT_NULL(jCallback)) {
            return;
        }
        std::shared_ptr<ViroContext> helperContext = helperContext_w.lock();
        if (!helperContext) {
            return;
        }
        VROVector3f position = helperContext->getInputController()->getPresenter()->getLastKnownForward();
        VROPlatformCallHostFunction(jCallback,
                                    "onGetForwardVector", "(FFF)V", position.x, position.y, position.z);
        VRO_DELETE_LOCAL_REF(jCallback);
        VRO_DELETE_WEAK_GLOBAL_REF(weakCallback);
    });
}
/**
 * TODO VIRO-704: Add APIs for custom controls - replacing Obj or adding tooltip support.
 */

}  // extern "C"
