#include <algorithm>

#include "PID.h"

using namespace std;

/*
* TODO: Complete the PID class.
*/

PID::PID() {
    this->min = -std::numeric_limits<const double>::infinity();
    this->max = std::numeric_limits<const double>::infinity();
}

PID::~PID() {}

void PID::Init(double Kp, double Ki, double Kd) {
    this->Kp = Kp;
    this->Ki = Ki;
    this->Kd = Kd;
    this->integral = 0;
    this->init_last_cte = false;
}

void PID::SetOutputLimits(double min, double max) {
    this->min = min;
    this->max = max;
}

double PID::Update(double cte) {
    if (!this->init_last_cte) {
        this->last_cte = cte;
        this->init_last_cte = true;
    }

    // proportional
    auto p = this->Kp * cte;
    // integral
    this->integral += this->Ki * cte;
    this->integral = std::max(this->min, std::min(this->max, this->integral));
    auto i = this->integral;
    // differential
    auto d = this->Kd * (cte - this->last_cte);
    this->last_cte = cte;

    auto output = -p - i - d;
    return std::max(this->min, std::min(this->max, output));
}
