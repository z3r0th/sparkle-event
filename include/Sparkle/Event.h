#ifndef SPARKLE_EVENT_H
#define SPARKLE_EVENT_H

#include <functional>
#include <algorithm>
#include <cassert>
#include <string>
#include <vector>
#include <memory>
#include <map>

// TODO: Support Handle to remove specific functions instead of all functions of specific object
// TODO: Improve performance of Raise function

namespace Sparkle
{
    /// Base Class for events
    class EventBase
    {
    private:
        std::string Name;

    public:
        explicit EventBase(std::string name) : Name(std::move(name)) {}
        explicit EventBase() : Name() {}

        /// Get this event string. The name is set at construction time.
        /// It might be empty
        [[maybe_unused]] [[nodiscard]] inline const std::string &GetName() const { return Name; }
    };

    template<typename... Args> class Event;

    template<typename... Args>
    class EventBinder
    {
        friend Event<Args...>;
        /// Callback wrap. If returning true its active and should be kept. If false, it finished the lifecycle and should be removed from event. Internal use only
        using LifecycleCallback = std::function<bool(Args...)>;
        /// Registered Callback. Public
        using Callback = std::function<void(Args...)>;

    private:
        std::unordered_map<void *, std::vector<LifecycleCallback>> Binds{};

        /// Complete the binding adding it to the Binds map
        /// \tparam T object type
        /// \param bound prepared lifecycle callback function
        /// \param t object reference
        template<typename T>
        void InternalBind(LifecycleCallback bound, T *const t)
        {
            auto it = Binds.find(t);
            if (it != Binds.end())
            {
                it->second.push_back(std::move(bound));
            }
            else
            {
                Binds[t] = {std::move(bound)};
            }
        }

        template<typename T>
        [[maybe_unused]] void Bind(Callback f, T *const t, bool bindOnce)
        {
            assert(t != nullptr && "Cannot bind to a null pointer");
            auto bound = [f, t, bindOnce](Args... args) -> bool {
                f(std::forward<Args>(args)...);
                return !bindOnce;
            };
            InternalBind(bound, t);
        }

        template<typename T>
        [[maybe_unused]] void Bind(Callback f, std::weak_ptr<T> weak, bool bindOnce)
        {
            if (auto t = weak.lock())
            {
                auto bound = [weak, f, bindOnce](Args... args) -> bool {
                    if (!weak.expired()) {
                        f(std::forward<Args>(args)...);
                        return !bindOnce;
                    }
                    return false;
                };
                InternalBind(bound, t.get());
            }
        }

        template<typename T>
        [[maybe_unused]] void Bind(void(T::* const f)(Args...), std::weak_ptr<T> weak, bool bindOnce)
        {
            if (auto t = weak.lock())
            {
                auto bound = [weak, f, bindOnce](Args... args) -> bool {
                    if (auto locked = weak.lock()) {
                        (locked.get()->*f)(std::forward<Args>(args)...);
                        return !bindOnce;
                    }
                    return false;
                };
                InternalBind(bound, t.get());
            }
        }

        template<typename T>
        [[maybe_unused]] void Bind(void(T::* const f)(Args...), T *const t, bool bindOnce)
        {
            assert(t != nullptr && "Cannot bind to a null pointer");
            auto bound = [t, f, bindOnce](Args... args) -> bool {
                (t->*f)(std::forward<Args>(args)...);
                return !bindOnce;
            };
            InternalBind(bound, t);
        }

        [[maybe_unused]] void Bind(Callback cb, bool bindOnce)
        {
            auto bound = [cb, bindOnce](Args... args) -> bool {
                cb(std::forward<Args>(args)...);
                return !bindOnce;
            };
            static void *StandaloneCallbackKey = reinterpret_cast<void *>(-1);
            InternalBind(bound, StandaloneCallbackKey);
        }

    public:

        /// Clears all references from this event
        [[maybe_unused]] void RemoveAll()
        {
            Binds.clear();
        }

        /// Is this object pointer bounded as observer with any function to this event?
        /// \tparam T object type
        /// \param t object pointer
        /// \return true if any reference to this object pointer is found
        template<typename T>
        [[maybe_unused]] [[nodiscard]] bool IsBound(T *t) const
        {
            assert(t != nullptr && "Cannot check bind of a null pointer");
            return Binds.find(t) != Binds.end();
        }

