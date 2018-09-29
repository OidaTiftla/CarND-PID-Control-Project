#ifndef TWIDDLE_HPP
#define TWIDDLE_HPP

#include <vector>

class Twiddle {
public:
    std::vector<double> parameters;
    std::vector<double> deltas;
    double best_err = std::numeric_limits<const double>::infinity();

    // returns whether the last parameters were better (true) or not (false)
    bool GenerateNextParameters(double err, double good_factor = 1.1, double bad_factor = 0.7) {
        static int param_index = 0;
        static bool tested_add = false;
        static bool tested_sub = false;
        bool better = false;

        if (tested_add) {
            if (err < this->best_err) {
                this->best_err = err;
                better = true;
                this->deltas[param_index] *= good_factor;
                ++param_index;
                if (param_index >= this->parameters.size()) {
                    param_index = 0;
                }
                this->parameters[param_index] += this->deltas[param_index];
                tested_add = true;
                tested_sub = false;
            } else {
                this->parameters[param_index] -= 2.0 * this->deltas[param_index];
                tested_add = false;
                tested_sub = true;
            }
        } else if (tested_sub) {
            if (err < this->best_err) {
                this->best_err = err;
                better = true;
                this->deltas[param_index] *= good_factor;
            } else {
                this->parameters[param_index] += this->deltas[param_index];
                this->deltas[param_index] *= bad_factor;
            }
            ++param_index;
            if (param_index >= this->parameters.size()) {
                param_index = 0;
            }
            this->parameters[param_index] += this->deltas[param_index];
            tested_add = true;
            tested_sub = false;
        } else {
            this->best_err = err;
            better = true;
            this->parameters[param_index] += this->deltas[param_index];
            tested_add = true;
            tested_sub = false;
        }
        return better;
    }
};

#endif /*TWIDDLE_HPP*/