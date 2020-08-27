float evolutionRule0(float center, float n) {
  float result;
  if (center == 1) {
    result = n == 2.0 || n == 3.0 ? 1.0 : 0.0;
  } else {
    result = n == 3.0 ? 1.0 : 0.0;
  }
  return result;
}


//no blob, weak population, small stabil configs, rare glider (seed-density: 10%)
float evolutionRule(float center, float n) {
    float result;
    const float deathRanges[6] = float[6](    	0.0, 3.0, 
					    	                    6.0, 9.0, 
                                        	    14.0, 27.0
                                        );

    const float birthRanges[4] = float[4](	5.0, 5.0,
						                    12.0, 13.0
					                    );

    result = center;    
    if (center == 1.0) {
        for (int i = 0; i < deathRanges.length(); i += 2) {
            if (n >= deathRanges[i] && n <= deathRanges[i + 1]) result = 0.0;
        }
    }
    else {
        for (int i = 0; i < birthRanges.length(); i += 2) {
            if (n >= birthRanges[i] && n <= birthRanges[i + 1]) result = 1.0;
        }
    }
    return result;
}


//rare blob, small stabil configs (seed-density: 10%)
float evolutionRule2(float center, float n) {
    float result;
    const float deathRanges[6] = float[6](    	0.0, 4.0, 
					    	                    6.0, 6.0, 
                                        	    10.0, 27.0
                                         );

    const float birthRanges[2] = float[2](	    5.0, 5.0);

    result = center;    
    if (center == 1.0) {
        for (int i = 0; i < deathRanges.length(); i += 2) {
            if (n >= deathRanges[i] && n <= deathRanges[i + 1]) result = 0.0;
        }
    }
    else {
        for (int i = 0; i < birthRanges.length(); i += 2) {
            if (n >= birthRanges[i] && n <= birthRanges[i + 1]) result = 1.0;
        }
    }
    return result;
}
