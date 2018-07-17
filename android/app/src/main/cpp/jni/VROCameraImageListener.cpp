//
// Created by Raj Advani on 7/7/18.
//

#include "VROCameraImageListener.h"
#include "arcore/VROARCameraARCore.h"
#include "VROPlatformUtil.h"
#include "VROSceneRendererARCore.h"
#include "VROSceneController.h"

// +---------------------------------------------------------------------------+
// | Camera Image Frame Listener
// +---------------------------------------------------------------------------+

void VROCameraImageFrameListener::onFrameWillRender(const VRORenderContext &context) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    std::shared_ptr<VROSceneRendererARCore> renderer = _renderer.lock();
    if (!renderer) {
        return;
    }
    std::shared_ptr<VROSceneController> sceneController = renderer->getSceneController();
    if (!sceneController) {
        return;
    }
    std::shared_ptr<VROARScene> scene = std::dynamic_pointer_cast<VROARScene>(sceneController->getScene());
    if (!scene) {
        return;
    }
    std::shared_ptr<VROARSession> session = scene->getARSession();
    if (!session) {
        return;
    }
    std::unique_ptr<VROARFrame> &frame = session->getLastFrame();
    if (!frame) {
        return;
    }
    std::shared_ptr<VROARCameraARCore> camera = std::dynamic_pointer_cast<VROARCameraARCore>(frame->getCamera());
    if (!camera || !camera->isImageDataAvailable()) {
        return;
    }
    int bufferIndex = _bufferIndex;
    _bufferIndex = (_bufferIndex + 1) % 3;

    VROVector3f size = camera->getImageSize();
    int width = (int) size.x;
    int height = (int) size.y;
    if (width <= 0 || height <= 0) {
        return;
    }

    int dataLength = width * height * 4;
    if (!_data[bufferIndex] || _data[bufferIndex]->getDataLength() < dataLength) {
        uint8_t *data = (uint8_t *) malloc(dataLength);
        _data[bufferIndex] = std::make_shared<VROData>(data, dataLength, VRODataOwnership::Move);
        _buffers[bufferIndex] = VRO_NEW_GLOBAL_REF(env->NewDirectByteBuffer(data, dataLength));
    }

    camera->getImageData((uint8_t *) _data[bufferIndex]->getData());

    VRO_WEAK listener_w = VRO_NEW_WEAK_GLOBAL_REF(_listener_j);
    jobject buffer_w = VRO_NEW_WEAK_GLOBAL_REF(_buffers[bufferIndex]);

    VROPlatformDispatchAsyncApplication([listener_w, width, height, buffer_w] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT listener = VRO_NEW_LOCAL_REF(listener_w);
        if (VRO_IS_OBJECT_NULL(listener)) {
            VRO_DELETE_WEAK_GLOBAL_REF(listener_w);
            VRO_DELETE_WEAK_GLOBAL_REF(buffer_w);
            return;
        }

        VRO_OBJECT buffer = VRO_NEW_LOCAL_REF(buffer_w);
        if (VRO_IS_OBJECT_NULL(buffer_w)) {
            VRO_DELETE_WEAK_GLOBAL_REF(listener_w);
            VRO_DELETE_WEAK_GLOBAL_REF(buffer_w);
            return;
        }

        VROPlatformCallHostObjectFunction(buffer, "rewind", "()Ljava/nio/Buffer;");
        VROPlatformCallHostObjectFunction(buffer, "limit", "(I)Ljava/nio/Buffer;", width * height * 4);
        VROPlatformCallHostFunction(listener, "onCameraImageUpdated", "(Ljava/nio/ByteBuffer;II)V", buffer, width, height);
        VRO_DELETE_LOCAL_REF(listener);
        VRO_DELETE_WEAK_GLOBAL_REF(listener_w);
    });
}

void VROCameraImageFrameListener::onFrameDidRender(const VRORenderContext &context) {

}