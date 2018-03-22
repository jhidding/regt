#pragma once
#include <memory>
#include <string>
#include <map>

namespace System
{
    template <typename T>
    class Global: public T
    {
        typedef std::map<std::string, T *> tmap;
        static std::unique_ptr<tmap> _dir;

        public:
            static bool exists(std::string const &name)
            {
                return dir().count(name) > 0;
            }

            static T const &at(std::string const &name)
            {
                return *dir()[name];
            }

            static tmap &dir()
            {
                if (not _dir)
                    _dir = std::unique_ptr<tmap>(new tmap);

                return *_dir;
            }

            template <typename ...Args>
            Global(std::string const &name, Args &&...args):
                T(std::forward<Args>(args)...)
            {
                dir()[name] = this;
            }
    };

    template <typename T>
    std::unique_ptr<std::map<std::string, T *>> System::Global<T>::_dir;
}

