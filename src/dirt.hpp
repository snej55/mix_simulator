// Created by Jens Kromdijk 21/03/2026
// Simple class to generate lens dirt texture

#ifndef DIRT_H
#define DIRT_H

class LensDirt
{
public:
    LensDirt(int width, int height);
    ~LensDirt();

    void generate(int width, int height);
    void free();

    void bind() const;

    [[nodiscard]] int getWidth() const { return m_width; }
    [[nodiscard]] int getHeight() const { return m_height; }

    [[nodiscard]] unsigned int getFBO() const { return m_FBO; }
    [[nodiscard]] unsigned int getTEX() const { return m_TEX; }
    [[nodiscard]] bool getInit() const { return m_init; }

private:
    int m_width;
    int m_height;

    unsigned int m_FBO{0};
    unsigned int m_TEX{0};
    bool m_init{false};
};

#endif // DIRT_H