        /// Is this pointer bounded as observer with any function to this event?
        /// \tparam T
        /// \param weak
        /// \return true if any reference to this object pointer is found
        template<typename T>
        [[maybe_unused]] [[nodiscard]] bool IsBound(std::weak_ptr<T> weak) const
        {
            if (auto shared = weak.lock())
            {
                return IsBound(shared.get());
            }
            return false;
        }

        /// Is this pointer bounded as observer with any function to this event?
        /// \tparam T
        /// \param weak
        /// \return true if any reference to this object pointer is found
        template<typename T>
        [[maybe_unused]] [[nodiscard]] bool IsBound(std::shared_ptr<T> shared) const
        {
            if (shared)
            {
                return IsBound(shared.get());
            }
            return false;
        }

        /// Binds this function to the event related to the object. The function will be called only on the next time the event is raised
        /// the function might not be tied to the object, but they will be referenced together, so when removing the object
        /// the function will also be removed
        /// \tparam T object type
        /// \param f function reference
        /// \param t object pointer
        /// \example event.Bind([]{...}, &reference);
        template<typename T>
        [[maybe_unused]] void BindOnce(Callback f, T *const t)
        {
            Bind(f, t, true);
        }

        /// Binds this function to the event related to the object
        /// the function might not be tied to the object, but they will be referenced together, so when removing the object
        /// the function will also be removed
        /// \tparam T object type
        /// \param f function reference
        /// \param t object pointer
        /// \example event.Bind([]{...}, &reference);
        template<typename T>
        [[maybe_unused]] void Bind(Callback f, T *const t)
        {
            Bind(f, t, false);
        }

        /// Converts the shared pointer to a weak pointer and binds this function to the event related to the object.
        /// The function will be called only on the next time the event is raised
        /// the function might not be tied to the object, but they will be referenced together, so when removing the object
        /// the function will also be removed. If the weak pointer is expired it will be removed on next Raise call.
        /// \tparam T object type
        /// \param f function reference
        /// \param weak weak pointer to the object
        /// \example event.Bind([]{...}, weak_ptr);
        template<typename T>
        [[maybe_unused]] void BindOnce(Callback f, std::shared_ptr<T> shared)
        {
            BindOnce(f, std::weak_ptr<T>(shared), true);
        }

        /// Binds this function to the event related to the object. The function will be called only on the next time the event is raised
        /// the function might not be tied to the object, but they will be referenced together, so when removing the object
        /// the function will also be removed. If the weak pointer is expired it will be removed on next Raise call.
        /// \tparam T object type
        /// \param f function reference
        /// \param weak weak pointer to the object
        /// \example event.Bind([]{...}, weak_ptr);
        template<typename T>
        [[maybe_unused]] void BindOnce(Callback f, std::weak_ptr<T> weak)
        {
            Bind(f, weak, true);
        }

        /// Converts the shared pointer to a weak pointer and binds this function to the event related to the object
        /// the function might not be tied to the object, but they will be referenced together, so when removing the object
        /// the function will also be removed. If the weak pointer is expired it will be removed on next Raise call.
        /// \tparam T object type
        /// \param f function reference
        /// \param weak weak pointer to the object
        /// \example event.Bind([]{...}, weak_ptr);
        template<typename T>
        [[maybe_unused]] void Bind(Callback f, std::shared_ptr<T> shared)
        {
            Bind(f, std::weak_ptr<T>(shared), false);
        }

        /// Binds this function to the event related to the object
        /// the function might not be tied to the object, but they will be referenced together, so when removing the object
        /// the function will also be removed. If the weak pointer is expired it will be removed on next Raise call.
        /// \tparam T object type
        /// \param f function reference
        /// \param weak weak pointer to the object
        /// \example event.Bind([]{...}, weak_ptr);
        template<typename T>
        [[maybe_unused]] void Bind(Callback f, std::weak_ptr<T> weak)
        {
            Bind(f, weak, false);
        }

        /// Converts the shared pointer to a weak pointer and binds this object's function to the event.
        /// The function will be called only on the next time the event is raised
        /// The object will call the function and both must be valid (t->*f(...))
        /// If the object expires before this Event does, all references to the object will be removed from this event on next Event Raise
        /// \tparam T object type
        /// \param f function reference
        /// \param weak weak pointer to the object
        /// \example event.Bind(&MyClass::Function, weak_ptr);
        template<typename T>
        [[maybe_unused]] void BindOnce(void(T::* const f)(Args...), std::shared_ptr<T> shared)
        {
            Bind(f, std::weak_ptr<T>(shared), true);
        }

