#ifndef STRATEGIES_HPP
#define STRATEGIES_HPP

#include <cstdint>
#include <random>
#include <bitset>
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
    Pavlov,
    Generous_TIT_FOR_TAT,
    Average_64,
    Tideman_and_Chieruzzi,
    Nydegger,
    Grogman,
    Shubik,
    Stein_Rapoport,
    Davis,
    Graaskamp,
    First_by_Downing,
    Feld
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

inline Move strategy_generous_tit_for_tat(uint64_t opp_history,  uint64_t /*my_history*/ = 0){
    if (opp_history == 0) return Move::COOPERATE;
    bool last_opp_move = opp_history & 1;
    if (last_opp_move == 0) return Move::COOPERATE;
    else{
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::bernoulli_distribution dist(2.0/3.0);
        return (dist(gen)) ? Move::DEFECT : Move::COOPERATE;

    }
}

inline Move strategy_average64(uint64_t opp_history,  uint64_t my_history = 0){
    if (opp_history == 0) return Move::COOPERATE;
    std::size_t defects = std::bitset<64>(opp_history).count();
    std::size_t rounds_played = 64 - __builtin_clzll(opp_history);
    double defect_rate = static_cast<double>(defects)/rounds_played;
    return (defect_rate <= 0.5) ? Move::COOPERATE : Move::DEFECT;
}

inline Move strategy_tideman_chieruzzi(uint64_t opp_history, uint64_t my_history = 0) {
    int opp_defects = std::bitset<64>(opp_history).count();
    int my_defects = std::bitset<64>(my_history).count();

    int rounds_played = 0;
    uint64_t temp = opp_history;
    while (temp) {
        rounds_played++;
        temp >>= 1;
    }

    if (rounds_played == 0) {
        return Move::DEFECT;
    }

    bool last_opp_defected = (opp_history & 1);

    if (opp_defects >= 5 && my_defects <= opp_defects - 3) {
        return Move::DEFECT;
    }
    int consecutive_coop = 0;
    uint64_t history = opp_history;

    while (history && !(history & 1)) {
        consecutive_coop++;
        history >>= 1;
    }
    if (consecutive_coop >= 3) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::bernoulli_distribution dist(0.85);
        return dist(gen) ? Move::COOPERATE : Move::DEFECT;
    }
    if (last_opp_defected) {
        uint64_t prev_two = (opp_history >> 1) & 3;
        if (prev_two == 0) {
            return Move::COOPERATE;
        }
        return Move::DEFECT;
    }

    return Move::COOPERATE;
}

inline Move strategy_nydegger(uint64_t opp_history, uint64_t my_history) {
    int rounds = 0;
    uint64_t temp = opp_history;
    while (temp) { rounds++; temp >>= 1; }

    if (rounds == 0) return Move::COOPERATE;
    if (rounds == 1) {
        bool opp_defected = (opp_history & 1);
        return opp_defected ? Move::DEFECT : Move::COOPERATE;
    }
    if (rounds == 2) {
        bool opp_last = (opp_history & 1);
        return opp_last ? Move::DEFECT : Move::COOPERATE;
    }
    int opp3 = opp_history & 0b111;
    int my3  = my_history & 0b111;

    static const uint8_t nydegger_table[64] = {
        // my3 = 0 (CCC)
        0, 0, 0, 1,  0, 0, 1, 1,
        // my3 = 1 (CCD)
        0, 0, 1, 1,  0, 1, 1, 1,
        // my3 = 2 (CDC)
        0, 1, 1, 1,  1, 1, 1, 1,
        // my3 = 3 (CDD)
        0, 1, 1, 1,  1, 1, 1, 1,
        // my3 = 4 (DCC)
        0, 0, 1, 1,  1, 1, 1, 1,
        // my3 = 5 (DCD)
        0, 0, 1, 1,  1, 1, 1, 1,
        // my3 = 6 (DDC)
        0, 1, 1, 1,  1, 1, 1, 1,
        // my3 = 7 (DDD)
        1, 1, 1, 1,  1, 1, 1, 1
    };

    int index = (my3 << 3) | opp3;
    bool next_defect = nydegger_table[index];
    return next_defect ? Move::DEFECT : Move::COOPERATE;
}

inline Move strategy_grofman(uint64_t opp_history, uint64_t my_history) {
    int round = 0;
    uint64_t temp = opp_history;
    while (temp) {
        ++round;
        temp >>= 1;
    }

    if (round < 2) {
        return Move::COOPERATE;
    }

    if (round < 7) {
        bool last_opp_move = opp_history & 1;
        return last_opp_move ? Move::DEFECT : Move::COOPERATE;
    }
    bool last_my_move = my_history & 1;
    bool last_opp_move = opp_history & 1;

    if (last_my_move == last_opp_move) {
        return Move::COOPERATE;
    }
    else {
        static thread_local std::mt19937 gen(std::random_device{}());
        std::bernoulli_distribution dist(2.0 / 7.0);
        return dist(gen) ? Move::COOPERATE : Move::DEFECT;
    }
}

