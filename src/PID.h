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
  * Update the PID with given cross track error.
  */
  double Update(double cte);
};

#endif /* PID_H */
