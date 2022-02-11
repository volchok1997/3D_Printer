#ifndef __YOCTO_PWM_HPP__
#define __YOCTO_PWM_HPP__

#include "yocto_api.h"
#include "yocto_pwminput.h"

class Yocto_PWM {
private:
    YPwmInput   *pwm;
    YPwmInput   *pwm1;
    YPwmInput   *pwm2;
    YModule     *m;
public:
    Yocto_PWM();
    int Detect(string target);
    int Test();
    void EnablePWMDetection(YPwmInputValueCallback callback);
    void EnterEventMode();
};

void yoctoTest(string dev);
void YoctoFreeAll();

#endif