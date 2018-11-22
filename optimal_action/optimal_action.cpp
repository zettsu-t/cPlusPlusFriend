// Solving a puzzle below
// https://twitter.com/CTak_S/status/1064819923195580417

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include <boost/multi_array.hpp>

namespace {
    using Count = long long int;
    // Bitmap (1 to alive) of survivors 0..(N_PLAYERS-1)
    using State = unsigned int;
    using Index = int;
    using Survivors = std::vector<Index>;

    struct Action {
        Index player;
        State state;
        Index target;
    };
    using ActionChain = std::vector<Action>;

    // Rule of the game
    constexpr Index N_PLAYERS = 3;
    constexpr Index N_STATES = 1 << N_PLAYERS;  // 2^N_PLAYERS combinations
    constexpr Index N_ACTIONS = N_PLAYERS + 1;  // Shoot Player 0, ... (N_PLAYERS-1), or nobody
    constexpr Index N_WINNERS = N_PLAYERS;
    constexpr Index ALIVE = 1;
    const std::vector<double> HIT_RATE {0.3, 0.5, 1.0};
    constexpr double INITIAL_VALUE = 1.0 / static_cast<double>(N_ACTIONS - 1);

    // Hyper parameter(s)
    constexpr double LEARNING_RATE = 0.00001;
    constexpr double EXPLORATION_EPSILON = 0.1;
    constexpr bool IID_CHOICE = false;
    constexpr Index MAX_DEPTH = 30;
    constexpr Count N_TRIALS = 10000000ll;
}

class OptimalAction {
public:
    OptimalAction(void) :
        rand_gen_(rand_dev_()), unit_distribution_(0.0, 1.0),
        value_(boost::extents[N_PLAYERS][N_STATES][N_ACTIONS]) {
        for (Index player = 0; player < N_PLAYERS; ++player) {
            for (Index state = 0; state < N_STATES; ++state) {
                for (Index action = 0; action < N_ACTIONS; ++action) {
                    // Do not shoot yourself!
                    auto count = checkAliveOrNobody(player, state);
                    value_[player][state][action] = count && checkAliveOrNobody(action, state) &&
                        (player != action) ? (1.0 / static_cast<double>(count)) : 0.0;
                }
            }
        }
        return;
    }

    virtual ~OptimalAction(void) = default;

    void Exec(Count n_trials) {
        for (Count i = 0; i < n_trials; ++i) {
            exec();
        }

        for (Index player = 0; player < N_PLAYERS; ++player) {
            std::cout << printValue(player);
        }
        return;
    }

    void exec(void) {
        Survivors survivors(N_PLAYERS, ALIVE);
        ActionChain actionChain;
        aimAndShoot(0, 0, survivors, actionChain);
        return;
    }

    char printIndexChar(Index player) {
        return player + 'A';
    }

    std::string printValue(Index player) {
        std::ostringstream os;

        os << std::setprecision(5) << "[States, actions and values for Player " << printIndexChar(player) << "]\n";
        for (Index state = 0; state < N_STATES; ++state) {
            // Exclude when the player is not alive or only alive
            if (checkPlayerAlive(player, state) < 2) {
                continue;
            }

            for (Index action = 0; action < N_ACTIONS; ++action) {
                if ((player != action) && checkPlayerAlive(action, state)) {
                    os << "Target " << printIndexChar(action) << ":" << value_[player][state][action] << ", ";
                }
                if (action >= N_PLAYERS) {
                    os << "Nobody:" << value_[player][state][action] << "\n" ;
                }
            }
        }

        os << "\n";
        return os.str();
    }

    // Convers an array to a bitmap
    State survivorsToState(const Survivors& survivors) {
        State state = 0;
        State index = 1;
        for(const auto value : survivors) {
            state += index * (value ? 1 : 0);
            index <<= 1;
        }
        return state;
    }

    // Notice that nobody is always not alive (population - player(1) + nobady(1))
    template<typename T>
    auto countPopulation(T state) {
        return __builtin_popcount(state);
    }

    // Return population of the state if the player is alive in the state, 0 othewise
    Index checkAliveOrNobody(Index player, State state) {
        return ((player >= N_PLAYERS) || (state & (1 << player))) ? countPopulation(state) : 0;
    }

    Index checkPlayerAlive(Index player, State state) {
        return (state & (1 << player)) ? countPopulation(state) : 0;
    }

