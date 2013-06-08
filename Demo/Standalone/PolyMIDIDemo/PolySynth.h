//
//  PolySynth.cpp
//  PolyMIDIDemo
//
//  Created by Michael Dewberry on 6/7/13.
//
//

#include "Tonic.h"

using namespace Tonic;

template<typename VoiceAllocator = LowestNoteStealingPolyphonicAllocator>
class PolySynth : public Synth
{
public:
    PolySynth() : Synth() 
    {
        setOutputGen(mixer); 
    }

    void addVoice(Synth synth)
    {
        allocator.addVoice(synth);
        mixer.addInput(synth);
    }

    typedef Synth (VoiceCreateFn)();
    void addVoices(VoiceCreateFn createFn, int count)
    {
        for (int i = 0; i < count; i++)
            addVoice(createFn());
    }

    void noteOn(int note, int velocity)
    {
        allocator.noteOn(note, velocity);
    }

    void noteOff(int note)
    {
        allocator.noteOff(note);
    }

protected:
    Mixer mixer;
    VoiceAllocator allocator;
};

class BasicPolyphonicAllocator
{
public:
    class PolyVoice
    {
    public:
        bool playing;
        int currentNote;
        Synth synth;
        int voiceNumber;
    };

    void addVoice(Synth synth)
    {
        PolyVoice v;
        v.voiceNumber = voiceData.size();
        v.synth = synth;
        v.playing = false;
        v.currentNote = 0;

        voiceData.push_back(v);
        inactiveVoiceQueue.push_back(v.voiceNumber);
    }

    void noteOn(int noteNumber, int velocity);
    void noteOff(int noteNumber);

protected:
    virtual int getNextVoice(int note);
    vector<PolyVoice> voiceData;
    list<int> inactiveVoiceQueue;
    list<int> activeVoiceQueue;
};

class OldestNoteStealingPolyphonicAllocator : public BasicPolyphonicAllocator
{
protected:
    virtual int getNextVoice(int note);
};

class LowestNoteStealingPolyphonicAllocator : public BasicPolyphonicAllocator
{
protected:
    virtual int getNextVoice(int note);
};