        /// Binds this object's function to the event. The function will be called only on the next time the event is raised
        /// The object will call the function and both must be valid (t->*f(...))
        /// If the object expires before this Event does, all references to the object will be removed from this event on next Event Raise
        /// \tparam T object type
        /// \param f function reference
        /// \param weak weak pointer to the object
        /// \example event.Bind(&MyClass::Function, weak_ptr);
        template<typename T>
        [[maybe_unused]] void BindOnce(void(T::* const f)(Args...), std::weak_ptr<T> weak)
        {
            Bind(f, weak, true);
        }

        /// Converts the shared pointer to a weak pointer and binds this object's function to the event.
        /// The object will call the function and both must be valid (t->*f(...))
        /// If the object expires before this Event does, all references to the object will be removed from this event on next Event Raise
        /// \tparam T object type
        /// \param f function reference
        /// \param weak weak pointer to the object
        /// \example event.Bind(&MyClass::Function, weak_ptr);
        template<typename T>
        [[maybe_unused]] void Bind(void(T::* const f)(Args...), std::shared_ptr<T> shared)
        {
            Bind(f, std::weak_ptr<T>(shared), false);
        }

        /// Binds this object's function to the event.
        /// The object will call the function and both must be valid (t->*f(...))
        /// If the object expires before this Event does, all references to the object will be removed from this event on next Event Raise
        /// \tparam T object type
        /// \param f function reference
        /// \param weak weak pointer to the object
        /// \example event.Bind(&MyClass::Function, weak_ptr);
        template<typename T>
        [[maybe_unused]] void Bind(void(T::* const f)(Args...), std::weak_ptr<T> weak)
        {
            Bind(f, weak, false);
        }

        /// Binds this object's function to the event. The function will be called only on the next time the event is raised
        /// The object will call the function and both must be valid (t->*f(...))
        /// If the object expires before this Event does, it will have undefined behavior.
        /// \tparam T object type
        /// \param f function reference
        /// \param t object pointer
        /// \example event.Bind(&MyClass::Function, &myClassObject);
        template<typename T>
        [[maybe_unused]] void BindOnce(void(T::* const f)(Args...), T * const t)
        {
            Bind(f, t, true);
        }

        /// Binds this object's function to the event.
        /// The object will call the function and both must be valid (t->*f(...))
        /// If the object expires before this Event does, it will have undefined behavior.
        /// \tparam T object type
        /// \param f function reference
        /// \param t object pointer
        /// \example event.Bind(&MyClass::Function, &myClassObject);
        template<typename T>
        [[maybe_unused]]void Bind(void(T::* const f)(Args...), T * const t)
        {
            Bind(f, t, false);
        }

        /// Binds this callback to this Event. The function will be called only on the next time the event is raised
        /// Note that this doesn't require a pointer or handler, so this Event might throw an exception if the callback
        /// lifetime expires before this Event does.
        /// \param cb the callback function
        /// \example event.Bind([]{...});
        [[maybe_unused]]void BindOnce(Callback cb)
        {
            Bind(cb, true);
        }

        /// Binds this callback to this Event
        /// Note that this doesn't require a pointer or handler, so this Event might throw an exception if the callback
        /// lifetime expires before this Event does.
        /// \param cb the callback function
        /// \example event.Bind([]{...});
        [[maybe_unused]]void Bind(Callback cb)
        {
            Bind(cb, false);
        }

        /// Remove all references to the object pointer
        /// \tparam T object type
        /// \param t object pointer
        /// \return true if we found and removed the object reference, false otherwise
        template<typename T>
        [[maybe_unused]]bool Remove(T * const t)
        {
            assert(t != nullptr && "Cannot remove a null pointer");
            auto it = Binds.find(t);
            bool removed = false;
            if (it != Binds.end())
            {
                Binds.erase(it);
                removed = true;
            }
            return removed;
        }

        /// Remove all references to the object this weak ptr is pointing to
        /// \tparam T object type
        /// \param ptr weak pointer
        /// \return true if we found and removed the object reference, false otherwise
        template<typename T>
        [[maybe_unused]]bool Remove(std::weak_ptr<T> ptr)
        {
            if (auto shared = ptr.lock())
            {
                return Remove(shared.get());
            }
            return false;
        }

        /// Remove all references to the object this ptr is pointing to
        /// \tparam T object type
        /// \param ptr weak pointer
        /// \return true if we found and removed the object reference, false otherwise
        template<typename T>
        [[maybe_unused]]bool Remove(std::shared_ptr<T> ptr)
        {
            if (ptr)
            {
                return Remove(ptr.get());
            }
            return false;
        }

    };

