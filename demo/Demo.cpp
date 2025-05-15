#include "Demo.h"
#include <Assets.h>
#include <Components.h>
#include <Input.h>
#include <MassUtil.h>
#include <Physecs/Components.h>
#include <Renderer.h>

void Demo::onCreate() {
    Assets::loadMesh("box");
    Assets::loadTexture("white.png");
    Assets::loadTexture("physicsTest.png");

    //create platform
    auto platform = registry.create();
    auto [VAO, elements] = Assets::getMesh("box");
    registry.emplace<TransformComponent>(platform, glm::vec3(0), glm::quat(1, 0, 0, 0), glm::vec3(20, 1, 20));
    registry.emplace<RenderComponent>(platform, VAO, elements, Assets::getTexture("white.png"), glm::mat4(1.0f));
    physecs::Geometry platformGeometry = { physecs::BOX };
    platformGeometry.box = { glm::vec3(20, 1, 20) };
    physecs::Collider platformCollider = { glm::vec3(0), glm::quat(1, 0, 0, 0), platformGeometry, defaultMaterial, false, true, 0 };
    registry.emplace<physecs::RigidBodyCollisionComponent>(platform, std::vector{ platformCollider });

    camera.position = glm::vec3(0, 5, 0);
    light.direction = glm::vec3(-0.1, -1, -0.3);
}

void Demo::spawnBox() {
    auto box = registry.create();
    auto [VAO, elements] = Assets::getMesh("box");
    registry.emplace<TransformComponent>(box, camera.position + camera.orientation * camera.defaultDir * 2.f, glm::quat(1, 0, 0, 0), glm::vec3(1, 1, 1));
    registry.emplace<RenderComponent>(box, VAO, elements, Assets::getTexture("physicsTest.png"), glm::mat4(1.0f));
    physecs::Geometry boxGeometry = { physecs::BOX };
    boxGeometry.box = { glm::vec3(1, 1, 1) };
    physecs::Collider boxCollider = { glm::vec3(0), glm::quat(1, 0, 0, 0), boxGeometry, defaultMaterial, false, true, 0 };
    auto col = registry.emplace<physecs::RigidBodyCollisionComponent>(box, std::vector{ boxCollider });
    glm::vec3 com;
    glm::mat3 invInertiaTensor;
    physecs::computeCOMAndInvInertiaTensor(col, 1.f, com, invInertiaTensor);
    registry.emplace<physecs::RigidBodyDynamicComponent>(box, 1.f, com, camera.orientation * camera.defaultDir * 20.f, invInertiaTensor, glm::vec3(0), false);
}

void Demo::onUpdate(float deltaTime) {
    cameraMovementSystem.onUpdate(deltaTime);

    if (Input::getMouseDown(SDL_BUTTON_LEFT)) spawnBox();

    physicsAccum += deltaTime;
    while (physicsAccum >= physicsTimeStep) {
        physicsAccum -= physicsTimeStep;
        physicsScene.simulate(physicsTimeStep);
    }
}

void Demo::draw() {
    for (auto [entity, transform, renderItem] : registry.view<TransformComponent, RenderComponent>().each()) {
        glm::mat4 model(1.0f);
        model = glm::translate(model, transform.position) * glm::toMat4(transform.orientation) * glm::scale(model, transform.scale);
        renderItem.model = model;
    }
    Renderer::draw(camera, light, registry);
}
