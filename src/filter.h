#ifndef FILTER_H
#define FILTER_H

#include "complex.h"

class Filter
{
    unsigned int _p = 3;
    complex _x[6], _y[6];
    unsigned int p(unsigned int i) const noexcept;
    const complex &x(unsigned int i) const noexcept;
    const complex &y(unsigned int i) const noexcept;

public:
    explicit Filter();
    complex operator()(complex v);

};

#endif // FILTER_H
