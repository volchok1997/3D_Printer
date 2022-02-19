#ifndef __SIGNAL_DETECTOR_HPP__
#define __SIGNAL_DETECTOR_HPP__


int signalDectorInitial(int pinX) ;
void enableTrigger(int pinX, int t, void* dispenser) ;
void cancelTrigger(int pinX) ;

#endif