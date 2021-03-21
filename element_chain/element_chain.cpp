#include <cctype>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <unordered_map>
#include <vector>

namespace {
    using ElementNumber = int;
    using ElementUsed = int;
    using ElementChar = char;
    constexpr ElementNumber SizeOfElements = 118;
    constexpr size_t SizeOfElementsPlusOne = SizeOfElements + 1;  // index 0 is unused

    struct Element {
        ElementNumber number_ = 0;
        ElementChar first_ = 0;
        ElementChar last_ = 0;
    };

    using ElementNumbers = std::vector<ElementNumber>;
    using ElementNumMap = std::unordered_map<ElementNumber, Element>;
    using ElementCharMap = std::unordered_map<ElementChar, ElementNumbers>;
    using ElementSize = ElementNumbers::size_type;
}

namespace {
    using Count = long long int;
    constexpr ElementSize baseChainSize = 77;

    struct State final {
        State(Count initialCount, bool limitEveryCase, bool selectOne) :
            initialCount_(initialCount), limitEveryCase_(limitEveryCase), selectOne_(selectOne),
            count_(initialCount), used_(SizeOfElementsPlusOne, 0) {
            chain_.reserve(SizeOfElementsPlusOne);
        }

        void Reset(void) {
            count_ = initialCount_;
            used_.clear();
            used_.insert(used_.end(), SizeOfElementsPlusOne, 0);
            chain_.clear();
            chain_.reserve(SizeOfElementsPlusOne);
        }

        Count initialCount_ = 0;
        bool limitEveryCase_ = false;
        bool selectOne_ = false;
        Count count_ = 0;
        std::vector<ElementUsed> used_;
        ElementNumbers chain_;
        ElementNumbers longestChain_;
    };

    ElementNumMap  allElements;
    ElementCharMap oneLetterElements;
    ElementCharMap twoLetterElements;

    // See the periodic table of elements in
    // https://gist.github.com/GoodmanSciences/c2dd862cd38f21b0ad36b8f96b4bf1ee
    constexpr char ElementSymbols[] = "H,He,Li,Be,B,C,N,O,F,Ne,Na,Mg,Al,Si,P,S,Cl,Ar,K,Ca,Sc,Ti,V,Cr,Mn,Fe,Co,Ni,Cu,Zn,Ga,Ge,As,Se,Br,Kr,Rb,Sr,Y,Zr,Nb,Mo,Tc,Ru,Rh,Pd,Ag,Cd,In,Sn,Sb,Te,I,Xe,Cs,Ba,La,Ce,Pr,Nd,Pm,Sm,Eu,Gd,Tb,Dy,Ho,Er,Tm,Yb,Lu,Hf,Ta,W,Re,Os,Ir,Pt,Au,Hg,Tl,Pb,Bi,Po,At,Rn,Fr,Ra,Ac,Th,Pa,U,Np,Pu,Am,Cm,Bk,Cf,Es,Fm,Md,No,Lr,Rf,Db,Sg,Bh,Hs,Mt,Ds,Rg,Cn,Nh,Fl,Mc,Lv,Ts,Og,";

    std::random_device g_seedGen;
    std::mt19937 g_engine(g_seedGen());
}

void setupElements(void) {
    Element emptyElement;
    Element element;
    ElementNumber num = 1;

    for (const auto& ch : ElementSymbols) {
        const auto& c = static_cast<ElementChar>(std::tolower(ch));

        if (c == ',') {
            element.number_ = num;
            if (element.last_) {
                if (twoLetterElements.find(element.first_) == twoLetterElements.end()) {
                    ElementNumbers nums {num};
                    twoLetterElements[element.first_] = nums;
                } else {
                    ElementNumbers& nums = twoLetterElements.at(element.first_);
                    nums.push_back(num);
                }
            } else {
                element.last_ = element.first_;
                ElementNumbers nums {num};
                oneLetterElements[element.first_] = nums;
            }

            allElements[num] = element;
            ++num;
            element = emptyElement;
        } else {
            if (element.first_) {
                element.last_ = c;
            } else {
                element.first_ = c;
            }
        }
    }

    assert(allElements.size() == SizeOfElements);

    auto total = oneLetterElements.size();
    for (const auto& es : twoLetterElements) {
        total += es.second.size();
    };

    assert(allElements.size() == SizeOfElements);
    assert(total == SizeOfElements);
    return;
}

void printElementNumMap(const ElementNumMap& numMap) {
    for (const auto& e : numMap) {
        const auto& element = e.second;
        std::cout << element.number_ << ":" << element.first_ << ":" << element.last_ << "\n";
    };
}

void printElementCharMap(const ElementCharMap& charMap) {
    for (const auto& es : charMap) {
        const auto& elements = es.second;
        for (const auto& n : elements) {
            const auto& element = allElements.at(n);
            std::cout << element.number_ << ":" << element.first_ << ":" << element.last_ << "\n";
        }
    };
}

void writeChain(const State& state, std::ostream& os) {
    const auto size = state.chain_.size();
    os << "[" << size << "]\n";
    for (const auto& n : state.chain_) {
        os << n << ", ";
    }
    os << "\n";
    return;
}

