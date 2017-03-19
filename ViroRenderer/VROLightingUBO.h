//
//  VROLightingUBO.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/11/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROOpenGL.h"
#include <vector>
#include <atomic>
#include <memory>

// Fixed binding point used for lighting
static int sUBOBindingPoint = 0;

// Grouped in 4N slots, matching lighting_general_functions.glsl
typedef struct {
    int type;
    float attenuation_start_distance;
    float attenuation_end_distance;
    float attenuation_falloff_exp;
    
    float position[4];
    float direction[4];
    
    float color[3];
    float spot_inner_angle;
    
    float spot_outer_angle;
    float padding3;
    float padding4;
    float padding5;
} VROLightData;

typedef struct {
    int num_lights;
    float padding0, padding1, padding2;
    
    float ambient_light_color[4];
    VROLightData lights[8];
} VROLightingData;

class VROLight;
class VROShaderProgram;

class VROLightingUBO {
    
public:
    
    VROLightingUBO(int hash, const std::vector<std::shared_ptr<VROLight>> &lights) :
        _hash(hash),
        _lightingUBOBindingPoint(sUBOBindingPoint),
        _lights(lights),
        _needsUpdate(false) {
        
        glGenBuffers(1, &_lightingUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, _lightingUBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(VROLightingData), NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        
        // Links the UBO and the binding point
        glBindBufferBase(GL_UNIFORM_BUFFER, _lightingUBOBindingPoint, _lightingUBO);
        updateLights();
    }
    
    virtual ~VROLightingUBO() {
        glDeleteBuffers(1, &_lightingUBO);
    }
    
    /*
     Bind this lighting UBO into the "lighting" block for the given program.
     */
    void bind(std::shared_ptr<VROShaderProgram> &program);
    
    /*
     Invoke to indicate that a light in this UBO has changed. When this occurs
     we have to rewrite the lights to the UBO.
     */
    void setNeedsUpdate() {
        _needsUpdate = true;
    }
    
    /*
     Hash which uniquely identifies the set of lights composing this UBO.
     */
    int getHash() const {
        return _hash;
    }
    
private:
    
    int _hash;
    
    /*
     The uniform buffer object ID and binding point for lighting parameters.
     */
    GLuint _lightingUBO;
    const int _lightingUBOBindingPoint = 0;
    
    /*
     The lights that are a part of this UBO.
     */
    std::vector<std::shared_ptr<VROLight>> _lights;
    
    /*
     True if a light in this UBO has changed.
     */
    bool _needsUpdate;
    
    /*
     Update the lights in this UBO, rewriting them to the buffer.
     */
    void updateLights();
    
};
