#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "src/engine.hpp"
#include "src/ibl.hpp"
#include "src/bones.hpp"
#include "src/renderer.hpp"
#include "src/util.hpp"

int main()
{
    // initialize engine
    Engine engine{};
    if (!engine.init(640, 480, "OpenGL Window"))
    {
        std::cout << "Failed to initialize engine!\n";
        return 1;
    }

    std::cout << "Initialized engine!\n";
    engine.setCameraEnabled(true);

    // use only gltf files for now
    engine.addModel("spartan", "data/models/spartan.glb");

    Model* spartan{engine.getModel("spartan")};
    BoneAnimation spartanAnimation{"data/models/spartan.glb", spartan};
    BoneAnimator spartanAnimator{&spartanAnimation};

    // ----------- IBL ------------ //
    IBLGenerator iblGenerator{&engine};
    iblGenerator.init("data/skyboxes/clouds.hdr", "data/IBL/clouds/output_iem.hdr", "data/IBL/brdf_lut.png", &engine);

    // ----------- Deferred Rendering ----------- //
    DeferredRenderer* dfRenderer{engine.getDeferredRenderer()};

    RenderQueue renderQueue{&engine};

    // reset window viewport
    glViewport(0, 0, engine.getWidth(), engine.getHeight());

    while (!engine.getQuit())
    {
        // update game state
        spartanAnimator.updateAnimation(engine.getDeltaTime());

        glm::mat4 model;
        for (std::size_t z{0}; z < 3; ++z)
        {
            for (std::size_t x{0}; x < 3; ++x)
            {
                model = glm::translate(
                    glm::mat4{1.0f},
                    {static_cast<float>(x) * 100.0f + std::sin(static_cast<float>(x * (z + 1))) * 10.f, 0.0f,
                     static_cast<float>(z) * 100.0f + std::cos(static_cast<float>(x * (z + 1))) * 10.f});
                renderQueue.addDynamicModel(spartan, model);
            }
        }

        renderQueue.renderFrame(engine.getShader("gBuffer"), dfRenderer, engine.getShader("texturePBR"),
                                engine.getPostProcessor(), &engine, &iblGenerator,
                                engine.getCameraPosition());

        // update engine
        renderQueue.update();
        engine.displayFrameTime();
        engine.update();
    }

    return 0;
}
