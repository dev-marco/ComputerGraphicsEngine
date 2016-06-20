#ifndef SRC_ENGINE_EASING_H_
#define SRC_ENGINE_EASING_H_

#include <cmath>
#include "spatial/defaults.h"

//
// TERMS OF USE - EASING EQUATIONS
//
// Open source under the BSD License [https://opensource.org/licenses/bsd-license.php].
//
// Copyright (c) 2001 Robert Penner
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
//     Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
//     Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
//     Neither the name of the author nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Equations taken from http://robertpenner.com/easing/
//

namespace Engine {

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
    			return b + c - (c * std::cos((t / d) * (Spatial::PI / 2.0)));
    		}

    		inline float_max_t Out (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {
    			return b + (c * std::sin((t / d) * (Spatial::PI / 2.0)));
    		}

    		inline float_max_t InOut (float_max_t t, float_max_t b, float_max_t c, float_max_t d) {

    			c *= 0.5;

    			return b - (c * (std::cos(Spatial::PI * (t / d)) - 1.0));
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

    			return b - (c * std::pow(2.0, 10.0 * t) * std::sin((((t * d) - s) * (2.0 * Spatial::PI)) / p));
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

    			return b + c + (c * std::pow(2.0, -10.0 * t) * std::sin(((t * d) - s) * ((2.0 * Spatial::PI) / p)));
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
    				return b - (0.5 * a * std::pow(2.0,  10.0 * t) * std::sin((t * d - s) * (2.0 * Spatial::PI) / p));
    			}

    			return b + c + (0.5 * a * std::pow(2.0, -10.0 * t) * std::sin((t * d - s) * (2.0 * Spatial::PI) / p));
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
