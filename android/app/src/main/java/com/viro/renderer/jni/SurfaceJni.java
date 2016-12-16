/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

/**
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Java JNI Wrapper     : com.viro.renderer.jni.SurfaceJni.java
 * Cpp JNI wrapper      : Surface_JNI.cpp
 * Cpp Object           : VROSurface.cpp
 */
public class SurfaceJni extends BaseGeometry {
    public SurfaceJni(float width, float height) {
        mNativeRef = nativeCreateSurface(width, height);
    }

    public SurfaceJni(float width, float height, SurfaceJni oldSurface) {
        mNativeRef = nativeCreateSurfaceFromSurface(width, height, oldSurface.mNativeRef);
    }

    public void destroy(){
        nativeDestroySurface(mNativeRef);
    }

    public void setVideoTexture(VideoTextureJni texture){
        nativeSetVideoTexture(mNativeRef, texture.mNativeRef);
    }

    public void setImageTexture(TextureJni texture) {
        nativeSetImageTexture(mNativeRef, texture.mNativeRef);
    }

    public void setMaterial(MaterialJni material) {
        nativeSetMaterial(mNativeRef, material.mNativeRef);
    }

    /**
     * Native Functions called into JNI
     */
    private native long nativeCreateSurface(float width, float height);
    private native long nativeCreateSurfaceFromSurface(float width, float height, long oldSurfaceRef);

    private native void nativeDestroySurface(long surfaceRef);

    private native void nativeSetVideoTexture(long surfaceRef, long textureRef);
    private native void nativeSetImageTexture(long surfaceRef, long textureRef);
    private native void nativeSetMaterial(long surfaceRef, long materialRef);

    @Override
    public void attachToNode(NodeJni node) {
        nativeAttachToNode(mNativeRef, node.mNativeRef);
    }
    private native void nativeAttachToNode(long surfaceRef, long nodeReference);
}