#if INTERFACE
#include <Udon/Standard.h>

class Ease {
public:

};
#endif

#include <Udon/Easing/Ease.h>

#ifndef PI
#define PI 3.1415926545
#endif

PUBLIC STATIC double Ease::InSine(double t) {
	return sin(1.5707963 * t);
}
PUBLIC STATIC double Ease::OutSine(double t) {
	t -= 1.0;
	return 1 + sin(1.5707963 * t);
}
PUBLIC STATIC double Ease::InOutSine(double t) {
	return 0.5 * (1 + sin(3.1415926 * (t - 0.5)));
}
PUBLIC STATIC double Ease::InQuad(double t) {
    return t * t;
}
PUBLIC STATIC double Ease::OutQuad(double t) {
    return t * (2 - t);
}
PUBLIC STATIC double Ease::InOutQuad(double t) {
    return t < 0.5 ? 2 * t * t : t * (4 - 2 * t) - 1;
}
PUBLIC STATIC double Ease::InCubic(double t) {
    return t * t * t;
}
PUBLIC STATIC double Ease::OutCubic(double t) {
	t -= 1.0;
    return 1 + t * t * t;
}
PUBLIC STATIC double Ease::InOutCubic(double t) {
	if (t >= 0.5)
		t -= 1.0;
    return t < 0.5 ? 4 * t * t * t : 1 + t * (2 * (t - 1)) * (2 * (t - 2));
}
PUBLIC STATIC double Ease::InQuart(double t) {
    t *= t;
    return t * t;
}
PUBLIC STATIC double Ease::OutQuart(double t) {
    t = (t - 1) * (t - 1);
    return 1 - t * t;
}
PUBLIC STATIC double Ease::InOutQuart(double t) {
    if (t < 0.5) {
        t *= t;
        return 8 * t * t;
    }
    else {
        t = (t - 1) * (t - 1);
        return 1 - 8 * t * t;
    }
}
PUBLIC STATIC double Ease::InQuint(double t) {
    double t2 = t * t;
    return t * t2 * t2;
}
PUBLIC STATIC double Ease::OutQuint(double t) {
    double t2 = (t - 1) * (t - 1);
    return 1 + t * t2 * t2;
}
PUBLIC STATIC double Ease::InOutQuint(double t) {
    double t2;
    if (t < 0.5) {
        t2 = t * t;
        return 16 * t * t2 * t2;
    }
    else {
        t2 = (t - 1) * (t - 1);
        return 1 + 16 * t * t2 * t2;
    }
}
PUBLIC STATIC double Ease::InExpo(double t) {
    return (pow(2, 8 * t) - 1) / 255;
}
PUBLIC STATIC double Ease::OutExpo(double t) {
    return 1 - pow(2, -8 * t);
}
PUBLIC STATIC double Ease::InOutExpo(double t) {
    if (t < 0.5) {
        return (pow(2, 16 * t) - 1) / 510;
    }
    else {
        return 1 - 0.5 * pow(2, -16 * (t - 0.5));
    }
}
PUBLIC STATIC double Ease::InCirc(double t) {
    return 1 - sqrt(1 - t);
}
PUBLIC STATIC double Ease::OutCirc(double t) {
    return sqrt(t);
}
PUBLIC STATIC double Ease::InOutCirc(double t) {
    if (t < 0.5) {
        return (1 - sqrt(1 - 2 * t)) * 0.5;
    }
    else {
        return (1 + sqrt(2 * t - 1)) * 0.5;
    }
}
PUBLIC STATIC double Ease::InBack(double t) {
    return t * t * (2.70158 * t - 1.70158);
}
PUBLIC STATIC double Ease::OutBack(double t) {
	t -= 1.0;
    return 1 + t * t * (2.70158 * t + 1.70158);
}
PUBLIC STATIC double Ease::InOutBack(double t) {
    if (t < 0.5) {
        return t * t * (7 * t - 2.5) * 2;
    }
    else {
		t -= 1.0;
        return 1 + t * t * 2 * (7 * t + 2.5);
    }
}
PUBLIC STATIC double Ease::InElastic(double t) {
    double t2 = t * t;
    return t2 * t2 * sin(t * PI * 4.5);
}
PUBLIC STATIC double Ease::OutElastic(double t) {
    double t2 = (t - 1) * (t - 1);
    return 1 - t2 * t2 * cos(t * PI * 4.5);
}
PUBLIC STATIC double Ease::InOutElastic(double t) {
    double t2;
    if (t < 0.45) {
        t2 = t * t;
        return 8 * t2 * t2 * sin(t * PI * 9);
    }
    else if (t < 0.55) {
        return 0.5 + 0.75 * sin(t * PI * 4);
    }
    else {
        t2 = (t - 1) * (t - 1);
        return 1 - 8 * t2 * t2 * sin(t * PI * 9);
    }
}
PUBLIC STATIC double Ease::InBounce(double t) {
    return pow(2, 6 * (t - 1)) * abs(sin(t * PI * 3.5));
}
PUBLIC STATIC double Ease::OutBounce(double t) {
    return 1 - pow(2, -6 * t) * abs(cos(t * PI * 3.5));
}
PUBLIC STATIC double Ease::InOutBounce(double t) {
    if (t < 0.5) {
        return 8 * pow(2, 8 * (t - 1)) * abs(sin(t * PI * 7));
    }
    else {
        return 1 - 8 * pow(2, -8 * t) * abs(sin(t * PI * 7));
    }
}
PUBLIC STATIC double Ease::Triangle(double t) {
    if (t < 0.5) {
        return t * 2.0;
    }
    else {
        return 2.0 - t * 2.0;
    }
}
