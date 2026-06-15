#ifndef ISTRATEGY_HPP
#define ISTRATEGY_HPP

#include <cstdint>
#include "move.hpp"

class IStrategy {
public:
    virtual ~IStrategy() = default;
    virtual void reset() = 0;
    virtual Move getMove(uint64_t opp_history, uint64_t my_history) = 0;
};

#endif
