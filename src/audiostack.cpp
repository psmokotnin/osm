#include "audiostack.h"

AudioStack::AudioStack(unsigned long size)
{
    this->_size = 0;
    this->sizeLimit = size;
    this->lastdata  = nullptr;
    this->firstdata = nullptr;
    this->pointer   = nullptr;
    this->add(0.0);
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

    while (this->_size > this->sizeLimit) //drop first
    {
        Cell * first = this->firstdata;
        if (this->pointer == this->firstdata)
            this->pointer = first->next;

        this->firstdata = first->next;
        delete (first);
        first = nullptr;
        this->_size --;
    }
}
bool AudioStack::halfAdd(float data)
{
    if (halfAdded) {
        add((halfData + data) / 2.0);
        halfAdded = false;
    } else {
        halfData = data;
        halfAdded = true;
    }
    return !halfAdded;
}
void AudioStack::reset(void)
{
    this->pointer = this->firstdata;
}
bool AudioStack::next(void)
{
    if (this->pointer->next)
    {
        this->pointer = this->pointer->next;
        return true;
    }
    return false;
}
float AudioStack::current(void)
{
    return this->pointer->value;
}
float AudioStack::first(void)
{
    return this->firstdata->value;
}
