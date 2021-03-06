/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_ELEMENTCOMPARATOR_JUCEHEADER__
#define __JUCE_ELEMENTCOMPARATOR_JUCEHEADER__


//==============================================================================
/**
    Sorts a range of elements in an array.

    The comparator object that is passed-in must define a public method with the following
    signature:
    @code
    int compareElements (ElementType first, ElementType second);
    @endcode

    ..and this method must return:
      - a value of < 0 if the first comes before the second
      - a value of 0 if the two objects are equivalent
      - a value of > 0 if the second comes before the first

    To improve performance, the compareElements() method can be declared as static or const.

    @param comparator       an object which defines a compareElements() method
    @param array            the array to sort
    @param firstElement     the index of the first element of the range to be sorted
    @param lastElement      the index of the last element in the range that needs
                            sorting (this is inclusive)
    @param retainOrderOfEquivalentItems     if true, the order of items that the
                            comparator deems the same will be maintained - this will be
                            a slower algorithm than if they are allowed to be moved around.

    @see sortArrayRetainingOrder
*/
template <class ElementType, class ElementComparator>
static void sortArray (ElementComparator& comparator,
                       ElementType* const array,
                       int firstElement,
                       int lastElement,
                       const bool retainOrderOfEquivalentItems)
{
    (void) comparator;  // if you pass in an object with a static compareElements() method, this
                        // avoids getting warning messages about the parameter being unused

    if (lastElement > firstElement) {
        if (retainOrderOfEquivalentItems) {
            for (int i = firstElement; i < lastElement; ++i) {
                if (comparator.compareElements (array[i], array [i + 1]) > 0) {
                    swapVariables(array[i], array[i + 1]);

                    if (i > firstElement)
                        i -= 2;
                }
            }
        }
        else {
            int fromStack[30], toStack[30];
            int stackIndex = 0;

            for ( ; ; ) {
                const int size = (lastElement - firstElement) + 1;

                if (size <= 8) {
                    int j = lastElement;
                    int maxIndex;

                    while (j > firstElement) {
                        maxIndex = firstElement;
                        for (int k = firstElement + 1; k <= j; ++k) {
                            if (comparator.compareElements (array[k], array [maxIndex]) > 0) {
                                maxIndex = k;
                            }
                        }

                        swapVariables(array[j], array[maxIndex]);
                        --j;
                    }
                }
                else
                {
                    const int mid = firstElement + (size >> 1);
                    swapVariables(array[mid], array[firstElement]);

                    int i = firstElement;
                    int j = lastElement + 1;

                    for (;;)
                    {
                        while (++i <= lastElement
                                && comparator.compareElements (array[i], array [firstElement]) <= 0)
                        {}

                        while (--j > firstElement
                                && comparator.compareElements (array[j], array [firstElement]) >= 0)
                        {}

                        if (j < i)
                            break;

                        swapVariables(array[i], array[j]);
                    }

                    swapVariables(array[j], array[firstElement]);

                    if (j - 1 - firstElement >= lastElement - i)
                    {
                        if (firstElement + 1 < j)
                        {
                            fromStack [stackIndex] = firstElement;
                            toStack [stackIndex] = j - 1;
                            ++stackIndex;
                        }

                        if (i < lastElement)
                        {
                            firstElement = i;
                            continue;
                        }
                    }
                    else
                    {
                        if (i < lastElement)
                        {
                            fromStack [stackIndex] = i;
                            toStack [stackIndex] = lastElement;
                            ++stackIndex;
                        }

                        if (firstElement + 1 < j)
                        {
                            lastElement = j - 1;
                            continue;
                        }
                    }
                }

                if (--stackIndex < 0)
                    break;

                firstElement = fromStack [stackIndex];
                lastElement = toStack [stackIndex];
            }
        }
    }
}


//==============================================================================
/**
    Searches a sorted array of elements, looking for the index at which a specified value
    should be inserted for it to be in the correct order.

    The comparator object that is passed-in must define a public method with the following
    signature:
    @code
    int compareElements (ElementType first, ElementType second);
    @endcode

    ..and this method must return:
      - a value of < 0 if the first comes before the second
      - a value of 0 if the two objects are equivalent
      - a value of > 0 if the second comes before the first

    To improve performance, the compareElements() method can be declared as static or const.

    @param comparator       an object which defines a compareElements() method
    @param array            the array to search
    @param newElement       the value that is going to be inserted
    @param firstElement     the index of the first element to search
    @param lastElement      the index of the last element in the range (this is non-inclusive)
*/
template <class ElementType, class ElementComparator>
static int findInsertIndexInSortedArray (ElementComparator& comparator,
                                         ElementType* const array,
                                         const ElementType newElement,
                                         int firstElement,
                                         int lastElement)
{
    jassert (firstElement <= lastElement);

    (void) comparator;  // if you pass in an object with a static compareElements() method, this
                        // avoids getting warning messages about the parameter being unused

    while (firstElement < lastElement)
    {
        if (comparator.compareElements (newElement, array [firstElement]) == 0)
        {
            ++firstElement;
            break;
        }
        else
        {
            const int halfway = (firstElement + lastElement) >> 1;

            if (halfway == firstElement)
            {
                if (comparator.compareElements (newElement, array [halfway]) >= 0)
                    ++firstElement;

                break;
            }
            else if (comparator.compareElements (newElement, array [halfway]) >= 0)
            {
                firstElement = halfway;
            }
            else
            {
                lastElement = halfway;
            }
        }
    }

    return firstElement;
}

//==============================================================================
/**
    A simple ElementComparator class that can be used to sort an array of
    objects that support the '<' operator.

    This will work for primitive types and objects that implement operator<().

    Example: @code
    Array <int> myArray;
    DefaultElementComparator<int> sorter;
    myArray.sort (sorter);
    @endcode

    @see ElementComparator
*/
template <class ElementType>
class DefaultElementComparator
{
private:
    typedef PARAMETER_TYPE (ElementType) ParameterType;

public:
    static int compareElements (ParameterType first, ParameterType second)
    {
        return (first < second) ? -1 : ((second < first) ? 1 : 0);
    }
};


#endif   // __JUCE_ELEMENTCOMPARATOR_JUCEHEADER__
