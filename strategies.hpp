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
    Feld,
    Tullock,
    GRASR,
    K31R,
    K32R,
    K33R,
    K35R,
    K36R,
    K37R,
    K38R,
    K39R
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
    static bool ever_defected = false;
    static int prev_rounds = 0;

    int rounds = 0;
    uint64_t temp = opp_history;
    while (temp) { ++rounds; temp >>= 1; }
    int current_round = rounds + 1;

    if (current_round == 1 || current_round < prev_rounds) {
        ever_defected = false;
    }
    prev_rounds = current_round;

    bool opp_last_defected = (opp_history & 1);
    if (opp_last_defected) ever_defected = true;

    return ever_defected ? Move::DEFECT : Move::COOPERATE;
}

inline Move strategy_random(uint64_t /*opp_history*/, uint64_t /*my_history*/ = 0){
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::bernoulli_distribution dist(0.5);
    bool random_boolean = dist(gen);
    return random_boolean ? Move::DEFECT : Move::COOPERATE;
}

inline Move strategy_joss(uint64_t opp_history, uint64_t /*my_history*/ = 0) {
    if (opp_history == 0) return Move::COOPERATE;

    bool last_opp_move = opp_history & 1;

    if (last_opp_move) return Move::DEFECT;

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::bernoulli_distribution dist(0.1);  // 10% defect
    return dist(gen) ? Move::DEFECT : Move::COOPERATE;
}

