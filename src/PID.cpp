#include "PID.h"

using namespace std;

/*
* TODO: Complete the PID class.
*/

PID::PID() {}

PID::~PID() {}

void PID::Init(double Kp, double Ki, double Kd) {
    this->Kp = Kp;
    this->Ki = Ki;
    this->Kd = Kd;
    this->integral = 0;
    this->init_last_cte = false;
}

double PID::Update(double cte) {
    if (!this->init_last_cte) {
        this->last_cte = cte;
        this->init_last_cte = true;
    }

    this->integral += cte;
    auto diff = cte - this->last_cte;
    this->last_cte = cte;

    auto p = this->Kp * cte;
    auto i = this->Ki * this->integral;
    auto d = this->Kd * diff;

    return -p - i - d;
}
