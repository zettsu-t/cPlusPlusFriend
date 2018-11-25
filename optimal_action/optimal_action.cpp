// Solving a puzzle below
// https://sist8.com/yougun

#include <cstdlib>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
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
    constexpr Count DEFAULT_SAMPLES_SOFTMAX = 1000000ll;
    constexpr Count DEFAULT_SAMPLES_LINEAR = 10000000ll;
    constexpr double DEFAULT_LEARNING_RATE_LINEAR = 0.00001;
    constexpr double DEFAULT_LEARNING_RATE_SOFTMAX = 0.0001;
    constexpr double DEFAULT_EXPLORATION_RATIO_LINEAR = 0.1;
    constexpr double DEFAULT_EXPLORATION_RATIO_SOFTMAX = 0.2;

    constexpr Count MIN_TRIALS = 1ll;
    constexpr bool DEFAULT_USE_SOFTMAX = false;
    constexpr bool IID_CHOICE = false;
    constexpr Index MAX_DEPTH = 15;
}

#define OPTION_NAME_TRIALS "trials"
#define OPTION_NAME_SAMPLES "samples"
#define OPTION_NAME_SOFTMAX "softmax"
#define OPTION_NAME_LEARNING_RATE "learning_rate"
#define OPTION_NAME_EXPLORATION_RATIO "exploration_ratio"
#define OPTION_NAME_LOGFILE "logfile"

class OptimalAction {
public:
    struct Setting {
        bool useSoftmax {DEFAULT_USE_SOFTMAX};
        boost::optional<Count> numSamples;
        boost::optional<double> learningRate;
        boost::optional<double> explorationRatio;
        std::string logFilename;
    };

    OptimalAction(const Setting& setting) :
        rand_gen_(rand_dev_()), unit_distribution_(0.0, 1.0),
        value_(boost::extents[N_PLAYERS][N_STATES][N_ACTIONS]),
        useSoftmax_(setting.useSoftmax) {
        numSamples_ = (setting.numSamples) ? (*setting.numSamples) :
            ((useSoftmax_) ? DEFAULT_SAMPLES_SOFTMAX : DEFAULT_SAMPLES_LINEAR);
        learningRate_ = (setting.learningRate) ? (*setting.learningRate) :
            ((useSoftmax_) ? DEFAULT_LEARNING_RATE_SOFTMAX : DEFAULT_LEARNING_RATE_LINEAR);
        explorationRatio_ = (setting.explorationRatio) ? (*setting.explorationRatio) :
            ((useSoftmax_) ? DEFAULT_EXPLORATION_RATIO_SOFTMAX : DEFAULT_EXPLORATION_RATIO_LINEAR);

        if (!setting.logFilename.empty()) {
            outStream_ = std::ofstream(setting.logFilename);
            *outStream_ << "B,C,Nobody\n";
        }

        for (Index player = 0; player < N_PLAYERS; ++player) {
            for (Index state = 0; state < N_STATES; ++state) {
                initialize(player, state);
            }
        }

        std::cout << "UseSoftmax=" << useSoftmax_ << ", # of Samples=" << numSamples_;
        std::cout << ", Learning Rate=" << learningRate_;
        std::cout << ", Exploration Ratio=" << explorationRatio_ << "\n";
        return;
    }

    virtual ~OptimalAction(void) = default;