inline Move strategy_shubik(uint64_t opp_history, uint64_t my_history) {
    int round = 0;
    uint64_t temp = opp_history;
    while (temp) {
        ++round;
        temp >>= 1;
    }

    if (round == 0) {
        return Move::COOPERATE;
    }

    static int grudge_level = 1;
    static int retaliation_remaining = 0;
    static bool in_retaliation = false;

    bool last_my_move   = my_history & 1;
    bool last_opp_move  = opp_history & 1;

    if (in_retaliation) {
        if (retaliation_remaining > 0) {
            retaliation_remaining--;
            if (retaliation_remaining == 0) {
                in_retaliation = false;
            }
            return Move::DEFECT;
        }
        in_retaliation = false;
    }

    if (!in_retaliation && last_opp_move == 1 && last_my_move == 0) {
        grudge_level++;
    }

    if (last_opp_move == 1) {
        in_retaliation = true;
        retaliation_remaining = grudge_level - 1;
        return Move::DEFECT;
    } else {
        return Move::COOPERATE;
    }
}

inline Move strategy_stein_rapoport(uint64_t opp_history, uint64_t my_history = 0) {
    int round = 0;
    uint64_t temp = opp_history;
    while (temp) {
        ++round;
        temp >>= 1;
    }

    if (round < 4) {
        return Move::COOPERATE;
    }

    int total_rounds = 0;
    temp = my_history;
    while (temp) {
        ++total_rounds;
        temp >>= 1;
    }

    total_rounds = round + 1;

    if (round >= total_rounds - 2) {
        return Move::DEFECT;
    }

    bool last_opp_move = opp_history & 1;
    return last_opp_move ? Move::DEFECT : Move::COOPERATE;
}

inline Move strategy_davis(uint64_t opp_history, uint64_t my_history = 0) {
    int round = 0;
    uint64_t temp = opp_history;
    while (temp) {
        ++round;
        temp >>= 1;
    }

    if (round < 10) {
        return Move::COOPERATE;
    }

    if (opp_history != 0) {
        return Move::DEFECT;
    } else {
        return Move::COOPERATE;
    }
}

inline Move strategy_graaskamp(uint64_t opp_history, uint64_t my_history) {
    int round = 0;
    uint64_t temp = opp_history;
    while (temp) {
        ++round;
        temp >>= 1;
    }
    int round_one_indexed = round + 1;

    if (round_one_indexed < 50) {
        if (round == 0) return Move::COOPERATE;
        bool last_opp_move = opp_history & 1;
        return last_opp_move ? Move::DEFECT : Move::COOPERATE;
    }

    if (round_one_indexed == 51) {
        return Move::DEFECT;
    }

    if (round_one_indexed <= 56) {
        bool last_opp_move = opp_history & 1;
        return last_opp_move ? Move::DEFECT : Move::COOPERATE;
    }

    static bool opponent_is_random = false;
    static int next_random_defection_turn = -1;
    static int rounds_played_at_last_check = 0;

    int rounds_played = round_one_indexed;

    if (rounds_played - rounds_played_at_last_check >= 50) {
        int opp_defections = std::bitset<64>(opp_history).count();
        int opp_cooperations = (rounds_played - 1) - opp_defections;

        double expected = (rounds_played - 1) / 2.0;
        double chi_square = (std::pow(opp_cooperations - expected, 2) / expected) +
                            (std::pow(opp_defections - expected, 2) / expected);

        opponent_is_random = (chi_square < 3.841);
        rounds_played_at_last_check = rounds_played;
    }

    if (opponent_is_random) {
        return Move::DEFECT;
    }

    bool opponent_is_tft = true;
    bool opponent_is_clone = true;

    if (rounds_played >= 2) {
        bool my_last_move = my_history & 1;
        bool opp_last_move = opp_history & 1;
        if (opp_last_move != my_last_move) {
            opponent_is_tft = false;
        }
    }

    if (opp_history != my_history) {
        opponent_is_clone = false;
    }

    if (opponent_is_tft || opponent_is_clone) {
        bool last_opp_move = opp_history & 1;
        return last_opp_move ? Move::DEFECT : Move::COOPERATE;
    }

    if (next_random_defection_turn == -1) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(5, 15);
        next_random_defection_turn = rounds_played + dist(gen);
    }

    if (rounds_played == next_random_defection_turn) {
        static std::random_device rd2;
        static std::mt19937 gen2(rd2());
        std::uniform_int_distribution<> dist2(5, 15);
        next_random_defection_turn = rounds_played + dist2(gen2);
        return Move::DEFECT;
    }

    return Move::COOPERATE;
}

