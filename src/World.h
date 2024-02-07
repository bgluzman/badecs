#pragma once

#include <any>
#include <cstdint>
#include <functional>
#include <gsl/gsl>
#include <iostream>
#include <ranges>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace bad {

// CopyConstructible is a requirement for std::any
template <typename T>
concept Component = std::is_copy_constructible_v<T>;

using EntityId = std::uint32_t;
using ComponentId = std::uint32_t;

struct Column {
  std::unordered_map<EntityId, std::unique_ptr<std::any>> components = {};
};

class ComponentStorage {
public:
  template <Component T>
  static ComponentId getComponentId();
  // TODO (bgluzman): we should try to unify these, I think?
  template <Component T>
  Column& getColumn();
  Column& getColumn(ComponentId componentId);

  template <Component T, typename... Ts>
  void add(EntityId entityId, Ts&&...args);
  template <Component T>
  std::optional<gsl::not_null<T *>> get(EntityId entityId);
  template <Component T>
  void set(EntityId entityId, const T& value);

private:
  static inline ComponentId kComponentIdCounter = 1;
  std::unordered_map<ComponentId, std::unique_ptr<Column>> columns_ = {};
};

class EntityHandle {

public:
  explicit EntityHandle(EntityId                          id,
                        gsl::not_null<ComponentStorage *> components);
  ~EntityHandle() noexcept = default;
  EntityHandle(EntityHandle&&) noexcept = default;
  EntityHandle& operator=(EntityHandle&&) noexcept = default;
  EntityHandle(const EntityHandle&) = delete;
  EntityHandle& operator=(const EntityHandle&) = delete;

  [[nodiscard]] EntityId getId() const noexcept { return id_; }

  template <Component T, typename... Ts>
  void add(Ts&&...args);
  template <Component T>
  std::optional<gsl::not_null<T *>> get();
  template <Component T>
  void set(const T& value);

private:
  EntityId                          id_;
  gsl::not_null<ComponentStorage *> components_;
};

class World {
public:
  EntityHandle                spawnEntity();
  std::optional<EntityHandle> getEntity(EntityId id);

  template <Component... Args>
  void addSystem(std::invocable<Args...> auto&& system);
  template <Component... Args>
  void addSystem(std::invocable<EntityHandle, Args...> auto&& system);

  template <Component... Args>
  void query(std::invocable<Args...> auto&& callback);
  template <Component... Args>
  void query(std::invocable<EntityHandle, Args...> auto&& callback);

  template <Component... Args>
  void tick();

private:
  template <Component Arg, Component... Args>
  std::set<EntityId> getQueryComponents();
  // TODO (bgluzman): unify this with generic version above
  std::set<EntityId> getQueryComponents(const std::vector<ComponentId>& ids);

  static inline EntityId            kEntityIdCounter = 1;
  std::unordered_set<EntityId>      entities_ = {};
  std::unique_ptr<ComponentStorage> componentStorage_ =
      std::make_unique<ComponentStorage>();

  class ErasedSystemSpec {
  private:
    template <Component... Args>
    struct SystemSpec {
      explicit SystemSpec(std::invocable<Args...> auto&& system)
          : system_(std::forward<decltype(system)>(system)) {}
      void operator()(World& world, EntityId entityId) {
        // TODO (bgluzman): fixup dereferencing?
        system_(**world.componentStorage_->get<Args>(entityId)...);
      }
      std::function<void(Args...)> system_;
    };

  public:
    template <Component... Args>
    struct tag {};

    template <Component... Args>
    explicit ErasedSystemSpec(std::invocable<Args...> auto&& system,
                              tag<Args...>)
        : systemSpec_(
              SystemSpec<Args...>(std::forward<decltype(system)>(system))) {}

    void operator()(World& world, EntityId entityId) {
      systemSpec_(world, entityId);
    }

  private:
    std::function<void(World&, EntityId)> systemSpec_;
  };
  std::vector<ErasedSystemSpec> systems_ = {};
};

template <Component T>
ComponentId ComponentStorage::getComponentId() {
  static ComponentId id = kComponentIdCounter++;
  return id;
}

template <Component T>
inline Column& ComponentStorage::getColumn() {
  return getColumn(getComponentId<T>());
}

inline Column& ComponentStorage::getColumn(ComponentId componentId) {
  std::unique_ptr<Column>& col = columns_[componentId];
  if (!col) {
    col = std::make_unique<Column>();
  }
  return *col;
}

