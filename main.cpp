#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "src/engine.hpp"
#include "src/ibl.hpp"
#include "src/renderer.hpp"
#include "src/scene.hpp"

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
    engine.addModel("spartan", "data/models/table.glb");

    Model* spartan{engine.getModel("spartan")};

    // ----------- IBL ------------ //
    IBLGenerator iblGenerator{&engine};
    iblGenerator.init("data/skyboxes/clouds.hdr", "data/IBL/clouds/output_iem.hdr", "data/IBL/brdf_lut.png", &engine);

    // ----------- Deferred Rendering ----------- //
    const DeferredRenderer* dfRenderer{engine.getDeferredRenderer()};
    RenderQueue renderQueue{&engine};

    // reset window viewport
    glViewport(0, 0, engine.getWidth(), engine.getHeight());

    Scene scene{&engine};
    for (std::size_t z{0}; z < 3; ++z)
    {
        for (std::size_t x{0}; x < 3; ++x)
        {
            Bounds::Transform transform;
            transform.setLocalPosition(
            {static_cast<float>(x) * 45.0f + std::sin(static_cast<float>(x * (z + 1))) * 10.f, 0.0f,
             static_cast<float>(z) * 45.0f + std::cos(static_cast<float>(x * (z + 1))) * 10.f});
            transform.setLocalScale({5.f, 5.f, 5.f});
            scene.addEntity("data/models/table.glb", transform, false);
        }
    }

    while (!engine.getQuit())
    {
        // update game state
        spartan->updateAnimation(engine.getDeltaTime());

        glm::mat4 model;
        const Bounds::Frustum camFrustum{engine.getCameraFrustum()};
        for (Entity* entity : scene.getEntities())
        {
            entity->update(engine.getDeltaTime());
            if (entity->getBoundingVolume()->onFrustum(camFrustum, entity->getTransform()))
            {
                model = entity->getTransform().getModelMat();
                renderQueue.addDynamicModel(entity->getModel(), model);
            }
        }

        renderQueue.renderFrame(engine.getShader("gBuffer"), dfRenderer, engine.getShader("texturePBR"),
                                engine.getPostProcessor(), &engine, &iblGenerator, engine.getCameraPosition());

        // update engine
        renderQueue.update();
        engine.displayFrameTime();
        engine.update();
    }

    return 0;
}
