#include <stdlib.h>

#include "math.h"

void stoogeSort(qreal *array, int left, int right)
{
    if (array[right] < array[left])
        std::swap(array[left], array[right]);

    if (right <= left + 1)
        return;

    int k = (int)((right - left + 1) / 3);
    stoogeSort(array, left,     right - k);
    stoogeSort(array, left + k, right    );
    stoogeSort(array, left,     right - k);
}
qreal median(qreal *data, int count)
{
    int lower, bigger;
    for (int i = 0; i < count; i++) {
        lower = bigger = 0;

        for (int j = 0; j < count; j++) {
            if (i == j) continue;

            if (data[i] >= data[j]) bigger++;
            if (data[i] <= data[j]) lower++;
        }
        if (abs(bigger - lower) <= 1)
            return data[i];
    }
    return data[0];
}
