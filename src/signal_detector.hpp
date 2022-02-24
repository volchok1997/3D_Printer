#ifndef __SIGNAL_DETECTOR_HPP__
#define __SIGNAL_DETECTOR_HPP__


int signalDetectorInitial(int pinX) ;
int sampleWave(int pinX, int timeout, void* dispencer);
void enableTrigger(int pinX, int t, void* dispenser) ;
void cancelTrigger(int pinX) ;

#endif
