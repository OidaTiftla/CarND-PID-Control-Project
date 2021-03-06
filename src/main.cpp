#include <uWS/uWS.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include "json.hpp"
#include "PID.h"
#include "twiddle.hpp"
#include <math.h>

// for convenience
using json = nlohmann::json;

// For converting back and forth between radians and degrees.
constexpr double pi() { return M_PI; }
double deg2rad(double x) { return x * pi() / 180; }
double rad2deg(double x) { return x * 180 / pi(); }

// Checks if the SocketIO event has JSON data.
// If there is data the JSON object in string format will be returned,
// else the empty string "" will be returned.
std::string hasData(std::string s) {
  auto found_null = s.find("null");
  auto b1 = s.find_first_of("[");
  auto b2 = s.find_last_of("]");
  if (found_null != std::string::npos) {
    return "";
  }
  else if (b1 != std::string::npos && b2 != std::string::npos) {
    return s.substr(b1, b2 - b1 + 1);
  }
  return "";
}

int main()
{
  uWS::Hub h;

  PID pid;
  // TODO: Initialize the pid variable.
  pid.Init(0.14, 0.000000006, 1.0);
  pid.SetOutputLimits(-1.0, 1.0);

  Twiddle twiddle;
  twiddle.parameters.push_back(pid.Kp);
  twiddle.parameters.push_back(pid.Ki);
  twiddle.parameters.push_back(pid.Kd);
  twiddle.deltas.push_back(pid.Kp * 0.3);
  twiddle.deltas.push_back(pid.Ki * 0.3);
  twiddle.deltas.push_back(pid.Kd * 0.3);
  bool do_twiddle_optimization = true;
  bool restarted = false;

  if (!do_twiddle_optimization) {
    // set evaluated optimal parameters for 0.3 throttle
    // pid.Init(0.402013, 5.55815e-08, 3.17194);
    // pid.Init(0.14, 0.000000006, 1.0);
    // pid.Init(0.21568, 4.38e-09, 1.19683); // New deltas: deltaKp=0.0370478, deltaKi=1.29908e-09, deltaKd=0.19683
    pid.Init(0.191373, 4.38e-09, 1.19683); // New deltas: deltaKp=0.024307, deltaKi=8.52325e-10, deltaKd=0.142054

    // set evaluated optimal parameters for 0.4 throttle
    // pid.Init(0.22358, 7.8e-09, 1.63); // New deltas: deltaKp=0.04158, deltaKi=1.782e-09, deltaKd=0.363
    // pid.Init(0.22358, 9.24342e-09, 1.63); // New deltas: deltaKp=0.0370478, deltaKi=1.44342e-09, deltaKd=0.29403

    // set evaluated optimal parameters for 0.5 throttle
    // pid.Init(0.14, 5.64342e-09, 1.3); // New deltas: deltaKp=0.0248006, deltaKi=1.44342e-09, deltaKd=0.24057
    // pid.Init(0.14, 7.23118e-09, 1.3); // New deltas: deltaKp=0.0180796, deltaKi=1.4147e-09, deltaKd=0.157838
    // pid.Init(0.172001, 7.23118e-09, 1.3); // New deltas: deltaKp=0.00126113, deltaKi=3.99618e-11, deltaKd=0.00636933
  }

  h.onMessage([&pid, &twiddle, &do_twiddle_optimization, &restarted](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length, uWS::OpCode opCode) {
    // "42" at the start of the message means there's a websocket message event.
    // The 4 signifies a websocket message
    // The 2 signifies a websocket event
    if (length && length > 2 && data[0] == '4' && data[1] == '2')
    {
      auto s = hasData(std::string(data).substr(0, length));
      if (s != "") {
        auto j = json::parse(s);
        std::string event = j[0].get<std::string>();
        if (event == "telemetry") {
          // j[1] is the data JSON object
          double cte = std::stod(j[1]["cte"].get<std::string>());
          double speed = std::stod(j[1]["speed"].get<std::string>());
          double angle = std::stod(j[1]["steering_angle"].get<std::string>());
          double steer_value;
          /*
          * TODO: Calcuate steering value here, remember the steering value is
          * [-1, 1].
          * NOTE: Feel free to play around with the throttle and speed. Maybe use
          * another PID controller to control the speed!
          */
          static double last_steer_value = 0;
          static double steer_value_delta = 0;
          if (do_twiddle_optimization) {
            static int twiddle_frames = 0;
            static double twiddle_error_pos = 0.0;
            static double twiddle_error_steer = 0.0;
            static bool wait_for_restart = false;
            if (wait_for_restart) {
              if (restarted) {
                wait_for_restart = false;
                restarted = false;
              }
            } else {
              if (twiddle_frames > 20) {
                twiddle_error_pos += std::max(0.0, pow(cte, 4) - 0.125);
                twiddle_error_steer += std::max(0.0, pow(steer_value_delta, 2) * 10);
              }
              if (twiddle_frames > 550) {
                auto better = twiddle.GenerateNextParameters(twiddle_error_pos + twiddle_error_steer, 1.1, 0.7);
                if (better) {
                  std::cout << "\033[0;32m"; // switch to green text
                } else {
                  std::cout << "\033[0;31m"; // switch to red text
                }
                std::cout << (better ? "better" : "worse") << " error_pos: " << twiddle_error_pos << " error_steer: " << twiddle_error_steer << std::endl;
                std::cout << "\033[0m"; // reset colors
                twiddle_frames = 0;
                twiddle_error_pos = 0.0;
                twiddle_error_steer = 0.0;
                restarted = false;
                wait_for_restart = true;
                std::cout << "Please restart the simulation (ESC and then start the simulation again; don't close the simulator completely)" << std::endl;
              } else {
                if (twiddle_frames <= 0) {
                  pid.Init(twiddle.parameters[0], twiddle.parameters[1], twiddle.parameters[2]);
                  std::cout << "\033[0;1;36m"; // switch to cyan bold text
                  std::cout << "New parameters: "
                    << "Kp=" << twiddle.parameters[0] << ", "
                    << "Ki=" << twiddle.parameters[1] << ", "
                    << "Kd=" << twiddle.parameters[2] << std::endl;
                  std::cout << "\033[0;37m"; // switch to gray text
                  std::cout << "New deltas: "
                    << "deltaKp=" << twiddle.deltas[0] << ", "
                    << "deltaKi=" << twiddle.deltas[1] << ", "
                    << "deltaKd=" << twiddle.deltas[2] << std::endl;
                  std::cout << "\033[0m"; // reset colors
                }
                ++twiddle_frames;
              }
            }
          }
          steer_value = pid.Update(cte);
          steer_value_delta = steer_value - last_steer_value;
          last_steer_value = steer_value;

          // DEBUG
          if (!do_twiddle_optimization) {
            std::cout << "CTE: " << cte << " Steering Value: " << steer_value << std::endl;
          }

          json msgJson;
          msgJson["steering_angle"] = steer_value;
          msgJson["throttle"] = 0.3;
          auto msg = "42[\"steer\"," + msgJson.dump() + "]";
          if (!do_twiddle_optimization) {
            std::cout << msg << std::endl;
          }
          ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
        }
      } else {
        // Manual driving
        std::string msg = "42[\"manual\",{}]";
        ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
      }
    }
  });

  // We don't need this since we're not using HTTP but if it's removed the program
  // doesn't compile :-(
  h.onHttpRequest([](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t, size_t) {
    const std::string s = "<h1>Hello world!</h1>";
    if (req.getUrl().valueLength == 1)
    {
      res->end(s.data(), s.length());
    }
    else
    {
      // i guess this should be done more gracefully?
      res->end(nullptr, 0);
    }
  });

  h.onConnection([&h, &restarted](uWS::WebSocket<uWS::SERVER> ws, uWS::HttpRequest req) {
    std::cout << "Connected!!!" << std::endl;
    restarted = true;
  });

  h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code, char *message, size_t length) {
    ws.close();
    std::cout << "Disconnected" << std::endl;
  });

  int port = 4567;
  if (h.listen(port))
  {
    std::cout << "Listening to port " << port << std::endl;
  }
  else
  {
    std::cerr << "Failed to listen to port" << std::endl;
    return -1;
  }
  h.run();
}