    template<typename... Args>
    class Event : public EventBase
    {
    private:
        EventBinder<Args...> Binder{};

    public:
        explicit Event(const std::string& name = "") : EventBase(name) {}

        /// Get the binder reference. This is a public and preferred way of subscribing objects/functions to this event
        /// \return Binder reference
        inline EventBinder<Args...>& GetBinder() { return Binder; }

        /// Raise/Trigger this Event
        /// \param args
        inline void operator()(Args... args)
        {
            Raise(std::forward<Args>(args)...);
        }

        /// Raise/Trigger this Event
        /// \param args
        [[maybe_unused]] void Raise([[maybe_unused]] Args... args)
        {
            for (auto it = Binder.Binds.begin(); it != Binder.Binds.end(); )
            {
                auto& functionVector = it->second;
                functionVector.erase(std::remove_if(functionVector.begin(), functionVector.end(), [&](const auto& cb)
                {
                    return !cb(std::forward<Args>(args)...);
                }), functionVector.end());

                if (functionVector.empty())
                {
                    it = Binder.Binds.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }

        /// How many objects are attached to this event.
        /// \return Objects observing this event count
        [[maybe_unused]] [[nodiscard]] inline int Size()
        {
            return Binder.Binds.size();
        }

        /// How many functions are attached to this event.
        /// \return This Event functions call count
        [[maybe_unused]] [[nodiscard]] inline int CallbackCount()
        {
            int total = 0;
            for (const auto& pair : Binder.Binds) total += pair.second.size();
            return total;
        }

        /// Cleanup expired weak pointers. (It automatically cleans up on Raise)
        [[maybe_unused]] inline void Cleanup()
        {
            assert(false && "Not implemented");
        }

#pragma region Binder Wrapper
        /** Convenient functions wrapper to Binder **/
        using Callback = std::function<void(Args...)>;
        [[maybe_unused]] inline void Bind(Callback f) { Binder.Bind(f); }
        [[maybe_unused]] inline void BindOnce(Callback f) { Binder.BindOnce(f); }
        template <typename T>
        [[maybe_unused]] inline void Bind(Callback f, T* t) { Binder.Bind(f,t); }
        template <typename T>
        [[maybe_unused]] inline void BindOnce(Callback f, T* t) { Binder.BindOnce(f,t); }
        template <typename T>
        [[maybe_unused]] inline void Bind(void(T::* const f)(Args...), T* const t) { Binder.Bind(f,t); }
        template <typename T>
        [[maybe_unused]] inline void BindOnce(void(T::* const f)(Args...), T* const t) { Binder.BindOnce(f,t); }
        template <typename T>
        [[maybe_unused]] inline void Bind(void(T::* const f)(Args...), std::weak_ptr<T> t) { Binder.Bind(f, t); }
        template <typename T>
        [[maybe_unused]] inline void Bind(void(T::* const f)(Args...), std::shared_ptr<T> t) { Binder.Bind(f, t); }
        template <typename T>
        [[maybe_unused]] inline void BindOnce(void(T::* const f)(Args...), std::shared_ptr<T> t) { Binder.BindOnce(f, t); }
        template <typename T>
        [[maybe_unused]] inline void BindOnce(void(T::* const f)(Args...), std::weak_ptr<T> t) { Binder.BindOnce(f, t); }
        template<typename T>
        [[maybe_unused]] inline void Bind(Callback f, std::shared_ptr<T> t) { Binder.Bind(f, t); }
        template<typename T>
        [[maybe_unused]] inline void Bind(Callback f, std::weak_ptr<T> t) { Binder.Bind(f, t); }
        template<typename T>
        [[maybe_unused]] inline void BindOnce(Callback f, std::shared_ptr<T> t) { Binder.BindOnce(f, t); }
        template<typename T>
        [[maybe_unused]] inline void BindOnce(Callback f, std::weak_ptr<T> t) { Binder.BindOnce(f, t); }
        template <typename T>
        [[maybe_unused]] inline bool Remove(T* const t) { return Binder.Remove(t); }
        template <typename T>
        [[maybe_unused]] inline bool Remove(std::shared_ptr<T> t) { return Binder.Remove(t); }
        template <typename T>
        [[maybe_unused]] inline bool Remove(std::weak_ptr<T> t) { return Binder.Remove(t); }
        [[maybe_unused]] inline void RemoveAll() { Binder.RemoveAll(); }
#pragma endregion Binder Wrapper

    };
}

#endif //SPARKLE_EVENT_H