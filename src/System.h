#pragma once

#include "Common.h"
#include "ComponentRegistry.h"
#include "EntityHandle.h"

#include <concepts>
#include <functional>
#include <memory>
#include <vector>

namespace bad {

// TODO (bgluzman): support first arg being EntityHandle
class System {
public:
  template <Component... Args>
  static System construct(SystemFunctor<Args...> auto&& system) {
    return System(std::forward<decltype(system)>(system), tag<Args...>{});
  }

  std::vector<EntityId> getQueryComponents(ComponentRegistry& components) {
    return system_->getQueryComponents(components);
  }
  void execute(ComponentRegistry& components, EntityId entityId) {
    system_->execute(components, entityId);
  }

private:
  // TODO (bgluzman): can this be done with a deduction template instead?
  template <Component... Args>
  struct tag {};
  template <Component... Args>
  explicit System(SystemFunctor<Args...> auto&& system, tag<Args...>)
      : system_(
            // TODO (bgluzman): refactor this and use named concept?
            std::make_unique<SystemImpl<
                std::is_invocable_v<decltype(system), EntityHandle, Args...>,
                Args...>>(std::forward<decltype(system)>(system))) {}

  struct SystemInterface {
    virtual ~SystemInterface() noexcept = default;
    virtual std::vector<EntityId>
                 getQueryComponents(ComponentRegistry& components) = 0;
    virtual void execute(ComponentRegistry& components,
                         EntityId           entityId) const = 0;
  };

  template <bool PassEntityHandle, Component... Args>
  struct SystemImpl : public SystemInterface {
  public:
    explicit SystemImpl(SystemFunctor<Args...> auto&& system)
        : system_(std::forward<decltype(system)>(system)) {}

    std::vector<EntityId>
    getQueryComponents(ComponentRegistry& components) override {
      return components.getQueryComponents<Args...>();
    }

    void execute(ComponentRegistry& components,
                 EntityId           entityId) const override {
      if constexpr (PassEntityHandle) {
        // TODO (bgluzman): pass gsl::not_null<ComponentRegistry*> instead?
        system_(EntityHandle(entityId, &components),
                components.getUnchecked<Args>(entityId)...);
      } else {
        system_(components.getUnchecked<Args>(entityId)...);
      }
    }

  private:
    using SystemFunction =
        std::conditional_t<PassEntityHandle,
                           std::function<void(EntityHandle, Args...)>,
                           std::function<void(Args...)>>;
    SystemFunction system_;
  };

  std::unique_ptr<SystemInterface> system_;
};

}  // namespace bad