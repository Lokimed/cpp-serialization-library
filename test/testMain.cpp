/**
 * @file      testMain.cpp
 *
 * @author    Alexander Ganyukhin (alexander.ganyukhin@mera.com)
 *
 * @date      2019-October-23
 *
 * Copyright 2019 Mera
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

#include <cstdint>
#include <iostream>

#include <object_invoke.h>

template<typename T>
struct InstanceCounter {
    static size_t instances;

    InstanceCounter() { this->incAndLog(); }
    InstanceCounter(InstanceCounter const &) { this->incAndLog(); }
    InstanceCounter(InstanceCounter &&) noexcept { this->incAndLog(); }
    ~InstanceCounter() { this->decAndLog(); }

    InstanceCounter & operator=(InstanceCounter const &) = default;
    InstanceCounter & operator=(InstanceCounter &&) = default;

    void incAndLog() const {
        ++instances;
        std::cout << "    +++New object \'" << typeid(T).name() << "\', addr: \'"
                  << static_cast<void const *>(this) << "\' created, total: \'"
                  << instances << "\'" << std::endl;
    }

    void decAndLog() const {
        --instances;
        std::cout << "    ---Object \'" << typeid(T).name() << "\', addr: \'"
                  << static_cast<void const *>(this) << "\' destroyed, total: \'"
                  << instances << "\'" << std::endl;
    }
};

template<typename T>
size_t InstanceCounter<T>::instances { 0ull };

struct Object1
    : public InstanceCounter<Object1> {
    void getValue(int & i) {
    //    std::cout << " getValue invoked!" << std::endl;
        ii += 11111;
        i = ii;
    }

    int retGetValue() const
    {
        std::cout << "retGetValue" << std::endl;
        ii += 11111;
        return ii;
    }

    const int& refValue() const
    {
        std::cout << "retGetValue" << std::endl;
        ii += 11111;
        return ii;
    }

    inline static int ii { 11111 };
};

struct Object2
    : public InstanceCounter<Object2> {
    void getObject1(Object1 & obj) {
      //  std::cout << "  getObject1 invoked!" << std::endl;
        obj = obj_;
    }

    Object1 retObject1() const {
        std::cout << "  retObject1 invoked" << std::endl;
        return obj_;
    }

    Object1 obj_;
};

struct Object3
    : public InstanceCounter<Object3> {
    void getObject2(Object2 * obj) {
        //std::cout << "  getObject2 invoked!" << std::endl;
        *obj = obj_;
    }

    void testFail(int c)
    {
        c = 15;
        std::cout << c << std::endl;
    }

    Object2 obj_;
};

struct Serializer {
    template<typename ... T>
    void operator()(char const * tag, std::tuple<T...> const & aTuple) {
        std::cout << "\'" << tag << "\': \'";
        putStream(std::cout, aTuple, std::make_index_sequence<sizeof...(T)>{});
        std::cout << "\'\n";
    }

    template<typename Tuple, size_t ... Idx>
    static std::ostream & putStream(std::ostream & aOs, Tuple const & t, std::index_sequence<Idx...>) {
        return (aOs << ... << std::get<Idx>(t));
    }
};


constexpr mil::object_invoke invoke {    
    mil::useAcceptor<Serializer>(),
    mil::delayedInvoke<&Object3::getObject2, &Object2::getObject1, &Object1::getValue>("call1"),
    mil::delayedInvoke<&Object3::getObject2, &Object2::getObject1, &Object1::getValue>("call2"),
    mil::delayedInvoke<&Object3::getObject2, &Object2::getObject1, &Object1::getValue>("call3"),
    mil::delayedInvoke<&Object3::getObject2, &Object2::getObject1, &Object1::getValue>("call4"),

    // this case not compile. it is okay because getters C style must accept only refs and pointers
    //mil::delayedInvoke<&Object3::testFail>("call5")

    mil::delayedInvoke<&Object3::getObject2, &Object2::getObject1, &Object1::retGetValue>("call6"),
    mil::delayedInvoke<&Object3::getObject2, &Object2::retObject1, &Object1::retGetValue>("call7"),

    // this case not compile, because still have no support for return types pointer and references
    // mil::delayedInvoke<&Object3::getObject2, &Object2::retObject1, &Object1::refValue>("call8")
};


int main() {
    Object3 obj {};
    Serializer si;

    //invoke.set_logger(std::function{[](const std::string& v){ std::cout << v << std::endl; }});
    invoke(obj, si);
    
    return 0;
}
