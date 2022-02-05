
#include <iostream>
#include <stdlib.h>
#include <string>
#include <chrono>
#include <ctime>   

#include "yocto_api.h"
#include "yocto_pwminput.h"

#include "yocto_pwm.hpp"

static void pwmChangeCallback(YPwmInput *fct, const string &value)
{
    auto n = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(n);
    std::cerr << std::ctime(&t) << " PWM Changed " << value<< std::endl;
}


Yocto_PWM::Yocto_PWM() {
    pwm = NULL;
    pwm1 = NULL;
    pwm2 = NULL;
    m = NULL;
}

int Yocto_PWM::Detect() {
    string       errmsg;
    string       target = "any";
    // Setup the API to use local USB devices
    if (YAPI::RegisterHub("usb", errmsg) != YAPI::SUCCESS) {
        std::cerr << "RegisterHub error: " << errmsg << std::endl;
        return -1;
    }

    pwm = YPwmInput::FirstPwmInput();
    if(pwm == NULL) {
        std::cerr << "No module connected (Check cable)" << std::endl;
        return -1;
    }

    pwm = YPwmInput::FindPwmInput(target + ".pwmInput1");

    if (pwm->isOnline()) {
        m = pwm->get_module();
        pwm1 = YPwmInput::FindPwmInput(m->get_serialNumber() + ".pwmInput1");
        pwm2 = YPwmInput::FindPwmInput(m->get_serialNumber() + ".pwmInput2");
    } else {
        std::cerr << "No module connected (Check cable)" << std::endl;
    }
    return 0;
}

int Yocto_PWM::Test() {
    string       errmsg;
    while (pwm1->isOnline()) {
        std::cout << "PWM1 : " << pwm1->get_frequency() << " Hz " << pwm1->get_dutyCycle()
             << " %  " << pwm1->get_pulseCounter() << "pulses edges" << std::endl;
        std::cout << "PWM2 : " << pwm2->get_frequency() << "  Hz " << pwm2->get_dutyCycle()
             << " %  " << pwm2->get_pulseCounter() << " pulses edges" << std::endl;

        std::cout << "  (press Ctrl-C to exit)" << std::endl;
        YAPI::Sleep(1000, errmsg);
    }
    std::cout << "Module disconnected" << std::endl;
    return 0;
}

void Yocto_PWM::EnablePWMDetection() {
    pwm1->registerValueCallback(pwmChangeCallback);
}

void Yocto_PWM::EnterEventMode() {
    string       errmsg;
    while (true)
    {
        // inactive waiting loop allowing you to trigger
        // value change callbacks
        YAPI::Sleep(500, errmsg);
    }
}


void yoctoTest() {
    Yocto_PWM pwm;
    pwm.Detect();
    pwm.EnablePWMDetection();
    pwm.EnterEventMode();
}





// YPwmInput *pwminput = YPwmInput::FindPwmInput("YPWMRX01-123456.pwmInput1");