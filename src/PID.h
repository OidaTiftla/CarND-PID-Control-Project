#ifndef PID_H
#define PID_H

class PID {
public:
  /*
  * state variables
  */
  double integral;
  double last_cte;
  bool init_last_cte;

  /*
  * limits for output value
  */
  double min;
  double max;

  /*
  * Coefficients
  */
  double Kp;
  double Ki;
  double Kd;

  /*
  * Constructor
  */
  PID();

  /*
  * Destructor.
  */
  virtual ~PID();

  /*
  * Initialize PID.
  */
  void Init(double Kp, double Ki, double Kd);

  /*
  * Set limits for output value.
  */
  void SetOutputLimits(double min, double max);

  /*
  * Update the PID with given cross track error.
  */
  double Update(double cte);
};

#endif /* PID_H */
