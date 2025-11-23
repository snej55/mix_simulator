//
// Created by jenskromdijk on 23/11/2025.
//

#ifndef MAIN_RENDERER_H
#define MAIN_RENDERER_H

#include "engine_types.hpp"

class DeferredRenderer final : public EngineObject
{
public:
    explicit DeferredRenderer(EngineObject* parent);
    ~DeferredRenderer() override;

    // setup framebuffers
    void init(unsigned int scrWidth, unsigned int scrHeight);

private:
    // graphics buffer
    unsigned int m_gBuffer{};
    // positionEBuffer.xyz = Position
    // positionEBuffer.w = Emissive r
    unsigned int m_positionEBuffer{};
    // colorBuffer.xyzw = rgba
    unsigned int m_colorBuffer{};
    // normalEBuffer.xyz = Normal
    // normalEBuffer.w = Emissive.g
    unsigned int m_normalEBuffer{};
    // x = AO
    // y = Roughness
    // z = Metallic
    // a = Emissive.b
    unsigned int m_ARMEBuffer{};
    // render buffer
    unsigned int m_RBO{};

    // view port dimensions
    unsigned int m_scrWidth{};
    unsigned int m_scrHeight{};
};


#endif // MAIN_RENDERER_H
