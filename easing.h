#ifndef SRC_ENGINE_EASING_H_
#define SRC_ENGINE_EASING_H_

#include <cmath>
#include "defaults.h"

namespace Engine {

    // NOTE http://robertpenner.com/easing/
    namespace Easing {

    	inline float_max_t Linear (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {
    		return c * (t / d) + b;
    	}

    	namespace Quad {

    		inline float_max_t In (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			t /= d;

    			return b + (c * t * t);
    		}

    		inline float_max_t Out (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			t /= d;

    			return b - (c * t * (t - 2.0));
    		}

    		inline float_max_t InOut (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			t /= (d * 0.5);
    			c *= 0.5;

    			if (t < 1.0) {
    		        return (c * t * t) + b;
    		    }

    			--t;

    			return b - (c * (t * (t - 2.0) - 1.0));
    		}
    	};

    	namespace Cubic {

    		inline float_max_t In (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			t /= d;

    			return b + (c * t * t * t);
    		}

    		inline float_max_t Out (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			t = (t / d) - 1.0;

    			return b + (c * ((t * t * t) + 1.0));
    		}

    		inline float_max_t InOut (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			t /= (d * 0.5);
    			c *= 0.5;

    			if (t < 1.0) {
    				return b + (c * t * t * t);
    			}

    			t -= 2.0;

    			return b + (c * ((t * t * t) + 2.0));
    		}
    	};

    	namespace Quart {

    		inline float_max_t In (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			t /= d;
    			t *= t;

    			return b + (c * t * t);
    		}

    		inline float_max_t Out (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			t = (t / d) - 1.0;
    			t *= t;

    			return b - (c * ((t * t) - 1.0));
    		}

    		inline float_max_t InOut (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			t /= (d * 0.5);
    			c *= 0.5;

    			if (t < 1.0) {
    				t *= t;
    				return b + (c * t * t);
    			}

    			t -= 2.0;
    			t *= t;

    			return b - (c * ((t * t) - 2.0));
    		}
    	};

    	namespace Quint {

    		inline float_max_t In (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			float_max_t tt;

    			t /= d;
    			tt = t * t;

    			return b + (c * tt * tt * t);
    		}

    		inline float_max_t Out (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			float_max_t tt;

    			t = (t / d) - 1.0;
    			tt = t * t;

    			return b + (c * ((tt * tt * t) + 1.0));
    		}

    		inline float_max_t InOut (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			float_max_t tt;

    			t /= (d * 0.5);
    			c *= 0.5;

    			if (t < 1.0) {
    				tt = t * t;
    				return b + (c * tt * tt * t);
    			}

    			t -= 2.0;
    			tt = t * t;

    			return b + (c * ((tt * tt * t) + 2.0));
    		}
    	};

    	namespace Sine {

    		inline float_max_t In (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {
    			return b + c - (c * std::cos((t / d) * (PI / 2.0)));
    		}

    		inline float_max_t Out (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {
    			return b + (c * std::sin((t / d) * (PI / 2.0)));
    		}

    		inline float_max_t InOut (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			c *= 0.5;

    			return b - (c * (std::cos(PI * (t / d)) - 1.0));
    		}
    	};

    	namespace Expo {

    		inline float_max_t In (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			if (t == 0.0) {
    				return b;
    			}

    			return b + (c * std::pow(2.0, 10.0 * ((t / d) - 1.0)));
    		}

    		inline float_max_t Out (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			if (t == d) {
    				return b + c;
    			}

    			return b + (c * (1.0 - std::pow(2.0, -10.0 * (t / d))));
    		}

    		inline float_max_t InOut (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			if (t == 0.0) {
    				return b;
    			}

    			if (t == d) {
    				return b + c;
    			}

    			t /= (d * 0.5);
    			c *= 0.5;

    			if (t < 1.0) {
    				return b + (c * std::pow(2.0, 10.0 * (t - 1.0)));
    			}

    			--t;

    			return b + (c * (2.0 - std::pow(2.0, -10.0 * t)));
    		}
    	};

    	namespace Circ {

    		inline float_max_t In (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			t /= d;

    			return b - (c * (sqrt(1.0 - (t * t)) - 1.0));
    		}

    		inline float_max_t Out (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			t = (t / d) - 1.0;

    			return b + (c * sqrt(1.0 - (t * t)));
    		}

    		inline float_max_t InOut (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			t /= (d * 0.5);
    			c *= 0.5;

    			if (t < 1.0) {
    				return b - (c * (sqrt(1.0 - (t * t)) - 1.0));
    			}

    			t -= 2.0;

    			return b + (c * (sqrt(1.0 - (t * t)) + 1.0));
    		}
    	};

    	namespace Elastic {

    		inline float_max_t In (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			float_max_t s, p;

    			if (t == 0.0) {
    				return b;
    			}

    			t /= d;

    			if (t == 1.0) {
    				return b + c;
    			}

    			p = 0.30 * d;
    			s = 0.25 * p;

    			t -= 1.0;

    			return b - (c * std::pow(2.0, 10.0 * t) * std::sin((((t * d) - s) * (2.0 * PI)) / p));
    		}

    		inline float_max_t Out (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			float_max_t s, p;

    			if (t == 0.0) {
    				return b;
    			}

    			t /= d;

    			if (t == 1.0) {
    				return b + c;
    			}

    			p = 0.30 * d;
    			s = 0.25 * p;

    			return b + c + (c * std::pow(2.0, -10.0 * t) * std::sin(((t * d) - s) * ((2.0 * PI) / p)));
    		}

    		inline float_max_t InOut (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			float_max_t s, p, a = c;

    			if (t == 0.0) {
    				return b;
    			}

    			t /= (d * 0.5);

    			if (t == 2.0) {
    				return b + c;
    			}

    			p = 0.45 * d;
    			s = 0.25 * p;

    			t -= 1.0;

    			if (t < 0.0) {
    				return b - (0.5 * a * std::pow(2.0,  10.0 * t) * std::sin((t * d - s) * (2.0 * PI) / p));
    			}

    			return b + c + (0.5 * a * std::pow(2.0, -10.0 * t) * std::sin((t * d - s) * (2.0 * PI) / p));
    		}
    	};

    	namespace Back {

    		inline float_max_t In (float_max_t t, float_max_t b, float_max_t c, float_max_t d, float_max_t s = 1.70158) {

    			t /= d;

    			return b + (c * t * t * (((s + 1.0) * t) - s));
    		}

    		inline float_max_t Out (float_max_t t, float_max_t b, float_max_t c, float_max_t d, float_max_t s = 1.70158) {

    			t = (t / d) - 1.0;

    			return b + (c * (t * t * (((s + 1.0) * t) + s) + 1.0));
    		}

    		inline float_max_t InOut (float_max_t t, float_max_t b, float_max_t c, float_max_t d, float_max_t s = 1.70158) {

    			t /= (d * 0.5);
    			c *= 0.5;
    			s *= 1.525;

    			if (t < 1.0) {
    				return b + c * (t * t * (((s + 1.0) * t) - s));
    			}

    			t -= 2.0;

    			return b + c * (t * t * (((s + 1.0) * t) + s) + 2.0);
    		}
    	};

    	namespace Bounce {

    		inline float_max_t Out (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			t /= d;

    			if (t < 0.363636364) {

    				return b + (c * (7.5625 * t * t));

    			} else if (t < 0.727272727) {

    				t -= 0.545454545;
    				return b + (c * ((7.5625 * t * t) + 0.75));

    			} else if (t < 0.909090909) {

    				t -= 0.818181818;
    				return b + (c * ((7.5625 * t * t) + 0.9375));

    			}

    			t -= 0.954545455;
    			return b + (c * ((7.5625 * t * t) + 0.984375));
    		}

    		inline float_max_t In (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {
    			return b + c - Out(d - t, 0.0, c, d);
    		}

    		inline float_max_t InOut (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			if (t < (d * 0.5)) {
    				return b + (0.5 * In(t * 2.0, 0.0, c, d));
    			}

    			return b + (c * 0.5) + (0.5 * Out(t * 2.0 - d, 0.0, c, d));
    		}
    	};
    };
};

#endif
