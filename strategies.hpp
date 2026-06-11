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
    Joss,
    TIT_FOR_2TAT,
    TITS2_FOR_TAT,
    Pavlov
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

inline Move strategy_tit_for_2tat(uint64_t opp_history, uint64_t /*my_history*/ = 0){
    if (opp_history == 0) return Move::COOPERATE;
    bool last_two = opp_history & 3;
    return (last_two == 3) ? Move::DEFECT : Move::COOPERATE;
}

inline Move strategy_2tits_for_tat(uint64_t opp_history,  uint64_t /*my_history*/ = 0){
    if (opp_history == 0) return Move::COOPERATE;
    return ((opp_history & 3) != 0) ? Move::DEFECT : Move::COOPERATE;

}

inline Move strategy_pavlov(uint64_t opp_history,  uint64_t my_history = 0){
    if (opp_history == 0) return Move::COOPERATE;
    bool last_opp = opp_history & 1;
    bool last_me = my_history & 1;
    bool win = (last_me == 0 && last_opp == 0) || (last_me == 1 && last_opp == 0);
    bool next_move = win ? last_me : !last_me;
    return next_move ? Move::DEFECT : Move::COOPERATE;
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
        case StrategyType::TIT_FOR_2TAT: return strategy_tit_for_2tat(opp_history, my_history);
        case StrategyType::TITS2_FOR_TAT: return strategy_2tits_for_tat(opp_history, my_history);
        case StrategyType::Pavlov: return strategy_pavlov(opp_history, my_history);
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
        case StrategyType::TIT_FOR_2TAT: return "TIT_FOR_2TAT";
        case StrategyType::TITS2_FOR_TAT: return "2TITS_FOR_TAT";
        case StrategyType::Pavlov: return "Pavlov";
        default:                        return "?";
    }
}

#endif
