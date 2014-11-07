#ifndef NO_FMOD

#pragma once
#include <iostream>
#include <cstdlib>

#include <fmod.hpp>
#include <fmod_errors.h>

#include "../utility/log.h"

/*
	FMOD Ex API :
		- fmodex_vc.lib
		- fmod.hpp
		- fmodex.dll
*/
void initSound();

class SoundHandler{
	public :
		float* FFT;
		float* WAVE;
		unsigned len; //Con el array y su longitud publicos se accede a los datos que pilles usando el getSpectrum ...
		FMOD::Sound* theSound;
		FMOD::Channel* thechannel;
		float playFreq;
		float playVel;

		SoundHandler(const char *SongFile, unsigned FFTLEN);
		void Play();
		void Pause();
		void Stop();
		void getSpectrum();
		void getWave();
		double SoundTime();
//		void PlotWave();

		double getEnergy();

		void setVel(float v); //Play needs to be called at some point before this.
};

#endif