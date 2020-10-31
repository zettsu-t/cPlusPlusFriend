#ifndef VTABLE_HPP
#define VTABLE_HPP

#include <cstdint>
#include <memory>
#include <string>

class Original {
public:
    virtual ~Original();
    virtual void Say();
};

class Replaced {
public:
    virtual ~Replaced();
    virtual void Say();
};

class Inherited : public Original {
public:
    virtual ~Inherited();
    virtual void Say();
};

class Cat {
public:
    virtual ~Cat();
    virtual void Mew();
};

struct ReplacedSet {
    std::unique_ptr<Original> pOriginal;
    std::unique_ptr<Replaced> pReplaced;
    uintptr_t vtable;
};

struct InheritedSet {
    std::unique_ptr<Original> pOriginal;
    std::unique_ptr<Inherited> pReplaced;
    uintptr_t vtable;
};

struct CatSet {
    std::unique_ptr<Original> pOriginal;
    std::unique_ptr<Cat> pReplaced;
    uintptr_t vtable;
};

extern ReplacedSet CreateReplaced();
extern InheritedSet CreateInherited();
extern CatSet CreateCat();

extern void RestoreVtable(ReplacedSet& ptrSet);
extern void RestoreVtable(InheritedSet& ptrSet);
extern void RestoreVtable(CatSet& ptrSet);

#endif // VTABLE_HPP

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
