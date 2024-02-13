#ifndef ACTUATOR_CONTROL_H
#define ACTUATOR_CONTROL_H

namespace ActuatorControl
{
    void init();
    void controlPump(bool on);
    void startWatering(float duration);
}

#endif // ACTUATOR_CONTROL_H
