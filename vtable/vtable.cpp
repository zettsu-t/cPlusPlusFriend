#include <iostream>
#include "vtable.hpp"

int main(int argc, char* argv[]) {
    {
        Original original;
        Replaced replaced;
        Inherited inherited;
        Cat cat;
        original.Say();
        replaced.Say();
        inherited.Say();
        cat.Mew();
    }
    std::cout << "\n";

    {
        auto ptrSet = CreateReplaced();
        ptrSet.pOriginal->Say();
        ptrSet.pReplaced->Say();
        RestoreVtable(ptrSet);
    }
    std::cout << "\n";

    {
        auto ptrSet = CreateInherited();
        ptrSet.pOriginal->Say();
        ptrSet.pReplaced->Say();
        RestoreVtable(ptrSet);
    }
    std::cout << "\n";

    {
        auto ptrSet = CreateCat();
        ptrSet.pOriginal->Say();
        ptrSet.pReplaced->Mew();
        RestoreVtable(ptrSet);
    }
    std::cout << "\n";

    return 0;
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
