#ifndef __SIGNAL_DETECTOR_HPP__
#define __SIGNAL_DETECTOR_HPP__


int signalDetectorInitial(int pinX) ;
int trackWave(int pinX, int timeout, void* dispencer);

#endif
