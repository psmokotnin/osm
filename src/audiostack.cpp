/**
 *  OSM
 *  Copyright (C) 2018  Pavel Smokotnin

 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
    std::lock_guard<std::mutex> lock(memoryMutex);
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

void AudioStack::reset()
{
    this->pointer = this->firstdata;
}
bool AudioStack::next()
{
    if (this->pointer && this->pointer->next)
    {
        this->pointer = this->pointer->next;
        return true;
    }
    return false;
}
bool AudioStack::isNext()
{
    return (this->pointer && this->pointer->next);
}
float AudioStack::current()
{
    if (this->pointer != nullptr)
        return this->pointer->value;
    return 0.0;
}
float AudioStack::shift()
{
    std::lock_guard<std::mutex> lock(memoryMutex);
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

float AudioStack::first()
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
    std::lock_guard<std::mutex> lock(memoryMutex);
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
