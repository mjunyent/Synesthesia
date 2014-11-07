#ifndef NO_FMOD

#include "SoundHandler.h"

using namespace std;

FMOD::System     *soundsystem; // no me hace ni puta gracia definirlo aqui, pero si no el compilador mete unas petadas ...
FMOD_RESULT debugme;

void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
		TOBAGO::log.write(ERROR) << "FMOD Error!" << result << FMOD_ErrorString(result);
		exit(EXIT_FAILURE);
    }
}

void initSound(){
	FMOD::System_Create(&soundsystem);
	soundsystem->init(32, FMOD_INIT_NORMAL, 0);
}

SoundHandler::SoundHandler(const char* SongFile, unsigned FFTLEN){
	FFT = (float*)malloc(FFTLEN*sizeof(float) );
	WAVE = (float*)malloc(FFTLEN*sizeof(float) );
	len = FFTLEN;
	thechannel = 0;
	soundsystem->createSound(SongFile, FMOD_DEFAULT, 0, &theSound);
	theSound->setMode( FMOD_LOOP_NORMAL );
	playVel = 1;
	//ERRCHECK(debugme);
}

void SoundHandler::Play(){
	soundsystem->playSound(FMOD_CHANNEL_FREE,theSound,0,&thechannel);

	thechannel->getFrequency(&playFreq);
	setVel(playVel);
	//ERRCHECK(debugme);
}

void SoundHandler::Pause(){
	bool isPaused;
	thechannel->getPaused(&isPaused);
	thechannel->setPaused(!isPaused);
}

void SoundHandler::Stop() {
	thechannel->stop();
}

void SoundHandler::getSpectrum(){
	thechannel->getSpectrum( &FFT[0], len, 0, FMOD_DSP_FFT_WINDOW_TRIANGLE );
	soundsystem->update();
	//ERRCHECK(debugme);
}

void SoundHandler::getWave() {
	thechannel->getWaveData( &WAVE[0], len, 0);
}

double SoundHandler::SoundTime(){
	unsigned ret;
	thechannel->getPosition( &ret, FMOD_TIMEUNIT_MS );
	return double(ret)/1000.0f;
}

/*
void SoundHandler::PlotWave(){
	float offs = 2 / float(len) ; //Avance de x
	float x = -1.00f;
	glBegin( GL_LINES );
	glLineWidth( 8.5f );
	glColor4f(0.00f,1.00f,0.00f,1.00f);
	for(unsigned i = 0; i < len; ++i){
		glVertex2f(x, FFT[i]);
		glVertex2f(x+offs, FFT[i]);
		x += offs;
	}
	glEnd();
}*/

void SoundHandler::setVel(float v) {
	playVel = v;
	thechannel->setFrequency(v*playFreq);
}

double SoundHandler::getEnergy() {
	getWave();
	double sum = 0.0;
	for(unsigned int i=0; i<len; i++) {
		sum += WAVE[i]*WAVE[i];
	}

	sum /= double(len);

	return sum;
}

#endif