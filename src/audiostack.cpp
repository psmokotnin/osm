#include "audiostack.h"
#include <cmath>
AudioStack::AudioStack(unsigned long size)
{
    this->_size = 0;
    this->sizeLimit = size;
    this->lastdata  = nullptr;
    this->firstdata = nullptr;
    this->pointer   = nullptr;
    this->add(0.0);
    reset();
}
AudioStack::AudioStack(AudioStack *original)
{
    this->_size = original->size();
    this->sizeLimit = original->sizeLimit;
    this->lastdata  = original->lastdata;
    this->firstdata = original->firstdata;
    this->pointer   = original->pointer;
    reset();
}
void AudioStack::setSize(unsigned long size)
{
    this->sizeLimit = size;
}
unsigned long AudioStack::size()
{
    return this->_size;
}
void AudioStack::fill(float value)
{
    for (unsigned long i = 0; i < this->sizeLimit; i++)
    {
        this->add(value);
    }
}
void AudioStack::add(float data)
{
    Cell * newData = new Cell;
    newData->value = data;
    newData->next  = nullptr;
    newData->pre   = nullptr;
    this->_size ++;

    if (this->lastdata)
    {
        this->lastdata->next = newData;
        newData->pre = this->lastdata;
        this->lastdata = newData;
    }
    else
        this->lastdata = newData;

    if (!this->firstdata)
        this->firstdata = newData;

    while (this->_size > this->sizeLimit) {
        dropFirst();
    }
}
void AudioStack::dropFirst()
{
    Cell * first = firstdata;
    if (pointer == firstdata)
        pointer = first->next;

    firstdata = first->next;
    delete (first);
    first = nullptr;
    _size --;
}

void AudioStack::reset(void)
{
    this->pointer = this->firstdata;
}
bool AudioStack::next(void)
{
    if (this->pointer && this->pointer->next)
    {
        this->pointer = this->pointer->next;
        return true;
    }
    return false;
}
bool AudioStack::isNext(void)
{
    return (this->pointer && this->pointer->next);
}
float AudioStack::current(void)
{
    if (this->pointer != nullptr)
        return this->pointer->value;
    return 0.0;
}
float AudioStack::shift(void)
{
    if (this->firstdata && _size > 1) {
        Cell * first = this->firstdata;
        float value = first->value;
        if (this->pointer == this->firstdata)
            this->pointer = first->next;

        this->firstdata = first->next;
        delete (first);
        first = nullptr;
        this->_size --;
        return value;
    }
    return 0.0;
}

float AudioStack::first(void)
{
    if (this->firstdata)
        return this->firstdata->value;
    return 0.0;
}
/*
 * Move pointer by delta points
 * delta < 0 => move pointer left
 * delta > 0 => move pointer right
 */
void AudioStack::rewind(long delta)
{
    bool direction = std::signbit(delta);
    for (long i = 0; i < std::abs(delta); i++) {
        if (direction && pointer->pre)
            pointer = pointer->pre;
        else if (!direction && pointer->next)
            pointer = pointer->next;
        else {
            Cell * newData = new Cell;
            newData->value = 0.0;
            newData->next  = (direction ? pointer : nullptr);
            newData->pre   = (direction ? nullptr : pointer);
            _size ++;
            pointer = newData;

            if (direction)
                firstdata = newData;
            else
                lastdata = newData;
        }
    }
}
