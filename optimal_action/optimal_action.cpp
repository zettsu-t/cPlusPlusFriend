// Solving a puzzle below
// https://twitter.com/CTak_S/status/1064819923195580417

#include <cstdlib>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
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

    // Optimal actions (responses)
    enum class ExpectedAction {
        A,
        B,
        C,
        Nobody,
        Undefined,
    };

    const ExpectedAction ExpectedActions[N_PLAYERS][N_STATES] = {
        {ExpectedAction::Undefined, ExpectedAction::Undefined, ExpectedAction::Undefined, ExpectedAction::B,
         ExpectedAction::Undefined, ExpectedAction::C, ExpectedAction::Undefined, ExpectedAction::Nobody},
        {ExpectedAction::Undefined, ExpectedAction::Undefined, ExpectedAction::Undefined, ExpectedAction::A,
         ExpectedAction::Undefined, ExpectedAction::Undefined, ExpectedAction::C, ExpectedAction::C},
        {ExpectedAction::Undefined, ExpectedAction::Undefined, ExpectedAction::Undefined, ExpectedAction::Undefined,
         ExpectedAction::Undefined, ExpectedAction::A, ExpectedAction::B, ExpectedAction::B}};

    // Hyper parameters
    constexpr double LEARNING_RATE = 0.0001;
    constexpr double EXPLORATION_EPSILON = 0.2;
    constexpr bool IID_CHOICE = false;
    constexpr bool USE_SOFTMAX = true;
    constexpr Index MAX_DEPTH = 15;
    constexpr Count N_SAMPLES = 1000000ll;
    constexpr Count N_TRIALS = 1ll;
}

class OptimalAction {
public:
    OptimalAction(void) :
        rand_gen_(rand_dev_()), unit_distribution_(0.0, 1.0),
        value_(boost::extents[N_PLAYERS][N_STATES][N_ACTIONS]) {
        for (Index player = 0; player < N_PLAYERS; ++player) {
            for (Index state = 0; state < N_STATES; ++state) {
                initialize(player, state);
            }
        }
        return;
    }

    virtual ~OptimalAction(void) = default;

    void Exec(Count n_samples) {
        for (Count i = 0; i < n_samples; ++i) {
            exec();
        }
        return;
    }

    void Print(void) {
        for (Index player = 0; player < N_PLAYERS; ++player) {
            std::cout << printValue(player);
        }
        return;
    }

    bool Check(void) {
        bool result = true;

        for (Index player = 0; player < N_PLAYERS; ++player) {
            for (Index state = 0; state < N_STATES; ++state) {
                const auto expectedAction = ExpectedActions[player][state];
                if (expectedAction == ExpectedAction::Undefined) {
                    continue;
                }

                Index actual = N_ACTIONS;
                double maxValue = -std::numeric_limits<double>::infinity();
                for (Index action = 0; action < N_ACTIONS; ++action) {
                    auto value = value_[player][state][action];
                    if (maxValue < value) {
                        actual = action;
                        maxValue = value;
                    }
                }

                const auto expected = static_cast<decltype(actual)>(expectedAction);
                if (static_cast<decltype(actual)>(expected) != actual) {
                    result = false;
                    std::cout << "! Player=" << player << ", State=" << state;
                    std::cout << ", Expected=" << expected << ", Actual=" << actual << "\n";
                }
            }
        }
        return result;
    }

