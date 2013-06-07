//
//  main.cpp
//  PolyMIDIDemo
//
//  Created by Michael Dewberry on 6/7/13.
//
//


#include <iostream>
#include "Tonic.h"
#include "RtAudio.h"
#include "RtMidi.h"
#include "RtError.h"
#include "PolySynth.h"

using namespace Tonic;

const unsigned int nChannels = 2;

static PolySynth poly;

int renderCallback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
        double streamTime, RtAudioStreamStatus status, void *userData )
{
    poly.fillBufferOfFloats((float*)outputBuffer, nBufferFrames, nChannels);
    return 0;
}

void midiCallback(double deltatime, vector<unsigned char>* msg, void* userData)
{
	int chan = (*msg)[0] & 0xf;
	int msgtype = (*msg)[0] & 0xf0;
	int b1 =  (*msg)[1];
	int b2 =  (*msg)[2];
	static int lastPat = 0;
	if (msgtype == 0x80) {
		std::cout << "MIDI Note OFF  N: " << b1 << std::endl;
		poly.processMidiNote(b1, 0);
	}
	else if (msgtype == 0x90) {
		std::cout << "MIDI Note ON   N: " << b1 << " V: " << b2 << std::endl;
		poly.processMidiNote(b1, b2);
	}
}

Synth createSynthVoice(ControlGenerator voiceFreq, ControlTrigger voiceTrigger)
{
	Synth newSynth;

	Generator tone = SquareWave().freq(voiceFreq) * SineWave().freq(50);
    
    ADSR env = ADSR()
    .attack(0.04)
    .decay( 0.1 )
    .sustain(0.8)
    .release(0.6)
    .doesSustain(true)
    .trigger(voiceTrigger);
   
    ControlGenerator filterFreq = voiceFreq * 0.25 + 200;
    
    LPF24 filter = LPF24().Q(2).cutoff( filterFreq );

	Generator output = (( tone * env ) >> filter) * 0.2;

	newSynth.setOutputGen(output);

	return newSynth;
}



int main(int argc, const char * argv[])
{
    // Configure RtAudio
    RtAudio dac;
    RtAudio::StreamParameters rtParams;
    rtParams.deviceId = dac.getDefaultOutputDevice();
    rtParams.nChannels = nChannels;
    unsigned int sampleRate = 44100;
    unsigned int bufferFrames = 512; // 512 sample frames
    
	RtMidiIn *midiIn = new RtMidiIn();

    // You don't necessarily have to do this - it will default to 44100 if not set.
    Tonic::setSampleRate(sampleRate);
    
	poly.addVoices(createSynthVoice, 8);
 
    // open rtaudio stream and rtmidi port
    try {
		if (midiIn->getPortCount() == 0) {
			std::cout << "No MIDI ports available!\n";
			cin.get();
			exit(0);
		}
		midiIn->openPort(0);
		midiIn->setCallback( &midiCallback );

        dac.openStream( &rtParams, NULL, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, &renderCallback, NULL, NULL );
        dac.startStream();
        
		// hacky, yes, but let's just hang out for awhile until someone presses a key
		printf("\n\nPress any key to stop\n\n");
		cin.get();
        dac.stopStream();
    }
    catch ( RtError& e ) {
        std::cout << '\n' << e.getMessage() << '\n' << std::endl;
		cin.get();
        exit( 0 );
    }
    
    return 0;
}