inline Move strategy_tit_for_2tat(uint64_t opp_history, uint64_t /*my_history*/ = 0){
    if (opp_history == 0) return Move::COOPERATE;
    bool last_two = opp_history & 3;
    return (last_two == 3ULL) ? Move::DEFECT : Move::COOPERATE;
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

inline Move strategy_tullock(uint64_t opp_history, uint64_t /*my_history*/ = 0) {
    int rounds = 0;
    uint64_t temp = opp_history;
    while (temp) {
        ++rounds;
        temp >>= 1;
    }

    if (rounds < 5) {
        return Move::COOPERATE;
    }
    if (rounds == 5) {
        return Move::DEFECT;
    }
    bool last_opp_move = opp_history & 1;
    return last_opp_move ? Move::DEFECT : Move::COOPERATE;
}

static int opponent_score(uint64_t opp_history, uint64_t my_history) {
    int rounds = 0;
    uint64_t temp = opp_history;
    while (temp) { ++rounds; temp >>= 1; }
    if (rounds == 0) return 0;

    int score = 0;
    for (int i = 0; i < rounds; ++i) {
        int bitpos = rounds - 1 - i;
        bool opp_move = (opp_history >> bitpos) & 1;
        bool my_move  = (my_history  >> bitpos) & 1;

        if (!opp_move && !my_move)      score += 3;
        else if (!opp_move && my_move)  score += 0;
        else if (opp_move && !my_move)  score += 5;
        else score += 1;
    }
    return score;
}

inline Move strategy_grasr(uint64_t opp_history, uint64_t my_history) {
    static int nmov[4] = {0};
    static int mmove = 0;
    static int igame = 0;
    static int n = 0;
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_real_distribution<double> dist(0.0, 1.0);

    int rounds = 0;
    uint64_t temp = opp_history;
    while (temp) { ++rounds; temp >>= 1; }
    int moven = rounds + 1;

    if (moven == 1) {
        for (int i = 0; i < 4; ++i) nmov[i] = 0;
        mmove = 0;
        igame = 0;
        n = 0;
        return Move::COOPERATE;
    }

    if (moven < 51) {
        bool last_opp = opp_history & 1;
        return last_opp ? Move::DEFECT : Move::COOPERATE;
    }

    if (moven == 51) {
        return Move::DEFECT;
    }

    if (moven >= 52 && moven <= 56) {
        if (moven == 52) {
            bool last_opp = opp_history & 1;
            return last_opp ? Move::DEFECT : Move::COOPERATE;
        } else {
            int idx = moven - 52;
            bool last_opp = opp_history & 1;
            int jpick = last_opp ? 1 : 0;
            nmov[idx-1] = mmove + jpick;
            mmove = (jpick == 0) ? 2 : 4;
            return last_opp ? Move::DEFECT : Move::COOPERATE;
        }
    }

    if (moven == 57) {
        int jscor = opponent_score(opp_history, my_history);
        if (jscor >= 135) {
            int j = nmov[1];
            if (j == 1 || j == 2) {
                igame = 1;
                n = static_cast<int>(dist(rng) * 10.0 + 5.0);
                return Move::COOPERATE;
            }
            else if (j == 3) {
                if (nmov[0] == 2 && nmov[2] == 4 && nmov[3] == 2) {
                    igame = 4;
                    return Move::COOPERATE;
                } else {
                    igame = 1;
                    n = static_cast<int>(dist(rng) * 10.0 + 5.0);
                    return Move::COOPERATE;
                }
            }
            else if (j == 4) {
                if (nmov[0] == 3 && nmov[2] == 3) {
                    igame = 2;
                    return Move::COOPERATE;
                } else {
                    igame = 1;
                    n = static_cast<int>(dist(rng) * 10.0 + 5.0);
                    return Move::COOPERATE;
                }
            }
            else if (j == 5) {
                if (nmov[0] == 5 && nmov[2] == 5) {
                    igame = 2;
                    return Move::COOPERATE;
                } else {
                    igame = 1;
                    n = static_cast<int>(dist(rng) * 10.0 + 5.0);
                    return Move::COOPERATE;
                }
            }
        } else {
            igame = 3;
            return Move::DEFECT;
        }
    }

    switch (igame) {
        case 1:
            if (n <= 0) {
                n = static_cast<int>(dist(rng) * 10.0 + 5.0);
                return Move::DEFECT;
            } else {
                n--;
                bool last_opp = opp_history & 1;
                return last_opp ? Move::DEFECT : Move::COOPERATE;
            }
        case 2:
            {
                bool last_opp = opp_history & 1;
                return last_opp ? Move::DEFECT : Move::COOPERATE;
            }
        case 3:
            return Move::DEFECT;
        case 4:
            if (moven >= 118) igame = 2;
            return Move::COOPERATE;
        default:
            return Move::COOPERATE;
    }
}

inline Move strategy_k31r(uint64_t opp_history, uint64_t /*my_history*/ = 0) {
    if (opp_history == 0) return Move::COOPERATE;

    int rounds = 0;
    int defects = 0;
    uint64_t hist = opp_history;
    while (hist) {
        if (hist & 1) ++defects;
        ++rounds;
        hist >>= 1;
    }

    if (defects * 2 < rounds)
        return Move::COOPERATE;
    else
        return Move::DEFECT;
}

inline Move strategy_k32r(uint64_t opp_history, uint64_t my_history) {
    static int C1 = 0, C2 = 0, C3 = 0, C4 = 0;
    static int J2 = 0, J1 = 0, I2 = 0, I1 = 0;
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_real_distribution<double> dist(0.0, 1.0);

    int rounds = 0;
    uint64_t temp = opp_history;
    while (temp) { ++rounds; temp >>= 1; }
    int M = rounds + 1;
    int move = 0;

    if (M == 1) {
        C1 = C2 = C3 = C4 = 0;
        J2 = J1 = I2 = I1 = 0;
        return Move::COOPERATE;
    }

    int J = (opp_history & 1) ? 1 : 0;

    if (M > 2) {
        I2 = (my_history >> 1) & 1;
        I1 = my_history & 1;

        if (I2 == 0) {
            if (J == 0) C3++; else C4++;
        } else {
            if (J == 0) C1++; else C2++;
        }
    }

    if (M >= 27) {
        int total_def_resp = C1 + C2;
        int total_coop_resp = C3 + C4;
        double threshold1 = (total_def_resp - 1.5 * sqrt(total_def_resp)) / 2.0;
        double threshold2 = (total_coop_resp - 1.5 * sqrt(total_coop_resp)) / 2.0;
        if (!(C1 < threshold1) && !(C4 < threshold2)) {
            move = 1;
            goto update;
        }
    }

    if (M >= 3) J1 = (opp_history >> 1) & 1;
    if (M >= 4) J2 = (opp_history >> 2) & 1;

    if (J1 == J) {
        if (J2 == J1) {
            move = J;
        } else {
            double P = 0.9;
            move = J;
            if (dist(rng) >= P) move = 1 - J;
        }
    } else {
        double P = (J == 1) ? 0.6 : 0.7;
        move = J;
        if (dist(rng) >= P) move = 1 - J;
    }

update:
    J2 = J1;
    J1 = J;
    I2 = I1;
    I1 = move;

    return (move == 0) ? Move::COOPERATE : Move::DEFECT;
}

inline Move strategy_k33r(uint64_t opp_history, uint64_t my_history) {
    static double coop[4] = {0.0};
    static double count[4] = {0.0};
    static int last1 = 1;
    static int last2 = 1;
    static bool twin = true;

    int rounds = 0;
    uint64_t temp = opp_history;
    while (temp) { ++rounds; temp >>= 1; }
    int M = rounds + 1;
    int J = (opp_history & 1) ? 1 : 0;

    if (M == 1) {
        for (int i = 0; i < 4; ++i) {
            coop[i] = 0.0;
            count[i] = 0.0;
        }
        last1 = 1;
        last2 = 1;
        twin = true;
        return Move::COOPERATE;
    }

    if (M > 2) {
        int idx_prev = 2 * last2 + last1 + 1;
        int idx0 = idx_prev - 1;
        coop[idx0] += (1 - J);
        count[idx0] += 1.0;
    }

    if (J != last1) twin = false;

    int idx_cur = 2 * last2 + last1 + 1;
    int move = 0;

    if (M <= 22) {
        move = (idx_cur == 1 || idx_cur == 2) ? 1 : 0;
    }
    else {
        if (twin) {
            move = 0;
        } else {
            double p[4];
            for (int i = 0; i < 4; ++i) {
                p[i] = (count[i] > 0.0) ? (coop[i] / count[i]) : 0.0;
            }

            const double CONST[6] = {0.0, 4.0, 6.0, 6.0, 8.0, 12.0};
            const double COEFF[6][4] = {
                {36.0,  0.0,  0.0,  0.0},
                {16.0, 12.0, 12.0,  0.0},
                { 0.0, 18.0, 24.0,  0.0},
                {12.0, 12.0,  9.0,  9.0},
                { 0.0, 16.0, 16.0, 12.0},
                { 0.0,  0.0,  0.0, 48.0}
            };

            double best = -1e9;
            int best_policy = 0;
            for (int pol = 0; pol < 6; ++pol) {
                double sum = CONST[pol];
                for (int hist = 0; hist < 4; ++hist) {
                    sum += COEFF[pol][hist] * p[hist];
                }
                if (sum > best) {
                    best = sum;
                    best_policy = pol;
                }
            }

            switch (best_policy) {
                case 0: move = 0; break;
                case 1: move = (idx_cur == 1) ? 1 : 0; break;
                case 2: move = (idx_cur == 1 || idx_cur == 3) ? 1 : 0; break;
                case 3: move = (idx_cur == 1 || idx_cur == 2) ? 1 : 0; break;
                case 4: move = (idx_cur == 1 || idx_cur == 2 || idx_cur == 3) ? 1 : 0; break;
                case 5: move = 1; break;
                default: move = 0;
            }
        }
    }

    last2 = last1;
    last1 = move;
    return (move == 0) ? Move::COOPERATE : Move::DEFECT;
}

inline Move strategy_k35r(uint64_t opp_history, uint64_t /*my_history*/ = 0) {
    static double flack = 0.0;
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_real_distribution<double> dist(0.0, 1.0);

    int rounds = 0;
    uint64_t temp = opp_history;
    while (temp) { ++rounds; temp >>= 1; }
    int M = rounds + 1;

    if (M == 1) {
        flack = 0.0;
        return Move::COOPERATE;
    }

    int J = (opp_history & 1) ? 1 : 0;
    flack = (flack + J) * 0.5;

    double R = dist(rng);
    return (flack > R) ? Move::DEFECT : Move::COOPERATE;
}

inline Move strategy_k36r(uint64_t opp_history, uint64_t /*my_history*/ = 0) {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_real_distribution<double> dist(0.0, 1.0);

    int rounds = 0;
    uint64_t temp = opp_history;
    while (temp) { ++rounds; temp >>= 1; }
    int M = rounds + 1;

    double probc;
    if (M < 100)          probc = 0.1;
    else if (M < 200)     probc = 0.05;
    else if (M < 300)     probc = 0.15;
    else                  probc = 0.0;

    double R = dist(rng);
    return (R < probc) ? Move::COOPERATE : Move::DEFECT;
}

inline Move strategy_k37r(uint64_t opp_history, uint64_t /*my_history*/ = 0) {
    static int nd = 0;

    int rounds = 0;
    uint64_t temp = opp_history;
    while (temp) { ++rounds; temp >>= 1; }
    int M = rounds + 1;

    if (M == 1) {
        nd = 0;
        return Move::COOPERATE;
    }

    int J = (opp_history & 1) ? 1 : 0;
    nd += J;

    return (5 * nd > M) ? Move::DEFECT : Move::COOPERATE;
}

inline Move strategy_k38r(uint64_t opp_history, uint64_t /*my_history*/ = 0) {
    static int move = 0;
    static int jhis = 0;

    int rounds = 0;
    uint64_t temp = opp_history;
    while (temp) { ++rounds; temp >>= 1; }
    int M = rounds + 1;

    if (M == 1) {
        move = 0;
        jhis = 0;
        return Move::COOPERATE;
    }

    int J = (opp_history & 1) ? 1 : 0;

    if (move == 0) {
        jhis = ((jhis & 3) << 1) | J;
        if (jhis == 0b111) {
            move = 1;
        }
    }

    return (move == 1) ? Move::DEFECT : Move::COOPERATE;
}

inline Move strategy_k39r(uint64_t opp_history, uint64_t my_history) {
    static int STEP = 1, SUBSTP = 1;
    static int BOTHD = 0, TITCNT = 0, TATCNT = 0;
    static int EVIL = 0, N = 1, F = 0;
    static int TOTK = 0, OLDMOV = 0, COUNT = 0, VOLDMV = 0;
    static int OK[4] = {0, 0, 0, 0};

    int M = 0;
    uint64_t temp = opp_history;
    while (temp) { ++M; temp >>= 1; }
    ++M;

    if (M == 1) {
        STEP = 1; SUBSTP = 1; BOTHD = 0; TITCNT = 0; TATCNT = 0;
        EVIL = 0; N = 1; F = 0; TOTK = 0; OLDMOV = 0; COUNT = 0; VOLDMV = 0;
        for (int i = 0; i < 4; ++i) OK[i] = 0;
        return Move::COOPERATE;
    }

    int J = (opp_history & 1) ? 1 : 0;
    int prev_own = (my_history & 1) ? 1 : 0;

    int K = 0;
    uint64_t o_hist = opp_history;
    uint64_t m_hist = my_history;
    while (o_hist || m_hist) {
        bool opp = (o_hist & 1);
        bool me  = (m_hist & 1);
        if (!me && !opp)      K += 3;
        else if (!me && opp)  K += 0;
        else if (me && !opp)  K += 5;
        else                  K += 1;
        o_hist >>= 1;
        m_hist >>= 1;
    }

    if (prev_own + J == 2) BOTHD++;
    if (prev_own + J < 2) BOTHD = 0;
    COUNT--;
    int move = 0;
    VOLDMV = OLDMOV;
    OLDMOV = J;
    if (J == 1) TATCNT++;
    if (EVIL == 0 && J == 1) EVIL = 1;

    bool done = false;
    while (!done) {
        switch (STEP) {
            case 1:
                switch (SUBSTP) {
                    case 1:
                        COUNT = 10;
                        TATCNT = 0;
                        TITCNT = 0;
                        SUBSTP = 2;
                        break;
                    case 2:
                        if ((VOLDMV + OLDMOV) == 2) move = 1;
                        TITCNT += move;
                        if (COUNT == 0) SUBSTP = 3;
                        done = true;
                        break;
                    case 3:
                        {
                            int OLDSTP = STEP;
                            OK[STEP] = K - TOTK;
                            TOTK = K;
                            SUBSTP = 1;
                            if (TATCNT > 0) {
                                STEP = 1;
                                for (int i1 = 1; i1 <= 2; ++i1)
                                    for (int i2 = 2; i2 <= 3; ++i2)
                                        if (OK[i1] != 0 && OK[i2] != 0 && OK[i1] >= OK[i2] && STEP == i1)
                                            STEP = i2;
                                if (STEP != 3 && OK[STEP+1] == 0 && (TATCNT >= 4 || TITCNT == 0))
                                    STEP++;
                                if (STEP < OLDSTP && BOTHD > 0) STEP = 5;
                            } else {
                                STEP = 4;
                                if (EVIL == 1) STEP = 1;
                                if (EVIL == 0) EVIL = -1;
                            }
                            continue;
                        }
                }
                break;

            case 2:
                switch (SUBSTP) {
                    case 1:
                        COUNT = 10;
                        TATCNT = 0;
                        TITCNT = 0;
                        SUBSTP = 2;
                        break;
                    case 2:
                        if (OLDMOV == 1) move = 1;
                        TITCNT += move;
                        if (COUNT == 0) SUBSTP = 3;
                        done = true;
                        break;
                    case 3:
                        {
                            int OLDSTP = STEP;
                            OK[STEP] = K - TOTK;
                            TOTK = K;
                            SUBSTP = 1;
                            if (TATCNT > 0) {
                                STEP = 1;
                                for (int i1 = 1; i1 <= 2; ++i1)
                                    for (int i2 = 2; i2 <= 3; ++i2)
                                        if (OK[i1] != 0 && OK[i2] != 0 && OK[i1] >= OK[i2] && STEP == i1)
                                            STEP = i2;
                                if (STEP != 3 && OK[STEP+1] == 0 && (TATCNT >= 4 || TITCNT == 0))
                                    STEP++;
                                if (STEP < OLDSTP && BOTHD > 0) STEP = 5;
                            } else {
                                STEP = 4;
                                if (EVIL == 1) STEP = 1;
                                if (EVIL == 0) EVIL = -1;
                            }
                            continue;
                        }
                }
                break;

            case 3:
                switch (SUBSTP) {
                    case 1:
                        COUNT = 10;
                        TATCNT = 0;
                        TITCNT = 0;
                        SUBSTP = 2;
                        break;
                    case 2:
                        move = 1;
                        TITCNT++;
                        if (COUNT == 0) SUBSTP = 3;
                        done = true;
                        break;
                    case 3:
                        {
                            int OLDSTP = STEP;
                            OK[STEP] = K - TOTK;
                            TOTK = K;
                            SUBSTP = 1;
                            if (TATCNT > 0) {
                                STEP = 1;
                                for (int i1 = 1; i1 <= 2; ++i1)
                                    for (int i2 = 2; i2 <= 3; ++i2)
                                        if (OK[i1] != 0 && OK[i2] != 0 && OK[i1] >= OK[i2] && STEP == i1)
                                            STEP = i2;
                                if (STEP != 3 && OK[STEP+1] == 0 && (TATCNT >= 4 || TITCNT == 0))
                                    STEP++;
                                if (STEP < OLDSTP && BOTHD > 0) STEP = 5;
                            } else {
                                STEP = 4;
                                if (EVIL == 1) STEP = 1;
                                if (EVIL == 0) EVIL = -1;
                            }
                            continue;
                        }
                }
                break;

            case 4:
                switch (SUBSTP) {
                    case 1:
                        SUBSTP = 2;
                        move = 1;
                        COUNT = N;
                        TATCNT = 0;
                        done = true;
                        break;
                    case 2:
                        if (COUNT == 0) SUBSTP = 3;
                        done = true;
                        break;
                    case 3:
                        if (TATCNT != 0) {
                            if (F == 0) {
                                SUBSTP = 4;
                                if (J == 1) N++;
                                TATCNT = J;
                                done = true;
                            } else {
                                N++;
                                SUBSTP = 1;
                                STEP = 1;
                                continue;
                            }
                        } else {
                            F = 1;
                            SUBSTP = 1;
                            continue;
                        }
                        break;
                    case 4:
                        if (TATCNT <= 4) {
                            done = true;
                        } else {
                            SUBSTP = 1;
                            STEP = 1;
                            continue;
                        }
                        break;
                }
                break;

            case 5:
                switch (SUBSTP) {
                    case 1:
                        COUNT = 5;
                        SUBSTP = 2;
                    case 2:
                        if (COUNT != 0) {
                            done = true;
                        } else {
                            SUBSTP = 1;
                            STEP = 1;
                            continue;
                        }
                        break;
                }
                break;
        }
    }

    return (move == 1) ? Move::DEFECT : Move::COOPERATE;
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
        case StrategyType::Tullock: return strategy_tullock(opp_history, my_history);
        case StrategyType::GRASR: return strategy_grasr(opp_history, my_history);
        case StrategyType::K31R: return strategy_k31r(opp_history, my_history);
        case StrategyType::K32R: return strategy_k32r(opp_history, my_history);
        case StrategyType::K33R: return strategy_k33r(opp_history, my_history);
        case StrategyType::K35R: return strategy_k35r(opp_history, my_history);
        case StrategyType::K36R: return strategy_k36r(opp_history, my_history);
        case StrategyType::K37R: return strategy_k37r(opp_history, my_history);
        case StrategyType::K38R: return strategy_k38r(opp_history, my_history);
        case StrategyType::K39R: return strategy_k39r(opp_history, my_history);
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
        case StrategyType::Tullock: return "Tullock";
        case StrategyType::GRASR: return "GRASR";
        case StrategyType::K31R: return "K31R";
        case StrategyType::K32R: return "K32R";
        case StrategyType::K33R: return "K33R";
        case StrategyType::K35R: return "K35R";
        case StrategyType::K36R: return "K36R";
        case StrategyType::K37R: return "K37R";
        case StrategyType::K38R: return "K38R";
        case StrategyType::K39R: return "K39R";
        default:                        return "?";
    }
}

#endif

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
