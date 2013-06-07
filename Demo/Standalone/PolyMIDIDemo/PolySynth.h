//
//  PolySynth.cpp
//  PolyMIDIDemo
//
//  Created by Michael Dewberry on 6/7/13.
//
//

#include "Tonic.h"

using namespace Tonic;

class PolySynth : public Synth
{
public:
	typedef Synth (VoiceCreateFn)();
	
	PolySynth();	
	void addVoice(Synth voice);
	void addVoices(VoiceCreateFn createFn, int count);
	void processMidiNote(int note, int velocity);

	class VoiceData
	{
	public:
		bool playing;
		int currentNote;
		Synth synth;
		int voiceNumber;
	};

protected:
	VoiceData* getVoiceForNote(int midiNote, bool noteOn = true);

	Mixer mixer;
	vector<VoiceData> voiceData;
	list<int> unusedVoices;
};

