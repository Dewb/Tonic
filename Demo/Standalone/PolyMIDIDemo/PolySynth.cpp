//
//  PolySynth.cpp
//  PolyMIDIDemo
//
//  Created by Michael Dewberry on 6/7/13.
//
//

#include "PolySynth.h"

void BasicPolyphonicAllocator::noteOn(int note, int velocity)
{
    int voiceNumber = getNextVoice(note);

    if (voiceNumber < 0)
        return; // no voice available

    cout << ">> " << "Starting note " << note << " on voice " << voiceNumber << "\n";

    PolyVoice& voice = voiceData[voiceNumber];

    voice.synth.setParameter("polyNote", note);
    voice.synth.setParameter("polyGate", 1.0);
    voice.synth.setParameter("polyVelocity", velocity);
    voice.synth.setParameter("polyVoiceNumber", voiceNumber);

    voice.playing = true;
    voice.currentNote = note;

    activeVoiceQueue.remove(voiceNumber);
    activeVoiceQueue.push_back(voiceNumber);
    inactiveVoiceQueue.remove(voiceNumber);
}

void BasicPolyphonicAllocator::noteOff(int note)
{
    // clear the oldest active voice with this note number
    for (auto iter = activeVoiceQueue.begin(), end = activeVoiceQueue.end(); iter != end; iter++)
    {
        PolyVoice& voice = voiceData[*iter];
        if (voice.currentNote == note && voice.playing)
        {
            cout << ">> " << "Stopping note " << note << " on voice " << voice.voiceNumber << "\n";

            voice.synth.setParameter("polyGate", 0.0);
            voice.playing = false;

            activeVoiceQueue.remove(voice.voiceNumber);
            inactiveVoiceQueue.remove(voice.voiceNumber);
            inactiveVoiceQueue.push_back(voice.voiceNumber);

            break;
        }
    }
}

int BasicPolyphonicAllocator::getNextVoice(int note)
{
    // Find a voice not playing any note
    if (inactiveVoiceQueue.size())
    {
        return inactiveVoiceQueue.front();
    }

    return -1;
}

int OldestNoteStealingPolyphonicAllocator::getNextVoice(int note)
{
    int voice = BasicPolyphonicAllocator::getNextVoice(note);
    if (voice >= 0)
        return voice;

    if (activeVoiceQueue.size())
    {
        return activeVoiceQueue.front();
    }

    return -1;
}

int LowestNoteStealingPolyphonicAllocator::getNextVoice(int note)
{
    int voice = BasicPolyphonicAllocator::getNextVoice(note);
    if (voice >= 0)
        return voice;

    // Find the playing voice with the lowest note that's lower than the requested note
    int lowestNote = note;
    int lowestVoice = -1;
    for (auto& voice : voiceData)
    {
        if (voice.currentNote < lowestNote)
        {
            lowestNote = voice.currentNote;
            lowestVoice = voice.voiceNumber;
        }
    }

    return lowestVoice;
}