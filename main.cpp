#include "strategies.hpp"
#include <iostream>
#include <vector>
#include <array>
#include <iomanip>

static const std::array<std::array<int, 2>, 2> payoff = {{
    {{3, 0}},
    {{5, 1}}
}};

inline int get_payoff(Move a, Move b) {
    int a_idx = (a == Move::DEFECT) ? 1 : 0;
    int b_idx = (b == Move::DEFECT) ? 1 : 0;
    return payoff[a_idx][b_idx];
}

inline char move_char(Move m) {
    return (m == Move::COOPERATE) ? 'C' : 'D';
}

void print_history(uint64_t hist, int rounds) {
    for (int r = rounds - 1; r >= 0; --r) {
        bool move = (hist >> r) & 1;
        std::cout << (move ? 'D' : 'C');
        if (r > 0) std::cout << " ";
    }
}

int main() {
    std::vector<StrategyType> strategies = {
        StrategyType::HOLY,
        StrategyType::TRAITOR,
        StrategyType::TIT_FOR_TAT,
        StrategyType::Friedman,
        StrategyType::Random,
        StrategyType::Joss,
        StrategyType::TIT_FOR_2TAT,
        StrategyType::TITS2_FOR_TAT,
        StrategyType::Pavlov,
        StrategyType::Generous_TIT_FOR_TAT,
        StrategyType::Average_64,
        StrategyType::Tideman_and_Chieruzzi,
        StrategyType::Nydegger,
        StrategyType::Grogman,
        StrategyType::Shubik,
        StrategyType::Stein_Rapoport,
        StrategyType::Davis,
        StrategyType::Graaskamp,
        StrategyType::First_by_Downing,
        StrategyType::Feld,
        StrategyType::Tullock,
        StrategyType::GRASR,
        StrategyType::K31R,
        StrategyType::K32R,
        StrategyType::K33R,
        StrategyType::K35R,
        StrategyType::K36R,
        StrategyType::K37R,
        StrategyType::K38R,
        StrategyType::K39R,
        StrategyType::K40R,
        StrategyType::K41R,
        StrategyType::K42R,
        StrategyType::K43R,
        StrategyType::K44R
    };
    const int ROUNDS = 500;
    const int NUM_STRATS = strategies.size();
    std::cout << "=== AMOUNT OF STRATEGIES ===" << std::endl;
    std::cout << "There are " << NUM_STRATS << " strategies" << std::endl;
    std::cout << std::endl;

    std::vector<int> scores(NUM_STRATS, 0);

    std::cout << "=== TOURNAMENT RESULTS ===" << std::endl;
    std::cout << "Rounds per pair: " << ROUNDS << std::endl;
    std::cout << std::endl;

    for (int i = 0; i < NUM_STRATS; ++i) {
        for (int j = 0; j < NUM_STRATS; ++j) {
            uint64_t hist_i = 0;
            uint64_t hist_j = 0;
            int score_i = 0, score_j = 0;

            for (int round = 0; round < ROUNDS; ++round) {
                Move move_i = get_move(strategies[i], hist_i);
                Move move_j = get_move(strategies[j], hist_j);

                score_i += get_payoff(move_i, move_j);
                score_j += get_payoff(move_j, move_i);

                hist_i = (hist_i << 1) | (move_j == Move::DEFECT ? 1ULL : 0ULL);
                hist_j = (hist_j << 1) | (move_i == Move::DEFECT ? 1ULL : 0ULL);
            }

            scores[i] += score_i;
            scores[j] += score_j;

            std::cout << std::setw(12) << strategy_name(strategies[i]) << " vs "
                      << std::setw(12) << strategy_name(strategies[j]) << " : "
                      << std::setw(3) << score_i << " - "
                      << std::setw(3) << score_j << std::endl;
        }
    }

    std::cout << "\n=== FINAL SCORES ===" << std::endl;
    for (int i = 0; i < NUM_STRATS; ++i) {
        std::cout << std::setw(12) << strategy_name(strategies[i]) << " : "
                  << scores[i] << std::endl;
    }

    int max_score = scores[0];
    int winner_idx = 0;
    for (int i = 1; i < NUM_STRATS; ++i) {
        if (scores[i] > max_score) {
            max_score = scores[i];
            winner_idx = i;
        }
    }

    std::cout << "\n=== WINNER ===" << std::endl;
    std::cout << strategy_name(strategies[winner_idx]) << " with "
              << max_score << " points!" << std::endl;

    return 0;
}
