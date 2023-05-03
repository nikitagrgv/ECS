#include "ECS.h"

#include <iostream>

ECS ecs;

struct Position {
    float x{0.0f};
    float y{0.0f};
};

struct Speed {
    float x{0.0f};
    float y{0.0f};
};

struct Mass {
    float mass{1.0f};
};

class PhysicsSystem : public System
{
public:
    void update()
    {
        for (const Entity &entity: entities_)
        {
            auto &pos = ecs.getComponent<Position>(entity);
            auto &mass = ecs.getComponent<Mass>(entity);

            std::cout << "pos: " << pos.x << " " << pos.y  << " | mass: " << mass.mass << std::endl;
            pos.x += 1;
            mass.mass -= 0.1;
        }
    }
};

DECLARE_TYPE_INFO(Position);
DECLARE_TYPE_INFO(Mass);
DECLARE_TYPE_INFO(PhysicsSystem);

int main()
{
    ecs.registerSystem<PhysicsSystem>();
    ecs.registerComponents<Position, Mass>();
    ecs.setSystemComponents<PhysicsSystem, Position, Mass>();


    auto *system = ecs.getSystem<PhysicsSystem>();
    for (int i = 0; i < 5; ++i)
    {
        const Entity ent = ecs.createEntity();

        Position pos = {(float) i, (float) i * 10};
        ecs.addComponent<Position>(ent, pos);

        Mass mass = {(float) i};
        ecs.addComponent(ent, mass);
    }

    for (int i = 0; i < 5; ++i)
    {
        const Entity ent = ecs.createEntity();
        Position pos = {(float) i, (float) i * 10};
        ecs.addComponent<Position>(ent, pos);
    }


    for (int i = 0; i < 5; ++i)
    {
        std::cout << "iterate " << i << std::endl;
        system->update();
    }


    //    //    ECS ecs;
    //    EntityManager entity_manager;
    //    Entity e0 = entity_manager.createEntity();
    //    Entity e1 = entity_manager.createEntity();
    //
    //
    //    ComponentManager component_manager;
    //    component_manager.registerComponent<Position>();
    //    component_manager.addComponent(e0, Position{1, 2, 3});
    //    component_manager.addComponent(e1, Position{1, 2, 3});
    //
    //    entity_manager.removeEntity(e0);
    //    component_manager.entityDestroyed(e0);
    //
    //    auto &pos = component_manager.getComponent<Position>(1);


    return 0;
}
