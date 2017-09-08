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
void AudioStack::fill(qreal value)
{
    for (unsigned long i = 0; i < this->sizeLimit; i++)
    {
        this->add(value);
    }
}
void AudioStack::add(qreal data)
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

    if (subStack != nullptr)
        subStack->partAdd(data);
}
bool AudioStack::partAdd(qreal data)
{
    parts.append(data);

    if (parts.count() == subParts) {
        qreal newData = 0.0;
        for (int i = 0; i < parts.count(); i++) {
            newData += parts[i];
        }
        newData /= parts.count();
        add(newData);
        parts.clear();
        return true;
    }
    return false;
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
qreal AudioStack::current(void)
{
    if (this->pointer != nullptr)
        return this->pointer->value;
    return 0.0;
}
qreal AudioStack::shift(void)
{
    if (this->firstdata && _size > 1) {
        Cell * first = this->firstdata;
        qreal value = first->value;
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

qreal AudioStack::first(void)
{
    if (this->firstdata)
        return this->firstdata->value;
    return 0.0;
}
