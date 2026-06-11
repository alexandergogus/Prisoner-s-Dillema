#ifndef STRATEGIES_HPP
#define STRATEGIES_HPP

#include <cstdint>
#include <random>
enum class Move : bool {
    COOPERATE = false,
    DEFECT = true
};

enum class StrategyType : uint8_t {
    HOLY,
    TRAITOR,
    TIT_FOR_TAT,
    Friedman,
    Random,
    Joss
};

inline Move strategy_holy(uint64_t /*opp_history*/, uint64_t /*my_history*/ = 0) {
    return Move::COOPERATE;
}

inline Move strategy_traitor(uint64_t /*opp_history*/, uint64_t /*my_history*/ = 0) {
    return Move::DEFECT;
}

inline Move strategy_tit_for_tat(uint64_t opp_history, uint64_t /*my_history*/ = 0) {
    if (opp_history == 0) return Move::COOPERATE;
    bool last_opp_move = opp_history & 1;
    return last_opp_move ? Move::DEFECT : Move::COOPERATE;
}

inline Move strategy_friedman(uint64_t opp_history, uint64_t /*my_history*/ = 0) {
    if (opp_history != 0) {
        return Move::DEFECT;
    }
    return Move::COOPERATE;
}

inline Move strategy_random(uint64_t /*opp_history*/, uint64_t /*my_history*/ = 0){
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::bernoulli_distribution dist(0.5);
    bool random_boolean = dist(gen);
    return random_boolean ? Move::DEFECT : Move::COOPERATE;
}

inline Move strategy_joss(uint64_t opp_history, uint64_t /*my_history*/ = 0){
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::bernoulli_distribution dist(0.1);
    if (opp_history == 0) return Move::COOPERATE;
    if (dist(gen)) return Move::DEFECT;
    bool last_opp_move = opp_history & 1;
    return last_opp_move ? Move::DEFECT : Move::COOPERATE;
}

// Dispatcher
inline Move get_move(StrategyType type, uint64_t opp_history, uint64_t my_history = 0) {
    switch (type) {
        case StrategyType::HOLY:        return strategy_holy(opp_history, my_history);
        case StrategyType::TRAITOR:     return strategy_traitor(opp_history, my_history);
        case StrategyType::TIT_FOR_TAT: return strategy_tit_for_tat(opp_history, my_history);
        case StrategyType::Friedman: return strategy_friedman(opp_history, my_history);
        case StrategyType::Random: return strategy_random(opp_history, my_history);
        case StrategyType::Joss: return strategy_joss(opp_history, my_history);
        default:                        return Move::COOPERATE;
    }
}

// Convert strategy type to string
inline const char* strategy_name(StrategyType type) {
    switch (type) {
        case StrategyType::HOLY:        return "HOLY";
        case StrategyType::TRAITOR:     return "TRAITOR";
        case StrategyType::TIT_FOR_TAT: return "TIT_FOR_TAT";
        case StrategyType::Friedman: return "Friedman";
        case StrategyType::Random: return "Random";
        case StrategyType::Joss: return "Joss";
        default:                        return "?";
    }
}

#endif
