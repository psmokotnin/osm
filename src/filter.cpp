#include "filter.h"

Filter::Filter()
{
}
unsigned int Filter::p(unsigned int i) const noexcept
{
    unsigned int p = _p + i;
    if (p > 5) p -= 6;
    return p;
}
const complex &Filter::x(unsigned int i) const noexcept
{
    return _x[p(i)];
}
const complex &Filter::y(unsigned int i) const noexcept
{
    return _y[p(i)];
}
/**
 * @url http://www-users.cs.york.ac.uk/~fisher/cgi-bin/mkfscript
 */
complex Filter::operator()(complex v)
{
    _p = p(1);

    _x[p(5)] = v / static_cast<float>(1.327313202e+05);
    _y[p(5)] =
           ( x(0) * 1.0)
         + ( x(1) * 5.0)
         + ( x(2) * 10.0)
         + ( x(3) * 10.0)
         + ( x(4) * 5.0)
         + ( x(5) * 1.0)

         + ( y(0) * static_cast<float>( 0.4600089841 ))
         + ( y(1) * static_cast<float>(-2.6653917847 ))
         + ( y(2) * static_cast<float>( 6.2006547950 ))
         + ( y(3) * static_cast<float>(-7.2408808951 ))
         + ( y(4) * static_cast<float>( 4.2453678122 ));

    return y(5);
}
