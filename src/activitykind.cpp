#include "activitykind.h"

ActivityKind::Type ActivityKind::fromBipType(int type)
{
    if (type == 1) {
        return Running;
    } else if (type == 2) {
        return Treadmill;
    } else if (type == 3) {
        return Walking;
    } else if (type == 4) {
        return Cycling;
    }
    return Activity;
}
