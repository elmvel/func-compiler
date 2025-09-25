#ifndef COMMON_H_
#define COMMON_H_

#define TODOF()                                 \
    do {                                        \
        fmt::println("TODO: {}", __FUNCTION__); \
        exit(101);                              \
    } while (0)

#define TRY(dst, expr) dst = (expr); if (!(dst).is_ok()) return dst;

#include <variant>

namespace dwg
{
    namespace internals
    {
        template <class T>
        struct Ok
        {
            Ok(const T& t): t(t) {}
            Ok(T&& t)     : t(std::move(t)) {}

            T t;
        };

        template<>
        struct Ok<void> {};

        template <class E>
        struct Err
        {
            Err(const E& e): e(e) {}
            Err(E&& e)     : e(std::move(e)) {}

            E e;
        };
    };

    template <class T, class E>
    class result
    {
    public:
        result(const internals::Ok<T>& t):  m_storage(t), m_is_ok(true) {}
        result(internals::Ok<T>&& t):       m_storage(std::move(t)), m_is_ok(true) {}
        result(const internals::Err<E>& e): m_storage(e), m_is_ok(false) {}
        result(internals::Err<E>&& e):      m_storage(std::move(e)), m_is_ok(false) {}
        
        bool is_ok()
        {
            return m_is_ok;
        }
        
        bool is_err()
        {
            return !m_is_ok;
        }
        
        T& value()
        {
            return std::get<internals::Ok<T>>(m_storage).t;
        }
        
        E& error()
        {
            return std::get<internals::Err<E>>(m_storage).e;
        }
        
        operator bool()
        {
            return m_is_ok;
        }
        
        T& operator*()
        {
            return value();
        }
    private:
        std::variant<internals::Ok<T>, internals::Err<E>> m_storage;
        bool m_is_ok;
    };

    template <class E>
    class result<void, E>
    {
    public:
        void value()
        {
        }

        void operator*()
        {
        }
    };
    
    template <class T>
    internals::Ok<T> Ok(const T& t)
    {
        return internals::Ok{t};
    }

    template <class E>
    internals::Err<E> Err(const E& e)
    {
        return internals::Err{e};
    }

}; // namespace dwg

// int main()
// {
//     dwg::result<int, bool> ok = dwg::Ok(5);
//     dwg::result<int, bool> err = dwg::Err(false);
// 
//     if (ok) {
//         fmt::println("ok was Ok({})", *ok);
//     } else {
//         fmt::println("ok was Err({})", ok.error());
//     }
// 
//     if (err) {
//         fmt::println("err was Ok({})", *err);
//     } else {
//         fmt::println("err was Err({})", err.error());
//     }
//     
//     return 0;
// }

#endif // COMMON_H_
