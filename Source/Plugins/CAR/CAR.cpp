
/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include "CAR.h"


CAR::CAR()
    : GenericProcessor ("Common Avg Ref") //, threshold(200.0), state(true)
{
    parameters.add (Parameter ("Gain (%)", 0.0, 100.0, 100.0, 0));

    m_avgBuffer = AudioSampleBuffer (1, 10000); // 1-dimensional buffer to hold the avg
}


CAR::~CAR()
{
}


void CAR::setParameter (int parameterIndex, float newValue)
{
    editor->updateParameterButtons (parameterIndex);
    // std::cout << "Setting CAR Gain" << std::endl;

    if (currentChannel >= 0)
    {
        Parameter& p =  parameters.getReference (parameterIndex);
        p.setValue (newValue, currentChannel);
    }
}


void CAR::process (AudioSampleBuffer& buffer, MidiBuffer& events)
{
    const int numSamples            = buffer.getNumSamples();
    const int numReferenceChannels  = m_referenceChannels.size();
    const int numAffectedChannels   = m_affectedChannels.size();

    // There are no sense to do any processing if either number of reference or affected channels is zero.
    if (! numReferenceChannels
        || ! numAffectedChannels)
    {
        return;
    }

    m_avgBuffer.clear();

    for (int i = 0; i < numReferenceChannels; ++i)
    {
        m_avgBuffer.addFrom (0,                         // destChannel
                             0,                         // destStartSample
                             buffer,                    // source
                             m_referenceChannels[i],    // sourceChannel
                             0,                         // sourceStartSample
                             numSamples,                // numSamples
                             1.0f);                     // gain to apply
    }

    m_avgBuffer.applyGain (1.0f / float (numReferenceChannels));


    // just use channel 0, since we can't have individual channel settings at the moment
    const float gain = -1.0f * float (getParameterVar (0, 0)) / 100.0f;

    for (int i = 0; i < numAffectedChannels; ++i)
    {
        buffer.addFrom (m_affectedChannels[i],  // destChannel
                        0,                      // destStartSample
                        m_avgBuffer,            // source
                        0,                      // sourceChannel
                        0,                      // sourceStartSample
                        numSamples,             // numSamples
                        gain);                  // gain to apply
    }
}


void CAR::setReferenceChannels (const Array<int> newReferenceChannels)
{
    const ScopedLock myScopedLock (objectLock);

    m_referenceChannels = Array (newReferenceChannels);
}


void CAR::setAffectedChannels (const Array<int> newAffectedChannels)
{
    const ScopedLock myScopedLock (objectLock);

    m_affectedChannels = Array (newAffectedChannels);
}