    void initialize(Index player, State state) {
        for (Index action = 0; action < N_ACTIONS; ++action) {
            // Do not shoot yourself!
            auto count = checkAliveOrNobody(player, state);
            if (USE_SOFTMAX) {
                value_[player][state][action] = count && checkAliveOrNobody(action, state) &&
                    (player != action) ? 0.0 : -std::numeric_limits<double>::infinity();
            } else {
                value_[player][state][action] = count && checkAliveOrNobody(action, state) &&
                    (player != action) ? (1.0 / static_cast<double>(count)) : 0.0;
            }
        }

        if (USE_SOFTMAX) {
            normalizeSoftmaxProbabilities(player, state);
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

    // Converts an array to a bitmap
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

    // exp(elements) /= exp(max(elements)) to avoid overflow
    void normalizeSoftmaxProbabilities(Index player, State state) {
        double max_log = -std::numeric_limits<double>::infinity();
        for (Index action = 0; action < N_ACTIONS; ++action) {
            max_log = std::max(max_log, value_[player][state][action]);
        }

        // Adjust max(log(probabilities)) to zero
        for (Index action = 0; action < N_ACTIONS; ++action) {
            value_[player][state][action] -= max_log;
        }
        return;
    }

    std::vector<double> getSoftmaxProbabilities(Index player, State state) {
        std::vector<double> probabilities(N_ACTIONS, 0.0);
        double sum = 0.0;

        for (Index action = 0; action < N_ACTIONS; ++action) {
            const auto value = ::exp(value_[player][state][action]);
            probabilities.at(action) = value;
            sum += value;
        }

        for (Index action = 0; action < N_ACTIONS; ++action) {
            probabilities.at(action) /= sum;
        }

        return probabilities;
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
                backpropagate(nextActionChain.at(index), winner);
            } else {
                // Reverse if you deduct rewards
                for(const auto& action : nextActionChain) {
                    backpropagate(action, winner);
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

    std::vector<double> getProportions(Index player, State state) {
        // Number of targets = number of survivors - player(1) + nobody(1)
        std::vector<double> proportions(N_ACTIONS, 0.0);

        // Epsilon-greedy
        const auto rand_proportional = unit_distribution_(rand_gen_);
        const bool proportional = (rand_proportional < EXPLORATION_EPSILON);

        if (proportional) {
            // Number of targets = number of survivors - player(1) + nobody(1)
            const auto population = checkPlayerAlive(player, state);
            const double proportion = (population > 0) ? (1.0 / static_cast<double>(population)) : 0.0;
            for (Index action = 0; action < N_ACTIONS; ++action) {
                proportions.at(action) = (checkPlayerAlive(player, state) &&
                                          checkAliveOrNobody(action, state) &&
                                          (player != action)) ? proportion : 0.0;
            }
        } else {
            if (USE_SOFTMAX) {
                proportions = getSoftmaxProbabilities(player, state);
            } else {
                for (Index action = 0; action < N_ACTIONS; ++action) {
                    proportions.at(action) = value_[player][state][action];
                }
            }
        }

        return proportions;
    }

    Index getActionTarget(Index player, State state, const Survivors& survivors, const std::vector<Index>& targets) {
        const auto proportions = getProportions(player, state);
        auto rand_value = unit_distribution_(rand_gen_);
        Index target = 0;

        while(rand_value >= 0.0) {
            rand_value -= proportions.at(target);
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

    void backpropagate(const Action& action, Index final_surviver) {
        // Normalizes such that the sum of values is 1
        if (USE_SOFTMAX) {
            // backpropagate Y-T * delta(error)/delta(y)
            auto exp_proportions = getSoftmaxProbabilities(action.player, action.state);
            for (Index i = 0; i < N_ACTIONS; ++i) {
                const auto delta = (exp_proportions[i] - ((action.player == final_surviver) ? 1.0 : 0.0)) * LEARNING_RATE;
                value_[action.player][action.state][action.target] -= delta;
            }
            normalizeSoftmaxProbabilities(action.player, action.state);
        } else {
            const auto target_value = value_[action.player][action.state][action.target];
            const auto delta = target_value * LEARNING_RATE * ((action.player == final_surviver) ? 1.0 : -1.0);
            value_[action.player][action.state][action.target] += delta;
            double sum = 0.0;
            for (Index i = 0; i < N_ACTIONS; ++i) {
                sum += value_[action.player][action.state][i];
            }
            for (Index i = 0; i < N_ACTIONS; ++i) {
                value_[action.player][action.state][i] /= sum;
            }
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
    Count n_samples = N_SAMPLES;
    if (argc > 1) {
        n_trials = ::atoll(argv[1]);
    }
    if (argc > 2) {
        n_samples = ::atoll(argv[2]);
    }

    Count n_correct = 0;
    Count n_wrong = 0;

    for (Count trial = 0; trial < n_trials; ++trial) {
        optimalAction.Exec(n_samples);
        const auto correct = optimalAction.Check();
        n_correct += (correct ? 1 : 0);
        n_wrong += (correct ? 0 : 1);
        if (!trial || !correct) {
            optimalAction.Print();
        } else {
            std::cerr << ".";
        }
    }

    std::cout << "Correct cases=" << n_correct << ", Wrong cases" << n_wrong << "\n";
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
