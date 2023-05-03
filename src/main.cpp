#include "ECS.h"

#include <iostream>

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

ECS ecs;

struct Position {
    sf::Vector2<double> pos;
};

struct Velocity {
    sf::Vector2<double> velocity;
};

struct Mass {
    double mass{1.0f};
};

class PhysicsSystem : public System
{
public:
    static constexpr double GRAVITY = 100;

    void update()
    {
        const int entity_count = entities_.size();
        for (const Entity& ent_0 : entities_)
        {
            auto &pos_0 = ecs.getComponent<Position>(ent_0);
            auto &speed_0 = ecs.getComponent<Velocity>(ent_0);
            auto &mass_0 = ecs.getComponent<Mass>(ent_0);

            for (const Entity& ent_1 : entities_)
            {
                if (&ent_0 == &ent_1)
                {
                    continue;
                }

                auto &pos_1 = ecs.getComponent<Position>(ent_1);
                auto &speed_1 = ecs.getComponent<Velocity>(ent_1);
                auto &mass_1 = ecs.getComponent<Mass>(ent_1);


            }
        }

//        for (const Entity &entity: entities_)
//        {
//            auto &pos = ecs.getComponent<Position>(entity);
//            auto &mass = ecs.getComponent<Mass>(entity);
//
//            std::cout << "pos: " << pos.x << " " << pos.y  << " | mass: " << mass.mass << std::endl;
//            pos.x += 1;
//            mass.mass -= 0.1;
//        }
    }
};


class RenderSystem : public System, public sf::Drawable
{
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override
    {
        for (const Entity &entity: entities_)
        {
            sf::CircleShape shape{5};
            const auto &pos = ecs.getComponent<Position>(entity);
            shape.setPosition((sf::Vector2f)pos.pos);
            target.draw(shape, states);
        }
    }
};

DECLARE_TYPE_INFO(Position);
DECLARE_TYPE_INFO(Mass);
DECLARE_TYPE_INFO(Velocity);

DECLARE_TYPE_INFO(PhysicsSystem);
DECLARE_TYPE_INFO(RenderSystem);

int main()
{
    sf::RenderWindow window(sf::VideoMode(1024, 768), "Circuits");
    window.setVerticalSyncEnabled(true);

    ecs.registerComponents<Position, Mass, Velocity>();

    ecs.registerSystem<PhysicsSystem>();
    ecs.setSystemComponents<PhysicsSystem, Position, Mass, Velocity>();

    ecs.registerSystem<RenderSystem>();
    ecs.setSystemComponents<RenderSystem, Position>();

    auto *physic_sys = ecs.getSystem<PhysicsSystem>();
    auto *render_sys = ecs.getSystem<RenderSystem>();

    for (int i = 0; i < 5; ++i)
    {
        const Entity ent = ecs.createEntity();

        Position pos = {{(double) i, (double) i * 10}};
        ecs.addComponent(ent, pos);

        Velocity vel = {{(double) i, (double) i * 10}};
        ecs.addComponent(ent, vel);

        Mass mass = {(double) i};
        ecs.addComponent(ent, mass);
    }



    std::vector<sf::Vertex> axis;
    constexpr float AXIS_LENGTH = 10000.0f;
    axis.emplace_back(sf::Vector2f{-AXIS_LENGTH, 0.f}, sf::Color::Red);
    axis.emplace_back(sf::Vector2f{AXIS_LENGTH, 0.f}, sf::Color::Red);
    axis.emplace_back(sf::Vector2f{0.f, -AXIS_LENGTH}, sf::Color::Green);
    axis.emplace_back(sf::Vector2f{0.f, AXIS_LENGTH}, sf::Color::Green);
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                {
                    window.close();
                }
            }
        }

        /////////////////////////////////////////////////////
        sf::Vector2f camera_move_dir;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            camera_move_dir.y -= 1;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            camera_move_dir.y += 1;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            camera_move_dir.x -= 1;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            camera_move_dir.x += 1;

        constexpr float CAMERA_MOVE_SPEED = 5.0f;
        sf::View view = window.getView();
        view.move(camera_move_dir * CAMERA_MOVE_SPEED);
        window.setView(view);

        /////////////////////////////////////////////////////

        physic_sys->update();
        window.clear();
        window.draw(axis.data(), 4, sf::Lines);
        window.draw(*render_sys);
        window.display();
    }

    return 0;
}
