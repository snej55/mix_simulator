// Created by Jens Kromdijk 25-01-2026

#ifndef UI_H
#define UI_H

#include "engine_types.hpp"

class UIRenderer : EngineObject
{
public:
    explicit UIRenderer(EngineObject* parent);
    ~UIRenderer() override;

    void init(int width, int height);
    void generate(int width, int height);

    void free() const;

    [[nodiscard]] int getWidth() const { return m_width; }
    [[nodiscard]] int getHeight() const { return m_height; }

    [[nodiscard]] unsigned int getFBO() const { return m_FBO; }
    [[nodiscard]] unsigned int getTEX() const { return m_TEX; }

private:
    int m_width{0};
    int m_height{0};

    unsigned int m_FBO{};
    unsigned int m_TEX{};
};

#endif