bool findLongestPath(State& state, std::ostream& os) {
    struct AddToChainAndFind final {
        AddToChainAndFind(State& state, std::ostream& os, ElementNumber num) :
            state_(state), num_(num) {
            state_.used_[num] = 1;
            state_.chain_.push_back(num);
            aborted_ = findLongestPath(state, os);
        }

        ~AddToChainAndFind() {
            state_.used_[num_] = 0;
            state_.chain_.pop_back();
        }

        State& state_;
        ElementNumber num_ = 0;
        bool aborted_ = false;
    };

    bool aborted = false;
    if (state.count_ <= 0) {
        return true;
    }

    assert(!state.chain_.empty());
    const auto& current = state.chain_.at(state.chain_.size() - 1);
    const auto& element = allElements.at(current);
    const auto nextChar = element.last_;

    if (oneLetterElements.find(nextChar) != oneLetterElements.end()) {
        const auto& nextNum = oneLetterElements.at(nextChar).at(0);
        if (!state.used_.at(nextNum)) {
            AddToChainAndFind caller(state, os, nextNum);
            aborted = caller.aborted_;
            return aborted;
        }
    }

    if (twoLetterElements.find(nextChar) != twoLetterElements.end()) {
        const auto& nextNums = twoLetterElements.at(nextChar);
        assert(!nextNums.empty());

        using SizeType = decltype(nextNums.size());
        std::uniform_int_distribution<SizeType> dist(0, nextNums.size() - 1);

        for(SizeType i = 0; i < nextNums.size(); ++i) {
            // Shuffle?
            const auto nextNum = nextNums.at(dist(g_engine));
            if (state.used_[nextNum]) {
                continue;
            }

            AddToChainAndFind caller(state, os, nextNum);
            aborted |= caller.aborted_;
            if (state.limitEveryCase_ && state.selectOne_) {
                return aborted;
            }
        }
        if (aborted) {
            return aborted;
        }
    } else {
        auto size = state.chain_.size();
        const bool updated = (state.longestChain_.size() < size);
        if (updated) {
            state.longestChain_ = state.chain_;
            writeChain(state, std::cout);
            writeChain(state, os);
        }

        if (state.limitEveryCase_) {
            state.count_ = (state.count_ <= 0) ? 0 : (state.count_ - 1);
        } else {
            if (updated && (size >= baseChainSize)) {
                decltype(size) shift = (size - baseChainSize);
                shift = std::min(static_cast<decltype(shift)>(6), shift * 2);
                state.count_ = state.initialCount_ << shift;
            } else {
                state.count_ = (state.count_ <= 0) ? 0 : (state.count_ - 1);
            }
        }
    }

    return aborted;
}

struct SetAllAndFind final {
    SetAllAndFind(State& state, std::ostream& os, const ElementNumbers& nums) :
        state_(state) {
        for (const auto& num : nums) {
            state.used_[num] = 1;
            state.chain_.push_back(num);
        }

        aborted_ = findLongestPath(state, os);
    }

    ~SetAllAndFind() {
        state_.Reset();
    }

    State& state_;
    bool aborted_ = false;
};

void findLongestPathFrom(const ElementNumbers& presetNumbers, std::ostream& os,
                         Count initialCount, bool limitEveryCase, bool selectOne,
                         Count nTimes) {
    State state(initialCount, limitEveryCase, selectOne);
    for (Count i=0; i<nTimes; ++i) {
        SetAllAndFind caller(state, os, presetNumbers);
    }
    return;
}

void findLongestPathFrom(ElementNumber headNumber, std::ostream& os,
                         Count initialCount, bool limitEveryCase, bool selectOne,
                         Count nTimes) {
    ElementNumbers nums {headNumber};
    findLongestPathFrom(nums, os, initialCount, limitEveryCase, selectOne, nTimes);
    return;
}

void findLongestInfinite(const ElementNumbers& headNumbers, std::ostream& os,
                         Count initialCount, bool limitEveryCase, bool selectOne) {
    State state(initialCount, limitEveryCase, selectOne);
    std::uniform_int_distribution<ElementNumber> dist(1, SizeOfElements);
    if (headNumbers.empty()) {
        for (;;) {
            const auto num = dist(g_engine);
            ElementNumbers nums {num};
            SetAllAndFind caller(state, os, nums);
        }
    } else {
        for (;;) {
            for (const auto& num : headNumbers) {
                ElementNumbers nums {num};
                SetAllAndFind caller(state, os, nums);
            }
        }
    }

    return;
}

int main(void) {
    setupElements();
    printElementNumMap(allElements);
    printElementCharMap(oneLetterElements);
    printElementCharMap(twoLetterElements);

    std::ofstream os("chain_found.txt", std::ios::app);

    Count initialCount = 1000000ll;
    bool limitEveryCase = false;
    bool selectOne = false;

    const ElementNumbers nums = {61, 25, 7, 11, 89, 6};
    limitEveryCase = false;
    selectOne = false;
//  Count nTimes = 1000000ll;
//  findLongestPathFrom(nums, os, initialCount, limitEveryCase, selectOne, nTimes);

    const ElementNumbers initialNums {69, 78, 15, 30, 17, 20, 30, 91, 117, 55, 61, 81, 98, 82, 112};
    limitEveryCase = true;
    selectOne = false;
//  findLongestInfinite(initialNums, os, initialCount, limitEveryCase, selectOne);

    initialCount = 10000ll;
    const ElementNumbers emptyNums;
    limitEveryCase = true;
    selectOne = false;
    findLongestInfinite(emptyNums, os, initialCount, limitEveryCase, selectOne);

    return 0;
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
Last:
*/
