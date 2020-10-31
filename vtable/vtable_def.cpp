#include <iostream>
#include "vtable.hpp"

Original::~Original() {
    std::cout << "~Original\n";
}

void Original::Say() {
    std::cout << "Original!\n";
}

Replaced::~Replaced() {
    std::cout << "~Replaced\n";
}

void Replaced::Say() {
    std::cout << "Replaced!\n";
}

Inherited::~Inherited() {
    std::cout << "~Inherited\n";
}

void Inherited::Say() {
    std::cout << "Inherited!\n";
}

Cat::~Cat() {
    std::cout << "~Cat\n";
}

void Cat::Mew() {
    std::cout << "Mew!\n";
}

namespace {
    template <typename T>
    void replaceVtable(T& ptrSet) {
        asm volatile (
            "mov (%0), %%r8\n\t"
            "mov %%r8, (%2)\n\t"
            "mov (%1), %%r8\n\t"
            "mov %%r8, (%0)\n\t"
            ::"r"(ptrSet.pOriginal.get()), "r"(ptrSet.pReplaced.get()), "r"(&ptrSet.vtable):"r8", "memory");
    }

    template <typename T>
    void restoreVtableImpl(T& ptrSet) {
        asm volatile (
            "mov (%0), %%r8\n\t"
            "mov %%r8, (%1)\n\t"
            ::"r"(&ptrSet.vtable), "r"(ptrSet.pOriginal.get()):"r8", "memory");
    }
}

ReplacedSet CreateReplaced() {
    ReplacedSet ptrSet {std::make_unique<Original>(), std::make_unique<Replaced>(), 0};
    replaceVtable(ptrSet);
    return ptrSet;
}

InheritedSet CreateInherited() {
    InheritedSet ptrSet {std::make_unique<Original>(), std::make_unique<Inherited>(), 0};
    replaceVtable(ptrSet);
    return ptrSet;
}

CatSet CreateCat() {
    CatSet ptrSet {std::make_unique<Original>(), std::make_unique<Cat>(), 0};
    replaceVtable(ptrSet);
    return ptrSet;
}

void RestoreVtable(ReplacedSet& ptrSet) {
    restoreVtableImpl(ptrSet);
}

void RestoreVtable(InheritedSet& ptrSet) {
    restoreVtableImpl(ptrSet);
}

void RestoreVtable(CatSet& ptrSet) {
    restoreVtableImpl(ptrSet);
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