    void Exec(void) {
        for (Count i = 0; i < numSamples_; ++i) {
            exec();
            if (!outStream_) {
                continue;
            }

            Index player = static_cast<decltype(player)>(ExpectedAction::A);
            for (Index action = 0; action < N_ACTIONS; ++action) {
                if (player != action) {
                    *outStream_ << value_[player][N_STATES-1][action];
                    if ((action + 1) < N_ACTIONS) {
                        *outStream_ << ",";
                    }
                }
            }
            *outStream_ << "\n";
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
            if (useSoftmax_) {
                value_[player][state][action] = count && checkAliveOrNobody(action, state) &&
                    (player != action) ? 0.0 : -std::numeric_limits<double>::infinity();
            } else {
                value_[player][state][action] = count && checkAliveOrNobody(action, state) &&
                    (player != action) ? (1.0 / static_cast<double>(count)) : 0.0;
            }
        }

        if (useSoftmax_) {
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
        const bool proportional = (rand_proportional < explorationRatio_);

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
            if (useSoftmax_) {
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
        if (useSoftmax_) {
            // backpropagate Y-T * delta(error)/delta(y)
            auto exp_proportions = getSoftmaxProbabilities(action.player, action.state);
            for (Index i = 0; i < N_ACTIONS; ++i) {
                const auto delta = (exp_proportions[i] - ((action.player == final_surviver) ? 1.0 : 0.0)) * learningRate_;
                value_[action.player][action.state][action.target] -= delta;
            }
            normalizeSoftmaxProbabilities(action.player, action.state);
        } else {
            const auto target_value = value_[action.player][action.state][action.target];
            const auto delta = target_value * learningRate_ * ((action.player == final_surviver) ? 1.0 : -1.0);
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
    using StateActionValue = boost::multi_array<double, 3>;
    std::random_device rand_dev_;
    std::mt19937 rand_gen_;
    std::uniform_real_distribution<double> unit_distribution_;
    StateActionValue value_;

    bool useSoftmax_ {DEFAULT_USE_SOFTMAX};
    Count numSamples_ {0};
    double learningRate_ {0.0};
    double explorationRatio_ {0.0};
    boost::optional<std::ofstream> outStream_;
};

int main(int argc, char* argv[]) {
    Count numTrials = MIN_TRIALS;
    OptimalAction::Setting setting;

    boost::program_options::options_description description("Options");
    description.add_options()
        (OPTION_NAME_TRIALS",t",
         boost::program_options::value<decltype(numTrials)>(),
         "Number of searches")
        (OPTION_NAME_SAMPLES",s",
         boost::program_options::value<decltype(setting.numSamples)>(),
         "Number of samples in a search")
        (OPTION_NAME_SOFTMAX",x",
         boost::program_options::value<decltype(setting.useSoftmax)>()->default_value(false),
         "Use softmax instead of linear")
        (OPTION_NAME_LEARNING_RATE",l",
         boost::program_options::value<decltype(setting.learningRate)::value_type>(),
         "Learning rate")
        (OPTION_NAME_EXPLORATION_RATIO",e",
         boost::program_options::value<decltype(setting.explorationRatio)::value_type>(),
         "Probability of explorations")
        (OPTION_NAME_LOGFILE",o",
         boost::program_options::value<decltype(setting.logFilename)>(),
         "Output log file name")
        ;

    boost::program_options::variables_map varMap;
    boost::program_options::store(parse_command_line(argc, argv, description), varMap);
    boost::program_options::notify(varMap);

    if (varMap.count(OPTION_NAME_TRIALS)) {
        numTrials = std::max(MIN_TRIALS, varMap[OPTION_NAME_TRIALS].
                             as<decltype(numTrials)>());
    }

    if (varMap.count(OPTION_NAME_SAMPLES)) {
        setting.numSamples = varMap[OPTION_NAME_SAMPLES].
            as<decltype(setting.numSamples)>();
    }

    if (varMap.count(OPTION_NAME_SOFTMAX)) {
        setting.useSoftmax = varMap[OPTION_NAME_SOFTMAX].
            as<decltype(setting.useSoftmax)>();
    }

    if (varMap.count(OPTION_NAME_LEARNING_RATE)) {
        setting.learningRate = varMap[OPTION_NAME_LEARNING_RATE].
            as<decltype(setting.learningRate)::value_type>();
    }

    if (varMap.count(OPTION_NAME_EXPLORATION_RATIO)) {
        setting.explorationRatio = varMap[OPTION_NAME_EXPLORATION_RATIO]
            .as<decltype(setting.explorationRatio)::value_type>();
    }

    if (varMap.count(OPTION_NAME_LOGFILE)) {
        setting.logFilename = varMap[OPTION_NAME_LOGFILE]
            .as<decltype(setting.logFilename)>();
    }

    OptimalAction optimalAction(setting);
    Count n_correct = 0;
    Count n_wrong = 0;
    for (Count trial = 0; trial < numTrials; ++trial) {
        optimalAction.Exec();
        const auto correct = optimalAction.Check();
        n_correct += (correct ? 1 : 0);
        n_wrong += (correct ? 0 : 1);

        if (!trial || !correct) {
            optimalAction.Print();
        } else {
            std::cerr << ".";
        }
    }

    std::cout << "Correct cases=" << n_correct << ", Wrong cases=" << n_wrong << "\n";
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
