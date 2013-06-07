//
//  PolySynth.cpp
//  PolyMIDIDemo
//
//  Created by Michael Dewberry on 6/7/13.
//
//

#include "PolySynth.h"

PolySynth::PolySynth()
: Synth()
{
	setOutputGen(mixer);
}

void PolySynth::addVoice(VoiceCreateFn createFn)
{
	VoiceData v;
	v.frequency.input(ControlValue(0));
	v.voiceNumber = voiceData.size();
	v.synth = createFn(v.frequency, v.trigger);
	v.playing = false;
	v.currentNote = 0;

	voiceData.push_back(v);
	unusedVoices.push_back(v.voiceNumber);
	mixer.addInput(v.synth);
}

void PolySynth::addVoices(VoiceCreateFn createFn, int count)
{
	for (int i = 0; i < count; i++)
		addVoice(createFn);
}

void PolySynth::processMidiNote(int note, int velocity)
{
	bool noteOn = velocity > 0;

	VoiceData* pVoice = getVoiceForNote(note, noteOn);
	if (!pVoice)
		return;
	
	cout << ">> " << (noteOn ? "Starting" : "Stopping") << " note " << note << " on voice " << pVoice->voiceNumber << "\n";
	pVoice->frequency.input(ControlValue(note));

	pVoice->playing = noteOn;
	pVoice->trigger.trigger(noteOn ? 1.0 : 0.0);
	pVoice->currentNote = note;

	if (noteOn)
		unusedVoices.remove(pVoice->voiceNumber);
	else
		unusedVoices.push_back(pVoice->voiceNumber);

}

PolySynth::VoiceData* PolySynth::getVoiceForNote(int note, bool noteOn)
{
	if (!noteOn) 
	{
		// Find the voice already playing this note
		for (auto& voice : voiceData)
			if (voice.currentNote == note && voice.playing)
				return &voice;

		return nullptr;
	}

	// Find a voice not playing any note
	if (unusedVoices.size())
	{
		int num = unusedVoices.front();
		unusedVoices.pop_front();
		return &(voiceData[num]);
	}

	// Find the lowest playing note
	int lowestNote = INT_MAX;
	for (auto& voice : voiceData)
		if (voice.currentNote < lowestNote)
			lowestNote = voice.currentNote;
	for (auto& voice : voiceData)
		if (voice.currentNote == lowestNote)
			return &voice;

	return nullptr;
}