inline Move strategy_first_by_downing(uint64_t opp_history, uint64_t my_history) {
    int round = 0;
    uint64_t temp = opp_history;
    while (temp) {
        round++;
        temp >>= 1;
    }

    static int opponent_coop_after_my_coop = 0;
    static int opponent_coop_after_my_defect = 0;
    static int total_my_coop = 0;
    static int total_my_defect = 0;

    if (round == 0) {
        return Move::DEFECT;
    }

    if (round == 1) {
        bool opp_last_move = opp_history & 1;
        if (opp_last_move == 0) {
            opponent_coop_after_my_coop++;
        }
        total_my_coop++;
        total_my_defect++;
        return Move::DEFECT;
    }

    bool my_prev_move = (my_history >> 1) & 1;
    bool opp_prev_move = (opp_history >> 1) & 1;
    bool opp_last_move = opp_history & 1;

    if (my_prev_move == 0) {
        total_my_coop++;
        if (opp_prev_move == 0) opponent_coop_after_my_coop++;
    }
    else {
        total_my_defect++;
        if (opp_prev_move == 0) opponent_coop_after_my_defect++;
    }

    double alpha = opponent_coop_after_my_coop / static_cast<double>(total_my_coop);
    double beta = opponent_coop_after_my_defect / static_cast<double>(total_my_defect);

    const int R = 3;
    const int P = 1;
    const int S = 0;
    const int T = 5;

    double expected_coop = alpha * R + (1 - alpha) * S;
    double expected_defect = beta * T + (1 - beta) * P;

    if (expected_coop > expected_defect) {
        return Move::COOPERATE;
    }
    else if (expected_coop < expected_defect) {
        return Move::DEFECT;
    }
    else {
        bool my_last_move = my_history & 1;
        return my_last_move ? Move::COOPERATE : Move::DEFECT;
    }
}

inline Move strategy_feld(uint64_t opp_history, uint64_t /*my_history*/ = 0) {
    int rounds = 0;
    int opp_defects = 0;
    uint64_t hist = opp_history;
    while (hist) {
        if (hist & 1) opp_defects++;
        rounds++;
        hist >>= 1;
    }

    if (rounds == 0) return Move::COOPERATE;

    bool last_opp_defected = (opp_history & 1);

    if (!last_opp_defected) {
        return Move::COOPERATE;
    }

    double prob = static_cast<double>(opp_defects) / rounds;
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::bernoulli_distribution dist(prob);
    return dist(gen) ? Move::DEFECT : Move::COOPERATE;
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
        case StrategyType::Generous_TIT_FOR_TAT: return strategy_generous_tit_for_tat(opp_history, my_history);
        case StrategyType::Average_64: return strategy_average64(opp_history, my_history);
        case StrategyType::Tideman_and_Chieruzzi: return strategy_tideman_chieruzzi(opp_history, my_history);
        case StrategyType::Nydegger: return strategy_nydegger(opp_history, my_history);
        case StrategyType::Grogman: return strategy_grofman(opp_history, my_history);
        case StrategyType::Shubik: return strategy_shubik(opp_history, my_history);
        case StrategyType::Stein_Rapoport: return strategy_stein_rapoport(opp_history, my_history);
        case StrategyType::Davis: return strategy_davis(opp_history, my_history);
        case StrategyType::Graaskamp: return strategy_graaskamp(opp_history, my_history);
        case StrategyType::First_by_Downing: return strategy_first_by_downing(opp_history, my_history);
        case StrategyType::Feld: return strategy_feld(opp_history, my_history);
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
        case StrategyType::Generous_TIT_FOR_TAT: return "Generous_TIT_FOR_TAT";
        case StrategyType::Average_64: return "Average_64";
        case StrategyType::Tideman_and_Chieruzzi: return "Tideman and Chieruzzi";
        case StrategyType::Nydegger: return "Nydegger";
        case StrategyType::Grogman: return "Grofman";
        case StrategyType::Shubik: return "Shubik";
        case StrategyType::Stein_Rapoport: return "Stein Rapoport";
        case StrategyType::Davis: return "Davis";
        case StrategyType::Graaskamp: return "Graaskamp";
        case StrategyType::First_by_Downing: return "First by Downing";
        case StrategyType::Feld: return "Feld";
        default:                        return "?";
    }
}

#endif
