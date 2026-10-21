#pragma once

// Minimaler PID Mock
class PID {
public:
    PID(double* input, double* output, double* setpoint,
        double kp, double ki, double kd, int dir)
        : _input(input), _output(output), _setpoint(setpoint),
          _kp(kp), _ki(ki), _kd(kd), _mode(0) {}

    void SetMode(int mode)                        { _mode = mode; }
    void SetOutputLimits(double min, double max)  { _min = min; _max = max; }
    void SetSampleTime(int ms)                    { _sampleTime = ms; }
    void SetTunings(double kp, double ki, double kd) {
        _kp = kp; _ki = ki; _kd = kd;
    }

    bool Compute() {
        if (_mode != 1) return false;
        // Simple P-control for tests
        double error = *_setpoint - *_input;
        *_output = _kp * error;
        if (*_output < _min) *_output = _min;
        if (*_output > _max) *_output = _max;
        return true;
    }

    double getKp() { return _kp; }
    double getKi() { return _ki; }
    double getKd() { return _kd; }
    int    getMode() { return _mode; }

private:
    double* _input;
    double* _output;
    double* _setpoint;
    double  _kp, _ki, _kd;
    double  _min = 0, _max = 255;
    int     _mode = 0;
    int     _sampleTime = 100;
};
