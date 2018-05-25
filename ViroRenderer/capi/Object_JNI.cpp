//
// Object_JNI.cpp
// ViroRenderer
//
// Copyright © 2016 Viro Media. All rights reserved.

#include <memory>
#include <VROModelIOUtil.h>
#include <VROGLTFLoader.h>
#include "VROOBJLoader.h"
#include "VROFBXLoader.h"
#include "VROGeometry.h"
#include "VRONode.h"
#include "Node_JNI.h"
#include "OBJLoaderDelegate_JNI.h"
#include "ViroContext_JNI.h"
#include "VRODefines.h"
#include VRO_C_INCLUDE

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
    JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Object3D_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type Object3D_##method_name
#endif

extern "C" {

VRO_METHOD(void, nativeLoadModelFromURL)(VRO_ARGS
                                         VRO_STRING jURL,
                                         VRO_REF(VRONode) node_j,
                                         VRO_REF(ViroContext) context_j,
                                         VRO_INT jModelType,
                                         VRO_LONG requestId) {
    VRO_METHOD_PREAMBLE;
    VROPlatformSetEnv(env); // Invoke in case renderer has not yet initialized
    ModelType modelType = VROGetModelType(jModelType);
    std::string URL = VRO_STRING_STL(jURL);
    std::shared_ptr<OBJLoaderDelegate> delegateRef = std::make_shared<OBJLoaderDelegate>(obj, env);
    std::function<void(std::shared_ptr<VRONode> node, bool success)> onFinish =
            [delegateRef, modelType, requestId](std::shared_ptr<VRONode> node, bool success) {
                if (!success) {
                    delegateRef->objFailed("Failed to load OBJ");
                }
                else {
                    delegateRef->objLoaded(node, modelType, requestId);
                }
            };

    std::shared_ptr<VRONode> node = VRO_REF_GET(VRONode, node_j);
    std::shared_ptr<ViroContext> context = VRO_REF_GET(ViroContext, context_j);
    std::shared_ptr<VRODriver> driver = context->getDriver();
    VROPlatformDispatchAsyncRenderer([modelType, node, URL, driver, onFinish] {
        if (modelType == ModelType::FBX) {
            VROFBXLoader::loadFBXFromResource(URL, VROResourceType::URL, node, driver, onFinish);
        } else if (modelType == ModelType::OBJ) {
            VROOBJLoader::loadOBJFromResource(URL, VROResourceType::URL, node, driver, onFinish);
        } else if (modelType == ModelType::GLTF || modelType == ModelType::GLB) {
            VROGLTFLoader::loadGLTFFromResource(URL, {}, VROResourceType::URL,
                                                node, modelType == ModelType::GLB, driver, onFinish);
        } else {
            perr("Viro: Attempted to load unsupported model type within Object_JNI.");
            onFinish(nullptr, false);
        }
    });
}

VRO_METHOD(void, nativeLoadModelFromResources)(VRO_ARGS
                                               VRO_STRING jresource,
                                               VRO_OBJECT resourceMap_j,
                                               VRO_REF(VRONode) node_j,
                                               VRO_REF(ViroContext) context_j,
                                               VRO_INT jModelType,
                                               VRO_LONG requestId) {
    VRO_METHOD_PREAMBLE;
    VROPlatformSetEnv(env); // Invoke in case renderer has not yet initialized
    ModelType modelType = VROGetModelType(jModelType);
    std::string resource = VRO_STRING_STL(jresource);
    std::shared_ptr<OBJLoaderDelegate> delegateRef = std::make_shared<OBJLoaderDelegate>(obj, env);
    std::function<void(std::shared_ptr<VRONode> node, bool success)> onFinish =
            [delegateRef, modelType, requestId](std::shared_ptr<VRONode> node, bool success) {
                if (!success) {
                    delegateRef->objFailed("Failed to load OBJ");
                }
                else {
                    delegateRef->objLoaded(node, modelType, requestId);
                }
            };

    std::map<std::string, std::string> resourceMap;
    bool hasResourceMap = false;
    if (!VRO_IS_OBJECT_NULL(resourceMap_j)) {
        resourceMap = VROPlatformConvertFromJavaMap(resourceMap_j);
        hasResourceMap = true;
    }

    std::shared_ptr<VRONode> node = VRO_REF_GET(VRONode, node_j);
    std::shared_ptr<ViroContext> context = VRO_REF_GET(ViroContext, context_j);
    std::shared_ptr<VRODriver> driver = context->getDriver();
    VROPlatformDispatchAsyncRenderer([modelType, resource, hasResourceMap, resourceMap, node, driver, onFinish] {
        if (modelType == ModelType::FBX) {
            if (!hasResourceMap) {
                VROFBXLoader::loadFBXFromResource(resource, VROResourceType::BundledResource, node,
                                                  driver, onFinish);
            } else {
                VROFBXLoader::loadFBXFromResources(resource, VROResourceType::BundledResource, node,
                                                   resourceMap, driver, onFinish);
            }
        } else if (modelType == ModelType::OBJ) {
            if (!hasResourceMap) {
                VROOBJLoader::loadOBJFromResource(resource, VROResourceType::BundledResource, node,
                                                  driver, onFinish);
            } else {
                VROOBJLoader::loadOBJFromResources(resource, VROResourceType::BundledResource, node,
                                                   resourceMap, driver, onFinish);
            }
        } else if (modelType == ModelType::GLTF || modelType == ModelType::GLB) {
            VROGLTFLoader::loadGLTFFromResource(resource, resourceMap, VROResourceType::BundledResource,
                                                node, modelType == ModelType::GLB,
                                                driver, onFinish);
        } else {
            perr("Viro: Attempted to load unsupported model type within Object_JNI.");
            onFinish(nullptr, false);
        }
    });
}

} // extern "C"