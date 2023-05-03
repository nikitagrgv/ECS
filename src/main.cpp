#include <iostream>

#include <array>
#include <bitset>
#include <cassert>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define DECLARE_TYPE(type) \
public:                    \
    static inline const char *getTypeStatic() { return #type; }

using Entity = int;

using ComponentType = std::uint8_t;
inline constexpr ComponentType MAX_COMPONENTS = 32;
using Signature = std::bitset<MAX_COMPONENTS>;

class EntityManager
{
public:
    [[nodiscard]] Entity createEntity()
    {
        Entity entity = get_free_entity();
        entities_.push_back(entity);
        entity_signatures_.emplace(entity, Signature{});
        return entity;
    }

    void removeEntity(Entity entity)
    {
        auto it = std::find(entities_.begin(), entities_.end(), entity);
        assert(it != entities_.end());
        entities_.erase(it);
        assert(entity_signatures_.find(entity) != entity_signatures_.end());
        entity_signatures_.erase(entity);
    }

    void setSignature(Entity entity, Signature signature)
    {
        assert(std::find(entities_.begin(), entities_.end(), entity) != entities_.end());
        assert(entity_signatures_.find(entity) != entity_signatures_.end());
        entity_signatures_[entity] = signature;
    }

    [[nodiscard]] Signature getSignature(Entity entity) const
    {
        assert(std::find(entities_.begin(), entities_.end(), entity) != entities_.end());
        auto it = entity_signatures_.find(entity);
        assert(it != entity_signatures_.end());
        return it->second;
    }

private:
    // TODO: optimize
    [[nodiscard]] Entity get_free_entity() const
    {
        Entity entity = 0;
        while (std::find(entities_.begin(), entities_.end(), entity) != entities_.end())
        {
            entity++;
        }
        return entity;
    }

private:
    std::vector<Entity> entities_;
    std::unordered_map<Entity, Signature> entity_signatures_;
};


class IComponentArray
{
public:
    virtual ~IComponentArray() = default;
    virtual void entityDestroyed(Entity entity) = 0;
};

template<class T>
class ComponentArray final : public IComponentArray
{
public:
    void addData(Entity entity, T data)
    {
        assert(entity_to_index_.find(entity) == entity_to_index_.end());
        const int index = component_arr_.size();
        component_arr_.push_back(data);
        entity_to_index_[entity] = index;
        index_to_entity_[index] = entity;
    }

    void removeData(Entity entity)
    {
        assert(entity_to_index_.find(entity) != entity_to_index_.end());
        const int cur_index = entity_to_index_[entity];
        const int last_index = component_arr_.size() - 1;

        component_arr_[cur_index] = component_arr_[last_index];
        component_arr_.pop_back();

        const Entity last_ent = index_to_entity_[last_index];
        index_to_entity_[cur_index] = last_ent;
        entity_to_index_[last_ent] = cur_index;

        index_to_entity_.erase(last_index);
        entity_to_index_.erase(entity);
    }

    T &getData(Entity entity)
    {
        assert(entity_to_index_.find(entity) != entity_to_index_.end());
        return component_arr_[entity_to_index_[entity]];
    }

    void entityDestroyed(Entity entity) override
    {
        if (entity_to_index_.find(entity) != entity_to_index_.end())
        {
            removeData(entity);
        }
    }

private:
    std::unordered_map<Entity, int> entity_to_index_;
    std::unordered_map<int, Entity> index_to_entity_;
    std::vector<T> component_arr_;
};


class ComponentManager
{
public:
    template<class T>
    void registerComponent()
    {
        const char *type = get_component_type<T>();
        assert(component_arrays_.find(type) == component_arrays_.end());
        assert(next_component_type < MAX_COMPONENTS);
        component_arrays_.emplace(type, std::make_unique<ComponentArray<T>>());
        component_types_.emplace(type, next_component_type);
        next_component_type++;
    }

    template<class T>
    [[nodiscard]] ComponentType getComponentType() const
    {
        const char *type = get_component_type<T>();
        auto it = component_types_.find(type);
        assert(it != component_types_.end());
        return it->second;
    }

    template<class T>
    void addComponent(Entity entity, T component)
    {
        const char *type = get_component_type<T>();
        assert(component_arrays_.find(type) != component_arrays_.end());
        get_component_array<T>()->addData(entity, component);
    }

    template<class T>
    void removeComponent(Entity entity)
    {
        const char *type = get_component_type<T>();
        assert(component_arrays_.find(type) != component_arrays_.end());
        get_component_array<T>()->removeData(entity);
    }

    template<class T>
    [[nodiscard]] T &getComponent(Entity entity) const
    {
        const char *type = get_component_type<T>();
        assert(component_arrays_.find(type) != component_arrays_.end());
        return get_component_array<T>()->getData(entity);
    }

    void entityDestroyed(Entity entity)
    {
        for (const auto &it: component_arrays_)
        {
            it.second->entityDestroyed(entity);
        }
    }

private:
    template<class T>
    static inline const char *get_component_type()
    {
        return T::getTypeStatic();
    }

    template<class T>
    [[nodiscard]] ComponentArray<T> *get_component_array() const
    {
        const char *type = get_component_type<T>();
        assert(component_arrays_.find(type) != component_arrays_.end());
        auto it = component_arrays_.find(type);
        assert(it != component_arrays_.end());
        assert(dynamic_cast<ComponentArray<T> *>(it->second.get()));
        return static_cast<ComponentArray<T> *>(it->second.get());
    }

private:
    using ComponentArrayPtr = std::unique_ptr<IComponentArray>;
    std::unordered_map<const char *, ComponentArrayPtr> component_arrays_;
    std::unordered_map<const char *, ComponentType> component_types_;
    ComponentType next_component_type = 0;
};


class System
{
public:
    ~System() = default;

    void addEntity(Entity entity)
    {
        entities_.emplace(entity);
    }

    void removeEntity(Entity entity)
    {
        entities_.erase(entity);
    }

    void entityDestroyed(Entity entity)
    {
        removeEntity(entity);
    }

protected:
    std::set<Entity> entities_;
};


class SystemManager
{
public:
    template<class T>
    T *registerSystem()
    {
        const char *type = get_system_type<T>();
        assert(systems_.find(type) == systems_.end());
        auto it = systems_.emplace(type, std::make_unique<T>());
        system_signatures_.emplace(type, Signature{});
        return static_cast<T *>(it.first->second.get());
    }

    template<class T>
    void setSignature(Signature signature)
    {
        const char *type = get_system_type<T>();
        assert(systems_.find(type) != systems_.end());
        assert(system_signatures_.find(type) != system_signatures_.end());
        system_signatures_[type] = signature;
    }

    void entityDestroyed(Entity entity)
    {
        for (const auto &it: systems_)
        {
            it.second->entityDestroyed(entity);
        }
    }

    void entitySignatureChanged(Entity entity, Signature entity_signature)
    {
        for (const auto &it: systems_)
        {
            const auto &system = it.second;
            const auto &system_signature = system_signatures_[it.first];

            if ((entity_signature & system_signature) == system_signature)
            {
                system->addEntity(entity);
            } else
            {
                system->removeEntity(entity);
            }
        }
    }

private:
    template<class T>
    static inline const char *get_system_type()
    {
        return T::getTypeStatic();
    }

private:
    using SystemPtr = std::unique_ptr<System>;
    std::unordered_map<const char *, SystemPtr> systems_;
    std::unordered_map<const char *, Signature> system_signatures_;
};


class ECS
{
public:
    [[nodiscard]] Entity createEntity()
    {
        return entity_manager_.createEntity();
    }

    void destroyEntity(Entity entity)
    {
        entity_manager_.removeEntity(entity);
        component_manager_.entityDestroyed(entity);
        system_manager_.entityDestroyed(entity);
    }

    template<class T>
    void registerComponent()
    {
        component_manager_.registerComponent<T>();
    }

    template<class... Comps>
    void registerComponents()
    {
        ComponentsRegistrator<Comps...>::registerComponents(component_manager_);
    }

    template<class T>
    void addComponent(Entity entity, T component)
    {
        component_manager_.addComponent<T>(entity, component);

        Signature signature = entity_manager_.getSignature(entity);
        signature.set(component_manager_.getComponentType<T>(), true);
        entity_manager_.setSignature(entity, signature);

        system_manager_.entitySignatureChanged(entity, signature);
    }

    template<class T>
    void removeComponent(Entity entity)
    {
        component_manager_.removeComponent<T>(entity);

        Signature signature = entity_manager_.getSignature(entity);
        signature.set(component_manager_.getComponentType<T>(), false);
        entity_manager_.setSignature(entity, signature);

        system_manager_.entitySignatureChanged(entity, signature);
    }

    template<class T>
    [[nodiscard]] T &getComponent(Entity entity)
    {
        return component_manager_.getComponent<T>(entity);
    }

    template<class T>
    ComponentType getComponentType()
    {
        return component_manager_.getComponentType<T>();
    }

    template <class ...Comps>
    Signature getSignature()
    {
        Signature signature;
        SignatureBuilder<Comps...>::addToSignature(component_manager_, signature);
        return signature;
    }

    template<class T>
    T *registerSystem()
    {
        return system_manager_.registerSystem<T>();
    }

    template<class T>
    void setSystemSignature(Signature signature)
    {
        system_manager_.setSignature<T>(signature);
    }

    template<class T, class ...Comps>
    void setSystemComponents()
    {
        setSystemSignature<T>(getSignature<Comps...>());
    }

private:
    //////////////////////////////////////////////////
    template<class... Comps>
    struct SignatureBuilder;

    template<>
    struct SignatureBuilder<>
    {
        static void addToSignature(const ComponentManager &cm, Signature &signature) {}
    };

    template<class First, class... Tail>
    struct SignatureBuilder<First, Tail...>
    {
        static void addToSignature(const ComponentManager &cm, Signature &signature)
        {
            const ComponentType type = cm.getComponentType<First>();
            signature.set(type, true);
            SignatureBuilder<Tail...>::addToSignature(cm, signature);
        }
    };

    //////////////////////////////////////////////////
    template<class... Comps>
    struct ComponentsRegistrator;

    template<>
    struct ComponentsRegistrator<>
    {
        static void registerComponents(ComponentManager &cm) {}
    };

    template<class First, class... Tail>
    struct ComponentsRegistrator<First, Tail...>
    {
        static void registerComponents(ComponentManager &cm)
        {
            cm.registerComponent<First>();
            ComponentsRegistrator<Tail...>::registerComponents(cm);
        }
    };

private:
    EntityManager entity_manager_;
    ComponentManager component_manager_;
    SystemManager system_manager_;
};


ECS ecs;

struct Position {
    DECLARE_TYPE(Position);

public:
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

struct Mass {
    DECLARE_TYPE(Mass);

public:
    float mass{1.0f};
};

class PhysicsSystem : public System
{
    DECLARE_TYPE(PhysicsSystem);

public:
    void update()
    {
        for (const Entity &entity: entities_)
        {
            auto &pos = ecs.getComponent<Position>(entity);
            auto &mass = ecs.getComponent<Mass>(entity);

            std::cout << "pos: " << pos.x << " " << pos.y << " " << pos.z << " | mass: " << mass.mass << std::endl;
            pos.x += 1;
            mass.mass -= 0.1;
        }
    }
};


int main()
{
    auto *system = ecs.registerSystem<PhysicsSystem>();

    ecs.registerComponents<Position, Mass>();
    ecs.setSystemComponents<PhysicsSystem, Position, Mass>();



    for (int i = 0; i < 5; ++i)
    {
        const Entity ent = ecs.createEntity();

        Position pos = {(float) i, (float) i * 10, (float) i * 100};
        ecs.addComponent<Position>(ent, pos);

        Mass mass = {(float) i};
        ecs.addComponent(ent, mass);
    }

    for (int i = 0; i < 5; ++i)
    {
        const Entity ent = ecs.createEntity();
        Position pos = {(float) i, (float) i * 10, (float) i * 100};
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