    // Overwrites survivors
    void aimAndShoot(Index player, Index depth, Survivors& survivors, const ActionChain& actionChain) {
        if (depth >= MAX_DEPTH) {
            return;
        }

        const auto targets = getTargets(player, survivors);
        const auto state = survivorsToState(survivors);
        const auto target = getActionTarget(player, state, survivors, targets);

        ActionChain nextActionChain = actionChain;
        Action nextAction {player, state, target};
        nextActionChain.push_back(nextAction);
        shoot(player, survivors, target);

        if (std::accumulate(survivors.begin(), survivors.end(), 0) == 1) {
            const auto winner = std::distance(survivors.begin(),
                                              std::find(survivors.begin(), survivors.end(), ALIVE));

            // Pick up one sample to i.i.d.
            if (IID_CHOICE) {
                auto raw_index = unit_distribution_(rand_gen_) * static_cast<double>(nextActionChain.size()) - 0.5;
                auto index = std::min(nextActionChain.size() - 1,
                                      static_cast<decltype(nextActionChain.size())>(
                                          std::max(0, static_cast<int>(raw_index))));
                propagate(nextActionChain.at(index), winner);
            } else {
                // Reverse if you deduct rewards
                for(const auto& action : nextActionChain) {
                    propagate(action, winner);
                }
            }
        } else {
            aimAndShoot((player + 1) % N_PLAYERS, depth + 1, survivors, nextActionChain);
        }

        return;
    }

    std::vector<Index> getTargets(Index player, const Survivors& survivors) {
        std::vector<Index> targets;
        for(Index target = 0; target < N_PLAYERS; ++target) {
            if ((target != player) && survivors.at(target)) {
                targets.push_back(target);
            }
        }

        if (targets.size() > 1) {
            // Can shoot nobody
            targets.push_back(N_PLAYERS);
        }

        return targets;
    }

    Index getActionTarget(Index player, State state, const Survivors& survivors, const std::vector<Index>& targets) {
        // Epsilon-greedy
        const auto rand_proportional = unit_distribution_(rand_gen_);
        const bool proportional = (rand_proportional < EXPLORATION_EPSILON);
        // Number of targets = number of survivors - player(1) + nobody(1)
        const auto population = checkAliveOrNobody(player, state);
        const double proportion = (population > 0) ? (1.0 / static_cast<double>(population)) : 0.0;

        auto rand_value = unit_distribution_(rand_gen_);
        Index target = 0;
        while(rand_value >= 0.0) {
            const auto value = (proportional) ?
                ((checkPlayerAlive(player, state) && checkAliveOrNobody(target, state) && (player != target)) ?
                 proportion : 0.0) : value_[player][state][target];

            rand_value -= value;
            target += 1;
            if (target >= N_ACTIONS) {
                break;
            }
        }

        return target - 1;
    }

    // Overwrites survivors
    void shoot(Index player, Survivors& survivors, Index target) {
        if (target < N_PLAYERS) {
            if (unit_distribution_(rand_gen_) < HIT_RATE.at(player)) {
                survivors.at(target) = 0;
            }
        }

        return;
    }

    void propagate(const Action& action, Index final_surviver) {
        // No deduction
        const auto target_value = value_[action.player][action.state][action.target];
        const auto delta = target_value * LEARNING_RATE * ((action.player == final_surviver) ? 1.0 : -1.0);
        value_[action.player][action.state][action.target] += delta;

        // Normalizes such that the sum of values is 1
        double sum = 0.0;
        for (Index i = 0; i < N_ACTIONS; ++i) {
            sum += value_[action.player][action.state][i];
        }
        for (Index i = 0; i < N_ACTIONS; ++i) {
            value_[action.player][action.state][i] /= sum;
        }
    }

private:
    using CountMatrix = boost::multi_array<Count, 4>;
    using StateActionValue = boost::multi_array<double, 3>;
    std::random_device rand_dev_;
    std::mt19937 rand_gen_;
    std::uniform_real_distribution<double> unit_distribution_;
    StateActionValue value_;
};

int main(int argc, char* argv[]) {
    OptimalAction optimalAction;

    Count n_trials = N_TRIALS;
    if (argc > 1) {
        n_trials = ::atoll(argv[1]);
    }
    optimalAction.Exec(n_trials);
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
