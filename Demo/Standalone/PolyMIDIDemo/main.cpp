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

static Synth synth;
static PolySynth poly;

int renderCallback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
        double streamTime, RtAudioStreamStatus status, void *userData )
{
    synth.fillBufferOfFloats((float*)outputBuffer, nBufferFrames, nChannels);
    return 0;
}

void midiCallback(double deltatime, vector<unsigned char>* msg, void* userData)
{
    int chan = (*msg)[0] & 0xf;
    int msgtype = (*msg)[0] & 0xf0;
    int b1 =  (*msg)[1];
    int b2 =  (*msg)[2];

    if (msgtype == 0x80 || (msgtype == 0x90 && b2 == 0)) {
        std::cout << "MIDI Note OFF  C: " << chan << " N: " << b1 << std::endl;
        poly.noteOff(b1);
    }
    else if (msgtype == 0x90) {
        std::cout << "MIDI Note ON   C: " << chan << " N: " << b1 << " V: " << b2 << std::endl;
        poly.noteOn(b1, b2);
    }
}

Synth createSynthVoice()
{
    Synth newSynth;

    ControlParameter noteNum = newSynth.addParameter("polyNote", 0.0);
    ControlParameter gate = newSynth.addParameter("polyGate", 0.0);
    ControlParameter noteVelocity = newSynth.addParameter("polyVelocity", 0.0);
    ControlParameter voiceNumber = newSynth.addParameter("polyVoiceNumber", 0.0);

    ControlGenerator voiceFreq = ControlMidiToFreq().input(noteNum) + voiceNumber * 1.2; // detune the voices slightly

    Generator tone = SquareWave().freq(voiceFreq) * SineWave().freq(50);
    
    ADSR env = ADSR()
        .attack(0.04)
        .decay( 0.1 )
        .sustain(0.8)
        .release(0.6)
        .doesSustain(true)
        .trigger(gate);
   
    ControlGenerator filterFreq = voiceFreq * 0.5 + 200;
    
    LPF24 filter = LPF24().Q(1.0 + noteVelocity * 0.02).cutoff( filterFreq );

    Generator output = (( tone * env ) >> filter) * (0.02 + noteVelocity * 0.005);

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
    
    StereoDelay delay = StereoDelay(3.0f,3.0f)
        .delayTimeLeft( 0.25 + SineWave().freq(0.2) * 0.01)
        .delayTimeRight(0.30 + SineWave().freq(0.23) * 0.01)
        .feedback(0.4)
        .dryLevel(0.8)
        .wetLevel(0.2);

    synth.setOutputGen(poly >> delay);

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
        
        printf("\n\nPress Enter to stop\n\n");
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