template <Component T, typename... Ts>
void ComponentStorage::add(EntityId entityId, Ts&&...args) {
  Column& col = getColumn<T>();
  col.components[entityId] = std::make_unique<std::any>(
      std::in_place_type<T>, std::forward<Ts>(args)...);
}

template <Component T>
std::optional<gsl::not_null<T *>> ComponentStorage::get(EntityId entityId) {
  ComponentId componentId = getComponentId<T>();
  if (auto it = columns_.find(componentId); it != columns_.end()) {
    return std::any_cast<T>(it->second->components[entityId].get());
  } else {
    return std::nullopt;
  }
}

template <Component T>
void ComponentStorage::set(EntityId entityId, const T& value) {
  Column& col = getColumn<T>();
  col.components[entityId] = std::make_unique<std::any>(value);
}

inline EntityHandle::EntityHandle(EntityId                          id,
                                  gsl::not_null<ComponentStorage *> components)
    : id_(id), components_(components) {}

template <Component T, typename... Ts>
void EntityHandle::add(Ts&&...args) {
  components_->add<T, Ts...>(id_, std::forward<Ts>(args)...);
}

template <Component T>
std::optional<gsl::not_null<T *>> EntityHandle::get() {
  return components_->get<T>(id_);
}

template <Component T>
void EntityHandle::set(const T& value) {
  components_->set(id_, value);
}

template <Component... Args>
void World::addSystem(std::invocable<Args...> auto&& system) {
  systems_.emplace_back(ErasedSystemSpec(std::forward<decltype(system)>(system),
                                         ErasedSystemSpec::tag<Args...>()));
}

template <Component... Args>
void World::addSystem(std::invocable<EntityHandle, Args...> auto&& system) {
  // TODO (bgluzman)
}

// TODO (bgluzman): actually implement these properly...
template <Component... Args>
void World::query(std::invocable<Args...> auto&& callback) {
  for (EntityId id : getQueryComponents<Args...>()) {
    // TODO (bgluzman): change return types to make this better or provide
    //  a getUnchecked member function?
    callback(**componentStorage_->get<Args>(id)...);
  }
}

template <Component... Args>
void World::query(std::invocable<EntityHandle, Args...> auto&& callback) {
  // TODO (bgluzman): DRY w.r.t above?
  for (EntityId id : getQueryComponents<Args...>()) {
    // TODO (bgluzman): change return types to make this better or provide
    //  a getUnchecked member function?
    callback(*getEntity(id), **componentStorage_->get<Args>(id)...);
  }
}

template <Component Arg, Component... Args>
std::set<EntityId> World::getQueryComponents() {
  if constexpr (sizeof...(Args) == 0) {
    return componentStorage_->getColumn<Arg>().components | std::views::keys |
           std::ranges::to<std::set<EntityId>>();
  } else {
    // TODO (bgluzman): could we reuse the set from the recursive call?
    std::set<EntityId> result;
    std::ranges::set_intersection(
        // TODO (bgluzman): can probably get around making this intermediate
        //  set; just have to maintain sorted order...
        componentStorage_->getColumn<Arg>().components | std::views::keys |
            std::ranges::to<std::set<EntityId>>(),
        getQueryComponents<Args...>(), std::inserter(result, result.begin()));
    return result;
  }
}

// TODO (bgluzman): unify this with generic version above
inline std::set<EntityId>
World::getQueryComponents(const std::vector<ComponentId>& ids) {
  if (std::empty(ids))
    return {};

  std::set<EntityId> result = componentStorage_->getColumn(ids[0]).components |
                              std::views::keys |
                              std::ranges::to<std::set<EntityId>>();
  for (std::size_t i = 1; i < ids.size(); ++i) {
    std::ranges::set_intersection(
        componentStorage_->getColumn(ids[i]).components | std::views::keys |
            std::ranges::to<std::set<EntityId>>(),
        result, std::inserter(result, result.begin()));
  }
  return result;
}

// TODO (bgluzman): obviously don't pass Args as template params...should be
//  computable from ErasedSystemSpec
template <Component... Args>
inline void World::tick() {
  for (ErasedSystemSpec& system : systems_) {
    for (EntityId id : getQueryComponents<Args...>()) {
      system(*this, id);
    }
  }
}

}  // namespace bad